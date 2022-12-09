#include "filters.h"
#include "settings.h"


/*
 Class for all filter functions (except for resizing the canvas for scale which is done in Canvas2D)
*/

//converts floats to ints for RGBA values
std::uint8_t Filters::floatToUint8(float x) {
    return round(x * 255.f);
}


//gets the wrappedIndex value to manage outofbounds situations
int Filters::getPixelWrappedIndex(int width, int height, int x, int y) {
    int newX = (x < 0) ? x + width  : x % width;
    int newY = (y < 0) ? y + height : y % height;
    return width * newY + newX;
}


//convolve1D performs convolution based on an inputed separted kernel.  It keeps track of direction of movement for indexing
void Filters::convolve1D(std::vector<RGBA> &data, int width, int height, const std::vector<float> &kernel, bool direction) {
    std::vector<RGBA> result = std::vector<RGBA>(data.size());
    int kernelRad = floor(kernel.size() / 2);

    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            size_t centerIndex = r * width + c;

            float redAcc = 0;
            float greenAcc = 0;
            float blueAcc = 0;
            float weightSum = 0;

            for (int i = kernelRad * 2; i >= 0; i--){
                    float weight = kernel[i];
                    weightSum += kernel[i];

                    int XInImage = 0;
                    int YInImage = 0;

                    if (direction == true){
                        //horizontally
                        XInImage = (c - kernelRad) + i;
                        YInImage = r;
                    }
                    else if (direction == false){
                        //vertically
                        XInImage = c;
                        YInImage = (r - kernelRad) + i;
                    }
                    RGBA pixel = data[Filters::getPixelWrappedIndex(width, height, XInImage, YInImage)];

                    redAcc += weight * pixel.r/255.0f;
                    greenAcc += weight * pixel.g/255.0f;
                    blueAcc += weight * pixel.b/255.0f;
                }
            redAcc = redAcc/weightSum;
            greenAcc = greenAcc/weightSum;
            blueAcc = blueAcc/weightSum;

            result[centerIndex] = RGBA{Filters::floatToUint8(redAcc), Filters::floatToUint8(greenAcc), Filters::floatToUint8(blueAcc), Filters::floatToUint8(255)};
            }
        }
    for(int i = 0; i < data.size(); i++){
        data[i] = result[i];
    }

}

//this function creates the 1D triangle kernel based on the blurRad
std::vector<float> Filters::createTriangleFilter(int blurRad){
    std::vector<float> filter =  std::vector<float>(2*blurRad + 1);
    for(int i = 0; i < 2 * blurRad + 1; i++){
        float distance = std::abs(blurRad - i);
        filter[i] = 1 - (distance/blurRad);
    }
    return filter;
}

//grayscales the image values, this is from lab03
std::uint8_t rgbaToGray(const RGBA &pixel) {
    return pixel.r * .299 + pixel.g * .587 + pixel.b * .114;
}

//grayscales the image, this is from lab03
void Filters::filterGray(std::vector<RGBA> &data, int width, int height) {
    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            size_t currentIndex = width * row + col;
            RGBA &currentPixel = data[currentIndex];

            std::uint8_t tempVal = rgbaToGray(currentPixel);

            currentPixel = RGBA{tempVal, tempVal, tempVal};
        }
    }
}

//applies the convolution for G
void Filters::edgeDetect(std::vector<RGBA> &data, int width, int height){
    //take in the float values of r grayscale and then convolve by those
    std::vector<float> Gx ;
    std::vector<float> Gy ;

    std::vector<float> GyKernel{1.0f, 2.0f, 1.0f};
    std::vector<float> GxKernel{-1.0f, 0.0f, 1.0f};

    for(RGBA &color : data){
        Gx.push_back(color.r);
        Gy.push_back(color.r);
    }

    Filters::convolve1DEdge(Gx, width, height, GxKernel, true);
    Filters::convolve1DEdge(Gx, width, height, GyKernel, false);

    Filters::convolve1DEdge(Gy, width, height, GyKernel, true);
    Filters::convolve1DEdge(Gy, width, height, GxKernel, false);

    for(int i = 0; i < data.size(); i ++){
        float Gval = sqrt(pow(Gx[i],2) + pow(Gy[i],2));
        if (Gval > 1.0f){
            Gval = 1.0f;
        }

        Gval = settings.edgeDetectSensitivity * Gval * 255.0f;

        data[i] = RGBA{Filters::floatToUint8(Gval), Filters::floatToUint8(Gval), Filters::floatToUint8(Gval), Filters::floatToUint8(255)};
    }
}


