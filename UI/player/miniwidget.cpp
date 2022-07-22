/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "miniwidget.h"
#include "UI/base/widgetstyle.h"
#include "UI/mainwidget.h"
#include <QDebug>

#define PT_9 9

miniWidget::miniWidget(QWidget *parent) : QFrame(parent)
{
    setFixedSize(300,60);
    this->setObjectName("miniWidget");

    m_mouseState = false;
    setWindowFlags(Qt::WindowStaysOnTopHint);
//    setWindowFlags(Qt::FramelessWindowHint|Qt::Tool|Qt::WindowStaysOnTopHint);
    this->setAttribute(Qt::WA_TranslucentBackground, true);     //窗体透明
//    this->setWindowTitle(tr("音乐"));
    this->setWindowTitle(tr("Music Player"));

    setMouseTracking(true);
    init_miniWidget();

    this->setLayout(m_HMainLayout);
    minicolor();

    initAction();
    initConnect();
    initStyle();
}

void miniWidget::initAction()
{
    playModeMenu = new QMenu(tr("列表循环"),this);
    playMode_Loop_Action = new QAction(QIcon(":/img/default/listloop.png"), tr("列表循环"), playModeMenu);
    playMode_Random_Action = new QAction(QIcon(":/img/default/random.png"), tr("随机播放"), playModeMenu);
    playMode_Sequential_Action = new QAction(QIcon(":/img/default/sequence.png"), tr("顺序播放"), playModeMenu);
    playMode_CurrentItemInLoop_Action = new QAction(QIcon(":/img/default/oneloop.png"), tr("单曲循环"), playModeMenu);
    playModeMenu->addAction(playMode_Loop_Action);
    playModeMenu->addAction(playMode_Random_Action);
    playModeMenu->addAction(playMode_Sequential_Action);
    playModeMenu->addAction(playMode_CurrentItemInLoop_Action);
    playModeMenu->addMenu(playModeMenu);
}

void miniWidget::mousePressEvent(QMouseEvent *event)
{
    m_WindowPos = this->pos();
    if(QRect(0,0,width(),height()).contains(event->pos()) && event->button() == Qt::LeftButton)
    {
        m_MousePos = event->globalPos();
        m_mouseState = true;
    }
}

void miniWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_mouseState)
    {
        move(m_WindowPos + (event->globalPos() - m_MousePos));
    }
}

void miniWidget::mouseReleaseEvent(QMouseEvent *)
{
    m_mouseState = false;
}

void miniWidget::enterEvent(QEvent *)
{
    m_songNameLab->setVisible(false);
    m_timeLab->setVisible(false);
    m_palyFrame->setVisible(true);
    coverWid->setVisible(true);
    this->setFixedWidth(328);
}

void miniWidget::leaveEvent(QEvent *)
{
    m_songNameLab->setVisible(true);
    m_timeLab->setVisible(true);
    m_palyFrame->setVisible(false);
    coverWid->setVisible(false);
    this->setFixedWidth(300);

}

void miniWidget::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Space)
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

    else if(event->key() == Qt::Key_Right)
    {
        return;
    }
    else if(event->key() == Qt::Key_Left)
    {
        return;
    }
    QWidget::keyPressEvent(event);
}

