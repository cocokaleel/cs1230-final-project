#include "glwidget.h"
#include "terraingenerator.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLVersionFunctionsFactory>
#include <QOpenGLFunctions_3_1>
#include <QMouseEvent>
#include <QSurfaceFormat>
#include <QDir>
#include <iostream>
#include "glm/gtc/matrix_transform.hpp"

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent), m_angleX(0), m_angleY(0), m_zoom(1.0)
{}

GLWidget::~GLWidget() {}

void GLWidget::initializeGL()
{
    // GLEW is a library which provides an implementation for the OpenGL API
    // Here, we are setting it up
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) fprintf(stderr, "Error while initializing GLEW: %s\n", glewGetErrorString(err));
    fprintf(stdout, "Successfully initialized GLEW %s\n", glewGetString(GLEW_VERSION));

    glClearColor(0, 0, 0, 1);
    m_program = new QOpenGLShaderProgram;
    std::cout << QDir::currentPath().toStdString() << std::endl;
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex,"resources/shader/vertex.vert");
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment,"resources/shader/fragment.frag");
    m_program->link();
    m_program->bind();

    m_projMatrixLoc = m_program->uniformLocation("projMatrix");
    m_mvMatrixLoc = m_program->uniformLocation("mvMatrix");

    m_fullscreenQuadVao.create();
    m_fullscreenQuadVao.bind();

    verts = m_terrain.generateTerrain();
    m_fullscreenQuadVbo.create();
    m_fullscreenQuadVbo.bind();
    m_fullscreenQuadVbo.allocate(fullscreen_quad_verts.data(),fullscreen_quad_verts.size()*sizeof(GLfloat));


    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                             nullptr);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                             reinterpret_cast<void *>(3 * sizeof(GLfloat)));

    m_fullscreenQuadVbo.release();


    m_world.setToIdentity();
    m_world.translate(QVector3D(-0.5,-0.5,0));


    m_camera.setToIdentity();

    m_camera.lookAt(QVector3D(1,1,1),QVector3D(0,0,0),QVector3D(0,0,1));

    m_program->release();
}


//reset vertex map//should change the value for future too???
void GLWidget::resetHeightMap(){
    verts = m_terrain.clearHeightMap();
    update();
};

void GLWidget::useNewHeightMap(std::vector<RGBA> canvasData){
    verts = m_terrain.newHeightMap(canvasData);
    update();
}

void GLWidget::finish(){
    this->makeCurrent();
    m_fullscreenQuadVbo.destroy();
    m_fullscreenQuadVao.destroy();
    delete m_program;
    m_program = nullptr;
    this->doneCurrent();
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    m_program->bind();
//    m_program->setUniformValue(m_projMatrixLoc, m_proj);
//    m_program->setUniformValue(m_mvMatrixLoc, m_camera * m_world);
//    m_program->setUniformValue(m_program->uniformLocation("wireshade"),m_terrain.m_wireshade);
    paintGeometryPhong();

//    glPolygonMode(GL_FRONT_AND_BACK,m_terrain.m_wireshade? GL_LINE : GL_FILL);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    m_program->release();
}

