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
    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();


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

    m_terrainVao.create();
    m_terrainVao.bind();

    verts = m_terrain.generateTerrain();
    m_terrainVbo.create();
    m_terrainVbo.bind();
    m_terrainVbo.allocate(verts.data(),verts.size()*sizeof(GLfloat));


    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat),
                             nullptr);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat),
                             reinterpret_cast<void *>(3 * sizeof(GLfloat)));

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat),
                             reinterpret_cast<void *>(6 * sizeof(GLfloat)));

    m_terrainVbo.release();


    m_world.setToIdentity();
    m_world.translate(QVector3D(-0.5,-0.5,0));


    m_camera.setToIdentity();
    m_camera.lookAt(QVector3D(1,1,1),QVector3D(0,0,0),QVector3D(0,0,1));

    m_program->release();
}


//reset vertex map//should change the value for future too???
void GLWidget::resetHeightMap(){
    verts = m_terrain.clearHeightMap();
    m_terrainVbo.bind();
    m_terrainVbo.allocate(verts.data(),verts.size()*sizeof(GLfloat));
    m_terrainVbo.release();

    m_elapsedTimer.restart();


    update();
};

void GLWidget::useNewHeightMap(std::vector<RGBA> canvasData){
    verts = m_terrain.newHeightMap(canvasData);
    m_terrainVbo.bind();
    m_terrainVbo.allocate(verts.data(),verts.size()*sizeof(GLfloat));
    m_terrainVbo.release();


    m_elapsedTimer.restart();

    update();
}

void GLWidget::useNewImage(const std::string &file){
    //verts.clear();
    verts = m_terrain.uploadNewImage(file);

    std::cout<<verts.size()<<std::endl;
    m_terrainVbo.bind();
    m_terrainVbo.allocate(verts.data(),verts.size()*sizeof(GLfloat));
    m_terrainVbo.release();

    update();

}

void GLWidget::finish(){
    this->makeCurrent();
    m_terrainVbo.destroy();
    m_terrainVao.destroy();
    delete m_program;
    m_program = nullptr;
    this->doneCurrent();
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    m_program->bind();
    m_program->setUniformValue(m_projMatrixLoc, m_proj);
    m_program->setUniformValue(m_mvMatrixLoc, m_camera * m_world);
    m_program->setUniformValue(m_program->uniformLocation("wireshade"),m_terrain.m_wireshade);

    int res = m_terrain.getResolution();

    glPolygonMode(GL_FRONT_AND_BACK,m_terrain.m_wireshade? GL_LINE : GL_FILL);
    glDrawArrays(GL_TRIANGLES, 0, res * res * 6);

    m_program->release();
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


    float camX = sin(m_elapsedTimer.elapsed() * 0.001f) * 2.f;
    float camZ = cos(m_elapsedTimer.elapsed() * 0.001f) * 2.f;


//    std::cout << m_elapsedTimer.elapsed() << " TIMER" << std::endl;

//    if(8.f < (m_elapsedTimer.elapsed() * 0.001f) && (m_elapsedTimer.elapsed() * 0.001f) < 8.05f){
//        std::cout<<"it's time"<<std::endl;
//        //GLWidget::resetHeightMap();
//        GLWidget::useNewImage("resources/zack.png");

//    }

//    if (16.f < (m_elapsedTimer.elapsed() * 0.001f) && (m_elapsedTimer.elapsed() * 0.001f) < 16.05f){
//        std::cout<<"it's time"<<std::endl;
//        //GLWidget::resetHeightMap();
//        GLWidget::useNewImage("resources/helen (1).jpg");

//    }

    m_camera.lookAt(QVector3D(camX, 0.f, camZ), QVector3D(0,0,0), QVector3D(0,-1,0));


    //m_camera.lookAt(eye,QVector3D(0,0,0),QVector3D(0,0,1));

    m_proj.setToIdentity();
    m_proj.perspective(45.0f, 1.0 * width() / height(), 0.01f, 100.0f);

    update();
}


void GLWidget::timerEvent(QTimerEvent *event){
    if(isAnimate){
        GLWidget::spinInCircle();
    }
}

