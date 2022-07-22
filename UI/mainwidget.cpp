#include <stdio.h>
#include <QDBusMessage>
#include "mainwidget.h"
#include "UI/base/xatom-helper.h"
#include <ukui-log4qt.h>

#define UKUI_FONT_SIZE "systemFontSize"
#define UKUI_THEME_COLOR "themeColor"

Widget *Widget::mutual = nullptr;
Widget::Widget(QStringList str, QWidget *parent)
    : QWidget(parent)
{
    mutual = this;

    //单例
    Single(str);

    stateMusicFile(str);
    int res;
    res = g_db->initDataBase();
    if(res != DB_OP_SUCC)
    {
        qDebug() << "数据库加载失败" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
    }
    qDebug() << "数据库加载成功";
    initAllComponent();
    initGSettings();
    allConnect();

    //初始化dbus
    initDbus();
    initStyle();
    if(!argName.isEmpty())
    {
        int num = argName.size();
        switch (num) {
        case 1:
            kylin_music_play_request(argName[0]);
            break;
        default:
            break;
        }
    }
    isFirstObject = false;
    //命令行删除到回收站
    process = new QProcess;
    m_initFinish = true;
}

Widget::~Widget()
{
    m_miniWidget->deleteLater();
}

void Widget::Single(QStringList path)   //单例
{
    QString str;
    if(path.size() > 1)
    {
        str = path[1];
    }
    QStringList homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    QString lockPath = homePath.at(0) + "/.config/kylin-music-lock";
    int fd = open(lockPath.toUtf8().data(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if(fd < 0)
    {
        exit(1);
    }
    if (lockf(fd, F_TLOCK, 0))
    {
        QDBusInterface interface( "org.ukui.kylin_music", "/org/ukui/kylin_music","org.ukui.kylin_music.play", QDBusConnection::sessionBus());
        if(path.size() == 1)
        {
            interface.call( "kylin_music_play_request", str);
        }
        else if(path.size() == 2)
        {
            if(str == "-sta" || str == "-state")
            {
                QDBusMessage play = interface.call("getState");

                if (play.type() == QDBusMessage::ReplyMessage)
                {
                    QString playStr = play.arguments().takeFirst().toString();
                    printf("当前播放状态 ：%s\n", (playStr.toStdString()).data());
                    fflush(stdout);
                }
            }
            if(str == "-t" || str == "-title")
            {
                QDBusMessage replay = interface.call("getTitle");

                if (replay.type() == QDBusMessage::ReplyMessage)
                {
                    QString replayStr = replay.arguments().takeFirst().toString();
                    printf("当前播放的歌曲名 ：%s\n", (replayStr.toStdString()).data());
                    fflush(stdout);
                }
            }
            interface.call( "kylin_music_play_request", str);
        }
        else if(path.size() == 4)
        {
            interface.call( "kylin_music_play_request", str, path[2], path[3]);
        }

        exit(0);
    }
    isFirstObject = true;//我是首个对象
    argName << str;
}

void Widget::stateMusicFile(QStringList args)
{
    if(args.length() != 2)
    {
        return;
    }

    QString file = args.at(1);

    if(!QFileInfo::exists(file))
    {
        return;
    }
    QFileInfo files(file);
    QString type = files.suffix().toLower();

    if(MusicFileInformation::getInstance().getMusicType().contains("*." + type))
    {
        playController::getInstance().setPlayListName(ALLMUSIC);
    }
}

void Widget::initDbus()//初始化dbus
{
    interface = new QDBusInterface("org.gnome.SessionManager",
                                   "/org/gnome/SessionManager",
                                   "org.gnome.SessionManager",
                                   QDBusConnection::sessionBus());

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if(sessionBus.registerService("org.ukui.kylin_music"))
    {
        sessionBus.registerObject("/org/ukui/kylin_music",this,
                                  QDBusConnection::ExportAllContents);
        qDebug()<<"初始化DBUS成功";
    }
    else
    {
        qDebug()<<"初始化DBUS失败";
    }
    //S3 S4策略
    QDBusConnection::systemBus().connect(QString("org.freedesktop.login1"),
                                         QString("/org/freedesktop/login1"),
                                         QString("org.freedesktop.login1.Manager"),
                                         QString("PrepareForShutdown"), this,
                                         SLOT(onPrepareForShutdown(bool)));
    QDBusConnection::systemBus().connect(QString("org.freedesktop.login1"),
                                         QString("/org/freedesktop/login1"),
                                         QString("org.freedesktop.login1.Manager"),
                                         QString("PrepareForSleep"), this,
                                         SLOT(onPrepareForSleep(bool)));
    // 锁屏
    QDBusConnection::sessionBus().connect(QString("org.ukui.ScreenSaver"),
                                         QString("/"),
                                         QString("org.ukui.ScreenSaver"),
                                         QString("lock"), this,
                                         SLOT(onScreenLock()));
    QDBusConnection::sessionBus().connect(QString("org.ukui.ScreenSaver"),
                                         QString("/"),
                                         QString("org.ukui.ScreenSaver"),
                                         QString("unlock"), this,
                                         SLOT(onScreenUnlock()));

    //蓝牙控
    QDBusConnection::systemBus().connect(QString(), QString("/"), "com.monitorkey.interface", "monitorkey", this, SLOT(client_get(QString)));
    //插拔
    QDBusConnection::sessionBus().connect(QString(), QString( "/"), "org.ukui.media", "sinkPortChanged",this, SLOT(inputDevice_get(QString)));
    //切换用户
    QDBusConnection::sessionBus().connect(QString("org.gnome.SessionManager"),
                                          QString("/org/gnome/SessionManager"),
                                          QString("org.gnome.SessionManager"),
                                          QString("PrepareForSwitchuser"), this,
                                          SLOT(slotPrepareForSwitchuser()));
    DbusAdapter *dbs_adapter = new DbusAdapter;
}

void Widget::initStyle()
{
    //不接受焦点。解决程序初始化时，空格键控制 播放/暂停 获取不到（Qt::Key_Space）事件
    QList<QPushButton*> list = this->findChildren<QPushButton*>();
    for(QPushButton *btn :list)
        btn->setFocusPolicy(Qt::NoFocus);
    QList<QToolButton*> lists = this->findChildren<QToolButton*>();
    for(QToolButton *btn :lists)
        btn->setFocusPolicy(Qt::NoFocus);
}

void Widget::onPrepareForShutdown(bool Shutdown)
{
    // 休眠
    KyInfo()<<"onPrepareForShutdown"<<Shutdown
           << "getState = " << playController::getInstance().getState();

    if(Shutdown) {
        if(playController::getInstance().getState() == playController::PLAY_STATE) {
            KyInfo() << "before pause";
            playController::getInstance().pauseOnly();
        }
    }
}

void Widget::onPrepareForSleep(bool isSleep)
{
    KyInfo()<<"onPrepareForShutdown"<< isSleep
           << "getState = " << playController::getInstance().getState();

    // 睡眠
    if(isSleep) {
        if(playController::getInstance().getState() == playController::PLAY_STATE) {
            KyInfo() << "before pause";
            playController::getInstance().pauseOnly();
        }
    }
}

void Widget::client_get(QString str)
{
    QString key = str.split(":").first();
    QString s = str.split(":").last();
    if(s == "1")
    {
        playSongArea->slotPlayClicked();
//        playController::getInstance().play();
    }
    else if(s == "2" || key == "163")
    {
        playController::getInstance().onNextSong();
    }
    else if(s == "3" || key == "165")
    {
        playController::getInstance().onPreviousSong();
    }
}

void Widget::inputDevice_get(QString str)
{
    if(playController::getInstance().getState() == playController::PLAY_STATE) {
        playController::getInstance().pause();
    }

}

void Widget::slotPrepareForSwitchuser()
{
    if(playController::getInstance().getState() == playController::PLAY_STATE) {
        playController::getInstance().pause();
    }
}

int Widget::kylin_music_play_request(QString cmd1, QString cmd2, QString cmd3)
{
    //无参数，单例触发
    if(!this->isActiveWindow())
    {
        KWindowSystem::forceActiveWindow(this->winId());
    }
    if(!m_miniWidget->isActiveWindow())
    {
        KWindowSystem::forceActiveWindow(m_miniWidget->winId());
    }
    if(isFirstObject&&!QFileInfo::exists(cmd1))
    {
        if(cmd1=="-c"||cmd1=="-close")
        {
            //------窗口关闭------
            this->close();
            exit(0);
        }
        isFirstObject = false;
        return 0;
    }

    //下一首
    if(cmd1 == "-n" || cmd1 == "-next")
    {
        //------下一首-------
        playController::getInstance().onNextSong();
        return 0;
    }
    //上一首
    if(cmd1 == "-b" || cmd1 == "-back")
    {
        //-------上一首------
        playController::getInstance().onPreviousSong();
        return 0;
    }
    //暂停
    if(cmd1 == "-p" || cmd1 == "-pause")
    {
        //------暂停------
        playController::getInstance().play();
        return 0;
    }
    //播放
    if(cmd1 == "-s" || cmd1 == "-start")
    {
        //------播放------
        playController::getInstance().play();
        return 0;
    }
    if(cmd1=="-i"||cmd1=="-increase")
    {
        //------调高音量------
        playSongArea->volumeIncrease();
        return 0;
    }
    if(cmd1=="-r"||cmd1=="-reduce")
    {
        //------减少音量------
        playSongArea->volumeReduce();
        return 0;
    }
    if(cmd1=="-sta"||cmd1=="-state")
    {
        //------检测当前播放状态
//        getState();
        return 0;
    }
    if(cmd1=="-init"||cmd1=="-initialization")
    {
        //------初始化音乐
        initMusic();
        return 0;
    }
    if(cmd1=="-t"||cmd1=="-title")
    {
        //------当前播放歌曲名
        getTitle();
        return 0;
    }
//    if(cmd1=="-S"||cmd1=="-Sequential")
//    {
//        //------顺序播放------
//        playController::getInstance().getPlaylist()->setPlaybackMode(MMediaPlaylist::Sequential);
//        return 0;
//    }
    if(cmd1=="-C"||cmd1=="-CurrentItemInLoop")
    {
        //------单曲循环------
        playController::getInstance().getPlaylist()->setPlaybackMode(MMediaPlaylist::CurrentItemInLoop);
        return 0;
    }
    if(cmd1=="-L"||cmd1=="-Loop")
    {
        //------列表循环------
        playController::getInstance().getPlaylist()->setPlaybackMode(MMediaPlaylist::Loop);
        return 0;
    }
    if(cmd1=="-R"||cmd1=="-Random")
    {
        //------随机播放------
        playController::getInstance().getPlaylist()->setPlaybackMode(MMediaPlaylist::Random);
        return 0;
    }
    if(cmd1=="-m"||cmd1=="-move")
    {
        if(cmd2!=""&&cmd3!="")
        {
            //------窗口移动------
            moveWidget(cmd2, cmd3);
            return 0;
        }
    }
    if(cmd1=="-c"||cmd1=="-close")
    {
        //------窗口关闭------
        this->close();
        return 0;
    }
    QStringList list;
    list << cmd1;
    if(list.isEmpty())
    {
        return 0;
    }
    importFile(list);
    playController::getInstance().delayMsecondSetVolume();

    return 0;
}

void Widget::slotText(QString btnText)
{
    listName = btnText;
}

void Widget::importFile(QStringList list)
{
//    MusicFileInformation::getInstance().addFile(list);
//    sideBarWid->playListBtn->click();
//    playController::getInstance().setPlayListName(ALLMUSIC);
    if (m_initFinish) {
        sideBarWid->playListBtn->click();
    }
    musicListTable->addMusicToDatebase(list);
    QList<musicDataStruct> resList;
    resList = MusicFileInformation::getInstance().resList;
    int ret;

    foreach (const musicDataStruct date, resList)
    {
        ret = g_db->addMusicToLocalMusic(date);
        if(ret == DB_OP_SUCC)
        {
            playController::getInstance().addSongToCurList(ALLMUSIC,date.filepath);
//            musicListTable->selectListChanged(tr("Song List"));
            if(listName == ALLMUSIC)
            {
                Q_EMIT signalRefreshList(ALLMUSIC);
            }
            QStringList pathList = getPath(ALLMUSIC);
            playController::getInstance().setCurPlaylist(ALLMUSIC,pathList);
            int index = MusicFileInformation::getInstance().findIndexFromPlayList(ALLMUSIC,date.filepath);
            playController::getInstance().play(ALLMUSIC,index);
        }
    }
    int ref;
    if(resList.size() > 0)
    {
        ref = g_db->getSongInfoListFromDB(resList,ALLMUSIC);
        if(ref == DB_OP_SUCC)
        {
            for(int i = 0;i < list.size();i++)
            {
                if(resList.at(i).filepath == list.at(i))
                {
                    QStringList pathList = getPath(ALLMUSIC);
                    playController::getInstance().setCurPlaylist(ALLMUSIC,pathList);
                    if(listName == ALLMUSIC)
                    {
                        Q_EMIT signalRefreshList(ALLMUSIC);
                    }
                    int index = MusicFileInformation::getInstance().findIndexFromPlayList(ALLMUSIC,resList.at(i).filepath);
                    if(index == -1)
                    {
                        return;
                    }
                    playController::getInstance().play(ALLMUSIC,index);
                }
            }
        }
    }
}

QStringList Widget::getPath(QString playListName)
{
    QStringList path;
    int ret;
    QList<musicDataStruct> resList;
    ret = g_db->getSongInfoListFromDB(resList,playListName);
    foreach (const musicDataStruct date, resList) {
        path.append(date.filepath);
    }
    return path;
}

QString Widget::getState()
{
    QString state;
    if(playController::getInstance().getState() == playController::PlayState::PLAY_STATE)
    {
        state = "播放";
    }
    else if(playController::getInstance().getState() == playController::PlayState::PAUSED_STATE)
    {
        state = "暂停";
    }
    else if(playController::getInstance().getState() == playController::PlayState::STOP_STATE)
    {
        state = "停止";
    }
    return state;
}

void Widget::slotStateChanged(playController::PlayState state)
{
    // 如果正在播放，阻止锁屏
    if(!interface->isValid())
    {
        return;
    }
    if(state == playController::PlayState::PLAY_STATE)
    {
        QDBusMessage reply = interface->call(QDBus::Block, "Inhibit", "kylin-music", (quint32)0, "music is playing", (quint32)8);
        m_inhibitValue = reply.arguments().takeFirst().toUInt();
    }
    else
    {
        interface->call("Uninhibit", m_inhibitValue);
    }
}

void Widget::onScreenLock()
{
    KyInfo() << "getState = " << playController::getInstance().getState();

    // 锁屏
    if(playController::getInstance().getState() == playController::PLAY_STATE) {
        KyInfo() << "screenlock pause";
        playController::getInstance().pauseOnly();
    }

}

void Widget::onScreenUnlock()
{
    KyInfo() << "getState = " << playController::getInstance().getState();

    // 锁屏解锁
#if 0
    if(playController::getInstance().getState() == playController::STOP_STATE
            || playController::getInstance().getState() == playController::PAUSED_STATE) {
        KyInfo() << "screenunlock play";
        playController::getInstance().play();
    }
#endif

}

void Widget::initMusic()
{
    QStringList playListName;
    g_db->getPlayList(playListName);
    for(auto listName : playListName)
    {
        if(listName == FAV)
        {
            continue;
        }
        sideBarWid->removePlayList(listName);
    }
    QList<musicDataStruct> retList;
    int sef = g_db->getSongInfoListFromDB(retList,FAV);
    if(sef == DB_OP_SUCC)
    {
        for(int i = 0;i < retList.size();i++)
        {
            int set = g_db->delMusicFromPlayList(retList.at(i).filepath,FAV);
            if(set == DB_OP_SUCC)
            {
                for(int i = 0; i < retList.size(); i++)
                {
                    playController::getInstance().removeSongFromCurList(FAV, 0);
                }
            }
        }
    }
    QList<musicDataStruct> resList;
    int ret = g_db->getSongInfoListFromDB(resList,ALLMUSIC);
    if(ret == DB_OP_SUCC)
    {
        for(int i = 0;i < resList.size();i++)
        {
            int ref = g_db->delSongFromEveryWhere(resList.at(i).filepath);
            if(ref == DB_OP_SUCC)
            {
                for(int i = 0; i < resList.size(); i++)
                {
                    playController::getInstance().removeSongFromCurList(ALLMUSIC, 0);
                }
            }
        }
    }
    sideBarWid->playListBtn->click();
    historyListTable->refreshHistoryTable();
}

void Widget::initAllComponent()
{

//    this->setProperty("useSystemStyleBlur", true);
    this->setAttribute(Qt::WA_TranslucentBackground, true);

//    this->setWindowFlag(Qt::FramelessWindowHint);
    setMinimumSize(960,640);
    this->setWindowTitle(tr("Music Player"));
//    this->setObjectName("mainWidget");
//    this->setWindowIcon(QIcon(":/img/kylin-music.png"));
    //窗体显示在中间
    QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
    this->move((availableGeometry.width() - this->width())/2, (availableGeometry.height() - this->height())/2);
    mainVBoxLayout = new QVBoxLayout();

//    musicListTable = new TableBaseView();
    QStringList lists;
    QString playlistName;
    int res = g_db->getPlayList(lists);
    if(res == DB_OP_SUCC)
    {
        playlistName = playController::getInstance().getPlayListName();
        if(playlistName == HISTORY)
        {
            playlistName = ALLMUSIC;
        }
        if(playlistName == SEARCH)
        {
            playlistName = ALLMUSIC;
        }
        if(lists.indexOf(playlistName) == -1)
        {
            playlistName = ALLMUSIC;
        }
    }

    musicListTable = new TableOne(playlistName,this);
    playSongArea = new PlaySongArea(this);
    m_titleBar = new TitleBar(this);

//    musicListTable->hide();
//    playSongArea->hide();


    // 去掉标题栏
    MotifWmHints hintt;
    hintt.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hintt.functions = MWM_FUNC_ALL;
    hintt.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hintt);

    m_miniWidget = new miniWidget();
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(m_miniWidget->winId(), hints);


    rightVWidget = new QWidget(this);
    rightVWidget->setLayout(mainVBoxLayout);
    mainVBoxLayout->addWidget(m_titleBar);
//    mainVBoxLayout->addSpacing(6);
    mainVBoxLayout->addWidget(musicListTable);
    mainVBoxLayout->addWidget(playSongArea,0,Qt::AlignBottom);
    mainHBoxLayout = new QHBoxLayout();
    sideBarWid = new SideBarWidget(this);
    sideBarWid->setFixedWidth(210);
    mainHBoxLayout->addWidget(sideBarWid);
    mainHBoxLayout->addWidget(rightVWidget);
    mainHBoxLayout->setSpacing(0);
    mainHBoxLayout->setMargin(0);
    mainVBoxLayout->setSpacing(0);
    mainVBoxLayout->setMargin(0);
    this->resize(960,640);
    this->setLayout(mainHBoxLayout);

    historyListTable = new TableHistory(this);
    MotifWmHints hint;
    hint.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hint.functions = MWM_FUNC_ALL;
    hint.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(historyListTable->winId(), hint);
    historyListTable->hide();

    this->setAutoFillBackground(true);

    m_quitWindow = new QShortcut(QKeySequence("Ctrl+Q"), this);
    m_quitWindow->setContext(Qt::WindowShortcut);
}

void Widget::allConnect()
{
    // 侧边栏区域
    connect(sideBarWid,&SideBarWidget::playListBtnClicked,musicListTable,&TableOne::selectListChanged);
    connect(sideBarWid,&SideBarWidget::playListRenamed,musicListTable,&TableOne::playListRenamed);
    connect(sideBarWid,&SideBarWidget::signalPlayAll,musicListTable,&TableOne::playAll);
    connect(sideBarWid,&SideBarWidget::playListBtnClicked,this,&Widget::slotText);
    connect(sideBarWid,&SideBarWidget::playListBtnClicked,playSongArea,&PlaySongArea::slotText);
    connect(sideBarWid,&SideBarWidget::playListBtnClicked,m_miniWidget,&miniWidget::slotText);

    // 歌单列表区域
    connect(this,&Widget::signalRefreshList,musicListTable,&TableOne::selectListChanged);
    connect(musicListTable,&TableOne::addILoveFilepathSignal,m_miniWidget,&miniWidget::slotFavIsExixts);
    connect(musicListTable,&TableOne::removeILoveFilepathSignal,m_miniWidget,&miniWidget::slotFavIsExixts);
    connect(musicListTable,&TableOne::addILoveFilepathSignal,playSongArea,&PlaySongArea::slotFavIsExixts);
    connect(musicListTable,&TableOne::removeILoveFilepathSignal,playSongArea,&PlaySongArea::slotFavIsExixts);
    connect(musicListTable,&TableOne::refreshHistoryListSignal,historyListTable,&TableHistory::refreshHistoryTable);
    connect(musicListTable,&TableOne::signalListSearch,sideBarWid,&SideBarWidget::slotListSearch);
    connect(musicListTable,&TableOne::signalSongListHigh,sideBarWid,&SideBarWidget::slotSongListHigh);

    // 标题栏区域
    connect(m_titleBar->searchEdit,&SearchEdit::signalReturnPressed,musicListTable,&TableOne::selectListChanged);
    connect(m_titleBar->searchEdit,&SearchEdit::signalReturnPressed,playSongArea,&PlaySongArea::slotText);
    connect(m_titleBar->searchEdit,&SearchEdit::signalReturnPressed,this,&Widget::slotText);
    connect(m_titleBar->searchEdit,&SearchEdit::signalReturnPressed,m_miniWidget,&miniWidget::slotText);
    connect(m_titleBar->searchEdit,&SearchEdit::signalReturnText,musicListTable,&TableOne::slotReturnText);
    connect(m_titleBar->searchEdit->m_result->m_MusicView,&MusicSearchListview::signalSearchTexts,musicListTable,&TableOne::slotSearchTexts);
    connect(m_titleBar->searchEdit->m_result,&SearchResult::signalFilePath,musicListTable,&TableOne::slotFilePath);
    connect(m_titleBar->searchEdit->m_result,&SearchResult::signalSongListBySinger,musicListTable,&TableOne::slotSongListBySinger);
    connect(m_titleBar->searchEdit->m_result,&SearchResult::signalSongListByAlbum,musicListTable,&TableOne::slotSongListByAlbum);
    connect(m_titleBar->searchEdit,&SearchEdit::signalReturnPressed,musicListTable,&TableOne::slotSearchReturnPressed);
    connect(m_titleBar->miniBtn,&QPushButton::clicked,this,&Widget::slotShowMiniWidget);
    connect(m_titleBar->closeBtn,&QPushButton::clicked,this,&Widget::slotClose);
    connect(m_titleBar->minimumBtn,&QPushButton::clicked,this,&Widget::slotShowMinimized);
    connect(m_titleBar->maximumBtn,&QPushButton::clicked,this,&Widget::slotShowMaximized);


    // 播放区域
//    connect(playSongArea,&PlaySongArea::signalAddFromFavButton,musicListTable,&TableOne::selectListChanged);
//    connect(playSongArea,&PlaySongArea::signalDelFromFavButton,musicListTable,&TableOne::selectListChanged);
    connect(playSongArea,&PlaySongArea::showHistoryListBtnClicked,historyListTable,&TableHistory::showHistroryPlayList);
    connect(playSongArea,&PlaySongArea::signalRefreshFav,musicListTable,&TableOne::selectListChanged);
    connect(playSongArea,&PlaySongArea::signalPlayingLab,this,&Widget::slotPlayingTitle);
    connect(playSongArea,&PlaySongArea::signalFavBtnChange,m_miniWidget,&miniWidget::slotFavBtnChange);
    connect(playSongArea,&PlaySongArea::signalPlayingLab,m_miniWidget,&miniWidget::slotPlayingLab);
    connect(playSongArea,&PlaySongArea::signalTimeLab,m_miniWidget,&miniWidget::slotTimeLab);

    connect(&playController::getInstance(),&playController::playerStateChange,this,&Widget::slotStateChanged);

    // 播放历史区域
    connect(historyListTable,&TableHistory::signalHistoryBtnChecked,playSongArea,&PlaySongArea::slotHistoryBtnChecked);

    // mini模式
    connect(m_miniWidget->m_recoveryWinBtn,&QPushButton::clicked,this,&Widget::slotRecoverNormalWidget);
    connect(m_miniWidget->m_closeBtn,&QPushButton::clicked,this,&Widget::slotCloseMiniWidget);
    connect(m_miniWidget->m_preBtn,&QPushButton::clicked,playSongArea,&PlaySongArea::slotPrevious);
    connect(m_miniWidget->m_playStateBtn,&QPushButton::clicked,playSongArea,&PlaySongArea::slotPlayClicked);
    connect(m_miniWidget->m_nextBtn,&QPushButton::clicked,playSongArea,&PlaySongArea::slotNext);
    connect(m_miniWidget,&miniWidget::signalFavBtnChange,playSongArea,&PlaySongArea::slotFavBtnChange);
    connect(m_miniWidget,&miniWidget::signalRefreshFav,musicListTable,&TableOne::selectListChanged);
    // mini模式快捷键
    connect(m_miniWidget, &miniWidget::playPauseKeySignal,playSongArea,&PlaySongArea::slotPlayClicked);
    connect(m_miniWidget, &miniWidget::previousPlayKeySignal, playSongArea,&PlaySongArea::slotPrevious);
    connect(m_miniWidget, &miniWidget::nextPlayKeySignal, playSongArea,&PlaySongArea::slotNext);
    connect(m_miniWidget, &miniWidget::addVolumeKeySignal, this, &Widget::VolumeUp);
    connect(m_miniWidget, &miniWidget::downVolumeKeySignal, this, &Widget::VolumeDown);
    connect(m_miniWidget, &miniWidget::miniCompleteSwitchKeySignal, this, &Widget::slotRecoverNormalWidget);
    connect(m_miniWidget, &miniWidget::loveSongKeySignal, playSongArea, &PlaySongArea::slotFav);

    // 快捷键设置
    connect(m_quitWindow, &QShortcut::activated, this, &Widget::slotClose);
    connect(this,&Widget::playPauseKeySignal,playSongArea,&PlaySongArea::slotPlayClicked);
    connect(this, &Widget::previousPlayKeySignal, playSongArea,&PlaySongArea::slotPrevious);
    connect(this, &Widget::nextPlayKeySignal, playSongArea,&PlaySongArea::slotNext);
    connect(this, &Widget::addVolumeKeySignal, this, &Widget::VolumeUp);
    connect(this, &Widget::downVolumeKeySignal, this, &Widget::VolumeDown);
    connect(this, &Widget::miniCompleteSwitchKeySignal, this,&Widget::slotShowMiniWidget);
    connect(this, &Widget::loveSongKeySignal, playSongArea, &PlaySongArea::slotFav);
}

void Widget::initGSettings()//初始化GSettings
{
    //只有非标准字号的控件才需要绑定
    connect(this,&Widget::signalSetFontSize,musicListTable,&TableOne::slotLableSetFontSize);
    connect(this,&Widget::signalSetFontSize,playSongArea,&PlaySongArea::slotLableSetFontSize);
    connect(this,&Widget::signalSetFontSize,m_miniWidget,&miniWidget::slotLableSetFontSize);
    connect(this,&Widget::signalSetFontSize,historyListTable,&TableHistory::slotLableSetFontSize);
    connect(this,&Widget::signalSetFontSize,m_titleBar->menumodule,&menuModule::slotLableSetFontSize);
    connect(this,&Widget::signalSetFontSize,sideBarWid->newSonglistPup,&AllPupWindow::slotLableSetFontSize);
    connect(this,&Widget::signalSetFontSize,sideBarWid->newSonglistPup->enterLineEdit,&LabEdit::slotLableSetFontSize);
    connect(this,&Widget::signalSetFontSize,sideBarWid->renameSongListPup,&AllPupWindow::slotLableSetFontSize);
    connect(this,&Widget::signalSetFontSize,sideBarWid->renameSongListPup->enterLineEdit,&LabEdit::slotLableSetFontSize);
    connect(this,&Widget::signalSetFontSize,m_titleBar->searchEdit->m_result,&SearchResult::slotLableSetFontSize);
    connect(this,&Widget::signalSetFontSize,musicListTable->infoDialog,&MusicInfoDialog::slotLableSetFontSize);

    if (QGSettings::isSchemaInstalled(FITCONTROLTRANS)) {
        m_transparencyGSettings = new QGSettings(FITCONTROLTRANS);
    }
//    if (m_transparencyGSettings != nullptr) {
//        connect(m_transparencyGSettings, &QGSettings::changed, this, [=](const QString &key) {
//            if (key == "transparency") {
//                transparencyChange();
//            }
//        });
//        transparencyChange();
//    }


    if(QGSettings::isSchemaInstalled(FITTHEMEWINDOWS))
    {
        themeData = new QGSettings(FITTHEMEWINDOWS);
        if(themeData->get("style-name").toString() == "ukui-dark" || themeData->get("style-name").toString() == "ukui-black"){
            WidgetStyle::themeColor = 1;
            changeDarkTheme();
        }
        else
        {
            WidgetStyle::themeColor = 0;
            changeLightTheme();
        }

        connect(themeData,&QGSettings::changed,this,[=]()
        {
            if(themeData->get("style-name").toString() == "ukui-dark" || themeData->get("style-name").toString() == "ukui-black"){
                WidgetStyle::themeColor = 1;
                changeDarkTheme();
            }
            else
            {
                WidgetStyle::themeColor = 0;
                changeLightTheme();
            }
        });

        connect(themeData,&QGSettings::changed,this,[=] (const QString &key) {
            if(key == UKUI_FONT_SIZE) {
                //获取字号的值
                int fontSizeKey = themeData->get(UKUI_FONT_SIZE).toString().toInt();
                //发送改变信号
                if (fontSizeKey > 0) {
                    Q_EMIT signalSetFontSize(fontSizeKey);
                }
            }
        });

        connect(themeData,&QGSettings::changed,this,[=] (const QString &key) {
            if(key == UKUI_THEME_COLOR) {
                //获取字号的值
                QString themeColorKey = themeData->get(UKUI_THEME_COLOR).toString();
                qDebug() << "主题色为： " << themeColorKey;
                //发送改变信号
                Q_EMIT signalSetThemeColor(themeColorKey);
            }
        });
    }

    //启动时设置字号
    int fontSizeKey = 11;//系统默认字号,魔鬼数字，自行处理
    if (themeData != nullptr) {
        int fontSizeKeyTmp = themeData->get(UKUI_FONT_SIZE).toString().toInt();
        if (fontSizeKeyTmp > 0) {
            fontSizeKey = fontSizeKeyTmp;
        }
    }
    Q_EMIT signalSetFontSize(fontSizeKey);
    qDebug()<<"初始化GSettings成功";
}

void Widget::resizeEvent(QResizeEvent *event)
{
    int max_w = qApp->primaryScreen()->size().width();

    //qApp->primaryScreen()->size().height() - this->height() [计算底边栏的高度]
    int max_h = qApp->primaryScreen()->size().height() - (qApp->primaryScreen()->size().height() - this->height());

    movePlayHistoryWid();
    if(this->width() >= max_w && this->height()>= max_h)
    {
        m_titleBar->maximumBtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));
        Minimize = true;
        m_titleBar->maximumBtn->setToolTip(tr("reduction"));
    }
    else if(this->isMaximized() == false || (this->width() < (max_w-20)) || this->height() < (max_h-20))
    {
        Minimize = false;
        m_titleBar->maximumBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
        m_titleBar->maximumBtn->setToolTip(tr("maximize"));
    }
    QWidget::resizeEvent(event);
}

