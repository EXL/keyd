/*
 * About:
 *   libredkey is a special library for hacking behavior of red key button on MotoMAGX platform.
 *
 * Author:
 *   EXL
 *
 * License:
 *   Public Domain
 *
 * History:
 *   13-Nov-2021: Implemented native C++/Qt API function for determining phone PID instead of using "pidof" utility.
 *   11-Oct-2021: Fixed ignoring slide/timeout/flip reasons.
 *   10-Oct-2021: Added custom config path through environment variables, added version without config.
 *   09-Oct-2021: Fixed wrong determinig of "phone" process PID.
 *   04-Oct-2021: Implemented "hide" and "desktop" commands.
 *   02-Oct-2021: Added reading configuration file.
 *   27-Sep-2021: Added first testing prototype.
 *
 * Information:
 *   Use LD_PRELOAD=libredkey.so for applying this hack.
 */

// C
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

// POSIX
#include <sys/types.h>

// Linux
#include <linux/limits.h>

// Qt
#include <qarray.h>
#include <qcopchannel_qws.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qwidget.h>
#include <qwidgetlist.h>

// EZX & MotoMAGX
#include <ZApplication.h>
#ifdef USE_ES_API
#include <ESCopChannel.h>
#include <ES_SubscriberAPI.h>
#endif

// Defines
#define IDLE_REASON_RED_KEY -5000
#define IDLE_REASON_FLIP    -5001
#define IDLE_REASON_TIMEOUT -5002
#define IDLE_REASON_SLIDER  -5003
#define HACK_LIBRARY        "libezxappbase.so.1"
#define HACK_METHOD         "_ZN12ZApplication16slotReturnToIdleEi"
#define PHONE_PROCESS_NAME  "phone"
#define TO_ERR(...)         fprintf(stderr, __VA_ARGS__)
#define TO_DBG(...)         \
	do { \
		if (G_DEBUG_OUTPUT) \
			fprintf(stderr, __VA_ARGS__); \
	} while(0)

typedef void (ZApplication::*qt_slot_method_t)(int aReason);

// Global variables
static bool G_CONFIG_PARSED = false;
static bool G_DEBUG_OUTPUT = true;
static QString G_GLOBAL_COMMAND = "original";
static QString G_CONFIG_PATH = "/ezxlocal/download/appwrite/setup/libredkey.cfg";
static QMap<QString, QString> G_APP_MAP;

static qt_slot_method_t GetOriginalSlotReturnToIdleDlSym(void) {
	static qt_slot_method_t lOriginalSlot = NULL;
	if (!lOriginalSlot) {
		void *lHandleLibrary = dlopen(HACK_LIBRARY, RTLD_LAZY);
		if (!lHandleLibrary) {
			TO_ERR("libredkey.so: Error: Cannot dlopen '%s' library!\n", HACK_LIBRARY);
			return NULL;
		}
		void *lHandleMethod = dlsym(lHandleLibrary, HACK_METHOD);
		if (!lHandleMethod) {
			TO_ERR("libredkey.so: Error: Cannot find '%s' method!\n", HACK_METHOD);
			dlclose(lHandleLibrary);
			return NULL;
		}
		TO_DBG("libredkey.so: Debug: Copying address of method '%s': '%p'.\n", HACK_METHOD, lHandleMethod);
		memcpy(&lOriginalSlot, &lHandleMethod, sizeof(void *));
		dlclose(lHandleLibrary);
	}
	return lOriginalSlot;
}

static void ParseConfigLine(const QString &aLine) {
	QString lLine = aLine.stripWhiteSpace();
	if (!lLine.startsWith("#") && !lLine.isEmpty()) { // Just drop comments and empty strings from config.
		if (lLine.startsWith("Debug=")) {
			if (!lLine.remove(0, 6).stripWhiteSpace().compare("off"))
				G_DEBUG_OUTPUT = false;
		} else if (lLine.startsWith("Global=")) {
			G_GLOBAL_COMMAND = lLine.remove(0, 7).stripWhiteSpace();
		} else if (lLine.startsWith("App=")) {
			const QStringList lTokenizedLine = QStringList::split('|', lLine.remove(0, 4));
			if (lTokenizedLine.count() == 2)
				G_APP_MAP.insert(lTokenizedLine[0], lTokenizedLine[1]);
			else
				TO_ERR("libredkey.so: Error: Wrong application command: '%s'!\n", lLine.data());
		} else
			TO_ERR("libredkey.so: Error: Unknown config line: '%s'!\n", lLine.data());
	}
}

static bool ReadConfigurationFile(void) {
	if (!G_CONFIG_PARSED) {
		char *lConfigPath = getenv("LIBREDKEY_CONFIG_PATH");
		if (lConfigPath)
			G_CONFIG_PATH = lConfigPath;
		if (!QFileInfo(G_CONFIG_PATH).isReadable())
			return false;
		QFile lConfigFile(G_CONFIG_PATH);
		if (lConfigFile.open(IO_ReadOnly)) {
			QTextStream lTextStream(&lConfigFile);
			while (!lTextStream.eof())
				ParseConfigLine(lTextStream.readLine());
			G_CONFIG_PARSED = true;
			lConfigFile.close();
		}
#ifndef NO_DEBUG
		TO_DBG("libredkey.so: Debug: Output of parsed configuration file.\n");
		QMap<QString, QString>::Iterator lIt;
		for (lIt = G_APP_MAP.begin(); lIt != G_APP_MAP.end(); ++lIt)
			TO_DBG("libredkey.so: Debug: => key: '%s', value: '%s'\n", lIt.key().data(), lIt.data().data());
#endif
	}
	return true;
}

