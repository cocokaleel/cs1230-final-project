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
#include <QSlider>

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

    //setting up shade slider
    shadeSlider = new QSlider(Qt::Orientation::Horizontal);
    shadeSlider->setTickInterval(1);
    shadeSlider->setMinimum(0);
    shadeSlider->setMaximum(255);
    shadeSlider->setValue(settings.brushColor.r);

    shadeBox = new QSpinBox();
    shadeBox->setSingleStep(1);
    shadeBox->setMinimum(0);
    shadeBox->setMaximum(255);
    shadeBox->setValue(settings.brushColor.r);

    connect(shadeSlider, &QSlider::valueChanged, this, &MainWindow::onValChangeShadeSlider);

    connect(shadeBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this,  &MainWindow::onValChangeShadeBox);

    QHBoxLayout *subLayout = new QHBoxLayout();
    addLabel(subLayout, "Shade");
    subLayout->addWidget(shadeBox);
    subLayout->addWidget(shadeSlider);
    brushLayout->addLayout(subLayout);

    //setting up alpha slider
    alphaSlider = new QSlider(Qt::Orientation::Horizontal);
    alphaSlider->setTickInterval(1);
    alphaSlider->setMinimum(0);
    alphaSlider->setMaximum(255);
    alphaSlider->setValue(settings.brushColor.a);

    alphaBox = new QSpinBox();
    alphaBox->setSingleStep(1);
    alphaBox->setMinimum(0);
    alphaBox->setMaximum(255);
    alphaBox->setValue(settings.brushColor.a);

    connect(alphaSlider, &QSlider::valueChanged, this, &MainWindow::onValChangeAlphaSlider);

    connect(alphaBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this,  &MainWindow::onValChangeAlphaBox);

    subLayout = new QHBoxLayout();
    addLabel(subLayout, "Alpha");
    subLayout->addWidget(alphaBox);
    subLayout->addWidget(alphaSlider);
    brushLayout->addLayout(subLayout);

    //setting up radius slider
    radiusSlider = new QSlider(Qt::Orientation::Horizontal);
    radiusSlider->setTickInterval(1);
    radiusSlider->setMinimum(0);
    radiusSlider->setMaximum(100);
    radiusSlider->setValue(settings.brushRadius);

    radiusBox = new QSpinBox();
    radiusBox->setSingleStep(1);
    radiusBox->setMinimum(0);
    radiusBox->setMaximum(100);
    radiusBox->setValue(settings.brushRadius);

    connect(radiusSlider, &QSlider::valueChanged, this, &MainWindow::onValChangeRadiusSlider);

    connect(radiusBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this,  &MainWindow::onValChangeRadiusBox);

    subLayout = new QHBoxLayout();
    addLabel(subLayout, "Radius");
    subLayout->addWidget(radiusBox);
    subLayout->addWidget(radiusSlider);
    brushLayout->addLayout(subLayout);

    // clearing canvas
    addPushButton(brushLayout, "Clear canvas", &MainWindow::onClearButtonClick);

    //using load image2
    addPushButton(brushLayout, "Load Image", &MainWindow::onUploadButtonClick);

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

void MainWindow::onValChangeShadeSlider(int newValue){
    shadeBox->setValue(newValue);

    settings.brushColor.r = shadeBox->value();
    settings.brushColor.g = shadeBox->value();
    settings.brushColor.b = shadeBox->value();

    m_canvas->settingsChanged();
}

void MainWindow::onValChangeShadeBox(int newValue){
    shadeSlider->setValue(newValue);
    settings.brushColor.r = shadeBox->value();
    settings.brushColor.g = shadeBox->value();
    settings.brushColor.b = shadeBox->value();

    m_canvas->settingsChanged();

}

void MainWindow::onValChangeAlphaSlider(int newValue){
    alphaBox->setValue(newValue);

    settings.brushColor.a = alphaBox->value();

    m_canvas->settingsChanged();
}

void MainWindow::onValChangeAlphaBox(int newValue){
    alphaSlider->setValue(newValue);
    settings.brushColor.a = alphaBox->value();
    m_canvas->settingsChanged();

}


void MainWindow::onValChangeRadiusSlider(int newValue){
    radiusBox->setValue(newValue);

    settings.brushRadius = radiusBox->value();

    m_canvas->settingsChanged();
}
void MainWindow::onValChangeRadiusBox(int newValue){
    radiusSlider->setValue(newValue);
    settings.brushRadius = radiusBox->value();

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
    m_canvas->clearCanvas();
}

void MainWindow::onUseMapButtonClick() {
    //passes down a scaled down 100 x 100 canvas image
    glWidget->useNewHeightMap(m_canvas->getCanvasData());
}

void MainWindow::setupCanvas2D() {
    m_canvas = new Canvas2D();
    m_canvas->init();

    if (!settings.imagePath.isEmpty()) {
        m_canvas->loadImageFromFile(settings.imagePath);
    }
}


void MainWindow::onUploadButtonClick() {
    // Get new image path selected by user
    QString file = QFileDialog::getOpenFileName(this, tr("Open Image"), QDir::homePath(), tr("Image Files (*.png *.jpg *.jpeg)"));
    if (file.isEmpty()) { return; }
    settings.imagePath = file;

    // Display new image
    m_canvas->loadImageFromFile(settings.imagePath);

    m_canvas->settingsChanged();
}


void MainWindow::finish(){
    glWidget->finish();
    delete(glWidget);
}


MainWindow::~MainWindow() {}

