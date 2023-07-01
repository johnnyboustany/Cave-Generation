#include "realtime.h"
#include "utils/sceneparser.h"
#include "shapes/Cube.h"
#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "settings.h"
#include "utils/shaderloader.h"
#include <string>
#include <glm/gtx/string_cast.hpp>

RenderData renderData;

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;

}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    glDeleteBuffers(1,&m_vbo);
    glDeleteVertexArrays(1,&m_vao);

    glDeleteTextures(1, &m_depth_map_texture);
    glDeleteRenderbuffers(1, &m_depth_map_fbo);
    this->doneCurrent();
}

void Realtime::updateShapes() {
    m_cube = Cube();
    m_cube.updateParams(2);
}

void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();
    SceneParser parser;
    renderData.cameraData.pos = glm::vec4(0.f, 0.f, 0.f, 1.f);
    renderData.cameraData.look = glm::vec4(parser.point_2 - parser.point_1, 0.f);
    renderData.cameraData.up = glm::vec4(0.f, 1.f, 0.f, 0.f);

    m_defaultFBO = 2;
    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width = m_screen_width;
    m_fbo_height = m_screen_height;

    // Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    // Allows OpenGL to draw objects appropriately on top of one another
    glEnable(GL_DEPTH_TEST);
    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    m_shader = ShaderLoader::createShaderProgram(":/resources/shaders/default.vert",
                                                 ":/resources/shaders/default.frag");

    m_depth_shader = ShaderLoader::createShaderProgram(":/resources/shaders/depthmap.vert", ":/resources/shaders/depthmap.frag");


    /* Additions for texture mapping */
    QString stone_filepath = QString(":/resources/images/stone_texture.png");
    m_image = QImage(stone_filepath);
    m_image = m_image.convertToFormat(QImage::Format_RGBA8888).mirrored();
    glGenTextures(1, &m_stone_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_stone_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_image.width(), m_image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_image.bits());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(m_shader);
    glUniform1i(glGetUniformLocation(m_shader, "stoneSampler"), 0);
    glUseProgram(0);


    std::vector<GLfloat> fullscreen_quad_data =
    { //     POSITIONS    //
        -1.f, 1.f, 0.0f,
         0.f, 1.f, 0.f,
        -1.f, -1.f, 0.0f,
         0.f, 0.f, 0.f,
         1.f, -1.f, 0.0f,
         1.f, 0.f, 0.f,
         1.f,  1.f, 0.0f,
         1.f, 1.f, 0.f,
        -1.f,  1.f, 0.0f,
         0.f, 1.f, 0.f,
         1.f, -1.f, 0.0f,
         1.f, 0.f, 0.f
    };

    // Generate and bind a VBO and a VAO for a fullscreen quad
    glGenBuffers(1, &m_fullscreen_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);
    glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size()*sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_fullscreen_vao);
    glBindVertexArray(m_fullscreen_vao);

    // Task 14: modify the code below to add a second attribute to the vertex attribute array
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(sizeof(GLfloat)*3));

    // Unbind the fullscreen quad's VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    renderData.cameraData.updateViewMatrix(renderData.cameraData.up,
            renderData.cameraData.look, renderData.cameraData.pos);
    createOffsets();

    // Pass camera data to Bezier class
    m_bezier.setCameraData(&renderData.cameraData);

    DepthMappingFBO();
}

const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;


