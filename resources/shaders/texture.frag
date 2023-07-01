#version 330 core

// Task 16: Create a UV coordinate in variable
layout(location = 0) in vec3 uv_in;

// Task 8: Add a sampler2D uniform
uniform sampler2D sampler;

// Task 29: Add a bool on whether or not to filter the texture
uniform bool pixel_filter;
uniform bool kernel_filter;
uniform bool greyscale_filter;
uniform int img_x;
uniform int img_y;

out vec4 fragColor;

void main()
{
    // Task 17: Set fragColor using the sampler2D at the UV coordinate

    // Task 33: Invert fragColor's r, g, and b color channels if your bool is true
    fragColor = texture(sampler, vec2(uv_in[0], uv_in[1]));
    if (pixel_filter) {
        fragColor = vec4(abs(fragColor[0] - 1), abs(fragColor[1] - 1), abs(fragColor[2] - 1), 1);
    }
    else if (greyscale_filter) {
        float gray_amt = 0.3*fragColor[0] + 0.59*fragColor[1] + 0.11*fragColor[2];
        fragColor = vec4(gray_amt, gray_amt, gray_amt, 1);
    }
    else if (kernel_filter) {
        fragColor = vec4(0.0,0.0,0.0,1.0);
        float inc_x = 1.f / float(img_x);
        float inc_y = 1.f / float(img_y);
        for (int i = 0; i<5; i++) {
            for (int j = 0; j<5; j++) {
                int offset_i = (i - 2);
                int offset_j = (j - 2);
                float uvx = uv_in[0] + offset_i*inc_x;
                float uvy = uv_in[1] + offset_j*inc_y;
                fragColor = fragColor + 0.04*(texture(sampler, vec2(uvx, uvy)));
            }
        }
    }
}