void miniWidget::minicolor()
{
    if(WidgetStyle::themeColor == 1)
    {
        m_mainFrame->setStyleSheet("#mainFrame{border-radius:6px;background-color:#252526;}");
        m_timeLab->setStyleSheet("QLabel{line-height:12px;color:#8F9399;}");
//        m_coverLabel->setPixmap(QPixmap(":/img/fengmian.png").scaled(48,48));
        m_coverLabel->setStyleSheet("border-image:url(:/img/fengmian.png);");

        m_songNameLab->setStyleSheet("color:#F9F9F9;");

        m_preBtn->setStyleSheet("QPushButton{background:transparent;border-radius:15px;border-image:url(:/img/dark/lastsong.png);}"
                               "QPushButton::hover{border-image:url(:/img/hover/lastsong.png);}"
                               "QPushButton::pressed{border-image:url(:/img/clicked/lastsong.png);}");

//        m_playStateBtn->setStyleSheet("QPushButton{border-radius:17px;border-image:url(:/img/default/play2.png);}"
//                                      "QPushButton::hover{border-image:url(:/img/hover/play2.png);}"
//                                      "QPushButton::pressed{border-image:url(:/img/clicked/play2.png);}");

        if(playController::getInstance().getState() == playController::PlayState::PLAY_STATE)
        {
            m_playStateBtn->setStyleSheet("QPushButton{border-radius:17px;border-image:url(:/img/default/pause2.png);}"
                                          "QPushButton::hover{border-image:url(:/img/hover/pause2.png);}"
                                          "QPushButton::pressed{border-image:url(:/img/clicked/pause2.png);}");
        }
        else if(playController::getInstance().getState() == playController::PlayState::PAUSED_STATE)
        {
            m_playStateBtn->setStyleSheet("QPushButton{border-radius:17px;border-image:url(:/img/default/play2.png);}"
                                          "QPushButton::hover{border-image:url(:/img/hover/play2.png);}"
                                          "QPushButton::pressed{border-image:url(:/img/clicked/play2.png);}");
        }
        else if(playController::getInstance().getState() == playController::PlayState::STOP_STATE)
        {
            m_playStateBtn->setStyleSheet("QPushButton{border-radius:17px;border-image:url(:/img/default/play2.png);}"
                                          "QPushButton::hover{border-image:url(:/img/hover/play2.png);}"
                                          "QPushButton::pressed{border-image:url(:/img/clicked/play2.png);}");
        }

        slotFavExixtsDark();


        m_nextBtn->setStyleSheet("QPushButton{background:transparent;border-radius:15px;border-image:url(:/img/dark/nextsong.png);}"
                               "QPushButton::hover{border-image:url(:/img/hover/nextsong.png);}"
                               "QPushButton::pressed{border-image:url(:/img/clicked/nextsong.png);}");

        coverWid->setStyleSheet("background-color:rgba(0,0,0,0.4);\
                                border-top-right-radius:6px;border-bottom-right-radius:6px;\
                                border-top-left-radius:0px;border-bottom-left-radius:0px");

    }
    else if(WidgetStyle::themeColor == 0)
    {
        m_mainFrame->setStyleSheet("#mainFrame{border-radius:6px;background-color:#FFFFFF;}");
        m_timeLab->setStyleSheet("QLabel{line-height:12px;color:#8F9399;}");
        m_coverLabel->setStyleSheet("border-image:url(:/img/fengmian.png);");
//        coverPhotoLabel->setStyleSheet("background:transparent;border-image:url(:/img/fengmian.png);");

        m_songNameLab->setStyleSheet("color:#303133;");

        m_preBtn->setStyleSheet("QPushButton{background:transparent;border-radius:15px;border-image:url(:/img/default/lastsong.png);}"
                               "QPushButton::hover{border-image:url(:/img/hover/lastsong.png);}"
                               "QPushButton::pressed{border-image:url(:/img/clicked/lastsong.png);}");
//        m_playStateBtn->setStyleSheet("QPushButton{border-radius:17px;border-image:url(:/img/default/play2.png);}"
//                                      "QPushButton::hover{border-image:url(:/img/hover/play2.png);}"
//                                      "QPushButton::pressed{border-image:url(:/img/clicked/play2.png);}");

        if(playController::getInstance().getState() == playController::PlayState::PLAY_STATE)
        {
            m_playStateBtn->setStyleSheet("QPushButton{border-radius:17px;border-image:url(:/img/default/pause2.png);}"
                                          "QPushButton::hover{border-image:url(:/img/hover/pause2.png);}"
                                          "QPushButton::pressed{border-image:url(:/img/clicked/pause2.png);}");
        }
        else if(playController::getInstance().getState() == playController::PlayState::PAUSED_STATE)
        {
            m_playStateBtn->setStyleSheet("QPushButton{border-radius:17px;border-image:url(:/img/default/play2.png);}"
                                          "QPushButton::hover{border-image:url(:/img/hover/play2.png);}"
                                          "QPushButton::pressed{border-image:url(:/img/clicked/play2.png);}");
        }
        else if(playController::getInstance().getState() == playController::PlayState::STOP_STATE)
        {
            m_playStateBtn->setStyleSheet("QPushButton{border-radius:17px;border-image:url(:/img/default/play2.png);}"
                                          "QPushButton::hover{border-image:url(:/img/hover/play2.png);}"
                                          "QPushButton::pressed{border-image:url(:/img/clicked/play2.png);}");
        }

        slotFavExixts();

        m_nextBtn->setStyleSheet("QPushButton{background:transparent;border-radius:15px;border-image:url(:/img/default/nextsong.png);}"
                                 "QPushButton::hover{border-image:url(:/img/hover/nextsong.png);}"
                                 "QPushButton::pressed{border-image:url(:/img/clicked/nextsong.png);}");

        coverWid->setStyleSheet("background-color:rgba(0,0,0,0.4);\
                                border-top-right-radius:6px;border-bottom-right-radius:6px;\
                                border-top-left-radius:0px;border-bottom-left-radius:0px");

    }
}