void Realtime::DepthMappingFBO(){
    // Generate and bind an empty texture, set its min/mag filter interpolation, then unbind

    glGenTextures(1, &m_depth_map_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_depth_map_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindTexture(GL_TEXTURE_2D, 0);


    // Set the texture.frag uniform for our texture
    glUseProgram(m_shader);
    glUniform1i(glGetUniformLocation(m_shader, "shadowMap"),1);
    glUseProgram(0);


    // Generate and bind an FBO

    glGenFramebuffers(1, &m_depth_map_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_depth_map_fbo);

    // Adding my texture as a color attachment, and my renderbuffer as a depth+stencil attachment, to my FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth_map_texture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // Unbind the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
}

void Realtime::handleObjects() {
    std::vector<float>* vbo_cube;
    //Create 4 vbos -- one for each shape
    vbo_cube = m_cube.generateShape();
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * numInstances, &translations[0], GL_STATIC_DRAW);
        glBufferData(GL_ARRAY_BUFFER,vbo_cube->size() * sizeof(GLfloat),vbo_cube->data(), GL_STATIC_DRAW);
        glBindVertexArray(m_vao);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8*sizeof(GLfloat),reinterpret_cast<void *>(0)); // Edited for uv coordinates
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(GLfloat),reinterpret_cast<void*>(sizeof(GLfloat)*3));  // Edited for uv coordinates
        /* Added new attribute for uv coordinates */
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3,2,GL_FLOAT,GL_FALSE,8*sizeof(GLfloat),reinterpret_cast<void*>(sizeof(GLfloat)*6));

        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glGenBuffers(1, &instance_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 100, &translations[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, instance_vbo);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribDivisor(2, 1);
}

void Realtime::createOffsets() {
    int index = 0;
    for (RenderShapeData shape: renderData.shapes) {
        translations[index] = shape.position;
        index = index + 1;
    }
}

void Realtime::loadLights() {
    int i = 0;
    glUniform1i(glGetUniformLocation(m_shader, "shadow_map_toggle"),settings.extraCredit1);

    glUniform1i(glGetUniformLocation(m_shader,"total_lights"), renderData.lights.size());


    float near_plane = 1.0f, far_plane = 7.5f;
    glm::vec3 lightInvDir = renderData.lights[0].dir;

     // Compute the MVP matrix from the light's point of view
     glm::mat4 depthProjectionMatrix = glm::ortho<float>(-10,10,-10,10,near_plane, far_plane);
     glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0,0,0), glm::vec3(0,1,0));
     glm::mat4 depthModelMatrix = glm::mat4(1.0);
     glm::mat4 lightSpaceMatrix = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;

    std::string lightMatrixStr = "lightSpaceMatrix";
    GLint loc8 = glGetUniformLocation(m_shader, lightMatrixStr.c_str());
    glUniformMatrix4fv(loc8, 1, GL_FALSE, &lightSpaceMatrix[0][0]);

    for (SceneLightData light: renderData.lights) {
        std::string arg_1 = "c_light[" + std::to_string(i) + "]";
        GLint c_light_loc = glGetUniformLocation(m_shader,arg_1.data());
        glUniform4f(c_light_loc, light.color[0], light.color[1], light.color[2], light.color[3]);
        std::string arg_2 = "m_lightDir[" + std::to_string(i) + "]";
        GLint light_loc = glGetUniformLocation(m_shader, arg_2.data());
        glUniform4f(light_loc, light.dir[0],light.dir[1],light.dir[2], light.dir[3]);
        std::string arg_3 = "light_type[" + std::to_string(i) + "]";
        GLint light_type_loc = glGetUniformLocation(m_shader,arg_3.data());
        std::string arg_4 = "atten[" + std::to_string(i) + "]";
        GLint atten_loc = glGetUniformLocation(m_shader,arg_4.data());
        std::string arg_5 = "m_lightPos[" + std::to_string(i) + "]";
        GLint light_pos_loc = glGetUniformLocation(m_shader,arg_5.data());
        std::string arg_6 = "m_angle[" + std::to_string(i) + "]";
        GLint light_angle_loc = glGetUniformLocation(m_shader,arg_6.data());
        std::string arg_7 = "m_penumbra[" + std::to_string(i) + "]";
        GLint light_penumbra_loc = glGetUniformLocation(m_shader,arg_7.data());


        float near_plane = 1.0f, far_plane = 7.5f;

        glm::vec3 lightInvDir = glm::vec3(renderData.lights[0].dir);

         // Compute the MVP matrix from the light's point of view
         glm::mat4 depthProjectionMatrix = glm::ortho<float>(-10,10,-10,10,near_plane, far_plane);
         glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0,0,0), glm::vec3(0,1,0));
         glm::mat4 depthModelMatrix = glm::mat4(1.0);
         glm::mat4 lightSpaceMatrix = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;

        std::string lightMatrixStr = "lightSpaceMatrix";
        GLint loc8 = glGetUniformLocation(m_shader, lightMatrixStr.c_str());
        glUniformMatrix4fv(loc8, 1, GL_FALSE, &lightSpaceMatrix[0][0]);

        if (light.type == LightType::LIGHT_DIRECTIONAL) {
            glUniform1i(light_type_loc, 0);
        } else if (light.type == LightType::LIGHT_POINT) {
            glUniform1i(light_type_loc, 1);
            glUniform3f(atten_loc, light.function[0], light.function[1], light.function[2]);
            glUniform4f(light_pos_loc, light.pos[0], light.pos[1], light.pos[2], light.pos[3]);
        } else if (light.type == LightType::LIGHT_SPOT) {
            glUniform1i(light_type_loc, 2);
            glUniform3f(atten_loc, light.function[0], light.function[1], light.function[2]);
            glUniform4f(light_pos_loc, light.pos[0], light.pos[1], light.pos[2], light.pos[3]);
            glUniform4f(light_loc,light.dir[0],light.dir[1],light.dir[2], light.dir[3]);
            glUniform1f(light_angle_loc, light.angle);
            glUniform1f(light_penumbra_loc, light.penumbra);
        }
        i = i + 1;
    }
}

