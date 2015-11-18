//
// Created by Patricio Villalobos on 6/21/15.
//

#ifndef TESSELLATION_COMMON_H
#define TESSELLATION_COMMON_H


#include <iostream>

#include <OpenGL/gl3.h>
#include <OpenGL/OpenGL.h>
#include <SDL2/SDL.h>


#include <glm/detail/type_vec3.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
//#include <glm/gtc/quaternion.hpp>
#include <vector>



using namespace std;

extern glm::vec3 upVector;
extern chrono::milliseconds frameTime;


struct Entity;
extern Entity camera;

struct Entity {
    static int count;

    Entity() {
        count++;
        cout << "Entity count : " << count << endl;
    }

    ~Entity() {
        count--;
        cout << "Entity count : " << count << endl;
    }

    void updateVectors() {
        front.x = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));
        front.y = std::sin(glm::radians(pitch));
        front.z = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
        front = glm::normalize(front);
        right = glm::normalize(glm::cross(front, upVector));
        up = glm::normalize(glm::cross(right, front));

    }

    void think() {

        updateVectors();
        speed = 0.0f;

        if (glm::length(vel) > 0.0f) {
            glm::vec3 dir{(front * vel.z) + (right * vel.x) + (upVector * vel.y)};
            glm::vec3 velocity{glm::normalize(dir) * (float)frameTime.count()};
            speed = glm::length<float>(velocity);
            position += velocity;
        }
    }


    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 up;

    float yaw = 0.0f;
    float pitch = 0.0f;
    float roll = 0.0f;

    unsigned long model = 0;

    glm::vec3 vel;

    float speed;
    float yawSpeed = 0.0f;
    float pitchSpeed = 0.0f;

};

enum ModelType {
    SIMPLE
};

struct Model {
    string name;
    unsigned long id;
    GLuint vao;
    GLuint vbo;
    ModelType type;
    GLint vec_size;

};

struct Matrices {
    glm::mat4 proj;
    glm::mat4 view;
    glm::mat4 model;
};

struct Frustum {
    glm::vec4 right;
    glm::vec4 left;
    glm::vec4 top;
    glm::vec4 bottom;
    glm::vec4 far;
    glm::vec4 near;
};


bool initGLFW();

bool loadShaders();

bool _loadShader(GLuint *shader, GLenum i, const char *source);

bool loadShaderProgram();

bool loadTessShaderProgram();

void addCube(const glm::vec3 &vec3);

unsigned long _loadModel(const char *s);

void setupCamera();

void draw();

void think();

void addPatch();

unsigned long loadSimpleCubeModel();

unsigned long loadSimplePatch();

void calcFrustum();


#endif //TESSELLATION_COMMON_H