void miniWidget::init_miniWidget()
{
    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);

    m_HMainLayout = new QHBoxLayout();
    m_hLayout = new QHBoxLayout();

    m_mainFrame = new QFrame;
    m_mainFrame->setFixedSize(300,60);
    m_mainFrame->setObjectName("mainFrame");

    m_hMainLayout = new QHBoxLayout(m_mainFrame);

    /*******************************************/
    m_coverLabel = new QLabel;
    m_coverLabel->setFixedSize(40,40);
    m_coverLabel->setCursor(Qt::PointingHandCursor);

    /************************************/
    m_vInfoLayout = new QVBoxLayout;

    m_songNameLab = new MyLabel;
    m_songNameLab->setFixedSize(170,28);
//    m_songNameLab->setAlignment(Qt::AlignLeft);

    m_timeLab = new QLabel;
//    m_timeLab->setAlignment(Qt::AlignLeft);

    QString playPath = playController::getInstance().getPath();
    if(playPath != "")
    {
        songInfo(playPath);
        //根据保存的路径设置我喜欢按钮图标样式
        filePath = playPath;
        slotPositionChanged(0);
    }
    else
    {
        m_songNameLab->setText(tr("Music Player"));
        m_timeLab->setText(tr("00:00/00:00"));
    }

//    m_vInfoLayout->setMargin(3);
//    m_vInfoLayout->setSpacing(3);
    QWidget *vWidget = new QWidget(this);
    vWidget->setFixedHeight(44);
    m_vInfoLayout->addStretch();
    m_vInfoLayout->addWidget(m_songNameLab,0,Qt::AlignVCenter);
//    m_vInfoLayout->addSpacing(6);
    m_vInfoLayout->addWidget(m_timeLab,0,Qt::AlignVCenter);
    m_vInfoLayout->addStretch();
    m_vInfoLayout->setContentsMargins(8,3,0,3);
    m_vInfoLayout->setMargin(0);
    vWidget->setLayout(m_vInfoLayout);
//    m_vInfoLayout->setAlignment(Qt::AlignVCenter);

    /******************************************/
    m_palyFrame = new QFrame;
    m_palyFrame->setFixedWidth(120);
    m_palyFrame->setVisible(false);

    m_hPlayLayout = new QHBoxLayout(m_palyFrame);

    m_preBtn = new QPushButton;
    m_preBtn->setFixedSize(16,16);
    m_preBtn->setCursor(Qt::PointingHandCursor);

    m_playStateBtn = new QPushButton;
    m_playStateBtn->setFixedSize(36,36);
    m_playStateBtn->setCursor(Qt::PointingHandCursor);

    m_nextBtn = new QPushButton;
    m_nextBtn->setFixedSize(16,16);
    m_nextBtn->setCursor(Qt::PointingHandCursor);

    m_hPlayLayout->setMargin(0);
    m_hPlayLayout->setSpacing(9);
    m_hPlayLayout->addWidget(m_preBtn);
    m_hPlayLayout->addWidget(m_playStateBtn);
    m_hPlayLayout->addWidget(m_nextBtn);
    /****************************************************/

    m_loveBtn = new QPushButton;
    m_loveBtn->setFixedSize(36, 36);
    m_loveBtn->setCursor(Qt::PointingHandCursor);