void Realtime::renderShapes() {
    // Initialize ctm to identity matrix
    glm::mat4 ctm = glm::mat4(1,0,0,0,
                              0,1,0,0,
                              0,0,1,0,
                              0,0,0,1);
    for (int i = 0; i < renderData.shapes.size(); i += numInstances) {
        for (unsigned int j = 0; j < numInstances; j++) {
            std::string arg = "offsets[" + std::to_string(j) + "]";
            glUniform3fv(glGetUniformLocation(m_shader, arg.data()), 1, &translations[i + j][0]);
        }
        glBindVertexArray(m_vao);
        // Pass model matrix to shader program
        // Pass view and projection matrices
        GLint view_loc = glGetUniformLocation(m_shader, "view_matrix");
        glUniformMatrix4fv(view_loc, 1, GL_FALSE, &renderData.cameraData.m_view[0][0]);
        GLint proj_loc = glGetUniformLocation(m_shader, "proj_matrix");
        glUniformMatrix4fv(proj_loc, 1, GL_FALSE, &m_proj[0][0]);
        // Pass ambient parameters
        GLint m_ka_loc = glGetUniformLocation(m_shader, "m_ka");
        glUniform1f(m_ka_loc, renderData.globalData.ka);
        GLint c_amb_loc = glGetUniformLocation(m_shader,"c_ambient");
        glUniform3fv(c_amb_loc, 1, &renderData.material.cAmbient[0]);
        // Pass diffuse parameters
        GLint m_kd_loc = glGetUniformLocation(m_shader, "m_kd");
        glUniform1f(m_kd_loc, renderData.globalData.kd);
        GLint c_diff_loc = glGetUniformLocation(m_shader,"c_diffuse");
        glUniform3fv(c_diff_loc, 1, &renderData.material.cDiffuse[0]);
        // Pass specular parameters
        GLint m_ks_loc = glGetUniformLocation(m_shader, "m_ks");
        glUniform1f(m_ks_loc, renderData.globalData.ks);
        GLint c_specular_loc = glGetUniformLocation(m_shader, "c_specular");
        glUniform3fv(c_specular_loc, 1, &renderData.material.cSpecular[0]);
        GLint m_shin_loc = glGetUniformLocation(m_shader, "m_shin");
        glUniform1f(m_shin_loc, renderData.material.shininess);
        glm::vec4 world_cam = renderData.cameraData.pos;
        GLint m_cam_loc = glGetUniformLocation(m_shader, "cam_pos");

        /* Note: added for texture mapping */
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_stone_texture);

        // Draw Command
        glDrawArraysInstanced(GL_TRIANGLES, 0, m_cube.generateShape()->size() / 3, numInstances);
        // Unbind VBO
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void Realtime::renderZBuffer(){

    for (int i = 0; i < renderData.shapes.size(); i += numInstances) {
        for (unsigned int j = 0; j < numInstances; j++) {
            std::string arg = "offsets[" + std::to_string(j) + "]";
            glUniform3fv(glGetUniformLocation(m_shader, arg.data()), 1, &translations[i + j][0]);
        }
        glBindVertexArray(m_vao);
        // Pass model matrix to shader program
        // Pass view and projection matrices

        glm::vec3 lightInvDir = renderData.lights[0].dir;
        float near_plane = 1.0f, far_plane = 7.5f;

         // Compute the MVP matrix from the light's point of view
         glm::mat4 depthProjectionMatrix = glm::ortho<float>(-10,10,-10,10,near_plane, far_plane);
         glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0,0,0), glm::vec3(0,1,0));
         glm::mat4 depthModelMatrix = glm::mat4(1.0);
         glm::mat4 lightSpaceMatrix = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;


        std::string dirStr = "lightSpaceMatrix";
        GLint loc1 = glGetUniformLocation(m_depth_shader, dirStr.c_str());
        glUniformMatrix4fv(loc1, 1, GL_FALSE, &lightSpaceMatrix[0][0]);

        // Draw Command
        glDrawArraysInstanced(GL_TRIANGLES, 0, m_cube.generateShape()->size() / 3, numInstances);
        // Unbind VBO
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void Realtime::paintGL() {


    glBindFramebuffer(GL_FRAMEBUFFER, m_depth_map_fbo);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glClear(GL_DEPTH_BUFFER_BIT);

    m_cube.updateParams(2);
    // Generate VAOs and VBOs
    glGenBuffers(1, &m_vbo);
    glGenVertexArrays(1, &m_vao);
    handleObjects();

    glUseProgram(m_depth_shader);
    renderZBuffer();

    // Unbind Vertex Array
    glBindVertexArray(0);

    glUseProgram(0);


    // Bind the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glViewport(0, 0, m_screen_width, m_screen_height);

   // Clear the color and depth buffers

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_depth_map_texture);

    m_cube.updateParams(2);
    // Generate VAOs and VBOs
    glGenBuffers(1, &m_vbo);
    glGenVertexArrays(1, &m_vao);
    handleObjects();

    // Create shader program
    glUseProgram(m_shader);
    loadLights();
    renderShapes();

    glBindTexture(GL_TEXTURE_2D, 0); /* Note: added for texture mapping */

    // Unbind Vertex Array
    glBindVertexArray(0);
    // Deactivate shader program
    glUseProgram(0);
}

