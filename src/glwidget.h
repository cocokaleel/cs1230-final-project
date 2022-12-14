#pragma once

#include "GL/glew.h"
#include <QOpenGLWidget>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include "terraingenerator.h"
#include <QMatrix4x4>

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

class GLWidget : public QOpenGLWidget
{
public:
    GLWidget(QWidget *parent = nullptr);

    void finish();
    void resetHeightMap();
    void useNewHeightMap(std::vector<RGBA> canvasData);

    ~GLWidget();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;

private:
    void rebuildMatrices();
    //void resetVBO(QOpenGLBuffer& vbo, std::vector<GLfloat>vertexData);


    std::vector<std::vector<float>> verts;
    std::vector<GLfloat> fullscreen_quad_verts =
        { //     POSITIONS    //
            -1.f,  1.f, 0.0f, //position
                  0.f, 1.f, //uv
            -1.f, -1.0f, 0.0f,
                  0.f, 0.f,
             1.f, -1.0f, 0.0f,
                  1.f, 0.f,
             1.f,  1.f, 0.0f,
                  1.f, 1.f,
            -1.f,  1.f, 0.0f,
                  0.f, 1.f,
             1.f, -1.f, 0.0f,
                  1.f, 0.f,
        };
    bool isClearing = false;

    int m_xRot = 0;
    int m_yRot = 0;
    int m_zRot = 0;

    QOpenGLShaderProgram *m_program = nullptr;
    QOpenGLVertexArrayObject m_fullscreenQuadVao;
    QOpenGLBuffer m_fullscreenQuadVbo;

    QMatrix4x4 m_proj;
    QMatrix4x4 m_camera;
    QMatrix4x4 m_world;
    TerrainGenerator m_terrain;

    int m_projMatrixLoc = 0;
    int m_mvMatrixLoc = 0;

    QPoint m_prevMousePos;
    float m_angleX;
    float m_angleY;
    float m_zoom;

    //UNIFORMS FOR PHONG AND RAYTRACING
    //TODO PUT THESE IN A SEPARATE FILE??
    //GLOBAL SCENE DATA: SHAPES, LIGHTS, AND CONSTANTS
    // Camera info
    glm::vec3 look;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::mat4 cameraRotMat;
    glm::mat4 cameraTransMat;
    glm::vec4 cameraPosition;

    struct cameraInfo {
        glm::vec4 pos;
        glm::vec4 look;
        glm::vec4 up;

        float heightAngle;
    };
    //camera from unit_sphere
    cameraInfo camera = {
        glm::vec4(3, 3, 3, 1),
        glm::vec4(glm::vec4(0,0,0,1)-glm::vec4(3,3,3,1)),
        glm::vec4(0,1,0,0),
        30 * M_PI / 180.f
    };

    void setupViewAndCamera();

    //uniform lighting values ripped from recursiveSpheres3.xml
    // Global lighting uniforms
     float ka = 0.5; //ambient coefficient
     float kd = 0.7; //diffuse coefficient
     float ks = 0.54; //specular coefficient

     // holds the number of lights that exist in the scene (for looping purposes)
     int numLights = 4;

