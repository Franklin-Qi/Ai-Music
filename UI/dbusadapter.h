#ifndef DBUSADAPTER_H
#define DBUSADAPTER_H

#include <QObject>
#include <QDBusConnection>
//#include "UIControl/player/player.h"
#include "mainwidget.h"

class DbusAdapter : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")
public:
    explicit DbusAdapter(QObject *parent = nullptr);

public Q_SLOTS:
    // mpris 使用
    void Stop() const;
    void Next() const;
    void Play() const;
    void Pause() const;
    void PlayPause() const;
    void Previous() const;
    void VolumeUp() const;
    void VolumeDown() const;
    void FullScreen() const;
    void Exit() const;


};

#endif // DBUSADAPTER_H