void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);
    // Students: anything requiring OpenGL calls when the program starts should be done here
    renderData.cameraData.aspectRatio = float(w) / float(h);
    m_asp_ratio = float(w) / float(h);;
    renderData.cameraData.updateProjMatrix(settings.nearPlane, settings.farPlane);
    m_proj = renderData.cameraData.m_proj;

    glDeleteTextures(1, &m_depth_map_texture);
    glDeleteFramebuffers(1, &m_depth_map_fbo);


    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width = size().width();
    m_fbo_height = size().height();
    // Regenerate the FBO
    //makeFBO();

    DepthMappingFBO();
}

void Realtime::sceneChanged() {
    SceneParser parser;
    parser.parse(renderData, settings.shapeParameter1, settings.shapeParameter2);
    renderData.cameraData.aspectRatio = m_asp_ratio;
    renderData.cameraData.updateProjMatrix(settings.nearPlane, settings.farPlane);
    m_proj = renderData.cameraData.m_proj;
    renderData.cameraData.updateViewMatrix(renderData.cameraData.up, renderData.cameraData.look, renderData.cameraData.pos);
    updateShapes();
    update(); // asks for a PaintGL() call to occur
}

void Realtime::settingsChanged() {
    SceneParser parser;
    parser.parse(renderData, settings.shapeParameter1, settings.shapeParameter2);
    renderData.cameraData.updateProjMatrix(settings.nearPlane, settings.farPlane);
    createOffsets();
    m_proj = renderData.cameraData.m_proj;
    m_cube.updateParams(2);
    update(); // asks for a PaintGL() call to occur
}