     //create a struct for holding light information
     struct LightColorPos {
         glm::vec4 color;
         glm::vec4 dir; //only for directional light
         glm::vec4 pos; //only for point light and spot light
         int lightType; //0 is directional, 1 is point, 2 is for spot
         glm::vec3 attenuation;
         float angle; //only for spot lights
         float penumbra; //only for spot lights
     };
    //from unit sphere
     LightColorPos lights[4] = {
         { //point light
             glm::vec4(1.f),//color;
             glm::vec4(-3,-2,-1,0), //dir //only for directional light
             glm::vec4(0),//pos; //only for point light and spot light
             0,//lightType; //0 is directional, 1 is point, 2 is for spot
             glm::vec3(0),//attenuation;
             0,//angle; //only for spot lights
             0//penumbra;
         },
//         { //point light
//             glm::vec4(1.f),//color;
//             glm::vec4(0.f), //dir //only for directional light
//             glm::vec4(10, 10, 10, 1.f),//pos; //only for point light and spot light
//             1,//lightType; //0 is directional, 1 is point, 2 is for spot
//             glm::vec3(1.5, 0, 0),//attenuation;
//             0,//angle; //only for spot lights
//             0//penumbra;
//         },
         { //point light
             glm::vec4(1.f),//color;
             glm::vec4(0.f), //dir //only for directional light
             glm::vec4(1, 10, -10, 1.f),//pos; //only for point light and spot light
             1,//lightType; //0 is directional, 1 is point, 2 is for spot
             glm::vec3(1.5, 0, 0),//attenuation;
             0,//angle; //only for spot lights
             0//penumbra;
         },
         {//directional light 1
              glm::vec4(1.f),//color;
              glm::vec4(0.25, 1, -1, 0), //dir //only for directional light
              glm::vec4(0),//pos; //only for point light and spot light
              0,//lightType; //0 is directional, 1 is point, 2 is for spot
              glm::vec3(0),//attenuation;
              0,//angle; //only for spot lights
              0//penumbra;
         },
         {//directional light 2
              glm::vec4(1.f),//color;
              glm::vec4(1, -1.8, -2, 0), //dir //only for directional light
              glm::vec4(0),//pos; //only for point light and spot light
              0,//lightType; //0 is directional, 1 is point, 2 is for spot
              glm::vec3(0),//attenuation;
              0,//angle; //only for spot lights
              0//penumbra;
         }
     };

//made up shapes to test raytracing
    struct ShapeData {
        glm::mat4 ctm;
        int type; //0 is sphere
        glm::vec4 cAmbient;
        glm::vec4 cDiffuse;
        glm::vec4 cSpecular;
        float shininess;
        glm::vec4 cReflective;
    };
    struct TriangleData {
        glm::vec3 points[3];
        glm::vec3 normals[3];
        glm::vec4 cAmbient;
        glm::vec4 cDiffuse;
        glm::vec4 cSpecular;
        float shininess;
        glm::vec4 cReflective;
    };
    //FAKE BASIC TRIANGLE INFO
    TriangleData triangle1 = {
        {glm::vec3(-0.5, -0.5, 1), glm::vec3(-0.5, 0.5, 0.5), glm::vec3(-0.5, -0.5, 0)}, //points
        {glm::vec3(0,0,1),glm::vec3(0,0,1),glm::vec3(0,0,1)},//normals
        glm::vec4(0.1f),//glm::vec4 cAmbient;
        glm::vec4(0.9,0.1,0.1, 1.f),//glm::vec4 cDiffuse;
        glm::vec4(1.f),//glm::vec4 cSpecular;
        25,//float shininess;
        glm::vec4(0.75, 1, 0.75, 1)
    };
    TriangleData triangle2 = {
        {glm::vec3(-0.5, -0.5, 0), glm::vec3(0, 0, 0), glm::vec3(0.5, -0.5, 0)}, //points
        {glm::vec3(0,0,1),glm::vec3(0,0,1),glm::vec3(0,0,1)},//normals
        glm::vec4(0.1f),//glm::vec4 cAmbient;
        glm::vec4(0.1,0.9,0.1, 1.f),//glm::vec4 cDiffuse;
        glm::vec4(1.f),//glm::vec4 cSpecular;
        25,//float shininess;
        glm::vec4(0.75, 1, 0.75, 1)
    };
    TriangleData triangle3 = {
        {glm::vec3(0.5, -0.5, 0), glm::vec3(1, 0.5, 0), glm::vec3(1.5, -0.5, 0)}, //points
        {glm::vec3(0,0,1),glm::vec3(0,0,1),glm::vec3(0,0,1)},//normals
        glm::vec4(0.1f),//glm::vec4 cAmbient;
        glm::vec4(0.1,0.1,0.9f, 1.f),//glm::vec4 cDiffuse;
        glm::vec4(1.f),//glm::vec4 cSpecular;
        25,//float shininess;
        glm::vec4(0.75, 1, 0.75, 1)
    };

    TriangleData triangle4 = {
        {glm::vec3(-0.5, -0.5, -1), glm::vec3(0, 0.5, -1), glm::vec3(0.5, -0.5, -1)}, //points
        {glm::vec3(0,0,1),glm::vec3(0,0,1),glm::vec3(0,0,1)},//normals
        glm::vec4(0.1f),//glm::vec4 cAmbient;
        glm::vec4(0.1,0.9,0.1, 1.f),//glm::vec4 cDiffuse;
        glm::vec4(1.f),//glm::vec4 cSpecular;
        25,//float shininess;
        glm::vec4(0.75, 1, 0.75, 1)
    };
    TriangleData triangles[4] = {triangle1, triangle2, triangle3, triangle4};
    std::vector<TriangleData> vertTriangles;

    void paintGeometryPhong();
};
