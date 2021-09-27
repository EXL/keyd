#include "TestApp.h"

#include <ZApplication.h>

void MainWidget::keyPressEvent(QKeyEvent *aEvent) {
	emit askToIdle(aEvent->key());
}

int main(int argc, char *argv[]) {
	ZApplication lApp(argc, argv);
	MainWidget lMainWidget;
	QObject::connect(&lMainWidget, SIGNAL(askToIdle(int)), &lApp, SLOT(slotReturnToIdle(int)));
	lApp.setMainWidget(&lMainWidget);
	lMainWidget.show();
	return lApp.exec();
}
