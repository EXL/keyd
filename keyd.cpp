/* Written by EXL, 17-SEP-2016 */

// MotoMAGX
#include <ZApplication.h>

// Qt
#include <qcopchannel_qws.h>
#include <qdatastream.h>
#include <qstring.h>
#include <qmap.h>

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

class KeyD: public ZApplication {
    Q_OBJECT
    QMap<int, QString> *config;
public:
    KeyD(int& argc, char** argv) : ZApplication(argc, argv) {
        std::cout << "Initializing keyd (keyboard deamon)" << std::endl;
        if (QCopChannel::isRegistered("/hardkey/bc")) {
            keyChannel = new QCopChannel("/hardkey/bc", this);
            connect(keyChannel,                                 // <- Throws event
                    SIGNAL(received(const QCString &, const QByteArray &)),
                    this,                                       // <- Catch event
                    SLOT(catchButton(const QCString &, const QByteArray &)));
        }
    }
    KeyD::~KeyD() { }
    void setConfigMap(QMap<int, QString> *a_config) {
        config = a_config;
    }
private slots:
    void catchButton(const QCString &message, const QByteArray &data) {
        QDataStream stream(data, IO_ReadOnly);
        if (message == "keyMsg(int,int)") {
            int key, type;
            stream >> key >> type;
            std::cout << "key: " << key << " type: " << type << std::endl;
            /*
            QString system_call = config->value(key);
            if (system_call) {
                system(system_call.ascii());
            }
            */
        }
    }
    void slotShutdown() { processEvents(); }
    void slotQuickQuit() { processEvents(); }
    void slotRaise() { processEvents(); }
};

// Start
int main(int argc, char *argv[])
{
    int res = 0;
    QString dDir = argv[0];
    int i = dDir.findRev("/");
    dDir.remove(i + 1, dDir.length() - i);
    dDir += "/keyd.cfg";
    ShittyCfgParser *cfgParser = new ShittyCfgParser(dDir);
    if (!cfgParser->getError()) {
        KeyD *keyD = new KeyD(argc, argv);
        keyD->setConfigMap(cfgParser->getConfigMap());
        res = keyD->exec();
    } else {
        std::cout << "FATAL: Config error! Shutdown!" << std::endl;
    }
    return res;
}

// MOC
#include "moc_keyd.cpp"