//    m_loveBtn->setFocusPolicy(Qt::NoFocus);
    m_loveBtn->setProperty("isWindowButton", 0x1);
    m_loveBtn->setProperty("useIconHighlightEffect", 0x2);
    m_loveBtn->setFlat(true);

    m_orderBtn = new QPushButton;
    m_orderBtn->setFixedSize(36, 36);
    m_orderBtn->setCursor(Qt::PointingHandCursor);

    switch (playController::getInstance().mode()) {
    case 1:
        m_orderBtn->setIcon(QIcon::fromTheme("media-playlist-repeat-one-symbolic"));
        m_orderBtn->setProperty("isWindowButton", 0x1);
        m_orderBtn->setProperty("useIconHighlightEffect", 0x2);
        m_orderBtn->setFlat(true);
        m_orderBtn->setToolTip(tr("CurrentItemInLoop"));
        playController::getInstance().setPlaymode(playController::CurrentItemInLoop);
        break;
    case 3:
        m_orderBtn->setIcon(QIcon::fromTheme("media-playlist-repeat-symbolic"));
        m_orderBtn->setProperty("isWindowButton", 0x1);
        m_orderBtn->setProperty("useIconHighlightEffect", 0x2);
        m_orderBtn->setFlat(true);
        m_orderBtn->setToolTip(tr("Loop"));
        playController::getInstance().setPlaymode(playController::Loop);
        break;
    case 4:
        m_orderBtn->setIcon(QIcon::fromTheme("media-playlist-shuffle-symbolic"));
        m_orderBtn->setProperty("isWindowButton", 0x1);
        m_orderBtn->setProperty("useIconHighlightEffect", 0x2);
        m_orderBtn->setFlat(true);
        m_orderBtn->setToolTip(tr("Random"));
        playController::getInstance().setPlaymode(playController::Random);
        break;
    default:
        break;
    }

    /***************************************/
    m_vSysLayout = new QVBoxLayout;
    coverWid = new QWidget(this);
//    coverWid->setAttribute(Qt::WA_TranslucentBackground, true);     //窗体透明

    m_closeBtn = new QPushButton;
    m_closeBtn->setFixedSize(30,30);
    m_closeBtn->setCursor(Qt::PointingHandCursor);
    m_closeBtn->setIcon(QIcon(":/img/lightshow/close.svg"));
    m_closeBtn->setIconSize(QSize(18, 18));
    m_closeBtn->setStyleSheet("QPushButton{background:transparent;}");

    connect(m_closeBtn,&QPushButton::clicked,this,&miniWidget::close);
    //just hide not close

    m_recoveryWinBtn = new QPushButton;
    m_recoveryWinBtn->setFixedSize(30,30);
    m_recoveryWinBtn->setCursor(Qt::PointingHandCursor);
    m_recoveryWinBtn->setIcon(QIcon(":/img/lightshow/maximize.svg"));
    m_recoveryWinBtn->setIconSize(QSize(14, 14));
    m_recoveryWinBtn->setStyleSheet("QPushButton{background-color:transparent;}");


    m_vSysLayout->setSpacing(0);
    m_vSysLayout->setMargin(0);
    m_vSysLayout->addWidget(m_closeBtn);
    m_vSysLayout->addWidget(m_recoveryWinBtn);
//    m_vSysLayout->addStretch();

    coverWid->setLayout(m_vSysLayout);
    /********************************************/

//    m_hMainLayout->setMargin(10);
    m_hMainLayout->addWidget(m_coverLabel);
    m_hMainLayout->addWidget(vWidget);
    m_hMainLayout->addStretch();
    m_hMainLayout->addWidget(m_palyFrame);
    m_hMainLayout->addStretch();
    m_hMainLayout->addWidget(m_loveBtn);
    m_hMainLayout->addWidget(m_orderBtn);
