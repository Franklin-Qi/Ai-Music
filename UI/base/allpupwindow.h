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

#ifndef ALLPUPWINDOW_H
#define ALLPUPWINDOW_H

#include <QMessageBox>
#include <QRegExp>
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QStackedWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QDialog>
#include <QPainter>
#include <QTextEdit>
#include <QLineEdit>
#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>
#include <QDebug>
#include <QMessageBox>
#include "UIControl/base/musicDataBase.h"
#include "UI/base/mylabel.h"
#include "UI/base/labedit.h"

class AllPupWindow :public QDialog
{
    Q_OBJECT
public:
    explicit AllPupWindow(QWidget *parent = nullptr);

    QPushButton *confirmBtn = nullptr;
    QPushButton *cancelBtn = nullptr;
    QToolButton *closeBtn = nullptr;
    QLabel      *titleLab = nullptr;
    QLabel      *tips = nullptr;
    LabEdit   *enterLineEdit = nullptr;
    QVBoxLayout *testLayout = nullptr;
    QHBoxLayout *titleLayout = nullptr;
    QHBoxLayout *btnLayout = nullptr;

    QStackedWidget *stackWid = nullptr;
    void dlgcolor();

    bool nameIsValid(QString textName);


public Q_SLOTS:
    void closeDialog();
    void slotTextChanged(QString text);
    void slotLableSetFontSize(int size);
private:
    void inpupdialog();

};
class  MusicInfoDialog : public QDialog
{
    Q_OBJECT
public:

    explicit MusicInfoDialog(musicDataStruct date);
    ~MusicInfoDialog();
    musicDataStruct musicDate;
public Q_SLOTS:
    //字体
    void slotLableSetFontSize(int size);
private:
    MyLabel *songNameLabel = nullptr;
    MyLabel *singerLabel = nullptr;
    MyLabel *albumLabel = nullptr;
    MyLabel *filePathLabel = nullptr;
    MyLabel *fileTypeLabel = nullptr;
    MyLabel *fileTimeLabel = nullptr;
    MyLabel *fileSizeLable = nullptr;

    QLabel *musicInfoLabel = nullptr;

    QVBoxLayout *mainVLayout = nullptr;

    QHBoxLayout *buttonLayout = nullptr;
    QWidget *btnWidget = nullptr;
    QPushButton *cancelBtn = nullptr;
    QPushButton *confirmBtn = nullptr;

    void initStyle();
};

#endif // ALLPUPWINDOW_H