void Widget::movePlayHistoryWid()
{
    historyListTable->setFixedSize(320,this->height() - playSongArea->height());

    QPoint historyPos = playSongArea->listBtn->mapToGlobal(playSongArea->listBtn->rect().topRight());
    QSize size = historyListTable->size();
    historyPos.setX(historyPos.x() + 26 - size.width());
    historyPos.setY(historyPos.y() - 30 - size.height());
    QSize historySize = playSongArea->listBtn->size();
    int newPosX = historyPos.x() - 26 + size.width();
    int newPosY = historyPos.y() + 30 + size.height();
    historyListTable->changePlayHistoryPos(newPosX, newPosY, historySize.width(), historySize.height());
    historyListTable->move(historyPos);
}

#if 0
void Widget::paintEvent(QPaintEvent *event)
{
//    return QWidget::paintEvent(event);
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect(), 0, 0);
    QStyleOption opt;
    opt.init(this);

    QColor mainColor;
//    KyInfo() << opt.palette.color(QPalette::Base);

    if (QColor(255,255,255) == opt.palette.color(QPalette::Base)
            || QColor(248,248,248) == opt.palette.color(QPalette::Base)
            || QColor(245, 245, 245) == opt.palette.color(QPalette::Base)) {
        mainColor = QColor(242, 242, 242, m_transparency);
    } else {
        mainColor = QColor(20, 20, 20, m_transparency);
    }

    p.fillPath(rectPath,QBrush(mainColor));

