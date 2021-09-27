#ifndef Z_APPLICATION_H
#define Z_APPLICATION_H

#include <qapplication.h>

class ZApplication : public QApplication {
	Q_OBJECT

protected slots:
	virtual void slotReturnToIdle(int aReason);

public:
	ZApplication(int argc, char **argv);
};

#endif // Z_APPLICATION_H
