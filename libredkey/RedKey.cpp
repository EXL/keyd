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
#include <stdio.h>

// Qt
#include <qfileinfo.h>

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
			fprintf(stderr, "libredkey.so: Error: Cannot dlopen '%s' library!\n", HACK_LIBRARY);
			return NULL;
		}
		void *lHandleMethod = dlsym(lHandleLibrary, HACK_METHOD);
		if (!lHandleMethod) {
			fprintf(stderr, "libredkey.so: Error: Cannot find '%s' method!\n", HACK_METHOD);
			dlclose(lHandleLibrary);
			return NULL;
		}
		fprintf(stderr, "libredkey.so: Info: Copying address of method '%s': '%p'.\n", HACK_METHOD, lHandleMethod);
		memcpy(&lOriginalSlot, &lHandleMethod, sizeof(void *));
		dlclose(lHandleLibrary);
	}
	return lOriginalSlot;
}

void ZApplication::slotReturnToIdle(int aReason) {
	fprintf(stderr, "libredkey.so: Info: Calling slotReturnToIdleFrom() method, parameter '%d'.\n", aReason);

	QWidget *lActiveWidget = qApp->activeWindow();
	if (lActiveWidget) {
		fprintf(stderr, "libredkey.so: Info: Footprint is '%s:%s:%s'\n",
			QFileInfo(qApp->argv()[0]).fileName().data(), lActiveWidget->className(), lActiveWidget->name());

		fprintf(stderr, "libredkey.so: Info: Trying to hide active widget instead of closing.\n");
		lActiveWidget->hide();
	}
	const qt_slot_method_t lMethod = GetOriginalSlotReturnToIdleDlSym();
	if (lMethod)
		(this->*lMethod)(aReason);
}