#if 0
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    p.setPen(Qt::NoPen);

//    KyInfo() << "transparency = " << m_transparencyGSettings->get("transparency").toDouble()
//             << "m_transparency = " << m_transparency;

    QColor color;

    color = palette().color(QPalette::Window);
    color.setAlpha(m_transparency);


    QPalette pal(this->palette());
    pal.setColor(QPalette::Window,QColor(color));
    this->setPalette(pal);
    QBrush brush =QBrush(color);
    p.setBrush(brush);
    p.drawRoundedRect(opt.rect,0,0);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    return QWidget::paintEvent(event);
#endif

}
#endif

void Widget::transparencyChange()
{
//    m_transparency = m_transparencyGSettings->get("transparency").toDouble() * 255;
    //    this->update();
}

void Widget::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_F1)
    {
        Q_EMIT signalShowGuide();
    }
    else if(event->key() == Qt::Key_Space)
    {
        Q_EMIT playPauseKeySignal();
    }
    if (event->modifiers() == Qt::ControlModifier
            && event->key() == Qt::Key_Left) {
        qDebug() << "ctrl+left";
        Q_EMIT previousPlayKeySignal();
    }

    if (event->modifiers() == Qt::ControlModifier
            && event->key() == Qt::Key_Right) {
        qDebug() << "ctrl+right";
        Q_EMIT nextPlayKeySignal();
    }

    if (event->modifiers() == Qt::ControlModifier
            && event->key() == Qt::Key_Up) {
        qDebug() << "ctrl+up";
        Q_EMIT addVolumeKeySignal();
    }

    if (event->modifiers() == Qt::ControlModifier
            && event->key() == Qt::Key_Down) {
        qDebug() << "ctrl+down";
        Q_EMIT downVolumeKeySignal();
    }

    if (event->modifiers() == Qt::ControlModifier
            && event->key() == Qt::Key_M) {
        qDebug() << "ctrl+m";
        Q_EMIT miniCompleteSwitchKeySignal();
    }

    if (event->modifiers() == Qt::ControlModifier
            && event->key() == Qt::Key_L) {
        qDebug() << "ctrl+l";
        Q_EMIT loveSongKeySignal();
    }

    QWidget::keyPressEvent(event);
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        this->setFocus();
    }

