#pragma once

#include <vector>
#include <string>

#include <glm/glm.hpp>

// Enum of the types of virtual lights that might be in the scene
enum class LightType {
    LIGHT_POINT,
    LIGHT_DIRECTIONAL,
    LIGHT_SPOT,
    LIGHT_AREA // No longer supported
};

// Enum of the types of primitives that might be in the scene
enum class PrimitiveType {
    PRIMITIVE_CUBE,
    PRIMITIVE_CONE,
    PRIMITIVE_CYLINDER,
    PRIMITIVE_TORUS,
    PRIMITIVE_SPHERE,
    PRIMITIVE_MESH
};

// Enum of the types of transformations that can be applied
enum class TransformationType {
    TRANSFORMATION_TRANSLATE,
    TRANSFORMATION_SCALE,
    TRANSFORMATION_ROTATE,
    TRANSFORMATION_MATRIX
};

// Type which can be used to store an RGBA color in floats [0,1]
using SceneColor = glm::vec4;

// Struct which contains the global color coefficients of a scene.
// These are multiplied with the object-specific materials in the lighting equation.
struct SceneGlobalData  {
    float ka; // Ambient term
    float kd; // Diffuse term
    float ks; // Specular term
    float kt; // Transparency; used for extra credit (refraction)
};

// Struct which contains data for a single light
struct SceneLightData {
    int id;
    LightType type;

    SceneColor color;
    glm::vec3 function;  // Attenuation function

    glm::vec4 pos;       // Not applicable to directional lights
    glm::vec4 dir;       // Not applicable to point lights

    float penumbra;      // Only applicable to spot lights, in RADIANS
    float angle;         // Only applicable to spot lights, in RADIANS

    float width, height; // No longer supported (area lights)
};

// Struct which contains data for the camera of a scene
struct SceneCameraData {
    glm::vec4 pos;
    glm::vec4 look;
    glm::vec4 up;

    float heightAngle;   // The height angle of the camera in RADIANS
    float aspectRatio;
    float aperture;      // Only applicable for depth of field
    float focalLength;   // Only applicable for depth of field
    glm::mat4 m_proj;
    glm::mat4 m_view;
    void updateViewMatrix(glm::vec3 up, glm::vec3 look, glm::vec3 pos) {
        glm::mat4 m_trans = glm::mat4(1,0,0,0,
                                      0,1,0,0,
                                      0,0,1,0,
                                      -pos[0], -pos[1], -pos[2],1);
        glm::vec3 w = -look / glm::length(look);
        glm::vec3 v = (up - glm::dot(up, w)*w) / glm::length(up - glm::dot(up,w)*w);
        glm::vec3 u = glm::cross(v, w);
        glm::mat4 m_rot(u[0], v[0], w[0], 0,
                        u[1], v[1], w[1], 0,
                        u[2], v[2], w[2], 0,
                        0,0,0,1);
        m_view= m_rot*m_trans;
    }
    void updateProjMatrix(float p_near, float p_far) {
        glm::mat4 remap_z = glm::mat4(1,0,0,0,
                                      0,1,0,0,
                                      0,0,-2,0,
                                      0,0,-1,1);
        float c = -1.f*p_near / p_far;
        glm::mat4 m_pp = glm::mat4(1,0,0,0,
                                   0,1,0,0,
                                   0,0,(1/(1+c)),-1,
                                   0,0,(-1*c)/(1+c),0);
        glm::mat4 scale = glm::mat4((1/(p_far*tan(heightAngle*aspectRatio / 2))),0,0,0,
                                    0,1/(p_far*tan(heightAngle / 2)),0,0,
                                    0,0,1.f/p_far,0,
                                    0,0,0,1);
        m_proj = remap_z*m_pp*scale;
    }
};

// Struct which contains data for texture mapping files
struct SceneFileMap {
    SceneFileMap() : isUsed(false) {}

    bool isUsed;
    std::string filename;

    float repeatU;
    float repeatV;

    void clear() {
       isUsed = false;
       repeatU = 0.0f;
       repeatV = 0.0f;
       filename = std::string();
    }
};

// Struct which contains data for a material (e.g. one which might be assigned to an object)
struct SceneMaterial {
   SceneColor cAmbient;     // Ambient term
   SceneColor cDiffuse;     // Diffuse term
   SceneColor cSpecular;    // Specular term
   float shininess;         // Specular exponent

   SceneColor cReflective;  // Used to weight contribution of reflected ray lighting (via multiplication)

   SceneColor cTransparent; // Transparency;        used for extra credit (refraction)
   float ior;               // Index of refraction; used for extra credit (refraction)

   SceneFileMap textureMap; // Used for texture mapping
   float blend;             // Used for texture mapping

   SceneColor cEmissive;    // Not used
   SceneFileMap bumpMap;    // Not used

   void clear() {
       cAmbient    = glm::vec4(0);
       cDiffuse    = glm::vec4(0);
       cSpecular   = glm::vec4(0);
       shininess   = 0;

       cReflective = glm::vec4(0);

       cTransparent = glm::vec4(0);
       ior = 0;

       textureMap.clear();
       blend = 0;

       cEmissive = glm::vec4(0);
       bumpMap.clear();
   }
};

// Struct which contains data for a transformation.
struct SceneTransformation {
    TransformationType type;
   
    glm::vec3 translate; // Only applicable when translating. Defines t_x, t_y, and t_z, the amounts to translate by, along each axis.
    glm::vec3 scale;     // Only applicable when scaling.     Defines s_x, s_y, and s_z, the amounts to scale by, along each axis.
    glm::vec3 rotate;    // Only applicable when rotating.    Defines the axis of rotation; should be a unit vector.
    float angle;         // Only applicable when rotating.    Defines the angle to rotate by in RADIANS, following the right-hand rule.
    glm::mat4 matrix;    // Only applicable when transforming by a custom matrix. This is that custom matrix.
};

// Struct which represents a node in the scene graph/tree, to be parsed by the student's `SceneParser`.
struct SceneNode {
   std::vector<SceneTransformation*> transformations; // Note the order of transformations described in lab 5
   std::vector<SceneNode*>           children;
};

