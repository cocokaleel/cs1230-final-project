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
  m_wireshade = false; // STENCIL CODE
  // m_wireshade = false; // TA SOLUTION

  // Define resolution of terrain generation
  m_resolution = 100;

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

//add points and normals and colors to verts
void TerrainGenerator::addToVerts(){
    for(int x = 0; x < m_resolution - 1; x++) {
        for(int y = 0; y < m_resolution - 1; y++) {
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



}

//helper to set vertex array
//Pass in scaled down image
std::vector<float>  TerrainGenerator::newHeightMap(std::vector<RGBA> newHeightMapInfo){
    //clear current vertex...
    verts.clear();
    heightInfo.clear();

    //how to access in reverse lol
    for(RGBA& color: newHeightMapInfo){
        heightInfo.push_back(color.r / 255.f);//check here if incorrect height
    }
    //set new height info

    std::cout<< heightInfo[5]<< std::endl;

    heightMapWidth = 100;
    heightMapHeight = 100;
    isResetTerrain = false;

    TerrainGenerator::addToVerts();

    return verts;
}

std::vector<float> TerrainGenerator::clearHeightMap(){
    verts.clear();
    isResetTerrain = true;

    TerrainGenerator::addToVerts();

    return verts;
}



// Generates the geometry of the output triangle mesh
std::vector<float> TerrainGenerator::generateTerrain() {
    isResetTerrain = true;

    verts.reserve(m_resolution * m_resolution * 6);
    TerrainGenerator::addToVerts();

    return verts;
}


// Takes a grid coordinate (row, col), [0, m_resolution), which describes a vertex in a plane mesh
// Returns a normalized position (x, y, z); x and y in range from [0, 1), and z is obtained from getHeight()
glm::vec3 TerrainGenerator::getPosition(int row, int col) {
    // Normalizing the planar coordinates to a unit square 
    // makes scaling independent of sampling resolution.
    float x = 1.f * row / m_resolution;
    float y = 1.f * col / m_resolution;
    float z = 0.f;

    if(!isResetTerrain){
        z = getHeight(row, col);
    } else {
        z = (122.f/255)/4;
    }

    return glm::vec3(x,y,z);
}

// ================== Students, please focus on the code below this point

///parse in an image of rgba.....
/// store as rgba vector and then compute height based off of the
/// rbga value stored at that location
/// white would be 0, black is 1
///
void TerrainGenerator::loadImageFromFile(const std::string &file) {
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
        texture_data.push_back((std::uint8_t) arr[4*i] / 255.f); //get RGBA value between 0.0
    }
    std::cout << "Loaded heightmap of size " << texture_H << " x " << texture_W << std::endl;

    heightInfo = texture_data;
    heightMapWidth = texture_W;
    heightMapHeight = texture_H;
}


// Takes a normalized (x, y) position, in range [0,1)
// Returns a height value, z, by sampling a noise function
float TerrainGenerator::getHeight(float x, float y) {

   float z = heightInfo[(heightMapWidth * y) + x]/4;
    return z ;
}

// Computes the normal of a vertex by averaging neighbors
glm::vec3 TerrainGenerator::getNormal(int row, int col) {
    // Task 9: Compute the average normal for the given input indices
    //8 neighbors
    glm::vec3 normal = glm::vec3(0, 0, 0);
    std::vector<std::vector<int>> neighborOffsets = { //around the vertex
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

//    if (glm::dot(normal, position) < .0001){
//         return glm::vec3(.5,.2,0.f);
//    }
//    else if(position.z > .2f){
//        return glm::vec3(.5,.2,1);
//    }
//    else {
//        return glm::vec3(.5,.5,.5);
//    }
    return glm::vec3(.5,.5,.5);

    // Return white as placeholder
    //return glm::vec3(1,1,1);
}
