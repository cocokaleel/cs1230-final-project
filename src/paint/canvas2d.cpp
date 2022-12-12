#include "canvas2d.h"
#include <QPainter>
#include <QMessageBox>
#include <QFileDialog>
#include <iostream>
#include "filters.h"
#include "settings.h"
#include "src/glwidget.h"

/**
 * @brief Initializes new 500x500 canvas
 */
void Canvas2D::init(){
    m_width = 500;
    m_height = 500;
    clearCanvas();
}

/**
 * @brief Canvas2D::clearCanvas sets all canvas pixels to blank white
 */
void Canvas2D::clearCanvas() {
    m_data.assign(m_width * m_height, RGBA{122, 122, 122, 255});
    settings.imagePath = "";
    displayImage();
}

/**
 * @brief Stores the image specified from the input file in this class's
 * `std::vector<RGBA> m_image`.
 * Also saves the image width and height to canvas width and height respectively.
 * @param file: file path to an image
 * @return True if successfully loads image, False otherwise.
 */
bool Canvas2D::loadImageFromFile(const QString &file) {
    QImage myImage;
    if (!myImage.load(file)) {
        std::cout<<"Failed to load in image"<<std::endl;
        return false;
    }
    myImage = myImage.convertToFormat(QImage::Format_RGBX8888);
    m_width = myImage.width();
    m_height = myImage.height();
    QByteArray arr = QByteArray::fromRawData((const char*) myImage.bits(), myImage.sizeInBytes());

    m_data.clear();
    m_data.reserve(m_width * m_height);
    for (int i = 0; i < arr.size() / 4.f; i++){
        m_data.push_back(RGBA{(std::uint8_t) arr[4*i], (std::uint8_t) arr[4*i+1], (std::uint8_t) arr[4*i+2], (std::uint8_t) arr[4*i+3]});
    }
    displayImage();
    return true;
}

/**
 * @brief Get Canvas2D's image data and display this to the GUI
 */
void Canvas2D::displayImage() {
    QByteArray* img = new QByteArray(reinterpret_cast<const char*>(m_data.data()), 4*m_data.size());
    QImage now = QImage((const uchar*)img->data(), m_width, m_height, QImage::Format_RGBX8888);
    setPixmap(QPixmap::fromImage(now));
    setFixedSize(m_width, m_height);
    update();
}

/**
 * @brief Canvas2D::resize resizes canvas to new width and height
 * @param w
 * @param h
 */
void Canvas2D::resize(int w, int h) {
    m_width = w;
    m_height = h;
    m_data.resize(w * h);
    displayImage();
}

/**
 * @brief Called when the filter button is pressed in the UI
 */
void Canvas2D::filterImage() {
    Canvas2D::settingsChanged();
    switch(settings.filterType) {
    case FILTER_EDGE_DETECT:
        Filters::filterGray(m_data, m_width, m_height);
        Filters::edgeDetect(m_data, m_width, m_height);
        break;
    case FILTER_BLUR:
        Filters::convolve1D(m_data, m_width, m_height, Filters::createTriangleFilter(blurfRadius), true);
        Filters::convolve1D(m_data, m_width, m_height, Filters::createTriangleFilter(blurfRadius), false);
        break;
    case FILTER_SCALE:
        Canvas2D::scaleW(m_data, m_width, m_height, scaleX);
        Canvas2D::scaleH(m_data, m_width, m_height, scaleY);
        break;
    }

    displayImage();
}

/**
 * @brief Called when any of the parameters in the UI are modified.
 */
void Canvas2D::settingsChanged() {
    // this saves your UI settings locally to load next time you run the program
    settings.saveSettings();

    //change the value for the filter radius
    this -> blurfRadius = settings.blurRadius;
    //change the value for the scale
    this -> scaleX = settings.scaleX;
    this -> scaleY = settings.scaleY;


    //creating a new mask using settings values
    Canvas2D::filledOpacityVector = fillMask(settings.brushType, settings.brushRadius);
    //creating the previous colors mask, which is used for smudge
    this->prevColors = std::vector<RGBA>(pow(2* settings.brushRadius + 1, 2));
}

/**
 * @brief These functions are called when the mouse is clicked and dragged on the canvas
 */
void Canvas2D::mouseDown(int x, int y) {
    //change settings everytime we officially click on the canvas, checking for new radius and brush type
    settingsChanged();
    //creates the first set of values for the smudge brush
    if(settings.brushType == BRUSH_SMUDGE){
        for (int i = 0; i < pow((2* settings.brushRadius + 1), 2)-1; i++){
            int maskXinCanvas = maskXtoCanvasX(i, y);
            int maskYinCanvas = maskYtoCanvasY(i, x);
            if(maskXinCanvas >= 0 && maskYinCanvas >= 0 && maskXinCanvas <= m_width -1  && maskYinCanvas <= m_width-1){
                Canvas2D::prevColors[i] = m_data[posToIndex(maskXinCanvas,maskYinCanvas)];
            }
        }
    }

    maskToCanvas(x,y);
    displayImage();
}


void Canvas2D::mouseDragged(int x, int y) {
    //changes the colors based on moving x,y
    maskToCanvas(x,y);
    displayImage();
}

void Canvas2D::mouseUp(int x, int y) {
    //i do not use mouseUp for brush
    //glWidg->resetHeightMap();

}

