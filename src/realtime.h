#pragma once

// Defined before including GLEW to suppress deprecation messages on macOS
#include "shapes/Cube.h"
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "utils/bezier.h"

#include <unordered_map>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>


class Realtime : public QOpenGLWidget
{
public:
    Realtime(QWidget *parent = nullptr);
    void finish();                                      // Called on program exit
    void sceneChanged();
    void settingsChanged();

public slots:
    void tick(QTimerEvent* event);                      // Called once per tick of m_timer

protected:
    void initializeGL() override;                       // Called once at the start of the program
    void paintGL() override;                            // Called whenever the OpenGL context changes or by an update() request
    void resizeGL(int width, int height) override;      // Called when window size changes

private:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    void updateShapes();

    // Tick Related Variables
    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames

    // Input Related Variables
    bool m_mouseDown = false;                           // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;                         // Stores mouse position
    std::unordered_map<Qt::Key, bool> m_keyMap;         // Stores whether keys are pressed or not

    // Device Correction Variables
    int m_devicePixelRatio;

    GLuint m_shader; // Stores id of shader program
    GLuint m_depth_shader; // Stores id of shader program

    GLuint m_texture_shader;
    int m_fbo_width;
    int m_fbo_height;
    int m_screen_width;
    int m_screen_height;
    GLuint m_fbo;
    GLuint m_fbo_texture;
    GLuint m_fbo_renderbuffer;
    GLuint m_defaultFBO;
    GLuint m_vbo;    // Stores id of VBO
    GLuint m_vao;    // Stores id of VAO
    GLuint m_fullscreen_vbo;
    GLuint m_fullscreen_vao;
    GLuint instance_vbo;

    Cube m_cube; // Stores cube mesh

    glm::mat4 m_proj;
    float m_asp_ratio;

    void handleObjects();
    void loadLights();
    void renderShapes();
    void makeFBO();
    void paintTexture(GLuint texture);
    void createOffsets();
    int numInstances = 256;
    glm::vec3 translations[100000];


    GLuint m_depth_map_texture;
    GLuint m_depth_map_fbo;
    void DepthMappingFBO();
    void renderZBuffer();

    /* Additions for Bezier */
    Bezier m_bezier;
    float m_bezierInc = 0.0; // Used to test functionality of curve

    /* Additions for texture mapping */
    QImage m_image;
    GLuint m_stone_texture;

//    glm::vec4 point_1 = glm::vec4(0.f,0.f,0.f, 1.0f);
//    glm::vec4 point_2 = glm::vec4(90.f,-30.f,20.f, 1.0f);
//    glm::vec4 point_3 = glm::vec4(10.f, -5.0f, 3.0f, 1.0f);
//    glm::vec4 point_4 = glm::vec4(80.f, -25.0f, 17.0f, 1.0f);

    glm::vec4 point_1 = glm::vec4(0.f,0.f,0.f, 1.0f);;
    glm::vec4 point_2 = glm::vec4(10.0f, -50.0f, 30.0f, 1.0f);
    glm::vec4 point_3 = glm::vec4(80.0f, -60.0f, 17.0f, 1.0f);
    glm::vec4 point_4 = glm::vec4(90.0f, -30.0f, 20.0f, 1.0f);
};
