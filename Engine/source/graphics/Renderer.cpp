// OpenGL draw API (clear, drawSprite, debug lines)

#include "graphics/Renderer.h"
#include <GLFW/glfw3.h>   // brings gl.h on Windows
#include <cmath>
#include <glm/gtc/constants.hpp>

namespace {
    constexpr float TWO_PI = 2.0f * glm::pi<float>();

    // helper used only inside this file: filled circle via triangle fan
    inline void filledCircle(float cx, float cy, float r, int segments = 64) {
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (int i = 0; i <= segments; ++i) {
            float a = (i * TWO_PI) / static_cast<float>(segments);
            glVertex2f(cx + r * std::cos(a), cy + r * std::sin(a));
        }
        glEnd();
    }
}

Renderer::Renderer(const char* vertex_path, const char* fragment_path):render_shader(vertex_path,fragment_path) {
    //vertex position->where to render on screen(-1.0 to +1.0)
    //texture UV->how to render texture(0.0 to 1.0)
    float vertices[] = {
    //vertex positions(x,y)  texture UV
    -1.0f, 1.0f,             0.0f, 1.0f,  //top left
    -1.0f, -1.0f,            0.0f, 0.0f,  //bottom left
    1.0f, 1.0f,              1.0f, 1.0f,  //top right 
    1.0f, -1.0f,             1.0f, 0.0f,  //bottom right 
    };

    //glGenVertexArrays returns the number of Vertex Array Objects(VAO) and store its unique ID in VAO_arr
    glGenVertexArrays(1, &VAO_arr);
    glBindVertexArray(VAO_arr); //glBindVertexArray binds the vertex array object with VAO_arr

    //glGenBuffers generates the number of Vertex Buffer Objects(VBO) and store its unique ID in VBO_arr
    glGenBuffers(1, &VBO_arr);
    //GL_ARRAY_BUFFER->vertex buffer that contains positions, colors, texture coordinates
    glBindBuffer(GL_ARRAY_BUFFER, VBO_arr); //glBindBuffer binds GL_ARRAY_BUFFER with VBO_arr
    //Assign vertex data stored in vertices to the currently bound vertex buffer(GL_ARRAY_BUFFER) 
    //the function allocates sizeof(vertices) to GPU
    //GL_STATIC_DRAW->The data store contents will be modified once and used many times and used for rendering
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //Position attribute in background VERT file(location = 0)
    //The function will go to location 0 and read 2 float data type(8 bytes) starting from first element index postion
    //Then it will jump 4 * sizeof(float) to get to next vertex's position, GL_FALSE->DO NOT NORMALIZE
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0); //how to read
    glEnableVertexAttribArray(0); //use location=0 for rendering
    //Texture coordinate attribute in background VERT file(location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1); //use location=1 for rendering
    glBindVertexArray(0); //unbinds the currently bound VAO
}

void Renderer::background_render(GLuint texture_ID, const GLchar* uniform_VARNAME)
{
    render_shader.user_Shader_program(); //use this shader program for rendering background image
    glActiveTexture(GL_TEXTURE0); //specifies which texture unit to make active
    glBindTexture(GL_TEXTURE_2D, texture_ID); //bind this current texture to texturing target
    //glGetUniformLocation takes in ID of shader program object AND uniform_VARNAME
    //to return the location of a uniform variable
    //glUniform1i takes in location of a uniform variable and 0(this means texture unit is GL_TEXTURE0)
    glUniform1i(glGetUniformLocation(render_shader.get_programID(), uniform_VARNAME), 0);

    glBindVertexArray(VAO_arr); //Bind Vertex Array Object to background mesh
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); //Render rectangle(4 vertices = 2 triangles)
    glBindVertexArray(0); //unbinds the currently bound VAO
    glUseProgram(0); //unbind the currently active shader program and go back to fixed-function pipeline mode
}