void GLWidget::spinInCircle(){
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

//    std::cout << m_elapsedTimer.elapsed() << " ELAPSED" << std::endl;
//    float timer = m_elapsedTimer.elapsed() * 0.001f;

//    eye += (QVector3D(timer, timer, timer));

//    m_camera.lookAt(eye, QVector3D(0,0,0), QVector3D(0,0,1));

    // NEW
    float camX = sin(m_elapsedTimer.elapsed() * 0.001f) * 2.f;
    float camY = sin(m_elapsedTimer.elapsed() * 0.001f) * 2.f;
    float camZ = cos(m_elapsedTimer.elapsed() * 0.001f) * 2.f;

//    std::cout << m_elapsedTimer.elapsed() * 0.001f << std::endl;

    if (5.f < (m_elapsedTimer.elapsed() * 0.001f) && (m_elapsedTimer.elapsed() * 0.001f) < 5.05f) {
        std::cout << "NO HITZY" << std::endl;

    }

//    QVector4D aespa = m_camera.column(3);
//    std::cout << eye.x() << " " << eye.y() << " " << eye.z() << " ASSPA" << std::endl;

//    if (aespa.x() == 0 || aespa.z() == 0) {
//        std::cout << "SHITZY" << std::endl;
//    }

//    while (true) {
        std::cout << "ASSPA" << std::endl;
        m_camera.lookAt(QVector3D(camX, camY, camZ), QVector3D(0,0,0), QVector3D(0,-1,0));
//    }

        if (0.f < (m_elapsedTimer.elapsed() * 0.001f) && (m_elapsedTimer.elapsed() * 0.001f) < .05f) {
            GLWidget::useNewImage("resources/staff/class_1.jpg");

        }

        if(3.f < (m_elapsedTimer.elapsed() * 0.001f) && (m_elapsedTimer.elapsed() * 0.001f) < 3.05f){
            std::cout<<"it's time"<<std::endl;
            //GLWidget::resetHeightMap();
            GLWidget::useNewImage("resources/staff/daniel.png");

        }

        if (6.f < (m_elapsedTimer.elapsed() * 0.001f) && (m_elapsedTimer.elapsed() * 0.001f) < 6.05f){
            std::cout<<"it's time"<<std::endl;
            //GLWidget::resetHeightMap();
            GLWidget::useNewImage("resources/staff/zack.png");

        }
        if (9.f < (m_elapsedTimer.elapsed() * 0.001f) && (m_elapsedTimer.elapsed() * 0.001f) < 9.05f){
            std::cout<<"it's time"<<std::endl;
            //GLWidget::resetHeightMap();
            GLWidget::useNewImage("resources/staff/logan.png");

        }

        if (12.f < (m_elapsedTimer.elapsed() * 0.001f) && (m_elapsedTimer.elapsed() * 0.001f) < 12.05f){
            std::cout<<"it's time"<<std::endl;
            //GLWidget::resetHeightMap();
            GLWidget::useNewImage("resources/staff/adrian.png");

        }

        if (15.f < (m_elapsedTimer.elapsed() * 0.001f) && (m_elapsedTimer.elapsed() * 0.001f) < 15.05f){
            std::cout<<"it's time"<<std::endl;
            //GLWidget::resetHeightMap();
            GLWidget::useNewImage("resources/staff/anna.png");

        }

        if (18.f < (m_elapsedTimer.elapsed() * 0.001f) && (m_elapsedTimer.elapsed() * 0.001f) < 18.05f){
            std::cout<<"it's time"<<std::endl;
            //GLWidget::resetHeightMap();
            GLWidget::useNewImage("resources/staff/derick.png");

        }

        if (21.f < (m_elapsedTimer.elapsed() * 0.001f) && (m_elapsedTimer.elapsed() * 0.001f) < 21.05f){
            std::cout<<"it's time"<<std::endl;
            //GLWidget::resetHeightMap();
            GLWidget::useNewImage("resources/staff/helen.png");

        }

        if (24.f < (m_elapsedTimer.elapsed() * 0.001f) && (m_elapsedTimer.elapsed() * 0.001f) < 24.05f){
            std::cout<<"it's time"<<std::endl;
            //GLWidget::resetHeightMap();
            GLWidget::useNewImage("resources/staff/jianxin.png");

        }

        if (27.f < (m_elapsedTimer.elapsed() * 0.001f) && (m_elapsedTimer.elapsed() * 0.001f) < 27.05f){
            std::cout<<"it's time"<<std::endl;
            //GLWidget::resetHeightMap();
            GLWidget::useNewImage("resources/staff/marc.png");

        }

        if (30.f < (m_elapsedTimer.elapsed() * 0.001f) && (m_elapsedTimer.elapsed() * 0.001f) < 30.05f){
            std::cout<<"it's time"<<std::endl;
            //GLWidget::resetHeightMap();
            GLWidget::useNewImage("resources/staff/mehek.png");

        }

        if (33.f < (m_elapsedTimer.elapsed() * 0.001f) && (m_elapsedTimer.elapsed() * 0.001f) < 33.05f){
            std::cout<<"it's time"<<std::endl;
            //GLWidget::resetHeightMap();
            GLWidget::useNewImage("resources/staff/nick.png");

        }

        if (36.f < (m_elapsedTimer.elapsed() * 0.001f) && (m_elapsedTimer.elapsed() * 0.001f) < 36.05f){
            std::cout<<"it's time"<<std::endl;
            //GLWidget::resetHeightMap();
            GLWidget::useNewImage("resources/staff/sean.png");

        }
        if (39.f < (m_elapsedTimer.elapsed() * 0.001f) && (m_elapsedTimer.elapsed() * 0.001f) < 39.05f){
            std::cout<<"it's time"<<std::endl;
            //GLWidget::resetHeightMap();
            GLWidget::useNewImage("resources/staff/yiwen.png");

        }
        if (42.f < (m_elapsedTimer.elapsed() * 0.001f) && (m_elapsedTimer.elapsed() * 0.001f) < 42.05f){
            std::cout<<"it's time"<<std::endl;
            //GLWidget::resetHeightMap();
            GLWidget::useNewImage("resources/staff/angela.png");

        }
        if (45.f < (m_elapsedTimer.elapsed() * 0.001f) && (m_elapsedTimer.elapsed() * 0.001f) < 45.05f){
            std::cout<<"it's time"<<std::endl;
            //GLWidget::resetHeightMap();
            GLWidget::useNewImage("resources/staff/geoffrey.png");

        }
        if (48.f < (m_elapsedTimer.elapsed() * 0.001f) && (m_elapsedTimer.elapsed() * 0.001f) < 48.05f){
            std::cout<<"it's time"<<std::endl;
            //GLWidget::resetHeightMap();
            GLWidget::useNewImage("resources/staff/ziang.png");

        }

    m_proj.setToIdentity();
    m_proj.perspective(45.0f, 1.0 * width() / height(), 0.01f, 100.0f);

//    itzy += 0.01f;
    update();
}