//convolving for edge, differs from the normal convolve because it takes a vector of floats
void Filters::convolve1DEdge(std::vector<float> &data, int width, int height, const std::vector<float> &kernel, bool direction) {
    std::vector<float> result = std::vector<float>(data.size());
    int kernelRad = floor(kernel.size() / 2);

    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            size_t centerIndex = r * width + c;

            float colAcc = 0;

            for (int i = kernelRad * 2; i >= 0; i--){
                    float weight = kernel[i];

                    int XInImage = 0;
                    int YInImage = 0;

                    if (direction == true){
                        //horizontally
                        XInImage = (c - kernelRad) + i;
                        YInImage = r;
                    }
                    else if (direction == false){
                        //vertically
                        XInImage = c;
                        YInImage = (r - kernelRad) + i;
                    }

                    float pixel = data[Filters::getPixelWrappedIndex(width, height, XInImage, YInImage)];
                    colAcc += weight * pixel / 255.0f;
                }
            result[centerIndex] = colAcc;

            }
        }
    for(int i = 0; i < data.size(); i++){
        data[i] = result[i];
    }

}

//gets the triangle filter weight for scaling
float Filters::g(float x, float a){
    float radius;
    if(a < 1){
        radius = 1.0 / a;
    } else {
        radius = 1.0;
    }

    if ((x < -radius || (x > radius))) {
        return 0;
    }
    else {
        return (1 - fabs(x) / radius) / radius ;
    }
}


//backmaps to the input pixel value that the output pixel lands on/needs to be set to
RGBA Filters::backmapping(std::vector<RGBA> &data, int row, int col, float scaleFactor, int m_width, int m_height, bool isHoriz){
    float redAcc = 0;
    float greenAcc = 0;
    float blueAcc = 0;

    float weights_sum = 0;
    float center;

    int bounds;

    //if isHoriz(ontal) is true, we index based off that directional movement
    if(isHoriz){
        center = (col /scaleFactor) + (1-scaleFactor)/(2*scaleFactor);
        bounds = m_width;

    } else if (!isHoriz){
        // if false, we are moving vertically
        center = (row /scaleFactor) + (1-scaleFactor)/(2*scaleFactor);
        bounds = m_height;
    }

    float radius = (scaleFactor > 1)? 1.0f : 1.0f / scaleFactor;

    int left = ceil(center - radius);
    int right = floor(center + radius);


    for (int i = left; i <= right; i++) {

        if((i >= 0) && (i < bounds)){

            float filter = Filters::g(i - center, scaleFactor);

            RGBA pixel;

            if(isHoriz){
                pixel = data[getPixelWrappedIndex(m_width, m_height, i, row)];

            } else if (!isHoriz){
                pixel = data[getPixelWrappedIndex(m_width, m_height, col, i)];
            }

            redAcc += filter * pixel.r/255.0f;
            greenAcc += filter * pixel.g/255.0f;
            blueAcc += filter * pixel.b/255.0f;

            weights_sum += filter;

        } else {
            redAcc += 0.0f;
            greenAcc += 0.0f;
            blueAcc += 0.0f;

            weights_sum += 0.0f;
        }
    }

    redAcc = redAcc/weights_sum;
    greenAcc = greenAcc/weights_sum;
    blueAcc = blueAcc/weights_sum;

    return RGBA{floatToUint8(redAcc), floatToUint8(greenAcc), floatToUint8(blueAcc),  floatToUint8(255)};
}



