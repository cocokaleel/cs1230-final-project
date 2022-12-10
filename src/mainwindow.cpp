#include "mainwindow.h"
#include "src/paint/settings.h"
#include <QHBoxLayout>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QPushButton>
#include <QScrollArea>
#include <QFileDialog>
#include <QRadioButton>
#include <QSpinBox>

#include <QLabel>
#include <QGroupBox>
#include <iostream>

void MainWindow::setupUI()
{
    glWidget = new GLWidget;
    glWidget -> setFixedSize(502,502);

    QHBoxLayout *hLayout = new QHBoxLayout(); // horizontal alignment

    setupCanvas2D();
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidget(m_canvas);
    scrollArea->setWidgetResizable(false);
    scrollArea->setFixedSize(502,502);

    QWidget *brushGroup = new QWidget();
    QVBoxLayout *brushLayout = new QVBoxLayout();
    brushLayout->setAlignment(Qt::AlignCenter);
    brushGroup->setLayout(brushLayout);

    // brush selection
    addRadioButton(brushLayout, "Constant", settings.brushType == BRUSH_CONSTANT, [this]{ setBrushType(BRUSH_CONSTANT); });
    addRadioButton(brushLayout, "Linear", settings.brushType == BRUSH_LINEAR, [this]{ setBrushType(BRUSH_LINEAR); });
    addRadioButton(brushLayout, "Quadratic", settings.brushType == BRUSH_QUADRATIC, [this]{ setBrushType(BRUSH_QUADRATIC); });
    addRadioButton(brushLayout, "Smudge", settings.brushType == BRUSH_SMUDGE, [this]{ setBrushType(BRUSH_SMUDGE); });

    // brush parameters
    addSpinBox(brushLayout, "shade", 0, 255, 1, settings.brushColor.r, [this](int value){ setUIntVal(settings.brushColor.r,
                                                                                                     settings.brushColor.g,
                                                                                                     settings.brushColor.b,
                                                                                                     value);});

    addSpinBox(brushLayout, "alpha", 0, 255, 1, settings.brushColor.a, [this](int value){ setUIntVal(settings.brushColor.a,
                                                                                                     settings.brushColor.a,
                                                                                                     settings.brushColor.a,
                                                                                                     value); });
    addSpinBox(brushLayout, "radius", 0, 100, 1, settings.brushRadius, [this](int value){ setIntVal(settings.brushRadius, value); });

    // clearing canvas
    addPushButton(brushLayout, "Clear canvas", &MainWindow::onClearButtonClick);
    //using new heightmap
    addPushButton(brushLayout, "Use New Height Map", &MainWindow::onUseMapButtonClick);

    brushGroup -> setFixedSize(165, 524);

    hLayout -> addWidget(brushGroup);
    hLayout->addWidget(scrollArea, 1);
    hLayout->addWidget(glWidget, 1);

    this->setLayout(hLayout);
}

void MainWindow::setBrushType(int type) {
    settings.brushType = type;
    m_canvas->settingsChanged();
}

void MainWindow::addRadioButton(QBoxLayout *layout, QString text, bool value, auto function) {
    QRadioButton *button = new QRadioButton(text);
    button->setChecked(value);
    layout->addWidget(button);
    connect(button, &QRadioButton::clicked, this, function);
}

void MainWindow::addSpinBox(QBoxLayout *layout, QString text, int min, int max, int step, int val, auto function) {
    QSpinBox *box = new QSpinBox();
    box->setMinimum(min);
    box->setMaximum(max);
    box->setSingleStep(step);
    box->setValue(val);
    QHBoxLayout *subLayout = new QHBoxLayout();
    addLabel(subLayout, text);
    subLayout->addWidget(box);
    layout->addLayout(subLayout);
    connect(box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, function);
}

void MainWindow::setUIntVal(std::uint8_t &r,std::uint8_t &b,std::uint8_t &g, int newValue) {
    r = newValue;
    b = newValue;
    g = newValue;
    m_canvas->settingsChanged();
}

void MainWindow::setIntVal(int &setValue, int newValue) {
    setValue = newValue;
    m_canvas->settingsChanged();
}

void MainWindow::addPushButton(QBoxLayout *layout, QString text, auto function) {
    QPushButton *button = new QPushButton(text);
    layout->addWidget(button);
    connect(button, &QPushButton::clicked, this, function);
}

void MainWindow::addHeading(QBoxLayout *layout, QString text) {
    QFont font;
    font.setPointSize(16);
    font.setBold(true);

    QLabel *label = new QLabel(text);
    label->setFont(font);
    layout->addWidget(label);
}

void MainWindow::addLabel(QBoxLayout *layout, QString text) {
    layout->addWidget(new QLabel(text));
}

void MainWindow::onClearButtonClick() {
    m_canvas->resize(m_canvas->parentWidget()->size().width(), m_canvas->parentWidget()->size().height());


    glWidget->resetHeightMap();


    std::cout<<m_canvas->parentWidget()->size().width()<<std::endl;
    std::cout<<m_canvas->parentWidget()->size().height()<<std::endl;

    m_canvas->clearCanvas();
}

void MainWindow::onUseMapButtonClick() {

    //when use use map button is clicked, clear current vector of verts,
    //set the new value of m_canvas in GLWidget to m_canvas from paint,
    //set new current vector of verts to use in glWidget...................

    //when clearing canvas, should the terrain also clear?


    //In terrainGenerator, you need to interpolate so that 500x500 maps to 100x100



    ////also need to figure out what the heck is going on with reading the height map!////
    /// -not traversing the height map horizontally......///

  //  glWidget->resetHeightMap(m_canvas->getCanvasData());
std::cout<<"reset terrain vertices..."<<std::endl;

}


void MainWindow::setupCanvas2D() {
    m_canvas = new Canvas2D();
    m_canvas->init();

    if (!settings.imagePath.isEmpty()) {
        m_canvas->loadImageFromFile(settings.imagePath);
    }
}

MainWindow::~MainWindow() {}

