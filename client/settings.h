#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>

class Settings
{
    QString serverIp;
    quint16 port;

    Settings();

public:

    bool loadSettings(QString fileName);

    QString getIp() const {return serverIp;}
    void setIp(QString newIp);

    quint16 getPort() const {return port;}
    void setPort();
};

#endif // SETTINGS_H
