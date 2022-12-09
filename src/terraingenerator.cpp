#include "terraingenerator.h"

#include <cmath>
#include <iostream>
#include "glm/glm.hpp"
#include "qimage.h"
#include "src/RGBA.h"

// Constructor
TerrainGenerator::TerrainGenerator()
{
  // Task 8: turn off wireframe shading
  m_wireshade = true; // STENCIL CODE
  // m_wireshade = false; // TA SOLUTION

  // Define resolution of terrain generation
  m_resolution = 100;

//  // Generate random vector lookup table
//  m_lookupSize = 1024;
//  m_randVecLookup.reserve(m_lookupSize);

//  // Initialize random number generator
//  std::srand(1230);

//  // Populate random vector lookup table
//  for (int i = 0; i < m_lookupSize; i++)
//  {
//    m_randVecLookup.push_back(glm::vec2(std::rand() * 2.0 / RAND_MAX - 1.0,
//                                        std::rand() * 2.0 / RAND_MAX - 1.0));
//    }
}

// Destructor
TerrainGenerator::~TerrainGenerator()
{
    m_randVecLookup.clear();
}

// Helper for generateTerrain()
void addPointToVector(glm::vec3 point, std::vector<float>& vector) {
    vector.push_back(point.x);
    vector.push_back(point.y);
    vector.push_back(point.z);
}

//helper to set vertex array
std::vector<float>  TerrainGenerator::newHeightMap(std::vector<RGBA> newHeightMapInfo){
    //going through triangle by triangle not pixel by pixel


//    //clear current vertex...
//    verts = std::vector<float>(0);
    heightInfo.clear();
//    for(RGBA& color: newHeightMapInfo){
//        heightInfo.push_back(color.r);
//    }

//    //set new height info
//    //heightInfo = newHeightMapInfo;
//    heightMapWidth = 500;
//    heightMapHeight = 500;
    isNotClear = false;
    return TerrainGenerator::generateTerrain();
}

// Generates the geometry of the output triangle mesh
std::vector<float> TerrainGenerator::generateTerrain() {
    TerrainGenerator::loadImageFromFile("resources/here.png", isNotClear);
    //verts = std::vector<float>(0);
    verts.reserve(m_resolution * m_resolution * 6);

    for(int x = 0; x < m_resolution; x++) {
        for(int y = 0; y < m_resolution; y++) {

            int x1 = x;
            int y1 = y;

            int x2 = x + 1;
            int y2 = y + 1;

            glm::vec3 p1 = getPosition(x1,y1);
            glm::vec3 p2 = getPosition(x2,y1);
            glm::vec3 p3 = getPosition(x2,y2);
            glm::vec3 p4 = getPosition(x1,y2);

            glm::vec3 n1 = getNormal(x1,y1);
            glm::vec3 n2 = getNormal(x2,y1);
            glm::vec3 n3 = getNormal(x2,y2);
            glm::vec3 n4 = getNormal(x1,y2);

            // tris 1
            // x1y1z1
            // x2y1z2
            // x2y2z3
            addPointToVector(p1, verts);
            addPointToVector(n1, verts);
            addPointToVector(getColor(n1, p1), verts);

            addPointToVector(p2, verts);
            addPointToVector(n2, verts);
            addPointToVector(getColor(n2, p2), verts);

            addPointToVector(p3, verts);
            addPointToVector(n3, verts);
            addPointToVector(getColor(n3, p3), verts);

            // tris 2
            // x1y1z1
            // x2y2z3
            // x1y2z4
            addPointToVector(p1, verts);
            addPointToVector(n1, verts);
            addPointToVector(getColor(n1, p1), verts);

            addPointToVector(p3, verts);
            addPointToVector(n3, verts);
            addPointToVector(getColor(n3, p3), verts);

            addPointToVector(p4, verts);
            addPointToVector(n4, verts);
            addPointToVector(getColor(n4, p4), verts);
        }
    }
    return verts;
}

// Samples the (infinite) random vector grid at (row, col)
glm::vec2 TerrainGenerator::sampleRandomVector(int row, int col)
{
    std::hash<int> intHash;
    int index = intHash(row * 41 + col * 43) % m_lookupSize;
    return m_randVecLookup.at(index);
}

// Takes a grid coordinate (row, col), [0, m_resolution), which describes a vertex in a plane mesh
// Returns a normalized position (x, y, z); x and y in range from [0, 1), and z is obtained from getHeight()
glm::vec3 TerrainGenerator::getPosition(int row, int col) {
    // Normalizing the planar coordinates to a unit square 
    // makes scaling independent of sampling resolution.
    float x = 1.0 * row / m_resolution;
    float y = 1.0 * col / m_resolution;
    float z = getHeight(x, y);

    return glm::vec3(x,y,z);
}

// ================== Students, please focus on the code below this point

///parse in an image of rgba.....
/// store as rgba vector and then compute height based off of the
/// rbga value stored at that location
/// white would be 0, black is 1
///
void TerrainGenerator::loadImageFromFile(const std::string &file, bool isnotClear) {
    std::vector<float> texture_data;

    QString inputFile = QString::fromStdString(file);
    QImage myImage;
    if (!myImage.load(inputFile)) {
        std::cout<<"Failed to load in image"<<std::endl;
    }
    myImage = myImage.convertToFormat(QImage::Format_RGBX8888);
    int texture_W = myImage.width();
    int texture_H = myImage.height();

    QByteArray arr = QByteArray::fromRawData((const char*) myImage.bits(), myImage.sizeInBytes());

    texture_data.clear();
    texture_data.reserve(texture_W * texture_H);

    for (int i = 0; i < arr.size() / 4.f; i++){
        //if(isNotClear){
            texture_data.push_back((std::uint8_t) arr[4*i] / 255.f); //get RGBA value between 0.0
        //}else{
            //texture_data.push_back(0.0); //get RGBA value between 0.0

        //}
    }
    std::cout << "Loaded heightmap of size " << texture_H << " x " << texture_W << std::endl;

    heightInfo = texture_data;
    heightMapWidth = 100;
    heightMapHeight = texture_H;
}