void Renderer::init2D(int w, int h) {
    resize(w, h);                 // set pixel-space projection

    // sensible defaults for 2D
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // ensure we’re not in wireframe
}

void Renderer::resize(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, h, 0, -1, 1);   // left, right, bottom, top, near, far
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Renderer::clear(float r, float g, float b) {
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::tri(float x1, float y1, float x2, float y2, float x3, float y3) {
    glLoadIdentity();
    glBegin(GL_TRIANGLES);
    glColor3f(1.f, 0.f, 0.f); glVertex2f(x1, y1);
    glColor3f(0.f, 1.f, 0.f); glVertex2f(x2, y2);
    glColor3f(0.f, 0.f, 1.f); glVertex2f(x3, y3);
    glEnd();
}

void Renderer::rect(float x, float y, float w, float h) {
    glLoadIdentity();
    glBegin(GL_TRIANGLES); // two triangles make a quad
    glColor3f(0.9f, 0.8f, 0.2f);
    glVertex2f(x, y); glVertex2f(x + w, y); glVertex2f(x + w, y + h);
    glVertex2f(x, y); glVertex2f(x + w, y + h); glVertex2f(x, y + h);
    glEnd();
}

void Renderer::line(float x1, float y1, float x2, float y2) {
    glLoadIdentity();
    glBegin(GL_LINES);
    glColor3f(1.f, 1.f, 1.f);
    glVertex2f(x1, y1); glVertex2f(x2, y2);
    glEnd();
}

void Renderer::rectOutline(float x, float y, float w, float h) {
    glLoadIdentity();
    glBegin(GL_LINE_LOOP);
    glColor3f(1.f, 1.f, 1.f);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void Renderer::circle(float cx, float cy, float r, int segments) {
    glLoadIdentity();
    glBegin(GL_LINE_LOOP);
    glColor3f(1.f, 1.f, 1.f);
    for (int i = 0; i < segments; ++i) {
        float a = (i * TWO_PI) / static_cast<float>(segments);
        glVertex2f(cx + r * std::cos(a), cy + r * std::sin(a));
    }
    glEnd();
}

void Renderer::initFBO(int width, int height) {
    fboWidth = width;
    fboHeight = height;

    // Delete old FBO/Texture if they exist
    if (viewportFBO)     glDeleteFramebuffers(1, &viewportFBO);
    if (viewportTexture) glDeleteTextures(1, &viewportTexture);

    glGenFramebuffers(1, &viewportFBO);
    glGenTextures(1, &viewportTexture);

    glBindTexture(GL_TEXTURE_2D, viewportTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fboWidth, fboHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, viewportFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, viewportTexture, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind
}

void Renderer::bindFBO() const {
    glBindFramebuffer(GL_FRAMEBUFFER, viewportFBO);
    glViewport(0, 0, fboWidth, fboHeight);
}
void Renderer::unbindFBO() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint Renderer::getFBOTexture() const {
    return viewportTexture;
}


void Renderer::drawViewportShapes(int w, int h) {
    // make sure our 2D projection matches the target size
    resize(w, h);

    // 2D pass: fixed pipeline + no depth
    glUseProgram(0);
    glDisable(GL_DEPTH_TEST);

    // RGB triangle (center/top)
    tri(w * 0.50f, h * 0.20f,
        w * 0.20f, h * 0.80f,
        w * 0.80f, h * 0.80f);

    // filled blue circle on the right
    {
        float r = h * 0.10f;
        float margin = w * 0.05f;
        float cx = w - (r + margin);
        if (cx < r) cx = r;
        float cy = h * 0.20f;

        glColor3f(0.f, 0.5f, 1.f);
        filledCircle(cx, cy, r, 64);
    }

    // white outline box (bottom-left)
    {
        float pad = w * 0.02f;
        float bw = w * 0.16f;
        float bh = h * 0.17f;
        rectOutline(pad, pad, bw, bh);
    }
}