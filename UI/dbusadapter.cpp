#include "dbusadapter.h"

DbusAdapter::DbusAdapter(QObject *parent) : QObject(parent)
{
    QDBusConnection::sessionBus().unregisterService("org.mpris.MediaPlayer2.KylinMusic");
    QDBusConnection::sessionBus().registerService("org.mpris.MediaPlayer2.KylinMusic");
    QDBusConnection::sessionBus().registerObject("/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", this, QDBusConnection::ExportNonScriptableSlots);
}

void DbusAdapter::Stop() const
{
    Widget::mutual->Stop();
}

void DbusAdapter::Next() const
{
    Widget::mutual->Next();
}

void DbusAdapter::Play() const
{
    Widget::mutual->Play();
}

void DbusAdapter::Pause() const
{
    Widget::mutual->Pause();
}

void DbusAdapter::PlayPause() const
{
    Widget::mutual->PlayPause();
}

void DbusAdapter::Previous() const
{
    Widget::mutual->Previous();
}

void DbusAdapter::VolumeUp() const
{
    Widget::mutual->VolumeUp();
}

void DbusAdapter::VolumeDown() const
{
    Widget::mutual->VolumeDown();
}

void DbusAdapter::FullScreen() const
{
    Widget::mutual->slotShowMaximized();
}

void DbusAdapter::Exit() const
{
    Widget::mutual->slotClose();
}
