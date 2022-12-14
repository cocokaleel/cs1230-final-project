#pragma once

#include <QMainWindow>
#include "glwidget.h"
#include "qboxlayout.h"
#include "src/paint/canvas2d.h"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    void setupUI();

    //MainWindow();
    ~MainWindow();

private:
    GLWidget *glWidget;

    void setupCanvas2D();
    Canvas2D *m_canvas;


    void onClearButtonClick();
    void onUseMapButtonClick();
    void onAnimateButtonClick();

    void setBrushType(int type);
    void setUIntVal(std::uint8_t &r, std::uint8_t &b, std::uint8_t &g, int newValue);
    void setIntVal(int &setValue, int newValue);
    void addPushButton(QBoxLayout *layout, QString text, auto function);
    void addHeading(QBoxLayout *layout, QString text);

    void addLabel(QBoxLayout *layout, QString text);

    void addRadioButton(QBoxLayout *layout, QString text, bool value, auto function);
    void addSpinBox(QBoxLayout *layout, QString text, int min, int max, int step, int val, auto function);

};