//    return Widget::mousePressEvent(event);
}

void Widget::slotShowMiniWidget()
{
    // 添加过渡动画
    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
    // 当数值为100后，重复性按快捷键ctrl+m会出现界面消失，所以需要减少到50
    animation->setDuration(50);
    animation->setStartValue(1);
    animation->setEndValue(0);
    connect(animation, &QPropertyAnimation::valueChanged, [&](QVariant value){
        update();
    });
    connect(animation, &QPropertyAnimation::finished, [&](){
        hide();
        setWindowOpacity(1);
    });

    QPropertyAnimation *animation_mini = new QPropertyAnimation(m_miniWidget, "windowOpacity");
    animation_mini->setDuration(50);
    animation_mini->setStartValue(0);
    animation_mini->setEndValue(1);
    connect(animation_mini, &QPropertyAnimation::valueChanged, [&](QVariant value){
        m_miniWidget->update();
    });

    m_miniWidget->setWindowOpacity(0);
    m_miniWidget->showNormal();
    m_miniWidget->activateWindow();
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    animation_mini->start(QAbstractAnimation::DeleteWhenStopped);
}

void Widget::moveWidget(QString newWidth, QString newHeight)
{
    int newX = newWidth.toInt();
    int newY = newHeight.toInt();
    this->move(newX, newY);
}

