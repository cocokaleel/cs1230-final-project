#ifndef CANVAS2D_H
#define CANVAS2D_H

#include "src/RGBA.h"
#include "src/glwidget.h"
#include <QLabel>
#include <QMouseEvent>
#include <array>


class Canvas2D : public QLabel {
    Q_OBJECT
public:
    int m_width = 0;
    int m_height = 0;

    void init();
    void clearCanvas();
    bool loadImageFromFile(const QString &file);
    void displayImage();
    void resize(int w, int h);

    // This will be called when the settings have changed
    void settingsChanged();

    // Filter TODO: implement
    void filterImage();

    //accessing canvas data to be passed...
    std::vector<RGBA> getCanvasData();

private:
    std::vector<RGBA> m_data;

    void mouseDown(int x, int y);
    void mouseDragged(int x, int y);
    void mouseUp(int x, int y);

    // These are functions overriden from QWidget that we've provided
    // to prevent you from having to interact with Qt's mouse events.
    // These will pass the mouse coordinates to the above mouse functions
    // that you will have to fill in.
    virtual void mousePressEvent(QMouseEvent* event) override {
        auto [x, y] = std::array{ event->position().x(), event->position().y() };
        mouseDown(static_cast<int>(x), static_cast<int>(y));
    }
    virtual void mouseMoveEvent(QMouseEvent* event) override {
        auto [x, y] = std::array{ event->position().x(), event->position().y() };
        mouseDragged(static_cast<int>(x), static_cast<int>(y));
    }
    virtual void mouseReleaseEvent(QMouseEvent* event) override {
        auto [x, y] = std::array{ event->position().x(), event->position().y() };
        mouseUp(static_cast<int>(x), static_cast<int>(y));
    }

    // TODO: add any member variables or functions you need
    int posToIndex(int x, int y);
    int maskYtoCanvasY(int maskIndex, int x);
    int maskXtoCanvasX(int maskIndex, int y);

    int maskYinCanvas;
    int maskXinCanvas;

    void maskToCanvas(int x, int y);
    std::vector<float> fillMask(int bType, int bRadius);
    std::vector<float> filledOpacityVector;
    RGBA brushColor;
    std::vector<RGBA> prevColors;
    void pickUpMaskColors(int x, int y);

    int blurfRadius;
    float scaleX;
    float scaleY;


    void scaleW(std::vector<RGBA> &data, int width, int height, float scaleW);
    void scaleH(std::vector<RGBA> &data, int width, int height, float scaleH);



};

#endif // CANVAS2D_H