//    m_hMainLayout->addLayout(m_vSysLayout);


    m_mainFrame->setLayout(m_hMainLayout);

    m_HMainLayout->setMargin(0);
    m_HMainLayout->addWidget(m_mainFrame);
    m_HMainLayout->addWidget(coverWid);

    //限制应用字体不随着主题变化
//    QFont sizeFont;
//    sizeFont.setPixelSize(14);
//    m_songNameLab->setFont(sizeFont);
//    m_timeLab->setFont(sizeFont);


//    QFrame *controlFrame = new QFrame(this);
//    controlFrame->setLayout(m_vSysLayout);

//    m_hLayout->addWidget(m_mainFrame);
//    m_hLayout->addWidget(controlFrame);

//    m_mainFrame->setStyleSheet("border-radius:12px;");
}

void miniWidget::slotLableSetFontSize(int size)
{
    //默认大小12px,换算成pt为9
    double lableBaseFontSize = PT_9;//魔鬼数字，自行处理
    double nowFontSize = lableBaseFontSize * double(size) / 11;//11为系统默认大小，魔鬼数字，自行处理
    QFont font;
    font.setPointSizeF(nowFontSize);
    m_timeLab->setFont(font);
}

//初始化样式
void miniWidget::initStyle()
{
    //不接受焦点高亮。解决点击后有蓝框的问题
    QList<QPushButton*> list = this->findChildren<QPushButton*>();
    for(QPushButton *btn :list)
        btn->setFocusPolicy(Qt::NoFocus);
}

void miniWidget::initConnect()
{
    connect(&playController::getInstance(),&playController::playerStateChange,this,&miniWidget::playerStateChange);
    connect(&playController::getInstance(),&playController::singalChangePath,this,&miniWidget::slotSongInfo);
    connect(m_loveBtn,&QPushButton::clicked,this,&miniWidget::slotFav);
    connect(m_orderBtn,&QPushButton::clicked,this,&miniWidget::slotPlayModeClicked);
    connect(&playController::getInstance(),&playController::signalPlayMode,this,&miniWidget::setPlayMode);
    connect(&playController::getInstance(),&playController::signalNotPlaying,this,&miniWidget::slotNotPlaying);
}

void miniWidget::playerStateChange(playController::PlayState newState)
{
    if(newState == playController::PlayState::PLAY_STATE)
    {
        m_playStateBtn->setStyleSheet("QPushButton{border-radius:17px;border-image:url(:/img/default/pause2.png);}"
                                      "QPushButton::hover{border-image:url(:/img/hover/pause2.png);}"
                                      "QPushButton::pressed{border-image:url(:/img/clicked/pause2.png);}");
    }
    else if(newState == playController::PlayState::PAUSED_STATE)
    {
        m_playStateBtn->setStyleSheet("QPushButton{border-radius:17px;border-image:url(:/img/default/play2.png);}"
                                      "QPushButton::hover{border-image:url(:/img/hover/play2.png);}"
                                      "QPushButton::pressed{border-image:url(:/img/clicked/play2.png);}");
    }
    else if(newState == playController::PlayState::STOP_STATE)
    {
        m_playStateBtn->setStyleSheet("QPushButton{border-radius:17px;border-image:url(:/img/default/play2.png);}"
                                      "QPushButton::hover{border-image:url(:/img/hover/play2.png);}"
                                      "QPushButton::pressed{border-image:url(:/img/clicked/play2.png);}");
    }
}

