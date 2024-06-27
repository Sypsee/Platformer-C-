#include "gl_renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "render_interface.h"

struct GLContext
{
    GLuint programID;
    GLuint textureId;
    GLuint transformSBOID;
    GLuint screenSizeID;
    GLuint orthoProjectionID;
};


static GLContext glContext;

static void APIENTRY gl_debug_callback(GLenum src, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, const void *user)
{
    if (severity == GL_DEBUG_SEVERITY_LOW ||
        severity == GL_DEBUG_SEVERITY_MEDIUM ||
        severity == GL_DEBUG_SEVERITY_HIGH)
    {
        GM_ASSERT(false, "OpenGL Error : %s", msg);
    }
    else
    {
        GM_TRACE((char*)msg);
    }
}

bool gl_init(BumpAllocator *transientStorage)
{
    load_gl_functions();

    glDebugMessageCallback(&gl_debug_callback, nullptr);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glEnable(GL_DEBUG_OUTPUT);

    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    int fileSize = 0;
    char* vertexShader = read_file("assets/shaders/quad.vert", &fileSize, transientStorage);
    char* fragmentShader = read_file("assets/shaders/quad.frag", &fileSize, transientStorage);

    if (!vertexShader || !fragmentShader)
    {
        GM_ASSERT(false, "Failed to load shaders!");
        return false;
    }

    glShaderSource(vertexShaderId, 1, &vertexShader, 0);
    glShaderSource(fragmentShaderId, 1, &fragmentShader, 0);

    glCompileShader(vertexShaderId);
    glCompileShader(fragmentShaderId);

    // Test if we have compiled the vertex shader successfully or not
    {
        int success;
        char shaderLog[2048] = {};

        glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShaderId, 2048, 0, shaderLog);
            GM_ASSERT(false, "Failed to Compile Vertex Shader : %s", shaderLog);
        }
    }

    // Test if we have compiled the fragment shader successfully or not
    {
        int success;
        char shaderLog[2048] = {};

        glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShaderId, 2048, 0, shaderLog);
            GM_ASSERT(false, "Failed to Compile Vertex Shader : %s", shaderLog);
        }
    }

    glContext.programID = glCreateProgram();
    glAttachShader(glContext.programID, vertexShaderId);
    glAttachShader(glContext.programID, fragmentShaderId);
    glLinkProgram(glContext.programID);


    glDetachShader(glContext.programID, vertexShaderId);
    glDetachShader(glContext.programID, fragmentShaderId);
    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Texture Loading using STBI
    {
        int width, height, channels;
        char* data = (char*)stbi_load("assets/textures/TEXTURE_ATLAS.png", &width, &height, &channels, 4);

        if (!data)
        {
            GM_ASSERT(false, "Failed to load texture");
            return false;
        }

        glGenTextures(1, &glContext.textureId);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, glContext.textureId);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }

    // Transform Storage Buffer
    {
        glGenBuffers(1, &glContext.transformSBOID);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, glContext.transformSBOID);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Transform) * MAX_TRANSFORMS,
                    renderData->transforms, GL_DYNAMIC_DRAW);
    }

    // Uniforms
    {
        glContext.screenSizeID = glGetUniformLocation(glContext.programID, "screenSize");
        glContext.orthoProjectionID = glGetUniformLocation(glContext.programID, "orthoProjection");
    }

    glEnable(GL_FRAMEBUFFER_SRGB);
    glDisable(0x809D);

    // Depth Testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);

    glUseProgram(glContext.programID);

    return true;
}

void gl_render()
{
    glClearColor(119.0f / 255.f, 33.0f / 255.0f, 111.0f / 255.0f, 1.0f);
    glClearDepth(0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, input->screenSizeX, input->screenSizeY);

    // Copy screen size to the GPU
    Vec2 screenSize = {(float)input->screenSizeX, (float)input->screenSizeY};
    glUniform2fv(glContext.screenSizeID, 1, &screenSize.x);

    // Orthographic projection
    OrthographicCamera2D camera = renderData->gameCamera;
    Mat4 orthoProjection = orthographic_projection(camera.pos.x - camera.dimensions.x / 2.0f, 
                                                   camera.pos.x + camera.dimensions.x / 2.0f, 
                                                   camera.pos.y - camera.dimensions.y / 2.0f, 
                                                   camera.pos.y + camera.dimensions.y / 2.0f);

    glUniformMatrix4fv(glContext.orthoProjectionID, 1, GL_FALSE, &orthoProjection.ax);

    {
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(Transform) * renderData->transformCount,
                        renderData->transforms);
        
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, renderData->transformCount);

        // Reset for next frame
        renderData->transformCount = 0;
    }
}