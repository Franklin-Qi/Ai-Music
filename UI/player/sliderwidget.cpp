#include "sliderwidget.h"
#include "UI/base/widgetstyle.h"
#include "UIControl/player/player.h"
//#include "UI/player/xatom-helper.h" 

SliderWidget::SliderWidget(QWidget *parent) : QDialog(parent)
{
    installEventFilter(this);
//    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);
    initUi(); //初始化样式
    initColor();
}

void SliderWidget::initUi()
{
    setFixedSize(30,90);
    vSlider = new QSlider(this);
    vSlider->installEventFilter(this);
    vSlider->setOrientation(Qt::Vertical);
    vSlider->setMinimum(0);
    vSlider->setMaximum(100);
    vSlider->setValue(playController::getInstance().getVolume());

    HLayout = new QHBoxLayout;
    HLayout->addWidget(vSlider);
    this->setLayout(HLayout);
}

bool SliderWidget::eventFilter(QObject *obj, QEvent *event)   //鼠标滑块点击
{
   if(obj == vSlider)
    {
        if (event->type()==QEvent::MouseButtonPress)           //判断类型
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) //判断左键
            {
               int dur = vSlider->maximum() - vSlider->minimum();
               int pos = vSlider->minimum() + (dur * (vSlider->height() - ((double)mouseEvent->y())) / vSlider->height());
               if(pos != vSlider->sliderPosition())
                {
                    vSlider->setValue(pos);
                }
            }
        }
    }
    return QObject::eventFilter(obj,event);
}

bool SliderWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(result);
    if(eventType != "xcb_generic_event_t")
    {
        return false;
    }

    xcb_generic_event_t *event = (xcb_generic_event_t*)message;
    switch (event->response_type & ~0x80)
    {
        case XCB_FOCUS_OUT:
            QRect rect(volunmPosX, volunmPosY, volunmPosWidth, volunmPosHeight);
            if(rect.contains(QCursor::pos(), false))
            {
                return 0;
            }
            else
            {
                this->hide();
                break;
            }
    }
    return false;
}

void SliderWidget::changeVolumePos(int posX, int posY, int width, int height)
{
    volunmPosX = posX;
    volunmPosY = posY;
    volunmPosWidth = width;
    volunmPosHeight = height;
}

void SliderWidget::initColor()
{
    if(WidgetStyle::themeColor == 1)
    {
//        vSlider->setStyleSheet(
//                               "QSlider::groove:vertical {width: 4px;\
//                                                       border: 1px solid #4A708B;\
//                                                       padding-left:-1px;\
//                                                       padding-right:-1px;\
//                                                       border-radius:2px;\
//                                                       padding-top:-1px;\
//                                                       padding-bottom:-1px;\
//                                                       }"
//                               "QSlider::add-page:vertical{background:#3790FA;}"
//                               "QSlider::sub-page:vertical{background:#4D4D4D;}"
//                               "QSlider::handle:vertical {\
//                                   background: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5,\
//                                   stop:0 #3790FA, stop:0.1 #3790FA);\
//                                   width: 10px;\
//                                   height: 10px;\
//                                   margin-top: -4px;\
//                                   margin-left: -3px;\
//                                   margin-bottom: -4px;\
//                                   margin-right: -3px;\
//                                   border-radius: 4px;\
//                                   margin: 0 -3px;\
//                               }"
//                               );
    }
    else if(WidgetStyle::themeColor == 0)
    {
//        vSlider->setStyleSheet(
//                               "QSlider::groove:vertical {width: 4px;\
//                                                       border: 1px solid #4A708B;\
//                                                       padding-left:-1px;\
//                                                       padding-right:-1px;\
//                                                       border-radius:2px;\
//                                                       padding-top:-1px;\
//                                                       padding-bottom:-1px;\
//                                                       }"
//                               "QSlider::add-page:vertical{background:#3790FA;}"
//                               "QSlider::sub-page:vertical{background:#ECEEF5;}"
//                               "QSlider::handle:vertical {\
//                                   background: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5,\
//                                   stop:0 #3790FA, stop:0.1 #3790FA);\
//                                   width: 10px;\
//                                   height: 10px;\
//                                   margin-top: -4px;\
//                                   margin-left: -3px;\
//                                   margin-bottom: -4px;\
//                                   margin-right: -3px;\
//                                   border-radius: 4px;\
//                                   margin: 0 -3px;\
//                               }"
//                               );
    }
}
