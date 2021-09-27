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
 *   27-Sep-2021: First testing prototype.
 *
 * Information:
 *   Use LD_PRELOAD=libredkey.so for applying this hack.
 */

// C
#include <dlfcn.h>

// Qt
#include <qfileinfo.h>
#include <qglobal.h>

// EZX & MotoMAGX
#include <ZApplication.h>

// Defines
#define HACK_LIBRARY "libezxappbase.so.1"
#define HACK_METHOD  "_ZN12ZApplication16slotReturnToIdleEi"

typedef void (ZApplication::*qt_slot_method_t)(int aReason);

static qt_slot_method_t GetOriginalSlotReturnToIdleDlSym() {
	static qt_slot_method_t lOriginalSlot = NULL;
	if (!lOriginalSlot) {
		void *lHandleLibrary = dlopen(HACK_LIBRARY, RTLD_LAZY);
		if (!lHandleLibrary) {
			qDebug("libredkey.so: Error: Cannot dlopen '%s' library!", HACK_LIBRARY);
			return NULL;
		}
		void *lHandleMethod = dlsym(lHandleLibrary, HACK_METHOD);
		if (!lHandleMethod) {
			qDebug("libredkey.so: Error: Cannot find '%s' method!", HACK_METHOD);
			dlclose(lHandleLibrary);
			return NULL;
		}
		qDebug("libredkey.so: Info: Copying address of method '%s': '%p'.", HACK_METHOD, lHandleMethod);
		memcpy(&lOriginalSlot, &lHandleMethod, sizeof(void *));
		dlclose(lHandleLibrary);
	}
	return lOriginalSlot;
}

void ZApplication::slotReturnToIdle(int aReason) {
	qDebug("libredkey.so: Info: Calling slotReturnToIdleFrom() method, parameter '%d'.", aReason);

	QWidget *lActiveWidget = qApp->activeWindow();
	qDebug("libredkey.so: Info: Footprint is '%s:%s:%s'",
		QFileInfo(qApp->argv()[0]).fileName().data(), lActiveWidget->className(), lActiveWidget->name());

	qDebug("libredkey.so: Info: Trying to hide active widget instead of closing.");
	lActiveWidget->hide();

	const qt_slot_method_t lMethod = GetOriginalSlotReturnToIdleDlSym();
	if (lMethod)
		(this->*lMethod)(aReason);
}