void Widget::slotClose()
{
    interface->call("Uninhibit", m_inhibitValue);
    this->close();
}

void Widget::slotShowMinimized()
{
    this->showMinimized();
    if(Minimize == true)
    {
        this->showMaximized();
    }
    else
    {
        this->showNormal();
    }
}

void Widget::slotShowMaximized()
{
    if(Minimize)
    {
        showNormal();
        Minimize = false;
        m_titleBar->maximumBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
        m_titleBar->maximumBtn->setToolTip(tr("maximize"));
        m_titleBar->maximumBtn->setProperty("isWindowButton", 0x1);
        m_titleBar->maximumBtn->setProperty("useIconHighlightEffect", 0x2);
        m_titleBar->maximumBtn->setFlat(true);
    }
    else
    {
        showMaximized();
        Minimize = true;
        m_titleBar->maximumBtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));
//                maximumBtn->setToolTip(tr("还原"));
        m_titleBar->maximumBtn->setToolTip(tr("reduction"));
        m_titleBar->maximumBtn->setProperty("isWindowButton", 0x1);
        m_titleBar->maximumBtn->setProperty("useIconHighlightEffect", 0x2);
        m_titleBar->maximumBtn->setFlat(true);
    }
}

/**
 * @brief Widget::Stop
 * 如果播放已停止，则不起作用；
 * 在此之后调用播放应会导致播放从曲目的开头重新开始
 */
