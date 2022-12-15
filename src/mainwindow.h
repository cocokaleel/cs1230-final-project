#pragma once

#include <QMainWindow>
#include "glwidget.h"
#include "qboxlayout.h"
#include "qslider.h"
#include "qspinbox.h"
#include "src/paint/canvas2d.h"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    void setupUI();
    void finish();

    //MainWindow();
    ~MainWindow();

private:
    GLWidget *glWidget;

    void setupCanvas2D();
    Canvas2D *m_canvas;


    void onClearButtonClick();
    void onUseMapButtonClick();

    void setBrushType(int type);
    void addPushButton(QBoxLayout *layout, QString text, auto function);
    void addHeading(QBoxLayout *layout, QString text);

    void addLabel(QBoxLayout *layout, QString text);

    void addRadioButton(QBoxLayout *layout, QString text, bool value, auto function);
    void addSpinBox(QBoxLayout *layout, QString text, int min, int max, int step, int val, auto function);


    QSlider *shadeSlider;
    QSpinBox *shadeBox;

    QSlider *alphaSlider;
    QSpinBox *alphaBox;

    QSlider *radiusSlider;
    QSpinBox *radiusBox;


    void onValChangeShadeSlider(int newValue);
    void onValChangeShadeBox(int newValue);


    void onValChangeAlphaSlider(int newValue);
    void onValChangeAlphaBox(int newValue);

    void onValChangeRadiusSlider(int newValue);
    void onValChangeRadiusBox(int newValue);

    void onAnimateButtonClick();

    void onUploadButtonClick();

};