//create the geometry with Phong lighting
void GLWidget::paintGeometryPhong() {
    // Clear screen color and depth before painting
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Bind full-screen quad
    m_fullscreenQuadVao.bind();

    glUniform4fv(glGetUniformLocation(m_shader, "cameraPositionWorld"), 1, &cameraPosition[0]);


    //pass in the height angle and width angle
    float widthAngle = heightAngle * float(width()) / float(height());
    int widthAngle_loc = m_program->uniformLocation("widthAngle");
    int heightAngle_loc = m_program->uniformLocation("heightAngle");
    m_program->setUniformValue(widthAngle_loc, widthAngle);
    m_program->setUniformValue(heightAngle_loc, heightAngle);

    // Pass in m_view and m_proj
    //TODO is m_camera * m_world the view matrix??? I feel like no
    QMatrix4x4 viewMatrix = m_camera*m_world;
    int viewMatrix_loc = m_program->uniformLocation("viewMatrix");
    int viewMatrixInv_loc = m_program->uniformLocation("viewMatrixInverse");
    m_program->setUniformValue(viewMatrix_loc, viewMatrix);
    m_program->setUniformValue(viewMatrixInv_loc, QMatrix4x4::inverted(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "viewMatrix"), 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "viewMatrixInverse"), 1, GL_FALSE, &glm::inverse(viewMatrix)[0][0]);

    //pass in ka, ks, and kd (global coefficients for ambient, specular, and diffuse lighting)
    int ka_loc = m_program->uniformLocation("ka");
    int ks_loc = m_program->uniformLocation("ks");
    int kd_loc = m_program->uniformLocation("kd");
    m_program->setUniformValue(ka_loc, ka);
    m_program->setUniformValue(kd_loc, kd);
    m_program->setUniformValue(ks_loc, ks);


    int lightCounter = 0;
    //Pass in light positional, directional, and color data into the fragment (and technically vertex) shader as uniforms
    for (LightColorPos light : lights) {
        GLint dir = m_program->uniformLocation(("lights[" + std::to_string(lightCounter) + "].dir").c_str());
        glUniform4fv(dir, 1, &light.dir[0]);
        GLint color = m_program->uniformLocation(("lights[" + std::to_string(lightCounter) + "].color").c_str());
        glUniform4fv(color, 1, &light.color[0]);
        GLint pos = m_program->uniformLocation(("lights[" + std::to_string(lightCounter) + "].pos").c_str());
        glUniform4fv(pos, 1, &light.pos[0]);
        GLint attenuationFunction = m_program->uniformLocation(("lights[" + std::to_string(lightCounter) + "].attenuation").c_str());
        glUniform3fv(attenuationFunction, 1, &light.attenuation[0]);
        GLint lightType = m_program->uniformLocation(("lights[" + std::to_string(lightCounter) + "].lightType").data());
        //pass in light type corresponding to the type of light
        //TODO make this an enum in frag shader??
        if (light.lightType == 0) {
            //DIRECTIONAL LIGHT
            glUniform1i(lightType, 0);
        } else if (light.lightType == 1) {
            //POINT LIGHT
            glUniform1i(lightType, 1);
        } else {
            //SPOTLIGHT
            glUniform1i(lightType, 2);

            GLint angle = m_program->uniformLocation(("lights[" + std::to_string(lightCounter) + "].angle").data());
            glUniform1f(angle, light.angle);

            GLint penumbra = m_program->uniformLocation(("lights[" + std::to_string(lightCounter) + "].penumbra").data());
            glUniform1f(penumbra, light.penumbra);
        }
        lightCounter++;
    }

    int numLights_loc = m_program->uniformLocation("numLights");
    m_program->setUniformValue(numLights_loc, numLights);

    int shapeCounter = 0;
    for (ShapeData shape : shapes) {

        //pass in all the shape-specific information to the first full-screen quad shader
        GLint matrix = m_program->uniformLocation(("shapes[" + std::to_string(shapeCounter) + "].ctm").c_str());
        GLint matrixInv = m_program->uniformLocation(("shapes[" + std::to_string(shapeCounter) + "].ctmInv").c_str());
        GLint normalMat = m_program->uniformLocation(("shapes[" + std::to_string(shapeCounter) + "].normalMat").c_str());
        GLint type = m_program->uniformLocation(("shapes[" + std::to_string(shapeCounter) + "].type").c_str());
        GLint cAmbient = m_program->uniformLocation(("shapes[" + std::to_string(shapeCounter) + "].cAmbient").c_str());
        GLint cDiffuse = m_program->uniformLocation(("shapes[" + std::to_string(shapeCounter) + "].cDiffuse").c_str());
        GLint cSpecular = m_program->uniformLocation(("shapes[" + std::to_string(shapeCounter) + "].cSpecular").c_str());
        GLint shininess = m_program->uniformLocation(("shapes[" + std::to_string(shapeCounter) + "].shininess").c_str());
        GLint cReflective = m_program->uniformLocation(("shapes[" + std::to_string(shapeCounter) + "].cReflective").c_str());

        //pass in shape matrix and inverse matrix because inverting is costly
        glUniformMatrix4fv(matrix, 1, GL_FALSE, &shape.ctm[0][0]);
        glUniformMatrix3fv(normalMat, 1, GL_FALSE, &glm::inverse(glm::transpose(glm::mat3(shape.ctm)))[0][0]);
        glUniformMatrix4fv(matrixInv, 1, GL_FALSE, &glm::inverse(shape.ctm)[0][0]);


        // Pass in shininess and color components
        glUniform1f(shininess, shape.shininess);
        glUniform4fv(cSpecular, 1, &shape.cSpecular[0]);
        glUniform4fv(cAmbient, 1, &shape.cAmbient[0]);
        glUniform4fv(cDiffuse, 1, &shape.cDiffuse[0]);
        glUniform4fv(cReflective, 1, &shape.cReflective[0]);

        //pass in the shape type
        switch(shape.type) {
        case 0:
            glUniform1i(type, 0);
            break;

        default:
            std::cerr << "Attempted to take on a shape that hasn't been modeled yet" << std::endl;
            exit(1);
            break;
        }

        shapeCounter++;
    }
    int numShapes_loc = m_program->uniformLocation("numShapes");
    m_program->setUniformValue(numShapes_loc, numShapes);

    glDrawArrays(GL_TRIANGLES, 0, 6); //draw the fullscreen quad (6 vertices)
    glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind buffer (though none should have been bound)
    // Unbind Vertex Array
    glBindVertexArray(0);
}

void GLWidget::resizeGL(int w, int h)
{
    m_proj.setToIdentity();
    m_proj.perspective(45.0f, GLfloat(w) / h, 0.01f, 100.0f);
}

void GLWidget::mousePressEvent(QMouseEvent *event) {
    m_prevMousePos = event->pos();
//    std::cout << m_prevMousePos.x() << " POS X" << std::endl;
//    std::cout << m_prevMousePos.y() << " POS Y" << std::endl;
}

void GLWidget::mouseMoveEvent(QMouseEvent *event) {
    m_angleX += 10 * (event->position().x() - m_prevMousePos.x()) / (float) width();
    m_angleY += 10 * (event->position().y() - m_prevMousePos.y()) / (float) height();
    m_prevMousePos = event->pos();
    rebuildMatrices();
}

void GLWidget::wheelEvent(QWheelEvent *event) {
    m_zoom -= event->angleDelta().y() / 100.f;
    rebuildMatrices();
}

void GLWidget::rebuildMatrices() {
    m_camera.setToIdentity();
    QMatrix4x4 rot;
    rot.setToIdentity();
    rot.rotate(-10 * m_angleX,QVector3D(0,0,1));
    QVector3D eye = QVector3D(1,1,1);
    eye = rot.map(eye);
    rot.setToIdentity();
    rot.rotate(-10 * m_angleY,QVector3D::crossProduct(QVector3D(0,0,1),eye));
    eye = rot.map(eye);

    eye = eye * m_zoom;

    m_camera.lookAt(eye,QVector3D(0,0,0),QVector3D(0,0,1));

    m_proj.setToIdentity();
    m_proj.perspective(45.0f, 1.0 * width() / height(), 0.01f, 100.0f);

    update();
}