void Widget::Stop() const
{
    playController::getInstance().stop();
}

/**
 * @brief Widget::VolumeUp
 * 音量增加
 */
void Widget::VolumeUp() const
{
    playSongArea->volumeIncrease();
}

/**
 * @brief Widget::VolumeDown
 * 音量降低
 */
void Widget::VolumeDown() const
{
    playSongArea->volumeReduce();
}

/**
 * @brief Widget::Next
 * 跳转到曲目列表的下一首曲目。
 * 如果没有下一首曲目（并且循环播放和列表播放都关闭），则停止播放。
 * 如果播放已暂停或停止，则保持原样。
 */
void Widget::Next() const
{
    playController::getInstance().onNextSong();
}

/**
 * 跳转到曲目列表的上一首曲目。
 * 如果没有上一首曲目（并且循环播放和列表播放都关闭），则停止播放。
 * 如果播放已暂停或停止，则保持原样。
 */
void Widget::Previous() const
{
    playController::getInstance().onPreviousSong();
}

void Widget::Play() const
{
    playController::getInstance().play();
}

/**
 * @brief Widget::Pause
 * 如果播放已暫停，则不起作用。
 *
 */
void Widget::Pause() const
{
    playController::getInstance().pauseOnly();
}

/**
 * @brief Widget::PlayPause
 * 如果播放已暫停，则恢复播放；
 * 如果播放停止，则开始播放。
 */
