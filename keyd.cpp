/* Written by EXL, 17-SEP-2016 */

// Defines
#define QT_THREAD_SUPPORT

// MotoMAGX
#include <ZApplication.h>

// Qt
#include <qwsevent_qws.h>
#include <qdatastream.h>
#include <qstring.h>
#include <qmap.h>
#include <qthread.h>

// C++
#include <iostream>

// Global
QCopChannel *keyChannel;
enum Errors { CONFIG_OK, CONFIG_ERROR };

// Classes
class ShittyCfgParser {
    int error;
    QString cfgData;
    QMap<int, QString> configMap;
    void parseConfigData() {
        QStringList configList = QStringList::split('\n', cfgData);
        for (uint i = 0; i < configList.count(); ++i) {
            QString configStr = configList[i];
            if (configStr.startsWith("#")) {
                continue;
            } else if (configStr[0].isDigit()) {
                QStringList tokenizeLine = QStringList::split(':', configStr);
                if (tokenizeLine.count() == 2) {
                    configMap.insert(tokenizeLine[0].toInt(), tokenizeLine[1]);
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
                std::cout << "Error opening file: " << fileName.ascii() << std::endl;
            }
        } else {
            std::cout << "Error: config file: " << fileName.ascii()
                      << " doesn't exist." << std::endl;
        }
        configFile.close();
    }
    ~ShittyCfgParser() { }
    int getError() const { return error; }
    QMap<int, QString> *getConfigMap() const {
        return const_cast<QMap<int, QString> *>(&configMap);
    }
};

class Application : public ZApplication {
    Q_OBJECT
public:
        Application(int argc, char *argv[]) : ZApplication(argc, argv) { }
protected:
        virtual bool qwsEventFilter(QWSEvent *event) {
            if (event->type == QWSEvent::Key) {
                QWSKeyEvent *keyEvent = (QWSKeyEvent *)event;
                fprintf(stderr, "stderr: KEY: win=%x unicode=%x keycode=%x modifier=%x press=%x\n",
                        keyEvent->simpleData.window, keyEvent->simpleData.unicode, keyEvent->simpleData.keycode,
                        keyEvent->simpleData.modifiers, keyEvent->simpleData.is_press);
                std::cout << "cout: " << keyEvent->simpleData.keycode << std::endl;
                qDebug("qDebug: " + keyEvent->simpleData.is_press);
            }
            return ZApplication::qwsEventFilter(event);
        }
protected slots:
        virtual void slotShutdown() { processEvents(); }
        virtual void slotQuickQuit() { processEvents(); }
        virtual void slotRaise() { processEvents(); }
};

class KeyD: public QObject, public QThread {
    Q_OBJECT
    QMap<int, QString> *config;
    QCopChannel *bcChannel;
    QCopChannel *sysChannel;
public:
    KeyD(QObject *parent = 0) : QObject(parent) {
        std::cout << "Initializing keyd (keyboard deamon)" << std::endl;
        config = NULL;
        /*if (QCopChannel::isRegistered("/hardkey/bc")) {
            keyChannel = new QCopChannel("/hardkey/bc", this);
            connect(keyChannel,                                 // <- Throws event
                    SIGNAL(received(const QCString &, const QByteArray &)),
                    this,                                       // <- Catch event
                    SLOT(catchButton(const QCString &, const QByteArray &)));
        }*/
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
    void catchButton(const QCString &message,const QByteArray &data) {
        std::cout << "message: " << message << "; data: ";

        QDataStream stream(data, IO_ReadOnly);
        while(!stream.atEnd()) {
            int var = 0;
            stream >> var;
            std::cout << var << " ";
        }

        std::cout << std::endl;

        /* QDataStream stream(data, IO_ReadOnly);
        if (message == "keyMsg(int,int)") {
            int key, type;
            stream >> key >> type;
            std::cout << "key: " << key << " type: " << type << std::endl;

            QString system_call = config->value(key);
            if (system_call) {
                system(system_call.ascii());
            }

        } */
    }
};

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
        /*KeyD *keyD = new KeyD(NULL);
        keyD->setConfigMap(cfgParser->getConfigMap());
        keyD->start();*/
        res = app->exec();
        /*keyD->wait();*/
    } else {
        std::cout << "FATAL: Config error! Shutdown!" << std::endl;
    }
    return res;
}

// MOC
#include "moc_keyd.cpp"
