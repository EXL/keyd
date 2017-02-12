/* Written by EXL, 17-SEP-2016
 * Version: v1.0 | 13-FEB-2017
 * License: Public Domain */

// Defines
#define QT_THREAD_SUPPORT

// MotoMAGX
#include <ZApplication.h>

// Qt
#include <qwsevent_qws.h>
#include <qdatastream.h>
#include <qcopchannel_qws.h>
#include <qstring.h>
#include <qmap.h>
#include <qthread.h>

// C/Linux
#include <stdlib.h>
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
        configMap.clear();
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
        error = CONFIG_OK;
    }
public:
    ShittyCfgParser(const QString &fileName) : error(CONFIG_ERROR) {
        readFileData(fileName);
    }
    ~ShittyCfgParser() { }
    int getError() const { return error; }
    bool getVibro() const { return vibro; }
    int getVibroDur() const { return vibrodur; }
    QMap<int, QString> *getConfigMap() const {
        return const_cast<QMap<int, QString> *>(&configMap);
    }
    void readFileData(const QString &fileName) {
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
};

class VibroThread: public QObject, public QThread {
    Q_OBJECT
    int duration; int toggle;
public:
    VibroThread(int vibro, int vibrodur, QObject *parent = 0) : QObject(parent) {
        toggle = vibro;
        duration = vibrodur;
    }
protected:
    void run() {
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
    QMap<int, QString> *config; VibroThread *vibThread; QCopChannel *bcChannel;
public:
    Application(int argc, char *argv[]) : ZApplication(argc, argv) { }
    void registerChannels() {
        if (QCopChannel::isRegistered("/hardkey/bc")) {
            bcChannel = new QCopChannel("/hardkey/bc", this);
            connect(bcChannel,                                  // <- Throws event
                    SIGNAL(received(const QCString &, const QByteArray &)),
                    this,                                       // <- Catch event
                    SLOT(catchCoButton(const QCString &, const QByteArray &)));
        }
    }
    ~Application() {
        delete bcChannel;
        bcChannel = NULL;
    }
    void setConfigMap(QMap<int, QString> *a_config) { config = a_config; }
    void setVibroThread(VibroThread *a_vibThread) { vibThread = a_vibThread; }
protected:
    virtual bool qwsEventFilter(QWSEvent *event) {
        if (event->type == QWSEvent::Key) {
            QWSKeyEvent *keyEvent = static_cast<QWSKeyEvent *>(event);
#ifdef DEBUG_LOG
            qDebug(QString("win: %1, unicode: %2, keycode: %3, modifier: %4, press: %5, repeat: %6")
                   .arg(keyEvent->simpleData.window)
                   .arg(keyEvent->simpleData.unicode)
                   .arg(keyEvent->simpleData.keycode)
                   .arg(keyEvent->simpleData.modifiers)
                   .arg(keyEvent->simpleData.is_press)
                   .arg(keyEvent->simpleData.is_auto_repeat));
#endif
            if (keyEvent->simpleData.is_press == 1 && keyEvent->simpleData.is_auto_repeat == 0) {
                if (keyEvent->simpleData.keycode != 65286) { vibThread->start(); } // Drop Camera Shutter
                catchButton(keyEvent->simpleData.keycode, keyEvent->simpleData.is_press);
            }
        }
        return true;
    }
/*
protected slots:
    virtual void slotShutdown() { processEvents(); }
    virtual void slotQuickQuit() { processEvents(); }
    virtual void slotRaise() { processEvents(); }
*/
private:
    void catchButton(uint keycode, uint is_press) {
        if (is_press) {
            QString system_call = config->find(keycode).data();
            if (system_call) {
                system(QString("%1 %2").arg(system_call).arg(" &").ascii());
            }
        }
    }
private slots:
    void catchCoButton(const QCString &message, const QByteArray &data) {
        QDataStream stream(data, IO_ReadOnly);
        if (message == "keyMsg(int,int)") {
            int key, type;
            stream >> key >> type;
#ifdef DEBUG_LOG
            qDebug(QString("key: %1, type: %2").arg(key).arg(type));
#endif
            if (key == 65285) { // Gallery Key
                vibThread->start();
                catchButton(key, type);
            }
        }
    }
};

// Start
int main(int argc, char *argv[]) {
    int res = 0;
    Application *app = new Application(argc, argv);
    app->registerChannels();
#ifndef CONFIG_APPWRITE
    QString daemonDir = argv[0];
    int i = daemonDir.findRev("/");
    daemonDir.remove(i + 1, daemonDir.length() - i);
    daemonDir += "/keyd.cfg";
#else
    QString daemonDir = "/ezxlocal/download/appwrite/setup/keyd.cfg";
#endif
    ShittyCfgParser *cfgParser = new ShittyCfgParser(daemonDir);
    VibroThread *vibroThread = NULL;
    if (!cfgParser->getError()) {
        vibroThread = new VibroThread(cfgParser->getVibro(), cfgParser->getVibroDur(), app);
        app->setConfigMap(cfgParser->getConfigMap());
        app->setVibroThread(vibroThread);
        res = app->exec();
    } else {
        qDebug("FATAL: Config error! Shutdown!");
    }
    delete vibroThread;
    vibroThread = NULL;
    delete cfgParser;
    cfgParser = NULL;
    delete app;
    app = NULL;
    return res;
}

// MOC
#include "moc_keyd.cpp"
