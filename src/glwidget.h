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


    std::vector<GLfloat> verts;
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
    glm::mat4 viewMatrix;

    glm::vec4 cameraPositionWorld;
    float heightAngle = 30 * M_PI / 180.f;

    //uniform lighting values ripped from recursiveSpheres3.xml
    // Global lighting uniforms
     float ka = 0.5; //ambient coefficient
     float kd = 0.7; //diffuse coefficient
     float ks = 0.54; //specular coefficient

     // holds the number of lights that exist in the scene (for looping purposes)
     int numLights = 3;

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

     LightColorPos lights[3] = {
         { //point light
             glm::vec4(1.f),//color;
             glm::vec4(0.f), //dir //only for directional light
             glm::vec4(10, 10, 10, 1.f),//pos; //only for point light and spot light
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
    ShapeData shapes[1] = {
        {
            glm::mat4( 1.0f ),//glm::mat4 ctm;
            0,//int type; //0 is sphere
            glm::vec4(0.f),//glm::vec4 cAmbient;
            glm::vec4(0.75, 1, 0.75, 1),//glm::vec4 cDiffuse;
            glm::vec4(1.f),//glm::vec4 cSpecular;
            25,//float shininess;
            glm::vec4(0.75, 1, 0.75, 1)//glm::vec4 cReflective;
        }
    };
    int numShapes = 1;
    void paintGeometryPhong();
};
