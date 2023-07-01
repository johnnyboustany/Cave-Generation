#include "Cube.h"

void Cube::updateParams(int param1) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    setVertexData();
}

void Cube::makeTile(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {

    // create a tile (i.e. 2 triangles) based on 4 given points.
    glm::vec3 normal = {0,0,1};
    glm::vec3 normalTopLeft = glm::normalize(glm::cross((topLeft - bottomLeft),(topLeft - topRight)));
    glm::vec3 normalTopRight = glm::normalize(glm::cross((topRight - topLeft), (topRight - bottomRight)));
    glm::vec3 normalBottomRight = glm::normalize(glm::cross((bottomRight - topRight), (bottomRight - bottomLeft)));
    glm::vec3 normalBottomLeft = glm::normalize(glm::cross((bottomLeft - bottomRight),(bottomLeft - topLeft)));
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normalBottomRight);

    /* Note: added UV coordinates */
    m_vertexData.push_back(1.0);
    m_vertexData.push_back(0.0);

    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, normalTopRight);
    m_vertexData.push_back(1.0);
    m_vertexData.push_back(1.0);

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normalTopLeft);
    m_vertexData.push_back(0.0);
    m_vertexData.push_back(1.0);

    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, normalBottomLeft);
    m_vertexData.push_back(0.0);
    m_vertexData.push_back(0.0);

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normalBottomRight);
    m_vertexData.push_back(1.0);
    m_vertexData.push_back(0.0);

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normalTopLeft);
    m_vertexData.push_back(0.0);
    m_vertexData.push_back(1.0);
}

void Cube::makeFace(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {
    // create a single side of the cube out of the 4
    // given points and makeTile()
    for (int i = 0; i < m_param1; i++) {
        for (int j = 0; j < m_param1; j++) {
            glm::vec3 nTopLeft = topLeft - float(j)*(topLeft - bottomLeft) / float(m_param1) + float(i)*(topRight - topLeft) / float(m_param1);
            glm::vec3 nTopRight = nTopLeft + ((topRight - topLeft) / float(m_param1));
            glm::vec3 nBottomLeft = nTopLeft - ((topLeft - bottomLeft) / float(m_param1));
            glm::vec3 nBottomRight = nBottomLeft + ((bottomRight - bottomLeft) / float(m_param1));
            makeTile(nTopLeft, nTopRight, nBottomLeft, nBottomRight);
        }
    }


}

void Cube::setVertexData() {


     makeFace(glm::vec3(-0.5f,  0.5f, 0.5f),
              glm::vec3( 0.5f,  0.5f, 0.5f),
              glm::vec3(-0.5f, -0.5f, 0.5f),
              glm::vec3( 0.5f, -0.5f, 0.5f));

    // Using the makeFace() function to make all 6 sides of the cube

     makeFace(glm::vec3(0.5f,  0.5f, 0.5f),
              glm::vec3( -0.5f,  0.5f, 0.5f),
              glm::vec3( 0.5f, 0.5f, -0.5f),
              glm::vec3(-0.5f, 0.5f, -0.5f));

     makeFace(glm::vec3( 0.5f,  0.5f, 0.5f),
              glm::vec3(0.5f,  0.5f, -0.5f),
              glm::vec3(0.5f, -0.5f, 0.5f),
              glm::vec3( 0.5f, -0.5f, -0.5f)
              );
     makeFace(glm::vec3(-0.5f,  0.5f, -0.5f),
              glm::vec3( -0.5f,  0.5f, 0.5f),
              glm::vec3( -0.5f, -0.5f, -0.5f),
              glm::vec3(-0.5f, -0.5f, 0.5f)
              );
     makeFace(glm::vec3( 0.5f,  -0.5f, -0.5f),
              glm::vec3(-0.5f,  -0.5f, -0.5f),
              glm::vec3(0.5f, -0.5f, 0.5f),
              glm::vec3( -0.5f, -0.5f, 0.5f)
              );
     makeFace(glm::vec3( 0.5f,  0.5f, -0.5f),
              glm::vec3(-0.5f,  0.5f, -0.5f),
              glm::vec3( 0.5f, -0.5f, -0.5f),
              glm::vec3(-0.5f, -0.5f, -0.5f)
              );
}

// Inserts a glm::vec3 into a vector of floats.
void Cube::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
