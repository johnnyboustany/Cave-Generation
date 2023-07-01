
#version 330 core
layout (location = 0) in vec3 object_space_position3d;
layout(location = 1) in vec3 object_space_normal3d;


// Task 6: declare a uniform mat4 to store model matrix
uniform vec3 offsets[256];

uniform mat4 lightSpaceMatrix;


void main()
{

    vec3 world_pos = offsets[gl_InstanceID] + object_space_position3d;
    gl_Position = lightSpaceMatrix * vec4(world_pos, 1.0);
}