glm::mat4 rotation_matrix(float theta, glm::vec3 axis) {
    float sin_t = glm::sin(theta);
    float cos_t = glm::cos(theta);
    axis = glm::normalize(axis);
    float ux = axis[0];
    float uy = axis[1];
    float uz = axis[2];
    return glm::mat4(cos_t + ux*ux*(1-cos_t), ux*uy*(1-cos_t) - uz*sin_t, ux*uz*(1-cos_t) + uy*sin_t, 0,
                     ux*uy*(1-cos_t)+uz*sin_t, cos_t + uy*uy*(1-cos_t), uy*uz*(1-cos_t) - ux*sin_t, 0,
                     ux*uz*(1-cos_t)-uy*sin_t, uy*uz*(1-cos_t)+ux*sin_t, cos_t+ux*ux*(1-cos_t), 0,
                     0,0,0,1);
}

void rotate_x(float x) {
    float theta = 1.f*x / 100.f;
    glm::vec3 axis = glm::vec3(0,1,0);
    glm::mat4 r_mat = rotation_matrix(theta, axis);
    renderData.cameraData.look = r_mat*renderData.cameraData.look;
    renderData.cameraData.up = r_mat*renderData.cameraData.up;
    renderData.cameraData.updateViewMatrix(renderData.cameraData.up,
            renderData.cameraData.look, renderData.cameraData.pos);
}

void rotate_y(float y) {
    float theta = 1.f*y / 100.f;
    glm::vec3 axis = glm::cross(glm::vec3(glm::normalize(renderData.cameraData.look)),
                                     glm::vec3(glm::normalize(renderData.cameraData.up)));
    glm::mat4 r_mat = rotation_matrix(theta, axis);
    renderData.cameraData.look = r_mat*renderData.cameraData.look;
    renderData.cameraData.up = r_mat*renderData.cameraData.up;
    renderData.cameraData.updateViewMatrix(renderData.cameraData.up,
            renderData.cameraData.look, renderData.cameraData.pos);
}

void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

void Realtime::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        // Use deltaX and deltaY here to rotate
        if (m_mouseDown) {
            rotate_x(deltaX);
            rotate_y(deltaY);
        }

        update(); // asks for a PaintGL() call to occur
    }
}

void press_w(float time) {
    glm::vec4 trans = 1.f*glm::normalize(renderData.cameraData.look)*5.f*time;
    glm::mat4 m_trans = glm::mat4(1.f,0.f,0.f,0.f,
                                  0.f,1.f,0.f,0.f,
                                  0.f,0.f,1.f,0.f,
                                  trans[0], trans[1], trans[2], 1.f);
    renderData.cameraData.pos = m_trans*renderData.cameraData.pos;
    renderData.cameraData.updateViewMatrix(renderData.cameraData.up,
            renderData.cameraData.look, renderData.cameraData.pos);
}

void press_s(float time) {
    glm::vec4 trans = -1.f*glm::normalize(renderData.cameraData.look)*5.f*time;
    glm::mat4 m_trans = glm::mat4(1.f,0.f,0.f,0.f,
                                  0.f,1.f,0.f,0.f,
                                  0.f,0.f,1.f,0.f,
                                  trans[0], trans[1], trans[2], 1.f);
    renderData.cameraData.pos = m_trans*renderData.cameraData.pos;
    renderData.cameraData.updateViewMatrix(renderData.cameraData.up,
            renderData.cameraData.look, renderData.cameraData.pos);
}

