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

    m_terrain.generateTerrain(verts);
    std::cout << "printing verts " << verts[0][0] << std::endl;
    m_fullscreenQuadVbo.create();
    m_fullscreenQuadVbo.bind();
    m_fullscreenQuadVbo.allocate(fullscreen_quad_verts.data(),fullscreen_quad_verts.size()*sizeof(GLfloat));


    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                             nullptr);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                             reinterpret_cast<void *>(3 * sizeof(GLfloat)));

    //create the mesh texture index
    glGenTextures(1, &m_meshTexture);
    glActiveTexture(0);
    glBindTexture(GL_TEXTURE_1D, m_meshTexture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //necessary??
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB16F, verts.size(), 0, GL_RGB, GL_FLOAT, &verts[0]);
    glBindTexture(GL_TEXTURE_1D, 0);

    //tell the sampler that the mesh info is at texture_sampler active slot 0
    int textureLoc = m_program->uniformLocation("triangle_sampler");
    glUniform1i(textureLoc, 0);

    m_fullscreenQuadVbo.release();


    m_world.setToIdentity();
    m_world.translate(QVector3D(-0.5,-0.5,0));


    m_camera.setToIdentity();

    m_camera.lookAt(QVector3D(1,1,1),QVector3D(0,0,0),QVector3D(0,0,1));

    m_program->release();

    //set up the additional view and camera matrices
    setupViewAndCamera();
}


//reset vertex map//should change the value for future too???
void GLWidget::resetHeightMap(){
    m_terrain.clearHeightMap(verts);
    update();
};

void GLWidget::useNewHeightMap(std::vector<RGBA> canvasData){
    m_terrain.newHeightMap(canvasData, verts);

    glActiveTexture(0);
    glBindTexture(GL_TEXTURE_1D, m_meshTexture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //necessary??
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB16F, verts.size(), 0, GL_RGB, GL_FLOAT, &verts[0]);
    glBindTexture(GL_TEXTURE_2D, 0);


//    for (int i = 0; i < verts.size(); i+=27) {
//        //verts is triangle: (position, normal, color) for each point
//        struct TriangleData {
//            glm::vec3 points[3];
//            glm::vec3 normals[3];
//            glm::vec4 cAmbient;
//            glm::vec4 cDiffuse;
//            glm::vec4 cSpecular;
//            float shininess;
//            glm::vec4 cReflective;
//        };
//        glm::vec3 p1 = glm::vec3(verts[i], verts[i+1], verts[i+2]);
//        glm::vec3 n1 = glm::vec3(verts[i+3], verts[i+4], verts[i+5]);
//        glm::vec2 c1 = glm::vec3(verts[i+6], verts[i+7], verts[i+8]);

//        glm::vec3 p2 = glm::vec3(verts[i+9], verts[i+10], verts[i+11]);
//        glm::vec3 n2 = glm::vec3(verts[i+12], verts[i+13], verts[i+14]);
//        glm::vec2 c2 = glm::vec3(verts[i+15], verts[i+16], verts[i+17]);

//        glm::vec3 p3 = glm::vec3(verts[i+18], verts[i+19], verts[i+20]);
//        glm::vec3 n3 = glm::vec3(verts[i+21], verts[i+22], verts[i+23]);
//        glm::vec2 c3 = glm::vec3(verts[i+24], verts[i+25], verts[i+26]);

//        GLWidget::TriangleData newTriangle = {
//            {p1,p2,p3},//points
//            {n1,n2,n3},//normals
//            //basic color can change
//            glm::vec4(0.1f),//glm::vec4 cAmbient;
//            glm::vec4(0.9,0.1,0.1, 1.f),//glm::vec4 cDiffuse;
//            glm::vec4(1.f),//glm::vec4 cSpecular;
//            25,//float shininess;
//            glm::vec4(0.75, 1, 0.75, 1)
//        };

//        vertTriangles.push_back(newTriangle);
//    }
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
//    glActiveTexture(0);
    glBindTexture(GL_TEXTURE_2D, m_meshTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 99*99*2, 27, 0, GL_RED, GL_UNSIGNED_BYTE, &verts);//load nothing in
    std::cout << " 2000,0 "<< verts[2000][0] << std::endl;
    paintGeometryPhong();

//    glPolygonMode(GL_FRONT_AND_BACK,m_terrain.m_wireshade? GL_LINE : GL_FILL);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);//unbind

    m_program->release();
}

