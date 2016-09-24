/* Written by EXL, 17-SEP-2016
 * Updated: 22-SEP-2016
 * License: Public Domain */

// Defines
#define QT_THREAD_SUPPORT

// MotoMAGX
#include <ZApplication.h>

// Qt
#include <qwsevent_qws.h>
#include <qstring.h>
#include <qmap.h>
#include <qthread.h>

// C/Linux
#include <unistd.h>
#include <linux/power_ic.h>
#include <sys/ioctl.h>
#include <fcntl.h>

// Global
enum Errors { CONFIG_OK, CONFIG_ERROR };

// Classes
class ShittyCfgParser {
    int error; bool vibro; int vibrodur;
    QString cfgData;
    QMap<int, QString> configMap;
    void parseConfigData() {
        QStringList configList = QStringList::split('\n', cfgData);
        int count_config_lines = 0;
        for (uint i = 0; i < configList.count(); ++i) {
            QString configStr = configList[i];
            if (configStr.startsWith("#")) {
                continue;
            } else if (configStr[0].isDigit()) {
                count_config_lines++;
                QStringList tokenizeLine = QStringList::split(':', configStr);
                if (tokenizeLine.count() == 2) {
                    if (count_config_lines == 1) { // First element for vibration
                        vibro = tokenizeLine[0].toInt(); vibrodur = tokenizeLine[1].toInt();
                    } else {
                        configMap.insert(tokenizeLine[0].toInt(), tokenizeLine[1]);
                    }
                }
            }
        }
        if (configMap.count() > 0) {
            error = CONFIG_OK;
        }
    }
public:
    ShittyCfgParser(const QString &fileName) : error(CONFIG_ERROR) {
        QFile configFile(fileName);
        if (configFile.exists()) {
            if (configFile.open(IO_ReadOnly)) {
                int size = configFile.size();
                char data[size];
                configFile.readBlock(data, sizeof(data));
                cfgData = data;
                parseConfigData();
            } else {
                qDebug(QString("Error opening file: %1.").arg(fileName));
            }
        } else {
            qDebug(QString("Error: config file: %1 doesn't exist.").arg(fileName));
        }
        configFile.close();
    }
    ~ShittyCfgParser() { }
    int getError() const { return error; }
    bool getVibro() const { return vibro; }
    int getVibroDur() const { return vibrodur; }
    QMap<int, QString> *getConfigMap() const {
        return const_cast<QMap<int, QString> *>(&configMap);
    }
};

class VibroThread: public QObject, public QThread {
    Q_OBJECT
    int duration; int toggle;
public:
    VibroThread(int vibro, int vibrodur, QObject *parent = 0) : QObject(parent) { toggle = vibro; duration = vibrodur; }
protected:
    void run() {
        qDebug(QString("Vibrate! %1 %2").arg(toggle).arg(duration));
        if (toggle == 1) {
            int power_ic = open("/dev/" POWER_IC_DEV_NAME, O_RDWR);
            ioctl(power_ic, POWER_IC_IOCTL_PERIPH_SET_VIBRATOR_ON, 1);
            this->msleep(duration);
            ioctl(power_ic, POWER_IC_IOCTL_PERIPH_SET_VIBRATOR_ON, 0);
            close(power_ic);
        }
    }
};

class Application : public ZApplication {
    Q_OBJECT
    QMap<int, QString> *config;
    VibroThread *vibThread;
public:
    Application(int argc, char *argv[]) : ZApplication(argc, argv) { }
    ~Application() { }
    void setConfigMap(QMap<int, QString> *a_config) { config = a_config; }
    void setVibroThread(VibroThread *a_vibThread) { vibThread = a_vibThread; }
protected:
    virtual bool qwsEventFilter(QWSEvent *event) {
        if (event->type == QWSEvent::Key) {
            QWSKeyEvent *keyEvent = static_cast<QWSKeyEvent *>(event);
            qDebug(QString("win: %1, unicode: %2, keycode: %3, modifier: %4, press: %5, repeat: %6")
                   .arg(keyEvent->simpleData.window)
                   .arg(keyEvent->simpleData.unicode)
                   .arg(keyEvent->simpleData.keycode)
                   .arg(keyEvent->simpleData.modifiers)
                   .arg(keyEvent->simpleData.is_press)
                   .arg(keyEvent->simpleData.is_auto_repeat));
            if (keyEvent->simpleData.is_press == 1 && keyEvent->simpleData.is_auto_repeat == 0) {
                vibThread->start();
                catchButton(keyEvent->simpleData.keycode, keyEvent->simpleData.is_press);
            }
        }
        return ZApplication::qwsEventFilter(event);
    }
protected slots:
    virtual void slotShutdown() { processEvents(); }
    virtual void slotQuickQuit() { processEvents(); }
    virtual void slotRaise() { processEvents(); }
private:
    void catchButton(uint keycode, uint is_press) {
        if (is_press) {
            QString system_call = config->find(keycode).data();
            if (system_call) {
                system(QString("%1 %2").arg(system_call).arg(" &").ascii());
            }
        }
    }
};

#if 0
/*
class KeyD: public QObject, public QThread {
    Q_OBJECT
    QMap<int, QString> *config;
    QCopChannel *bcChannel;
    QCopChannel *sysChannel;
public:
    KeyD(QObject *parent = 0) : QObject(parent) {
        std::cout << "Initializing keyd (keyboard deamon)" << std::endl;
        config = NULL;
    }
    KeyD::~KeyD() {
        std::cout << "Shutdown keyd (keyboard deamon)...";
        delete sysChannel;
        sysChannel = NULL;
        delete bcChannel;
        bcChannel = NULL;
        std::cout << "done." << std::endl;
    }
    void setConfigMap(QMap<int, QString> *a_config) {
        config = a_config;
    }
protected:
    void run() {
        std::cout << "Starting keyd (keyboard deamon)...";
        bcChannel = new QCopChannel("/hardkey/bc", this);
        connect(bcChannel,
                SIGNAL(received(const QCString &,const QByteArray &)),
                this,
                SLOT(catchButton(const QCString &,const QByteArray &)));

        sysChannel = new QCopChannel("EZX/System", this);
        connect(sysChannel,
                SIGNAL(received(const QCString &,const QByteArray &)),
                this,
                SLOT(catchButton(const QCString &,const QByteArray &)));
        std::cout << "done." << std::endl;
    }
private slots:

};*/
#endif

// Start
int main(int argc, char *argv[]) {
    int res = 0;
    Application *app = new Application(argc, argv);
    QString daemonDir = argv[0];
    int i = daemonDir.findRev("/");
    daemonDir.remove(i + 1, daemonDir.length() - i);
    daemonDir += "/keyd.cfg";
    ShittyCfgParser *cfgParser = new ShittyCfgParser(daemonDir);
    if (!cfgParser->getError()) {
        VibroThread *vibroThread = new VibroThread(cfgParser->getVibro(), cfgParser->getVibroDur(), app);
        app->setConfigMap(cfgParser->getConfigMap());
        app->setVibroThread(vibroThread);
        res = app->exec();
    } else {
        qDebug("FATAL: Config error! Shutdown!");
    }
    return res;
}

// MOC
#include "moc_keyd.cpp"