// Helper for computePerlin() and, possibly, getColor()
float interpolate(float A, float B, float alpha) {
    // Task 4: implement your easing/interpolation function below
    float easeValue = 3 * pow(alpha, 2) - 2 * pow(alpha, 3);

    return A + easeValue * (B - A);
}

// Takes a normalized (x, y) position, in range [0,1)
// Returns a height value, z, by sampling a noise function
float TerrainGenerator::getHeight(float x, float y) {

   float z = heightInfo[heightMapWidth * y + x]/10;
    //float z = abs(x) + abs(y);

    return z ;
}

// Computes the normal of a vertex by averaging neighbors
glm::vec3 TerrainGenerator::getNormal(int row, int col) {
    // Task 9: Compute the average normal for the given input indices
    //8 neighbors
    glm::vec3 normal = glm::vec3(0, 0, 0);
    std::vector<std::vector<int>> neighborOffsets = { // Counter-clockwise around the vertex
     {-1, -1},
     { 0, -1},
     { 1, -1},
     { 1,  0},
     { 1,  1},
     { 0,  1},
     {-1,  1},
     {-1,  0}
    };
    glm::vec3 V = getPosition(row,col);
    for (int i = 0; i < 8; ++i) {
     int n1RowOffset = neighborOffsets[i][0];
     int n1ColOffset = neighborOffsets[i][1];
     int n2RowOffset = neighborOffsets[(i + 1) % 8][0];
     int n2ColOffset = neighborOffsets[(i + 1) % 8][1];
     glm::vec3 n1 = getPosition(row + n1RowOffset, col + n1ColOffset);
     glm::vec3 n2 = getPosition(row + n2RowOffset, col + n2ColOffset);
     normal = normal + glm::cross(n1 - V, n2 - V);
    }
    return glm::normalize(normal);
}

// Computes color of vertex using normal and, optionally, position
glm::vec3 TerrainGenerator::getColor(glm::vec3 normal, glm::vec3 position) {
    // Task 10: compute color as a function of the normal and position
    //between 0,1.. because normals are NORMALIZED DUH
//    if(position.z > .1){
//        return glm::vec3(1,1,1);
//    }
//    else {
//        return glm::vec3(.5,.5,.5);
//    }

//    if(glm::dot(normal, position) < .0001){
//        return glm::vec3(1,1,1);
//    }
//    else {
//        return glm::vec3(.5,.5,.5);
//    }

    if(glm::dot(normal, position) < .0001 && position.z > .1){
        return glm::vec3(1,1,1);
    }
    else {
        return glm::vec3(.5,.5,.5);
    }

    // Return white as placeholder
    //return glm::vec3(1,1,1);
}

//// Computes the intensity of Perlin noise at some point
//float TerrainGenerator::computePerlin(float x, float y) {
//    // Task 1: get grid indices (as ints)

//    //round down float x
//    int indexPt_1 = floor(x);
//    //round down float x and add 1
//    int indexPt_2 = indexPt_1 + 1;
//    //round down float y
//    int indexPt_3 = floor(y);
//    //round down float y and add 1
//    int indexPt_4 = indexPt_3 + 1;

//    glm::vec3 gridIndex1 = glm::vec3(indexPt_1,indexPt_3, 1);
//    glm::vec3 gridIndex2 = glm::vec3(indexPt_2,indexPt_3, 1);
//    glm::vec3 gridIndex3 = glm::vec3(indexPt_1,indexPt_4, 1);
//    glm::vec3 gridIndex4 = glm::vec3(indexPt_2,indexPt_4, 1);

//    glm::vec2 interestPt = glm::vec2(x, y);

//    // Task 2: compute offset vectors
//    glm::vec2 offsetVector1 = glm::vec2(interestPt.x - gridIndex1.x, interestPt.y - gridIndex1.y);
//    glm::vec2 offsetVector2 = glm::vec2(interestPt.x - gridIndex2.x, interestPt.y - gridIndex2.y);
//    glm::vec2 offsetVector3 = glm::vec2(interestPt.x - gridIndex3.x, interestPt.y - gridIndex3.y);
//    glm::vec2 offsetVector4 = glm::vec2(interestPt.x - gridIndex4.x, interestPt.y - gridIndex4.y);

//    // Task 3: compute the dot product between offset and grid vectors
//    float dotProd_1 = glm::dot(sampleRandomVector(gridIndex1.x, gridIndex1.y), offsetVector1);
//    float dotProd_2 = glm::dot(sampleRandomVector(gridIndex2.x, gridIndex2.y), offsetVector2);
//    float dotProd_3 = glm::dot(sampleRandomVector(gridIndex3.x, gridIndex3.y), offsetVector3);
//    float dotProd_4 = glm::dot(sampleRandomVector(gridIndex4.x, gridIndex4.y), offsetVector4);

//    // Task 5: use your interpolation function to produce the final value
//    //g pos - index point in grid

//    float dx = interestPt.x - gridIndex1.x;
//    float intermediate_G = interpolate(dotProd_1, dotProd_2, dx);
//    float intermediate_H = interpolate(dotProd_3, dotProd_4, dx);

//    float dy = interestPt.y - indexPt_3;
//    float result = interpolate(intermediate_G, intermediate_H, dy);

//    // Return 0 as a placeholder
//    return result;
//}