void press_a(float time) {
    glm::vec4 trans = -1.f*glm::vec4(glm::cross(glm::vec3(glm::normalize(renderData.cameraData.look)),
                                 glm::vec3(glm::normalize(renderData.cameraData.up))), 1.f)*5.f*time;
    glm::mat4 m_trans = glm::mat4(1.f,0.f,0.f,0.f,
                                  0.f,1.f,0.f,0.f,
                                  0.f,0.f,1.f,0.f,
                                  trans[0], trans[1], trans[2], 1.f);
    renderData.cameraData.pos = m_trans*renderData.cameraData.pos;
    renderData.cameraData.updateViewMatrix(renderData.cameraData.up,
            renderData.cameraData.look, renderData.cameraData.pos);
}

void press_d(float time) {
    glm::vec4 trans = glm::vec4(glm::cross(glm::vec3(glm::normalize(renderData.cameraData.look)),
                                 glm::vec3(glm::normalize(renderData.cameraData.up))), 1.f)*5.f*time;
    glm::mat4 m_trans = glm::mat4(1.f,0.f,0.f,0.f,
                                  0.f,1.f,0.f,0.f,
                                  0.f,0.f,1.f,0.f,
                                  trans[0], trans[1], trans[2], 1.f);
    renderData.cameraData.pos = m_trans*renderData.cameraData.pos;
    renderData.cameraData.updateViewMatrix(renderData.cameraData.up,
            renderData.cameraData.look, renderData.cameraData.pos);
}

void press_space(float time) {
    glm::vec4 trans = glm::vec4(0.f,1.f,0.f,0.f)*5.f*time;
    glm::mat4 m_trans = glm::mat4(1.f,0.f,0.f,0.f,
                                  0.f,1.f,0.f,0.f,
                                  0.f,0.f,1.f,0.f,
                                  trans[0], trans[1], trans[2], 1.f);
    renderData.cameraData.pos = m_trans*renderData.cameraData.pos;
    renderData.cameraData.updateViewMatrix(renderData.cameraData.up,
            renderData.cameraData.look, renderData.cameraData.pos);
}

void press_ctrl(float time) {
    glm::vec4 trans = -1.f*glm::vec4(0.f,1.f,0.f,0.f)*5.f*time;
    glm::mat4 m_trans = glm::mat4(1.f,0.f,0.f,0.f,
                                  0.f,1.f,0.f,0.f,
                                  0.f,0.f,1.f,0.f,
                                  trans[0], trans[1], trans[2], 1.f);
    renderData.cameraData.pos = m_trans*renderData.cameraData.pos;
    renderData.cameraData.updateViewMatrix(renderData.cameraData.up,
            renderData.cameraData.look, renderData.cameraData.pos);
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // Use deltaTime and m_keyMap here to move around
    if (m_keyMap[Qt::Key_W]) {
        press_w(deltaTime);
    }
    if (m_keyMap[Qt::Key_S]) {
        press_s(deltaTime);
    }
    if (m_keyMap[Qt::Key_A]) {
        press_a(deltaTime);
    }
    if (m_keyMap[Qt::Key_D]) {
        press_d(deltaTime);
    }
    if (m_keyMap[Qt::Key_Space]) {
        press_space(deltaTime);
    }
    if (m_keyMap[Qt::Key_Control]) {
        press_ctrl(deltaTime);
    }
    /* Testing Bezier (can delete later) */
    else if (m_keyMap[Qt::Key_F]) {
        m_bezier.updatePos(point_1, point_2, point_3, point_4, m_bezierInc);
        m_bezierInc += 0.0012;
    }

    update(); // asks for a PaintGL() call to occur
}