//turns x,y coordinates into an index for the 1D canvas array
int Canvas2D::posToIndex(int x, int y){
    return m_width * x + y;
}

//maps the mask x coord to the whole canvas X coord
int Canvas2D::maskXtoCanvasX(int maskIndex, int y){
    return (floor(maskIndex/(2 * settings.brushRadius + 1)) + (y - settings.brushRadius));
}

//maps the mask Y coord to the whole canvas Y coord
int Canvas2D::maskYtoCanvasY(int maskIndex, int x){
    return (maskIndex % (2 * settings.brushRadius + 1) + (x - settings.brushRadius));

}

//creates the opacity mask/main brush mask; filled according to brush type
std::vector<float> Canvas2D::fillMask(int bType, int bRadius){
    std::vector<float> opacityMask(pow(2* bRadius + 1, 2));
    fill(opacityMask.begin(), opacityMask.end() - 1, 0);

    for (int i = 0; i < pow((2* bRadius + 1), 2)-1; i++){
        double indexDistance = sqrt(pow(abs(floor(i/(2 * bRadius + 1) - bRadius)), 2) +
                                    pow(abs(i % (2* bRadius + 1) - bRadius), 2));
        if (indexDistance <= bRadius){

            if(bType == BRUSH_CONSTANT){
                opacityMask[i] = 1;
            }
            else if(bType == BRUSH_LINEAR || bType == BRUSH_SMUDGE){
                opacityMask[i] = fmax(0, 1 - (indexDistance/bRadius));
            }
            else if(bType == BRUSH_QUADRATIC){
                float _a = 1.f/pow(bRadius,2);
                float _b = -2.f/bRadius;
                opacityMask[i] = (_a*pow(indexDistance, 2) + (_b*indexDistance) + 1);
            }
        }
        else{
            opacityMask[i] = 0;
        }
    }

    return opacityMask;
}

//"paints" the canvas; it also keeps track of the previous colors if smudge is selected
void Canvas2D::maskToCanvas(int x, int y){
    Canvas2D::brushColor = settings.brushColor;

    for (int i = 0; i < pow(2* settings.brushRadius + 1, 2); i++){
        if(settings.brushType == BRUSH_SMUDGE){
            Canvas2D::brushColor = Canvas2D::prevColors[i];
        }

        int maskXinCanvas = maskXtoCanvasX(i, y);
        int maskYinCanvas = maskYtoCanvasY(i, x);

        if(maskXinCanvas >= 0 && maskYinCanvas >= 0 && maskXinCanvas <= m_width -1  && maskYinCanvas <= m_width-1){
            float final_a = Canvas2D::brushColor.a/255.f * Canvas2D::filledOpacityVector[i];

            uint8_t final_r = (((final_a * Canvas2D::brushColor.r) + ((1-final_a)* m_data[posToIndex(maskXinCanvas, maskYinCanvas)].r))) + .05f;
            uint8_t final_g = (((final_a * Canvas2D::brushColor.g) + ((1-final_a)* m_data[posToIndex(maskXinCanvas, maskYinCanvas)].g))) +.05f;
            uint8_t final_b = (((final_a * Canvas2D::brushColor.b) + ((1-final_a)* m_data[posToIndex(maskXinCanvas, maskYinCanvas)].b))) +.05f;

            m_data[posToIndex(maskXinCanvas, maskYinCanvas)] = RGBA{final_r, final_g, final_b, 255};
            Canvas2D::prevColors[i] = m_data[posToIndex(maskXinCanvas, maskYinCanvas)];

        }
    }
}

//scales horizontally...the for-loop loops through every col in every row

void Canvas2D::scaleW(std::vector<RGBA> &data, int width, int height, float scaleW){
    std::vector<RGBA> newCanvas =  std::vector<RGBA>(width * scaleW * height);

        for (int row = 0; row < height; row++){
            for (int col = 0; col < floor(width * scaleW); col++){
                newCanvas[col + (row * width * scaleW)] = Filters::backmapping(data, row, col, scaleW, width, height, true);
            }
        }

        //Canvas2D::resize(width * scaleW, height);
        data = newCanvas;
}

//scales vertically...the for-loop loops through every row in every col
void Canvas2D::scaleH(std::vector<RGBA> &data, int width, int height, float scaleH){
    std::vector<RGBA> newCanvas =  std::vector<RGBA>(width * height * scaleH);

        for (int col = 0; col < width; col++){
            for (int row = 0; row < floor(height * scaleH); row++){
                newCanvas[col + (row * width)] = Filters::backmapping(data, row, col, scaleH, width, height, false);
            }
        }

        //Canvas2D::resize(width, height * scaleH);
        data = newCanvas;
}

std::vector<RGBA> Canvas2D::getCanvasData(){
    std::vector<RGBA> newData = m_data;
    Canvas2D::scaleW(newData, 500, 500, 1.f/5);
    Canvas2D::scaleH(newData, 100, 500, 1.f/5);

//    std::vector<RGBA> mirrorData;
//    for (int y = 0; y < 100; y++){
//        for (int x = 0; x < 100; x++){

//            //(heightMapWidth * y) + x
//            mirrorData[(100 * y) + x] = newData[100 - (100 * y) - 1 + x];

//        }
//    }


    return newData;
}











