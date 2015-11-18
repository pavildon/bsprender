#define GLFW_INCLUDE_GLCOREARB

#include <random>
#include <thread>
#include <OpenGL/OpenGL.h>


#include "common.h"

#include "shaders.h"
#include "simple_cube.h"
#include "input.h"
#include "shader_sources.h"
#include "bsp/bsp.h"
#include "Benchmarks.h"


using namespace std;

int Entity::count = 0;

using EntityP = unique_ptr<Entity>;

//GLFWwindow *_window;
SDL_Window *_window;

ShaderInfo shaderInfo;
ShaderInfo tessShaderInfo;

vector<EntityP> entities;

vector<Model> models;
GLuint vao = 99999;

GLuint vao_tess = 999999;
Entity camera;

glm::vec3 upVector = glm::vec3 {0.0f, 1.0f, 0.0f};

Matrices matrices;
chrono::high_resolution_clock::time_point newTime;
chrono::high_resolution_clock::time_point oldTime;

chrono::milliseconds frameTime;
std::random_device rd;
std::default_random_engine e1(rd());

std::uniform_int_distribution<int> uniform_dist(-50, 90);

BSP_P bspMap{nullptr};

Frustum frustum;

GLint currentTex = 99999;
GLint currentRealTex = 99999;


bool showset = false;

bool shouldExit = false;

bool comparePlaneBox(const glm::vec4 &plane, const glm::vec3 &mins, const glm::vec3 &maxs);


extern unsigned long maxFacesSize;

int main() {
    if (!initGLFW()) return -1;
    if (!loadShaders()) return -1;

    cout << models.size() << endl;
//    setupInput();

    oldTime = chrono::high_resolution_clock::now();

    BSP::loadBSP("fi_ctf1m.bsp");

    unsigned long timeCounter = 0;
    int fps = 0;

    glUseProgram(shaderInfo.shader_program);
    glBindVertexArray(bspMap->vao);
    long long maxtimeframe = (long long int) (1000.0f / 120.0f);
    while (!shouldExit) {
        Benchmarks::start("0TODO");
        newTime = chrono::high_resolution_clock::now();
        frameTime = chrono::duration_cast<chrono::milliseconds>(newTime - oldTime);

        while(frameTime.count() < maxtimeframe) {
            std::this_thread::sleep_for(chrono::nanoseconds(10));
            newTime = chrono::high_resolution_clock::now();
            frameTime = chrono::duration_cast<chrono::milliseconds>(newTime - oldTime);
        }

        timeCounter += frameTime.count();
        fps++;
        if (timeCounter > 1000) {
            cout << "FPS: " << fps << " Max Faces Size = " << maxFacesSize << endl;
//            " Position : " << glm::to_string(camera.position) << endl;
            timeCounter = timeCounter - 1000;
            fps = 0;
        }

        inputFrame();
        setupCamera();

        think();
        draw();

        SDL_GL_SwapWindow(_window);
        oldTime = newTime;
        Benchmarks::stop("0TODO");

    }

    Benchmarks::results();

    return 0;
}

void think() {

    for (EntityP &e: entities) {
        e->think();
    }

}

void draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    BSP::drawMap();
}

void setupCamera() {

//    camera.updateVectors();

    camera.think();

    matrices.view = glm::lookAt(
            camera.position,
            camera.front + camera.position,
            glm::vec3(0.0f, 1.0f, 0.0f)
    );

    calcFrustum();

}

bool initGLFW() {

//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//
//    glfwWindowHint(GLFW_SAMPLES, 4);
//
//    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
//    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
//    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
//    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
//    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
//    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
//    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
//    cout << mode->width << " " << mode->height << endl;

    int width = 1024;
    int height = 768;



//    _window = glfwCreateWindow(width, height, "tess",NULL, NULL);
//
//    if (_window == nullptr) return false;
//
//    glfwMakeContextCurrent(_window);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        cout << "Failed to init SDL Video" << endl;
    };

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
//
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);


    _window = SDL_CreateWindow("BSP Test", 0, 0, width, height,
                                SDL_WINDOW_OPENGL);

    if (_window == nullptr) {
        SDL_Quit();
        return false;
    }

    SDL_GLContext ctx = SDL_GL_CreateContext(_window);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);


    int ee = 0;
    glGetIntegerv(GL_MAX_PATCH_VERTICES, &ee);
    cout << "Max patch vertices : " << ee << endl;

