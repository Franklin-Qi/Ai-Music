#include "searchedit.h"
#include "UI/base/xatom-helper.h"

SearchEdit::SearchEdit(QWidget *parent) : KSearchLineEdit(parent)
{
    setContextMenuPolicy(Qt::DefaultContextMenu);
    setCompleter(nullptr); // kdk自带记忆功能，取消搜索记忆功能

    connect(this, &SearchEdit::textChanged,this,&SearchEdit::slotTextChanged);
    connect(this, &SearchEdit::returnPressed,this,&SearchEdit::slotReturnPressed);
//    connect(this, &SearchEdit::editingFinished,this,&SearchEdit::slotEditingFinished);

    m_result = new SearchResult(m_mainWidget);
    m_result->setSearchEdit(this);
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(m_result->winId(), hints);

    moveSearchResult();
}

void SearchEdit::keyPressEvent(QKeyEvent *event)
{
    if(m_result == nullptr)
    {
        return;
    }
    if(event->key() == Qt::Key_Up)
    {
        m_result->selectUp();
    }
    if(event->key() == Qt::Key_Down)
    {
        m_result->selectDown();
    }
    QLineEdit::keyPressEvent(event);
}

void SearchEdit::focusInEvent(QFocusEvent *event)
{
    this->setFocus();

    return QLineEdit::focusInEvent(event);
}

void SearchEdit::focusOutEvent(QFocusEvent *event)
{
    this->clearFocus();

    return QLineEdit::focusOutEvent(event);
}

void SearchEdit::slotTextChanged()
{
    if (m_result == nullptr) {
        return;
    }

    QString alltext = this->text();
    if (alltext.isEmpty()) {
        m_result->hide();
        return;
    }/* else {
        m_result->show();
        m_result->raise();
    }*/

    this->setCursorPosition(this->cursorPosition());

    auto text = this->text().trimmed();

    if (this->text().size() == 0) {
        m_result->hide();
        return;
    }
    if (text.length() >= 1) {
        m_result->setListviewSearchString(text);

        moveSearchResult();
        m_result->show();
//        m_result->activateWindow();
//        m_result->raise();
        m_result->autoResize();
//        m_result->setFocusPolicy(Qt::StrongFocus);
//        m_result->raise();
    } else {
        m_result->hide();
    }
}

void SearchEdit::slotReturnPressed()
{
    if (m_result == nullptr) {
        return;
    }

    if (m_result->getCurrentIndex() >= 0) {
        m_result->onReturnPressed();
        m_result->hide();
        return;
    }

    auto text = QString(this->text().trimmed());
    if(text.length() == 0)
        return;
    m_result->hide();
    Q_EMIT signalReturnPressed(tr("Search Result"));
    Q_EMIT signalReturnText(text);
}

void SearchEdit::setWidget(QWidget *mainWidget)
{
    m_mainWidget = mainWidget;
}

void SearchEdit::moveSearchResult()
{
    m_result->setFixedWidth(240);
    QPoint resultPos = this->mapToGlobal(this->rect().bottomLeft());
    resultPos.setX(resultPos.x());
    resultPos.setY(resultPos.y() + 12);
    QSize editSize = this->size();
    int newPosX = resultPos.x();
    int newPosY = resultPos.y() - editSize.height() - 12;
    m_result->changeSrearchResultPos(newPosX, newPosY, editSize.width(), editSize.height());
    m_result->move(resultPos);
}

