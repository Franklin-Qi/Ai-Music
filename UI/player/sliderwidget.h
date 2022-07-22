#ifndef SLIDERWIDGET_H
#define SLIDERWIDGET_H

#include <QWidget>
#include <QDialog>
#include <QHBoxLayout>
#include <QSlider>
#include <QEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QFocusEvent>
#include <QX11Info>

class SliderWidget : public QDialog
{
    Q_OBJECT
public:
    explicit SliderWidget(QWidget *parent = nullptr);
//    QDialog *vSliderDialog;
    QHBoxLayout *HLayout = nullptr;
    QSlider *vSlider = nullptr;
public:
    void changeVolumePos(int posX, int posY, int width, int height);
    void initColor();
protected:
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;   //鼠标滑块点击  事件过滤器
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) Q_DECL_OVERRIDE;
private:
    void initUi();

private:
    int volunmPosX;
    int volunmPosY;
    int volunmPosWidth;
    int volunmPosHeight;
};

#endif // SLIDERWIDGET_H
