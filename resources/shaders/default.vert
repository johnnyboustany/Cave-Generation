#version 330 core

// Task 4: declare a vec3 object-space position variable, using
//         the `layout` and `in` keywords.
layout(location = 0) in vec3 obj_pos;
layout(location = 1) in vec3 obj_norm;
layout(location = 3) in vec2 obj_uv; /* Note: added for texture */

// Task 5: declare `out` variables for the world-space position and normal,
//         to be passed to the fragment shader
out vec3 world_pos;
out vec3 world_norm;
out vec2 uvCoords; /* Note: added for texture */

out vec4 FragPosLightSpace;

// Task 6: declare a uniform mat4 to store model matrix
uniform vec3 offsets[256];

// Task 7: declare uniform mat4's for the view and projection matrix
uniform mat4 view_matrix;
uniform mat4 proj_matrix;


uniform mat4 lightSpaceMatrix;

void main() {
    // Task 8: compute the world-space position and normal, then pass them to
    //         the fragment shader using the variables created in task 5

    world_pos = offsets[gl_InstanceID] + obj_pos;
    world_norm = obj_norm;
    uvCoords = obj_uv;

    FragPosLightSpace = lightSpaceMatrix * vec4(world_pos, 1.0);

    // Recall that transforming normals requires obtaining the inverse-transpose of the model matrix!
    // In projects 5 and 6, consider the performance implications of performing this here.

    // Task 9: set gl_Position to the object space position transformed to clip space
    vec4 pos_4 = proj_matrix*view_matrix*vec4(world_pos, 1.0);
    gl_Position = pos_4;
}
