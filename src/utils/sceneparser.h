#pragma once

#include "scenedata.h"
#include <vector>
#include <string>
#include <map>

// Struct which contains data for a single primitive, to be used for rendering
struct RenderShapeData {
    glm::vec3 position;
};

// Struct which contains all the data needed to render a scene
struct RenderData {
    SceneGlobalData globalData;
    SceneCameraData cameraData;
    SceneMaterial material;
    std::vector<SceneLightData> lights;
    std::vector<RenderShapeData> shapes;
};

class SceneParser {
private:
    glm::vec3 getWorldCoord(glm::vec3 origin, glm::vec3 xyz, float radius);
    bool inSphere(glm::vec3 origin, float radius, glm::vec3 xyz);
    RenderShapeData createCube(glm::vec3 location);
    void createSphere(glm::vec3 origin, float radius);
    void createLine(float radius, glm::vec3 point_1, glm::vec3 point_2);
    void radiusLoop(glm::vec3 point_1, glm::vec3 point_2, float radius_1, float radius_2);
    std::vector<SceneLightData> getLights();
    std::vector<RenderShapeData> cube_data;
    std::map<std::vector<int>, bool> isCovered;
    std::map<std::vector<int>, bool> isBlock;
    std::map<std::vector<int>, bool> prevSphereBlock;
    float computePerlin3D(glm::vec3 posVec);

    int m_lookupSize;
    std::vector<glm::vec2> m_randVecLookup;


    float interpolate(float A, float B, float alpha);
    glm::vec3 sampleRandomVector();
    float getRadius(glm::vec3 posVec, float radius);
    glm::vec3 getPerlinPos(glm::vec3 posVec);
    float computePerlin(glm::vec3 posVec);
public:
    // Parse the scene and store the results in renderData.
    // @param filepath    The path of the scene file to load.
    // @param renderData  On return, this will contain the metadata of the loaded scene.
    // @return            A boolean value indicating whether the parse was successful.
    static bool parse(RenderData &renderData, float radius, float seed);
    glm::vec3 point_1 = glm::vec3(0.f,0.f,0.f);
    glm::vec3 point_2 = glm::vec3(90.f,-30.f,20.f);
    glm::vec4 point_3 = glm::vec4(10.f, -5.0f, 3.0f, 1.0f);
    glm::vec4 point_4 = glm::vec4(80.f, -60.0f, 17.0f, 1.0f);
    //glm::vec3 point_2 = glm::vec3(270.f,-90.f,60.f);

};
