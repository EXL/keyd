/*
 * About:
 *   RedKey is a special library for hacking red key button on MotoMAGX platform.
 *
 * Author:
 *   EXL
 *
 * License:
 *   MIT
 *
 * History:
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

// Qt
#include <qfile.h>
#include <qfileinfo.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextstream.h>

// EZX & MotoMAGX
#include <ZApplication.h>

// Defines
#define IDLE_REASON_RED_KEY -5000
#define IDLE_REASON_FLIP    -5001
#define IDLE_REASON_TIMEOUT -5002
#define IDLE_REASON_SLIDER  -5003
#define HACK_LIBRARY        "libezxappbase.so.1"
#define HACK_METHOD         "_ZN12ZApplication16slotReturnToIdleEi"
// #define CONFIG_PATH         "/mmc/mmca1/libredkey.cfg" // TODO
#define CONFIG_PATH         "/home/exl/Projects/keyd/libredkey/libredkey.cfg"
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
			if (!lLine.remove(0, 6).stripWhiteSpace().compare("Off"))
				G_DEBUG_OUTPUT = false;
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
		if (!QFileInfo(CONFIG_PATH).isReadable())
			return false;
		QFile lConfigFile(CONFIG_PATH);
		if (lConfigFile.open(IO_ReadOnly)) {
			QTextStream lTextStream(&lConfigFile);
			while (!lTextStream.eof())
				ParseConfigLine(lTextStream.readLine());
			G_CONFIG_PARSED = true;
			lConfigFile.close();
		}
		TO_DBG("libredkey.so: Debug: Output of parsed configuration file.\n");
		QMap<QString, QString>::Iterator lIt;
		for (lIt = G_APP_MAP.begin(); lIt != G_APP_MAP.end(); ++lIt)
			TO_DBG("libredkey.so: Debug: => key: '%s', value: '%s'\n", lIt.key().data(), lIt.data().data());
	}
	return true;
}

static void CallOriginalSlotReturnToIdle(ZApplication *aZApp, int aReason) {
	const qt_slot_method_t lMethod = GetOriginalSlotReturnToIdleDlSym();
	if (lMethod)
		(aZApp->*lMethod)(aReason);
}

static void ProcessCustomRedKeyCommand(ZApplication *aZApp, QApplication *aQApp, int aReason) {
	if (aReason == IDLE_REASON_RED_KEY) { // Only for Red Key reason now.
		const QWidget *lActiveWidget = aQApp->activeWindow();
		if (lActiveWidget) {
			const QString lWidgetFootPrint = QFileInfo(qApp->argv()[0]).fileName() + ":" + lActiveWidget->className();
			TO_DBG("libredkey.so: Debug: Widget footprint is: '%s'\n", lWidgetFootPrint.data());
			const QString lCommand = G_APP_MAP[lWidgetFootPrint];
			if (lCommand) {
				if (!lCommand.compare("hide"))
					TO_DBG("libredkey.so: Debug: Hide command not yet implemented.\n");
				else if (!lCommand.compare("original")) {
					TO_DBG("libredkey.so: Debug: Original command on '%s' app.\n", lWidgetFootPrint.data());
					CallOriginalSlotReturnToIdle(aZApp, aReason);
				} else if (lCommand.startsWith("/")) {
					TO_DBG("libredkey.so: Debug: Run command '%s' in '%s' application.\n",
						lCommand.data(), lWidgetFootPrint.data());
					system(lCommand.data());
				} else
					TO_DBG("libredkey.so: Debug: Ignore command in '%s' application.\n", lWidgetFootPrint.data());
			}
		}
	} else
		TO_DBG("libredkey.so: Debug: Reason '%d' is not implemented now.\n", aReason);
}

void ZApplication::slotReturnToIdle(int aReason) {
	if (ReadConfigurationFile() && !G_APP_MAP.isEmpty()) {
		TO_DBG("libredkey.so: Debug: Calling slotReturnToIdleFrom() method, parameter '%d'.\n", aReason);
		ProcessCustomRedKeyCommand(this, qApp, aReason);
	} else {
		TO_ERR("libredkey.so: Error: Config '%s' not exist or wrong, will call original slot method.\n", CONFIG_PATH);
		CallOriginalSlotReturnToIdle(this, aReason);
	}
}