void Widget::PlayPause() const
{
    if (playController::getInstance().getState() == playController::PlayState::PLAY_STATE) {
        this->Pause();
    } else if (playController::getInstance().getState() == playController::PlayState::PAUSED_STATE) {
        this->Play();
    }
}

void Widget::slotRecoverNormalWidget()
{
    if(Minimize == true) {
        this->showMaximized();
    } else {
        this->showNormal();
    }
    m_miniWidget->hide();
}

void Widget::slotCloseMiniWidget()
{
    this->close();
}

void Widget::slotPlayingTitle(QString title)
{
    m_playTitle = title;
}

QString Widget::getTitle()
{
    qDebug() << "m_playTitle" << m_playTitle;
    return m_playTitle;
}

void Widget::slotReturnPressed()
{

}

void Widget::creartFinish()
{
    if (m_creatFinishEnum == MESSAGE) {
        if (m_creatFinishMsg.isEmpty()) {
            return;
        }
        QMessageBox *warn = new QMessageBox(QMessageBox::Warning,tr("Prompt information"),m_creatFinishMsg.toLocal8Bit().data(),QMessageBox::Yes,Widget::mutual);
        warn->button(QMessageBox::Yes)->setText("确定");
        warn->exec();
    }
}

void Widget::setCreatFinishMsg(QString msg)
{
    m_creatFinishEnum = MESSAGE;
    m_creatFinishMsg = msg;

    //如果界面已经显示，调用弹窗
    if (!this->isHidden()) {
        creartFinish();
    }
}