//create the geometry with Phong lighting
void GLWidget::paintGeometryPhong() {
    // Clear screen color and depth before painting
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Bind full-screen quad
    m_fullscreenQuadVao.bind();

    GLuint cameraPositionWorld_loc = m_program->uniformLocation("cameraPositionWorld");
    glUniform4fv(cameraPositionWorld_loc, 1, &cameraPosition[0]);


    //pass in the height angle and width angle
    float widthAngle = camera.heightAngle * float(width()) / float(height());
    int widthAngle_loc = m_program->uniformLocation("widthAngle");
    int heightAngle_loc = m_program->uniformLocation("heightAngle");
    m_program->setUniformValue(widthAngle_loc, widthAngle);
    m_program->setUniformValue(heightAngle_loc, camera.heightAngle);

    // Pass in m_view and m_proj
    //TODO is m_camera * m_world the view matrix??? I feel like no
    int viewMatrix_loc = m_program->uniformLocation("viewMatrix");
    int viewMatrixInv_loc = m_program->uniformLocation("viewMatrixInverse");
//    m_program->setUniformValue(viewMatrix_loc, viewMatrix);
//    m_program->setUniformValue(viewMatrixInv_loc, glm::inverse(viewMatrix));
    glUniformMatrix4fv(viewMatrix_loc, 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(viewMatrixInv_loc, 1, GL_FALSE, &glm::inverse(viewMatrix)[0][0]);

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

    int triangleCounter = 0;
    for (TriangleData triangle : triangles) {

        //pass in all the shape-specific information to the first full-screen quad shader
        GLint points0 = m_program->uniformLocation(("triangles[" + std::to_string(triangleCounter) + "].points[0]").c_str());
        GLint points1 = m_program->uniformLocation(("triangles[" + std::to_string(triangleCounter) + "].points[1]").c_str());
        GLint points2 = m_program->uniformLocation(("triangles[" + std::to_string(triangleCounter) + "].points[2]").c_str());
        GLint normals0 = m_program->uniformLocation(("triangles[" + std::to_string(triangleCounter) + "].normals[0]").c_str());
        GLint normals1 = m_program->uniformLocation(("triangles[" + std::to_string(triangleCounter) + "].normals[1]").c_str());
        GLint normals2 = m_program->uniformLocation(("triangles[" + std::to_string(triangleCounter) + "].normals[2]").c_str());
        GLint cAmbient = m_program->uniformLocation(("triangles[" + std::to_string(triangleCounter) + "].cAmbient").c_str());
        GLint cDiffuse = m_program->uniformLocation(("triangles[" + std::to_string(triangleCounter) + "].cDiffuse").c_str());
        GLint cSpecular = m_program->uniformLocation(("triangles[" + std::to_string(triangleCounter) + "].cSpecular").c_str());
        GLint shininess = m_program->uniformLocation(("triangles[" + std::to_string(triangleCounter) + "].shininess").c_str());
        GLint cReflective = m_program->uniformLocation(("triangles[" + std::to_string(triangleCounter) + "].cReflective").c_str());

        glUniform3fv(points0, 1, &triangle.points[0][0]);
        glUniform3fv(points1, 1, &triangle.points[1][0]);
        glUniform3fv(points2, 1, &triangle.points[2][0]);

        glUniform3fv(normals0, 1, &triangle.normals[0][0]);
        glUniform3fv(normals1, 1, &triangle.normals[1][0]);
        glUniform3fv(normals2, 1, &triangle.normals[2][0]);
        // Pass in shininess and color components
        glUniform1f(shininess, triangle.shininess);
        glUniform4fv(cSpecular, 1, &triangle.cSpecular[0]);
        glUniform4fv(cAmbient, 1, &triangle.cAmbient[0]);
        glUniform4fv(cDiffuse, 1, &triangle.cDiffuse[0]);
        glUniform4fv(cReflective, 1, &triangle.cReflective[0]);


        triangleCounter++;
    }
    int numTriangles_loc = m_program->uniformLocation("numTriangles");
    m_program->setUniformValue(numTriangles_loc, triangleCounter);

    glDrawArrays(GL_TRIANGLES, 0, 6); //draw the fullscreen quad (6 vertices)
    glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind buffer (though none should have been bound)
    // Unbind Vertex Array
    glBindVertexArray(0);
}

//calculate the view, perspective, and camera matrices
//only called at the beginning of each scene
void GLWidget::setupViewAndCamera() {
    //If in need of debugging, use glm::lookAt() and glm::perspective()

    //View matrix
//    SceneCameraData cameraData = metaData.cameraData;
    look = camera.look;
    glm::vec4 w = glm::normalize(-camera.look);
    glm::vec4 v = glm::normalize(camera.up - glm::dot(camera.up, w)*w);
    glm::vec3 u = glm::cross(glm::vec3(v),glm::vec3(w));

    //camera rotation matrix (initialized COLUMN-WISE)
    cameraRotMat = {u[0],v[0],w[0],0,
                              u[1],v[1],w[1],0,
                              u[2],v[2],w[2],0,
                              0,0,0,1};

    cameraTransMat = {1,0,0,0,
                                0,1,0,0,
                                0,0,1,0,
                                -camera.pos[0], -camera.pos[1], -camera.pos[2], 1};

    viewMatrix = cameraRotMat * cameraTransMat;

    cameraPosition = (glm::inverse(viewMatrix)*glm::vec4(0,0,0,1));
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
