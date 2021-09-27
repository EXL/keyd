#include "TestApp.h"

#include <qpopupmenu.h>

#include <ZApplication.h>

void MainWidget::keyPressEvent(QKeyEvent *aKeyEvent) {
	emit askToIdle(aKeyEvent->key());
}

void MainWidget::mouseMoveEvent(QMouseEvent *aMouseEvent) {
	QPopupMenu menu(this, "Test App");
	menu.insertItem("Test Item", 0, 0);
	menu.exec(aMouseEvent->pos());
	emit askToIdle(aMouseEvent->pos().x());
}

int main(int argc, char *argv[]) {
	ZApplication lApp(argc, argv);
	MainWidget lMainWidget;
	lApp.setGlobalMouseTracking(true);
	lMainWidget.setMouseTracking(true);
	QObject::connect(&lMainWidget, SIGNAL(askToIdle(int)), &lApp, SLOT(slotReturnToIdle(int)));
	lApp.setMainWidget(&lMainWidget);
	lMainWidget.show();
	return lApp.exec();
}
