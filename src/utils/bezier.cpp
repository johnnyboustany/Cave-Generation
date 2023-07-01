#include "bezier.h"

void Bezier::setCameraData(SceneCameraData *cameraData) {
    m_cameraData = cameraData;
}

glm::vec4 Bezier::returnCoefficients(float p1, float p2, float p3, float p4) {
    return m_hermiteMat * m_bezierMat * glm::vec4(p1, p2, p3, p4);
}

void Bezier::updatePos(glm::vec3 p1, glm::vec4 p2, glm::vec4 p3, glm::vec4 p4, float t) {
    glm::vec4 xCoeffs = returnCoefficients(p1[0], p2[0], p3[0], p4[0]);
    glm::vec4 yCoeffs = returnCoefficients(p1[1], p2[1], p3[1], p4[1]);
    glm::vec4 zCoeffs = returnCoefficients(p1[2], p2[2], p3[2], p4[2]);
    float newX = xCoeffs[0]*pow(t, 3) + xCoeffs[1]*pow(t, 2) + xCoeffs[2]*t + xCoeffs[3];
    float newY = yCoeffs[0]*pow(t, 3) + yCoeffs[1]*pow(t, 2) + yCoeffs[2]*t + yCoeffs[3];
    float newZ = zCoeffs[0]*pow(t, 3) + zCoeffs[1]*pow(t, 2) + zCoeffs[2]*t + zCoeffs[3];
    float lookX = 3*xCoeffs[0]*pow(t, 2) + 2*xCoeffs[1]*t + xCoeffs[2];
    float lookY = 3*yCoeffs[0]*pow(t, 2) + 2*yCoeffs[1]*t + yCoeffs[2];
    float lookZ = 3*zCoeffs[0]*pow(t, 2) + 2*zCoeffs[1]*t + zCoeffs[2];
//    float upX = 6*xCoeffs[0]*t + 2*xCoeffs[1];
//    float upY = 6*yCoeffs[0]*t + 2*yCoeffs[1];
//    float upZ = 6*zCoeffs[0]*t + 2*zCoeffs[1];
    m_cameraData->pos = glm::vec4(newX, newY, newZ, 1.0);
    m_cameraData->look = glm::normalize(glm::vec4(lookX, lookY, lookZ, 0.0f));
//    if (t > 0.5) {
//        m_cameraData->up = glm::normalize(glm::vec4(-upX, -upY, -upZ, 0.0f));
//    } else {
//        m_cameraData->up = glm::normalize(glm::vec4(upX, upY, upZ, 0.0f));
//    }
    m_cameraData->updateViewMatrix(m_cameraData->up, m_cameraData->look, m_cameraData->pos);
}
