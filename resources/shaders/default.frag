#version 330 core

uniform sampler2D stoneSampler;
in vec2 uvCoords;

// Task 5: declare "in" variables for the world-space position and normal,
//         received post-interpolation from the vertex shader
in vec3 world_pos;
in vec3 world_norm;
in vec4 FragPosLightSpace;

// Task 10: declare an out vec4 for your output color
out vec4 fragColor;

// Task 12: declare relevant uniform(s) here, for ambient lighting
uniform float m_ka;
uniform vec3 c_ambient;
// Task 13: declare relevant uniform(s) here, for diffuse lighting
uniform float m_kd;
// Specifies the kind of light -- 0 -> directional, 1 -> point, 2 -> spotlight
uniform int light_type[8];
uniform vec4 m_lightDir[8];
uniform vec4 c_light[8];
uniform vec3 atten[8];
uniform vec4 m_lightPos[8];
uniform vec3 m_lightDirSpot[8];
uniform float m_angle[8];
uniform float m_penumbra[8];
uniform vec3 c_diffuse;
uniform int total_lights;

uniform bool shadow_map_toggle;

// Task 14: declare relevant uniform(s) here, for specular lighting
uniform float m_ks;
uniform float m_shin;
uniform vec4 cam_pos;
uniform vec3 c_specular;

uniform sampler2D shadowMap;