//切换深色主题
void Widget::changeDarkTheme()
{
    QPalette pal(palette());
    pal.setColor(QPalette::Background, QColor(38, 38, 38));
    setAutoFillBackground(true);
    setPalette(pal);

    sideBarWid->newSonglistPup->dlgcolor();
    sideBarWid->renameSongListPup->dlgcolor();
    sideBarWid->sidecolor();
    playSongArea->playcolor();
    m_miniWidget->minicolor();
    m_titleBar->titlecolor();
    musicListTable->initTableViewStyle();
    musicListTable->setHightLightAndSelect();
    musicListTable->initStyle();
    historyListTable->initStyle();
//    historyListTable->noRefreshHistory();
    musicListTable->tableView->setAlternatingRowColors(false);
    musicListTable->tableView->setShowGrid(false);
    playSongArea->m_volSliderWid->initColor();
    playSongArea->m_playBackModeWid->playModecolor();
    historyListTable->initStyle();
    historyListTable->noRefreshHistory();


}

//切换浅色主题
void Widget::changeLightTheme()
{
    QPalette pal(palette());
    pal.setColor(QPalette::Background, QColor(255, 255, 255));
    setAutoFillBackground(true);
    setPalette(pal);

    sideBarWid->newSonglistPup->dlgcolor();
    sideBarWid->renameSongListPup->dlgcolor();
    sideBarWid->sidecolor();
    playSongArea->playcolor();
    m_miniWidget->minicolor();
    m_titleBar->titlecolor();
    musicListTable->initTableViewStyle();
    musicListTable->setHightLightAndSelect();
    musicListTable->initStyle();
    musicListTable->tableView->setAlternatingRowColors(false);
    musicListTable->tableView->setShowGrid(false);
    playSongArea->m_volSliderWid->initColor();
    playSongArea->m_playBackModeWid->playModecolor();
    historyListTable->initStyle();
    historyListTable->noRefreshHistory();

}