static void CallOriginalSlotReturnToIdle(ZApplication *aZApp, int aReason) {
	const qt_slot_method_t lMethod = GetOriginalSlotReturnToIdleDlSym();
	if (lMethod)
		(aZApp->*lMethod)(aReason);
}

static void HideAllApplicationWidgets(void) {
	QWidgetList *lWidgetList = ZApplication::allWidgets();
	QWidgetListIt lWidgetListItterator(*lWidgetList);
	QWidget *lWidget = NULL;
	while ((lWidget = lWidgetListItterator.current()) != NULL) {
		++lWidgetListItterator;
		TO_DBG("libredkey.so: Debug: Hide '%s' widget.\n", lWidget->className());
		lWidget->hide();
	}
	delete lWidgetList;
}

static pid_t getFirstPidOfProcess(const QString &aProcessName) {
	const QDir lProcDir("/proc");
	if (lProcDir.isReadable()) {
		QFileInfoListIterator lIt(*lProcDir.entryInfoList(QDir::Dirs | QDir::NoSymLinks));
		for (bool lIsOk = false; const QFileInfo *lFi = lIt.current(); ++lIt) {
			const QString lProcDirName = lFi->fileName();
			const pid_t lPid = lProcDirName.toInt(&lIsOk);
			if (lIsOk) {
				QFile lCmdLineFile("/proc/" + lProcDirName + "/cmdline");
				if (lCmdLineFile.open(IO_ReadOnly)) {
					QString lCmdLine;
					if ((lCmdLineFile.readLine(lCmdLine, PATH_MAX) != -1) && lCmdLine.contains(aProcessName))
						return lPid;
				}
			}
		}
	}
	return -1;
}

static void ShowDesktopMainScreen(ZApplication *aZApp) {
	const pid_t lPidPhone = getFirstPidOfProcess(PHONE_PROCESS_NAME);
	if (lPidPhone != -1) {
		TO_DBG("libredkey.so: Debug: PID of 'phone' process is '%d'.\n", lPidPhone);
		QCString lBroadCastChannel = QCString("EZX/Application/" + QString::number(lPidPhone));
		QCString lBroadCastMessage = QCString("raise()");
		TO_DBG("libredkey.so: Debug: Will send message '%s' to the '%s' channel.\n",
			lBroadCastMessage.data(), lBroadCastChannel.data());
#ifdef USE_ES_API
		ESCopChannel::send(lBroadCastChannel, lBroadCastMessage, QByteArray(), false);
#else
		QCopChannel::send(lBroadCastChannel, lBroadCastMessage, QByteArray());
#endif
		aZApp->processEvents();
	} else
		TO_ERR("libredkey.so: Error: Cannot determine PID of '%s' process name.\n", PHONE_PROCESS_NAME);
}

static void ExecCommand(ZApplication *aZApp, int aReason, const QString &aCommand, const QString &aWidgetName) {
	const QString lWidgetName = (!aCommand) ? aWidgetName + " [global]" : aWidgetName;
	const QString lCommand = (!aCommand) ? G_GLOBAL_COMMAND : aCommand;
	if (!lCommand.compare("hide")) {
		TO_DBG("libredkey.so: Debug: Hide command on '%s' app.\n", lWidgetName.data());
		HideAllApplicationWidgets();
	} else if (!lCommand.compare("desktop")) {
		TO_DBG("libredkey.so: Debug: Desktop command on '%s' app.\n", lWidgetName.data());
		ShowDesktopMainScreen(aZApp);
	} else if (!lCommand.compare("original")) {
		TO_DBG("libredkey.so: Debug: Original command on '%s' app.\n", lWidgetName.data());
		CallOriginalSlotReturnToIdle(aZApp, aReason);
	} else if (lCommand.startsWith("/")) {
		TO_DBG("libredkey.so: Debug: Run command '%s' in '%s' application.\n",
			lCommand.data(), lWidgetName.data());
		system(lCommand.data());
	} else
		TO_DBG("libredkey.so: Debug: Ignore command in '%s' application.\n", lWidgetName.data());
}

static void ProcessCustomRedKeyCommand(ZApplication *aZApp, int aReason) {
	if (aReason == IDLE_REASON_RED_KEY) { // Only for Red Key reason now.
		const QWidget *lActiveWidget = aZApp->activeWindow();
		if (lActiveWidget) {
			const QString lWidgetFootPrint = QFileInfo(aZApp->argv()[0]).fileName() + ":" + lActiveWidget->className();
			TO_DBG("libredkey.so: Debug: Widget footprint is: '%s'\n", lWidgetFootPrint.data());
			ExecCommand(aZApp, aReason, G_APP_MAP[lWidgetFootPrint], lWidgetFootPrint);
		}
	} else {
		TO_DBG("libredkey.so: Debug: Reason '%d' is not implemented now.\n", aReason);
		CallOriginalSlotReturnToIdle(aZApp, aReason);
	}
}

void ZApplication::slotReturnToIdle(int aReason) {
#ifndef NO_CONFIG
	if (ReadConfigurationFile()) {
		TO_DBG("libredkey.so: Debug: Calling slotReturnToIdleFrom() method, parameter '%d'.\n", aReason);
		ProcessCustomRedKeyCommand(this, aReason);
	} else {
		TO_ERR("libredkey.so: Error: Cannot read '%s' config file, will call original slot method.\n",
			G_CONFIG_PATH.data());
		CallOriginalSlotReturnToIdle(this, aReason);
	}
#else
	if (getenv("LIBREDKEY_ON") && aReason == IDLE_REASON_RED_KEY) {
		G_DEBUG_OUTPUT = false;
		ShowDesktopMainScreen(this);
	} else
		CallOriginalSlotReturnToIdle(this, aReason);
#endif
}
