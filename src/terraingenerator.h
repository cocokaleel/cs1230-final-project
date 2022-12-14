#pragma once

#include <string>
#include <vector>
#include "glm/glm.hpp"
#include "src/RGBA.h"

#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>

#include <chrono>

class TerrainGenerator
{







public:
    typedef std::chrono::milliseconds chrono;
//    auto start;

    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames

    void tick(QTimerEvent* event);                      // Called once per tick of m_timer

    bool m_wireshade;

    TerrainGenerator();
    ~TerrainGenerator();
    int getResolution() { return m_resolution; };
    std::vector<float> generateTerrain();

    std::vector<float> newHeightMap(std::vector<RGBA> newHeightMapInfo);

    std::vector<float> clearHeightMap();

private:


    // Member variables for terrain generation. You will not need to use these directly.
    std::vector<glm::vec2> m_randVecLookup;
    int m_resolution;
    int m_lookupSize;

    std::vector<float> verts;

    bool isResetTerrain = false;

    std::vector<float> heightInfo;
    int heightMapWidth;
    int heightMapHeight;

    // Samples the (infinite) random vector grid at (row, col)
    glm::vec2 sampleRandomVector(int row, int col);

    // Takes a grid coordinate (row, col), [0, m_resolution), which describes a vertex in a plane mesh
    // Returns a normalized position (x, y, z); x and y in range from [0, 1), and z is obtained from getHeight()
    glm::vec3 getPosition(int row, int col);
    glm::vec3 getRipple(int row, int col, auto clock);

    // ================== Students, please focus on the code below this point

    // Takes a normalized (x, y) position, in range [0,1)
    // Returns a height value, z, by sampling a noise function
    float getHeight(float x, float y);

    // Computes the normal of a vertex by averaging neighbors
    glm::vec3 getNormal(int row, int col);

    // Computes color of vertex using normal and, optionally, position
    glm::vec3 getColor(glm::vec3 normal, glm::vec3 position);

    // Computes the intensity of Perlin noise at some point
    float computePerlin(float x, float y);

    void loadImageFromFile(const std::string &file);

};
