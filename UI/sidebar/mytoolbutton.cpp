#include "mytoolbutton.h"
#include "UI/base/widgetstyle.h"
#include "UIControl/base/musicDataBase.h"
#include <QDebug>

MyToolButton::MyToolButton()
{
    //在按钮没有默认选中时，实例化时先调用（故注释）
//    defaultStyle();
    this->setFixedSize(162,32);
    //文字在图标旁边
    this->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    this->setIconSize(QSize(16,16));
    connect(this,&MyToolButton::clicked,this,&MyToolButton::selectChanged);

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this,&MyToolButton::customContextMenuRequested,this,&MyToolButton::selectChanged);
    connect(this,&MyToolButton::customContextMenuRequested,this,&MyToolButton::requestCustomContextMenu);

    //限制应用内字体固定大小
//    QFont sizeFont;
//    sizeFont.setPixelSize(14);
//    this->setFont(sizeFont);

}

MyToolButton::~MyToolButton()
{

}

void MyToolButton::selectChanged()
{
    if(this->statusTip() == IS_SELECT)
    {
        Q_EMIT selectButtonChanged(this->text());
        return;
    }
    Q_EMIT selectButtonChanged(this->text());
    QList<MyToolButton *> list = this->parent()->parent()->parent()->findChildren<MyToolButton *>();
    for(MyToolButton *tmp : list)
    {
        if(tmp->statusTip() == IS_SELECT)
        {
            tmp->setStatusTip("");
            tmp->defaultStyle();
        }
    }
    this->setStatusTip(IS_SELECT);
    this->defaultStyle();
}

void MyToolButton::defaultStyle()
{
    if(this->statusTip() == IS_SELECT)
    {
        this->setStyleSheet("QToolButton{background-color:#3790FA;padding-left:14px;color:#FFFFFF;border-radius: 6px;}");
        if(buttonListName == ALLMUSIC)
        {
            this->setIcon(QIcon(":/img/clicked/ukui-folder-music-symbolic.svg"));
        }
        else if(buttonListName == FAV)
        {
            this->setIcon(QIcon(":/img/clicked/ukui-play-love-symbolic.svg"));
        }
        else
        {
            this->setIcon(QIcon(":/img/clicked/audio-card-symbolic.svg"));
        }
        return;
    }
    else if(this->statusTip() != IS_SELECT)
    {
        if(WidgetStyle::themeColor == 0)
        {
            //padding-left:15px;  左内边距
            this->setStyleSheet("QToolButton{padding-left:14px; background-color:#FFFFFF;color:#303133;border-radius:4px;}"
                                "QToolButton::hover{background-color:#EEEEEE;border-radius:4px;}");
            if(buttonListName == ALLMUSIC)
            {
                this->setIcon(QIcon(":/img/default/ukui-folder-music-symbolic.svg"));
            }
            else if(buttonListName == FAV)
            {
                this->setIcon(QIcon(":/img/default/ukui-play-love-symbolic.svg"));
            }
            else
            {
                this->setIcon(QIcon(":/img/default/audio-card-symbolic.svg"));
            }
        }

        else if(WidgetStyle::themeColor == 1)
        {
            this->setStyleSheet("QToolButton{padding-left:14px;background-color:#252526;color:#f9f9f9;border-radius:4px;}"
                                "QToolButton::hover{background-color:#303032;border-radius:4px;}");
            if(buttonListName == ALLMUSIC)
            {
                this->setIcon(QIcon(":/img/dark/ukui-folder-music-symbolic.svg"));
            }
            else if(buttonListName == FAV)
            {
                this->setIcon(QIcon(":/img/dark/ukui-play-love-symbolic.svg"));
            }
            else
            {
                this->setIcon(QIcon(":/img/dark/audio-card-symbolic.svg"));
            }
        }
    }
}

void MyToolButton::requestCustomContextMenu(const QPoint &pos)
{
    // 不用this，因此可以使用主题的QMenu
    QMenu *menu = new QMenu();

//    QFont sizeFont;
//    sizeFont.setPixelSize(14);
//    menu->setFont(sizeFont);
    QAction *playAct = new QAction(this);
    QAction *pauseAct = new QAction(this);
    renameAct = new QAction(this);
    deleteAct = new QAction(this);
    playAct->setText(tr("Play"));
    pauseAct->setText(tr("Pause"));
    renameAct->setText(tr("Rename"));
    deleteAct->setText(tr("Delete"));
    menu->addAction(playAct);
//    menu->addAction(pauseAct);

    QString text = this->text();
    QString btnText = text;

    connect(menu,&QMenu::triggered,this,[ = ](QAction * action)
    {

        if(action->text() == tr("Play"))
        {
            Q_EMIT playall(text);
        }
        else if(action->text() == tr("Pause"))
        {
//            Q_EMIT
        }
        else if(action->text() == tr("Rename"))
        {
            Q_EMIT renamePlayList(text);
        }
        else if(action->text() == tr("Delete"))
        {
            Q_EMIT removePlayList(text);
        }
    });

    if(btnText != tr("Song List") && btnText != tr("I Love"))
    {
        menu->addAction(renameAct);
        menu->addAction(deleteAct);
    }
    menu->exec(mapToGlobal(pos));
    delete menu;
    delete playAct;
    delete renameAct;
    delete deleteAct;
}

