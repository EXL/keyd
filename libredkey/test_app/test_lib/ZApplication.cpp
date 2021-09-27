#include "ZApplication.h"

#include <qglobal.h>

ZApplication::ZApplication(int argc, char **argv) : QApplication(argc, argv) { }

void ZApplication::slotReturnToIdle(int aReason) {
	qDebug("Calling slotReturnToIdleFrom() from libezxappbase.so, parameter '%d'.", aReason);
}