//    glPatchParameteri(GL_PATCH_VERTICES, 9);

    cout << "GL ERROR LOAD  :" << glGetError() << endl;

    glClearColor(0.0f, 0.0f, 0.01f, 1.0f);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_LINE_SMOOTH);

    glEnable (GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,   GL_ONE_MINUS_SRC_ALPHA   );

    SDL_GL_SetSwapInterval(0);

    matrices.proj = glm::perspective(glm::radians(90.0f), (float) width / (float) height, 1.0f, 4000.0f);

    camera.position = glm::vec3{-41.415211f, 320.293121f, -537.225281f};

    return true;
}

bool loadShaders() {

    auto shader1 = _loadShader(&shaderInfo.vertex_shader, GL_VERTEX_SHADER, vertex_shader_source) &&
                   _loadShader(&shaderInfo.fragment_shader, GL_FRAGMENT_SHADER, fragment_shader_source) &&
                   //           _loadShader(&shaderInfo.tcs_shader, GL_TESS_CONTROL_SHADER, tcs_shader_source) &&
                   //           _loadShader(&shaderInfo.tes_shader, GL_TESS_EVALUATION_SHADER, tes_shader_source) &&
                   loadShaderProgram();

    auto shader2 = _loadShader(&tessShaderInfo.vertex_shader, GL_VERTEX_SHADER, vertex_shader_source_2) &&
                   _loadShader(&tessShaderInfo.fragment_shader, GL_FRAGMENT_SHADER, fragment_shader_source) &&
                   //                              _loadShader(&tessShaderInfo.tcs_shader, GL_TESS_CONTROL_SHADER, tcs_shader_source) &&
                   //                              _loadShader(&tessShaderInfo.tes_shader, GL_TESS_EVALUATION_SHADER, tes_shader_source) &&
                   loadTessShaderProgram();

    return shader1 && shader2;


}

bool loadShaderProgram() {

    shaderInfo.shader_program = glCreateProgram();

    shaderInfo.current_shader = shaderInfo.shader_program;

    glAttachShader(shaderInfo.shader_program, shaderInfo.vertex_shader);

    glAttachShader(shaderInfo.shader_program, shaderInfo.fragment_shader);
    glLinkProgram(shaderInfo.shader_program);

    shaderInfo.uniView = glGetUniformLocation(shaderInfo.shader_program, "view");
    shaderInfo.uniModel = glGetUniformLocation(shaderInfo.shader_program, "model");
    shaderInfo.uniProj = glGetUniformLocation(shaderInfo.shader_program, "proj");

    shaderInfo.tex_lm_attrib = glGetUniformLocation(shaderInfo.shader_program, "lm_texture");
    shaderInfo.realtexture = glGetUniformLocation(shaderInfo.shader_program, "realtex");
    shaderInfo.onecolor_attrib = glGetUniformLocation(shaderInfo.shader_program, "onecolor");
    shaderInfo.realt = glGetUniformLocation(shaderInfo.shader_program, "realt");
    shaderInfo.tex = glGetUniformLocation(shaderInfo.shader_program, "tex");

    shaderInfo.pos_attrb = glGetAttribLocation(shaderInfo.shader_program, "position");
    shaderInfo.col_attrb = glGetAttribLocation(shaderInfo.shader_program, "color");
    shaderInfo.texcoord_attrib = glGetAttribLocation(shaderInfo.shader_program, "texcoord");
    shaderInfo.tex_attrib = glGetAttribLocation(shaderInfo.shader_program, "coord");
    cout << "GL ERROR LOAD SHADER 1 :" << glGetError() << endl;


    return true;
}

bool loadTessShaderProgram() {

//    tessShaderInfo.shader_program = glCreateProgram();
//
//    glAttachShader(tessShaderInfo.shader_program, tessShaderInfo.vertex_shader);
////    glAttachShader(tessShaderInfo.shader_program, tessShaderInfo.tcs_shader);
////    glAttachShader(tessShaderInfo.shader_program, tessShaderInfo.tes_shader);
//    glAttachShader(tessShaderInfo.shader_program, tessShaderInfo.fragment_shader);
//    glLinkProgram(tessShaderInfo.shader_program);
//
//    tessShaderInfo.uniView = glGetUniformLocation(tessShaderInfo.shader_program, "view");
//    tessShaderInfo.uniModel = glGetUniformLocation(tessShaderInfo.shader_program, "model");
//    tessShaderInfo.uniProj = glGetUniformLocation(tessShaderInfo.shader_program, "proj");
//    tessShaderInfo.tex_lm_attrib = glGetUniformLocation(tessShaderInfo.shader_program, "lm_texture");
//    tessShaderInfo.onecolor_attrib = glGetUniformLocation(tessShaderInfo.shader_program, "onecolor");
//
//    tessShaderInfo.pos_attrb = glGetAttribLocation(tessShaderInfo.shader_program, "position");
//    tessShaderInfo.col_attrb = glGetAttribLocation(tessShaderInfo.shader_program, "color");
//    tessShaderInfo.texcoord_attrib = glGetAttribLocation(tessShaderInfo.shader_program, "texcoord");
//
//    cout << "GL ERROR LOAD TESS SHADER 1 888:" << glGetError() << endl;
//
    return true;
}