void miniWidget::slotSongInfo(QString path)
{
    filePath = path.remove("file://");
    QPixmap pix = MusicFileInformation::getInstance().getCoverPhotoPixmap(filePath);
    if(pix.isNull())
    {
        pix = QIcon(":/img/fengmian.png").pixmap(QSize(40,40));
    }
    else
    {
        pix = setCoverPhotoPixmapRadius(pix);
        pix = pix.scaled(QSize(40,40),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    }
    m_coverLabel->setPixmap(pix);
    slotFavExixts();
}

void miniWidget::slotText(QString btnText)
{
    listName = btnText;
}

void miniWidget::slotFav()
{
    if(g_db->checkSongIsInFav(filePath))
    {
        QList<musicDataStruct> resList;
        int ref = g_db->getSongInfoListFromDB(resList, FAV);
        int ret = g_db->delMusicFromPlayList(filePath,FAV);
        if(ref == DB_OP_SUCC)
        {
            //根据歌单名title值查询对应歌单列表
//            int ref = g_db->getSongInfoListFromDB(resList, "我喜欢");
            if(ret == DB_OP_SUCC)
            {
                for(int i = 0; i < resList.size(); i++)
                {
                    if(resList.at(i).filepath == filePath)
                    {
                        playController::getInstance().removeSongFromCurList(FAV, i);
                        //当前为我喜欢界面才刷新
                        if(listName == tr("I Love"))
                        {
                            Q_EMIT signalRefreshFav(FAV);
                        }
                        break;
                    }
                }
            }
        }
    }
    else
    {
        int ref = g_db->addMusicToPlayList(filePath,FAV);
        if(ref == DB_OP_SUCC)
        {
            playController::getInstance().addSongToCurList(FAV, filePath);
            if(listName == tr("I Love"))
            {
                Q_EMIT signalRefreshFav(FAV);
            }
        }
    }
    slotFavExixts();

}

void miniWidget::slotFavExixts()
{
    if (g_db->checkSongIsInFav(filePath)) {
        m_loveBtn->setIcon(QIcon::fromTheme("favorite-new-symbolic"));
        m_loveBtn->setProperty("useIconHighlightEffect", 0x4);
    } else {
        m_loveBtn->setIcon(QIcon::fromTheme("ukui-love-symbolic"));
        m_loveBtn->setProperty("useIconHighlightEffect", 0x2);
    }

    Q_EMIT signalFavBtnChange(filePath);
}

void miniWidget::slotFavExixtsDark()
{
    if(g_db->checkSongIsInFav(filePath))
    {
        m_loveBtn->setIcon(QIcon::fromTheme("favorite-new-symbolic"));
        m_loveBtn->setProperty("useIconHighlightEffect", 0x4);
    }
    else
    {
        m_loveBtn->setIcon(QIcon::fromTheme("ukui-love-symbolic"));
        m_loveBtn->setProperty("useIconHighlightEffect", 0x2);
    }
    Q_EMIT signalFavBtnChange(filePath);
}

void miniWidget::slotFavIsExixts(QString filePaths)
{
    if(g_db->checkSongIsInFav(filePaths))
    {
        //由于mini歌曲title是播放区传送故判断 ""
        if(m_songNameLab->text() == "")
        {
            return;
        }
        if(filePath != filePaths)
        {
            return;
        }
        m_loveBtn->setIcon(QIcon::fromTheme("favorite-new-symbolic"));
        m_loveBtn->setProperty("useIconHighlightEffect", 0x4);
    }
    else
    {
        if(filePath != filePaths)
        {
            return;
        }
        m_loveBtn->setIcon(QIcon::fromTheme("ukui-love-symbolic"));
        m_loveBtn->setProperty("useIconHighlightEffect", 0x2);
    }
}

/**
 * @brief miniWidget::setCoverPhotoPixmapRadius 修改封面圆角10px
 * @param pixmap
 * @return
 */
QPixmap miniWidget::setCoverPhotoPixmapRadius(QPixmap pixmap)
{
    int width = m_coverLabel->width();
    int height = m_coverLabel->height();
    QSize size(width, height);

    QBitmap mask(size);

    QPainter painter(&mask);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.fillRect(0, 0, size.width(), size.height(), Qt::white);
    painter.setBrush(QColor(0, 0, 0));
    painter.drawRoundedRect(0, 0, size.width(), size.height(), 10, 10); //修改这个值，可以改弧度，和直径相等就是圆形

    QPixmap image = pixmap.scaled(size);
    image.setMask(mask);

    return image;

}

void miniWidget::slotFavBtnChange(QString filePath)
{
    if(g_db->checkSongIsInFav(filePath))
    {
        m_loveBtn->setIcon(QIcon::fromTheme("favorite-new-symbolic"));
        m_loveBtn->setProperty("useIconHighlightEffect", 0x4);
    }
    else
    {
        m_loveBtn->setIcon(QIcon::fromTheme("ukui-love-symbolic"));
        m_loveBtn->setProperty("useIconHighlightEffect", 0x2);
    }
}

void miniWidget::songInfo(QString path)
{
    QString filepath = path.remove("file://");

    musicDataStruct musicStruct;
    g_db->getSongInfoFromDB(filepath, musicStruct);
    //使用库解析总时间
    m_time = musicStruct.time;
    if(musicStruct.title == "")
    {
        m_songNameLab->setText(tr("Music Player"));
        m_coverLabel->setPixmap(QIcon(":/img/fengmian.png").pixmap(QSize(40,40)));
    }
    else
    {
        m_songNameLab->setText(musicStruct.title);
        QPixmap pix = MusicFileInformation::getInstance().getCoverPhotoPixmap(filepath);
        if(pix.isNull())
        {
            pix = QIcon(":/img/fengmian.png").pixmap(QSize(40,40));
        }
        else
        {
            pix = setCoverPhotoPixmapRadius(pix);
            pix = pix.scaled(QSize(40,40),Qt::KeepAspectRatio,Qt::SmoothTransformation);
        }
        m_coverLabel->setPixmap(pix);
    }
}

void miniWidget::slotPositionChanged(qint64 position)
{
    QString str_time;
    int pos = position / 1000;
    int hour = static_cast<int>(pos / 3600);
    int minutes = static_cast<int>(pos % 3600 / 60);
    int seconds = static_cast<int>(pos % 60);

    QTime duration(hour, minutes, seconds);
    QStringList s = m_time.split(":");
    if(s.size() == 3) {
        str_time = duration.toString("hh:mm:ss");
    }
    else {
        str_time = duration.toString("mm:ss");
    }

    QString length = str_time + "/" + m_time;
    if(m_time == "")
    {
        m_timeLab->setText("00:00/00:00");
    }
    else
    {
        m_timeLab->setText(length);
    }
}

void miniWidget::slotPlayingLab(QString playing)
{
    m_songNameLab->setText(playing);
}

void miniWidget::slotTimeLab(QString time)
{
    m_timeLab->setText(time);
}

void miniWidget::slotPlayModeClicked()
{
    int playMode = playController::getInstance().playmode();
    switch (playMode)
    {
    case 1:
        ++playMode;
        ++playMode;
        break;
    case 3:
        ++playMode;
        break;
   case 4:
        playMode = 1;
    default:
        break;
    }
    setPlayMode(playMode);
    playController::getInstance().setMode(static_cast<playController::PlayMode>(playMode));
}

void miniWidget::setPlayMode(int playModel)
{
    switch (playModel) {
    case 1:
        m_orderBtn->setIcon(QIcon::fromTheme("media-playlist-repeat-one-symbolic"));
        m_orderBtn->setToolTip(tr("CurrentItemInLoop"));
        playController::getInstance().setPlaymode(playController::CurrentItemInLoop);
        break;
    case 3:
        m_orderBtn->setIcon(QIcon::fromTheme("media-playlist-repeat-symbolic"));
        m_orderBtn->setToolTip(tr("Loop"));
        playController::getInstance().setPlaymode(playController::Loop);
        break;
    case 4:
        m_orderBtn->setIcon(QIcon::fromTheme("media-playlist-shuffle-symbolic"));
        m_orderBtn->setToolTip(tr("Random"));
        playController::getInstance().setPlaymode(playController::Random);
        break;
    default:
        break;
    }

    playController::getInstance().setMode(static_cast<playController::PlayMode>(playModel));
}

void miniWidget::slotNotPlaying()
{
    m_songNameLab->setText(tr("Music Player"));
    m_timeLab->setText("00:00/00:00");
}

void miniWidget::songText(QString songName)
{
    QString show_songName = "";
    if(songName.length() > 10)
    {
        show_songName = songName.mid(0,9);
        show_songName.append("...");
        m_songNameLab->setText(show_songName);
        m_songNameLab->setToolTip(songName);
    }
    else
    {
        m_songNameLab->setText(songName);
        m_songNameLab->setToolTip(songName);
    }
}
