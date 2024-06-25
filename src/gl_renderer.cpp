#include "gl_renderer.h"

struct GLContext
{
    GLuint programID;
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
    glViewport(0, 0, input.screenSizeX, input.screenSizeY);


    glDrawArrays(GL_TRIANGLES, 0, 6);
}