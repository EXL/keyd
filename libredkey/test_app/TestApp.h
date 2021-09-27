#ifndef TEST_APP_H
#define TEST_APP_H

#include <qwidget.h>

class MainWidget : public QWidget {
	Q_OBJECT

protected:
	void keyPressEvent(QKeyEvent *aEvent);

signals:
	void askToIdle(int aReason);
};

#endif // TEST_APP_H