bool _loadShader(GLuint *shader, GLenum i, const char *source) {

    *shader = glCreateShader(i);

    glShaderSource(*shader, 1, &source, nullptr);
    glCompileShader(*shader);


    GLint compilation_status = 0;
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &compilation_status);

    if (compilation_status != GL_TRUE) {
        char buf[512];
        glGetShaderInfoLog(*shader, 512, NULL, buf);
        cerr << "Error compiling shader ( " << (*shader) << ") : " << buf << endl;
        return false;
    }

    cout << "GL ERROR LOAD SHADER SOURCE :" << glGetError() << endl;

    return true;
}

bool boxInFrustum(const int32_t min[3], const int32_t max[3]) {
    glm::vec3 mins{min[0], min[2], min[1]};
    glm::vec3 maxs{max[0], max[2], max[1]};

    return comparePlaneBox(frustum.right, mins, maxs) && comparePlaneBox(frustum.left, mins, maxs) &&
           comparePlaneBox(frustum.top, mins, maxs) && comparePlaneBox(frustum.bottom, mins, maxs) &&
           comparePlaneBox(frustum.far, mins, maxs) && comparePlaneBox(frustum.near, mins, maxs);
}

bool boxInFrustum2(const glm::vec3 mins, const glm::vec3 maxs) {
    return comparePlaneBox(frustum.right, mins, maxs) && comparePlaneBox(frustum.left, mins, maxs) &&
           comparePlaneBox(frustum.top, mins, maxs) && comparePlaneBox(frustum.bottom, mins, maxs) &&
           comparePlaneBox(frustum.far, mins, maxs) && comparePlaneBox(frustum.near, mins, maxs);
}

bool comparePlaneBox(const glm::vec4 &plane, const glm::vec3 &mins, const glm::vec3 &maxs) {
    return (plane.x * mins.x + plane.y * mins.y + plane.z * mins.z + plane.w > 0 ||
            plane.x * maxs.x + plane.y * mins.y + plane.z * mins.z + plane.w > 0 ||
            plane.x * mins.x + plane.y * maxs.y + plane.z * mins.z + plane.w > 0 ||
            plane.x * maxs.x + plane.y * maxs.y + plane.z * mins.z + plane.w > 0 ||
            plane.x * mins.x + plane.y * mins.y + plane.z * maxs.z + plane.w > 0 ||
            plane.x * maxs.x + plane.y * mins.y + plane.z * maxs.z + plane.w > 0 ||
            plane.x * mins.x + plane.y * maxs.y + plane.z * maxs.z + plane.w > 0 ||
            plane.x * maxs.x + plane.y * maxs.y + plane.z * maxs.z + plane.w > 0);
}

void calcFrustum() {
    glm::mat4 mul = matrices.proj * matrices.model * matrices.view;

    frustum.right.x = mul[0][3] - mul[0][0];
    frustum.right.y = mul[1][3] - mul[1][0];
    frustum.right.z = mul[2][3] - mul[2][0];
    frustum.right.w = mul[3][3] - mul[3][0];


    frustum.left.x = mul[0][3] + mul[0][0];
    frustum.left.y = mul[1][3] + mul[1][0];
    frustum.left.z = mul[2][3] + mul[2][0];
    frustum.left.w = mul[3][3] + mul[3][0];

    frustum.top.x = mul[0][3] - mul[0][1];
    frustum.top.y = mul[1][3] - mul[1][1];
    frustum.top.z = mul[2][3] - mul[2][1];
    frustum.top.w = mul[3][3] - mul[3][1];


    frustum.bottom.x = mul[0][3] + mul[0][1];
    frustum.bottom.y = mul[1][3] + mul[1][1];
    frustum.bottom.z = mul[2][3] + mul[2][1];
    frustum.bottom.w = mul[3][3] + mul[3][1];

    frustum.far.x = mul[0][3] - mul[0][2];
    frustum.far.y = mul[1][3] - mul[1][2];
    frustum.far.z = mul[2][3] - mul[2][2];
    frustum.far.w = mul[3][3] - mul[3][2];

    frustum.near.x = mul[0][3] + mul[0][2];
    frustum.near.y = mul[1][3] + mul[1][2];
    frustum.near.z = mul[2][3] + mul[2][2];
    frustum.near.w = mul[3][3] + mul[3][2];
}
