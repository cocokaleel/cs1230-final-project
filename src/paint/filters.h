#ifndef FILTERS_H
#define FILTERS_H


#include "src/RGBA.h"
#include <QLabel>
#include <QMouseEvent>
#include <array>

class Filters
{
public:
    static void convolve1D(std::vector<RGBA>& data,
                    int width,
                    int height,
                    const std::vector<float>& kernel,
                    bool direction);
    static std::uint8_t floatToUint8(float x);
    static int getPixelWrappedIndex(int width, int height, int x, int y);
    static std::vector<float> createTriangleFilter(int blurRad);
    static void filterGray(std::vector<RGBA> &data,int width, int height);
    static void edgeDetect(std::vector<RGBA> &data, int width, int height);
    static void convolve1DEdge(std::vector<float> &data, int width, int height, const std::vector<float> &kernel, bool direction);


    static RGBA backmapping(std::vector<RGBA> &data, int row, int col, float scaleFactor, int m_width, int m_height, bool isHoriz);

    static float g(float x, float a);
};

#endif // FILTERS_H
