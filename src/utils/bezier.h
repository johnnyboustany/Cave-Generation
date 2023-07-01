#pragma once

#include "utils/scenedata.h"
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>

class Bezier {
public:
    SceneCameraData *m_cameraData;
    void updatePos(glm::vec3 p1, glm::vec4 p2, glm::vec4 p3, glm::vec4 p4, float t);
    void setCameraData(SceneCameraData *cameraData);
private:
    glm::mat4 m_hermiteMat = glm::inverse(glm::mat4(0.0, 1.0, 0.0, 3.0,
                                                  0.0, 1.0, 0.0, 2.0,
                                                  0.0, 1.0, 1.0, 1.0,
                                                  1.0, 1.0, 0.0, 0.0));
    glm::mat4 m_bezierMat = glm::mat4(1.0, 0.0, -3.0, 0.0,
                                    0.0, 0.0, 3.0, 0.0,
                                    0.0, 0.0, 0.0, -3.0,
                                    0.0, 1.0, 0.0, 3.0);
    glm::vec4 returnCoefficients(float p1, float p2, float p3, float p4);
};