float ShadowCalculation(vec4 fragPosLightSpace, vec3 lightDir)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float bias = max(0.05 * (1.0 - dot(normalize(world_norm), lightDir)), 0.005);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 2; ++x)
    {
        for(int y = -1; y <= 2; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 16.0;

    return shadow;
}


vec4 directional(int index, vec4 norm) {

    vec4 color = vec4(0.0,0.0,0.0,1.0);

    vec4 dir_to_light = -1.f*normalize(m_lightDir[index]);
//            if(dot(norm, dir_to_light) > 0) {
//                color = color + c_light[index]*(m_kd*dot(norm, dir_to_light)*vec4(c_diffuse,0.0));
//            }
        vec4 colorToAdd = texture(stoneSampler, uvCoords);
        float surfaceAngle = dot(norm, dir_to_light);
        if (surfaceAngle > 0.0f) {
            if (surfaceAngle > 1.0f) {
                surfaceAngle = 1.0f;
            }
            float blendRed = clamp((0.6f*(colorToAdd[0]) + 0.4f*(m_kd*c_diffuse[0])), 0.0, 1.0);
            float blendGreen = clamp((0.6f*(colorToAdd[1]) + 0.4f*(m_kd*c_diffuse[1])), 0.0, 1.0);
            float blendBlue = clamp((0.6f*(colorToAdd[2]) + 0.4f*(m_kd*c_diffuse[2])), 0.0, 1.0);
            color[0] = clamp(c_light[index][0]*(blendRed)*surfaceAngle, 0.0, 1.0);
            color[1] = clamp(c_light[index][1]*(blendGreen)*surfaceAngle, 0.0, 1.0);
            color[2] = clamp(c_light[index][2]*(blendBlue)*surfaceAngle, 0.0, 1.0);
        }

            vec3 light_to_pos = normalize(vec3(m_lightDir[index][0], m_lightDir[index][1], m_lightDir[index][2]));

            // Task 14: add specular component to output color
            vec3 r = normalize(reflect(light_to_pos, normalize(world_norm)));
            vec3 e = normalize(vec3(cam_pos[0],cam_pos[1],cam_pos[2]) - world_pos);
            if (dot(r,e) > 0) {
                color = color + c_light[index]*m_ks*pow(dot(r,e),m_shin);
            }
    return color;
}

vec4 point(int index, vec4 norm) {
    vec4 color = vec4(0.0,0.0,0.0,1.0);
    vec3 dir_to_light = normalize(vec3(m_lightPos[index][0], m_lightPos[index][1], m_lightPos[index][2]) - world_pos);
    vec3 light_to_pos = normalize(-1.f*dir_to_light);
    float distance = distance(vec3(m_lightPos[index][0], m_lightPos[index][1], m_lightPos[index][2]), world_pos);
    float f_att = min(1.f,1.f / (atten[index][0]+distance*atten[index][1] + distance*distance*atten[index][2]));

//    if(dot(vec3(norm[0], norm[1], norm[2]), dir_to_light) > 0) {
//        color = color + f_att*c_light[index]*(m_kd*dot(vec3(norm[0], norm[1], norm[2]), dir_to_light)*vec4(c_diffuse,0.0));
//    }

    vec4 colorToAdd = texture(stoneSampler, uvCoords);
    float surfaceAngle = dot(vec3(norm[0], norm[1], norm[2]), dir_to_light);
    //if (surfaceAngle > 0.0f) {
        if (surfaceAngle < 0.0f) {
            surfaceAngle = 0.0f;
        }
        if (surfaceAngle > 1.0f) {
            surfaceAngle = 1.0f;
        }
        float blendRed = clamp((0.6f*(colorToAdd[0]) + 0.4f*(m_kd*c_diffuse[0])), 0.0, 1.0);
        float blendGreen = clamp((0.6f*(colorToAdd[1]) + 0.4f*(m_kd*c_diffuse[1])), 0.0, 1.0);
        float blendBlue = clamp((0.6f*(colorToAdd[2]) + 0.4f*(m_kd*c_diffuse[2])), 0.0, 1.0);
        color[0] = clamp(f_att*c_light[index][0]*(blendRed)*surfaceAngle, 0.0, 1.0);
        color[1] = clamp(f_att*c_light[index][1]*(blendGreen)*surfaceAngle, 0.0, 1.0);
        color[2] = clamp(f_att*c_light[index][2]*(blendBlue)*surfaceAngle, 0.0, 1.0);
    //}
//    color = color + m_kd * f_att * texture(stoneSampler, uvCoords); /* Texture in place of diffuse */

    vec3 v = normalize(vec3(cam_pos[0],cam_pos[1],cam_pos[2]) - world_pos);
    vec3 r = normalize(reflect(light_to_pos, normalize(world_norm)));
    if (dot(dir_to_light,vec3(norm[0], norm[1], norm[2])) >= 0) {
        color = color + f_att*c_light[index]*m_ks*vec4(c_specular, 1.0)*pow(dot(r,v), m_shin);
    }
    return color;
}

vec4 spot(int index, vec4 norm) {
    vec4 color = vec4(0.0,0.0,0.0,1.0);
    vec3 dir_to_light = normalize(vec3(m_lightPos[index][0], m_lightPos[index][1], m_lightPos[index][2]) - world_pos);
    vec3 light_to_pos = normalize(-1.f*dir_to_light);
    float distance = distance(vec3(m_lightPos[index][0], m_lightPos[index][1], m_lightPos[index][2]), world_pos);
    float f_att = min(1.f,1.f / (atten[index][0]+distance*atten[index][1] + distance*distance*atten[index][2]));
    vec3 light_dir = vec3(m_lightDir[index][0], m_lightDir[index][1], m_lightDir[index][2]);
    float light_pos_angle = acos(dot(light_dir, light_to_pos) / (length(light_dir)*length(light_to_pos)));
    vec3 li = normalize(vec3(m_lightPos[index][0], m_lightPos[index][1], m_lightPos[index][2]) - world_pos);
    vec4 light_color;
    float theta_inner = m_angle[index] - m_penumbra[index];
    float theta_outer = m_angle[index];
    if (light_pos_angle <= theta_inner) {
        light_color = c_light[index];
    } else if (light_pos_angle >= theta_inner && light_pos_angle <= theta_outer) {
        float inner_term = ((light_pos_angle - theta_inner) / (theta_outer - theta_inner));
        float falloff = -2*pow(inner_term, 3) + 3*pow(inner_term,2);
        light_color = c_light[index]*(1.f-falloff);
    } else {
        light_color = vec4(0,0,0,1);
    }

//    if (dot(vec3(norm[0], norm[1], norm[2]), li) >= 0) {
//        vec3 diffuse =  f_att*vec3(light_color[0], light_color[1], light_color[2])*m_kd*(c_diffuse*dot(vec3(norm[0], norm[1], norm[2]), li));
//        color = color + vec4(diffuse, 0.f);
//    }
    vec4 colorToAdd = texture(stoneSampler, uvCoords);
    float surfaceAngle = dot(vec3(norm[0], norm[1], norm[2]), li);
    if (surfaceAngle > 0.0f) {
        if (surfaceAngle > 1.0f) {
            surfaceAngle = 1.0f;
        }
        float blendRed = clamp((0.6f*(colorToAdd[0]) + 0.4f*(m_kd*c_diffuse[0])), 0.0, 1.0);
        float blendGreen = clamp((0.6f*(colorToAdd[1]) + 0.4f*(m_kd*c_diffuse[1])), 0.0, 1.0);
        float blendBlue = clamp((0.6f*(colorToAdd[2]) + 0.4f*(m_kd*c_diffuse[2])), 0.0, 1.0);
        color[0] = clamp(f_att*light_color[0]*(blendRed)*surfaceAngle, 0.0, 1.0);
        color[1] = clamp(f_att*light_color[1]*(blendGreen)*surfaceAngle, 0.0, 1.0);
        color[2] = clamp(f_att*light_color[2]*(blendBlue)*surfaceAngle, 0.0, 1.0);
    }

    vec3 v = normalize(vec3(cam_pos[0],cam_pos[1],cam_pos[2]) - world_pos);
    vec3 r = normalize(reflect(light_to_pos, normalize(world_norm)));
    if (dot(vec3(norm[0], norm[1], norm[2]),li) >=0) {
        vec3 specular =  f_att*vec3(light_color[0], light_color[1], light_color[2])*m_ks*c_specular*pow(dot(r,v), m_shin);
        color = color + vec4(specular, 0.f);
    }
    return color;
}

void main() {
    vec4 color = vec4(0.0,0.0,0.0,1.0);
    color = color + m_ka*vec4(c_ambient,0.0);
    vec4 norm_4 = vec4(normalize(world_norm),0.0);

    for (int i = 0; i<total_lights; i++) {
        if (light_type[i] == 0) {

            if(shadow_map_toggle){
                float shadow = ShadowCalculation(FragPosLightSpace, normalize(vec3(m_lightDir[i])));
                color = color + (1.0 - shadow)*directional(i, norm_4);
            } else {
                color = color + directional(i, norm_4);
            }


        } else if (light_type[i] == 1) {
            color = color + clamp(point(i, norm_4), 0.0f, 1.0f);
        } else if (light_type[i] == 2) {
            color = color + spot(i, norm_4);
        }
    }
    color[0] = clamp(color[0], 0.0, 1.0);
    color[1] = clamp(color[1], 0.0, 1.0);
    color[2] = clamp(color[2], 0.0, 1.0);
    //color = texture(stoneSampler, uvCoords);
    fragColor = color;
}
