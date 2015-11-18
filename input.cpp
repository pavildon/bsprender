#include <iostream>

#include "Benchmarks.h"
#include "input.h"
#include "common.h"


extern Entity camera;

extern bool showset;

double oldMouseX = 0.0;
double oldMouseY = 0.0;

extern bool shouldExit;

bool relativeMouse = false;

void keyEvent(const SDL_Event &event);

void mouseMotion(const SDL_Event &event);

void inputFrame() {


    int posX = 0, posY = 0;
////        glfwGetCursorPos(_window, &posX, &posY);
//        SDL_GetMouseState(&posX, &posY);
//
    if (oldMouseX != posX || oldMouseY != posY) {
        double offsetX = oldMouseX - posX;
        double offsetY = oldMouseY - posY;

        camera.yaw += offsetX * 0.1f;
        camera.pitch += offsetY * 0.1f;

        if (camera.yaw >= 360.0f) camera.yaw = camera.yaw - 360.0f;
        else if (camera.yaw <= 0.0f) camera.yaw = camera.yaw + 360.0f;
        if (camera.pitch >= 89.9f) camera.pitch = 89.9f;
        else if (camera.pitch <= -89.9f) camera.pitch = -89.9f;

        oldMouseX = posX;
        oldMouseY = posY;
    }

    SDL_Event evt;

    while (SDL_PollEvent(&evt)) {

        switch (evt.type) {
            case SDL_WINDOWEVENT:
                if (evt.window.event == SDL_WINDOWEVENT_CLOSE) {
                    shouldExit = true;
                }
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                keyEvent(evt);
                break;
            case SDL_MOUSEMOTION:
                if (relativeMouse)
                    mouseMotion(evt);
                break;
            case SDL_MOUSEBUTTONDOWN:
                relativeMouse = !relativeMouse;
                SDL_SetRelativeMouseMode(relativeMouse ? SDL_TRUE: SDL_FALSE);
                break;
//            case SDL_MOUSEBUTTONUP:
//                SDL_SetRelativeMouseMode(SDL_FALSE);
//                relativeMouse = false;
//                break;
            default:
                break;
        }
    }


}

void keyEvent(const SDL_Event &event) {

    Uint32 action = event.key.state;

    if(event.key.repeat > 0) return;
    switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
            shouldExit = true;
            break;
        case SDLK_a:
            if (action == SDL_PRESSED) camera.vel.x += -1.0f;
            else if (action == SDL_RELEASED) camera.vel.x += 1.0f;
            break;
        case SDLK_d:
            if (action == SDL_PRESSED) camera.vel.x += 1.0f;
            else if (action == SDL_RELEASED) camera.vel.x += -1.0f;
            break;
        case SDLK_w:
            if (action == SDL_PRESSED) camera.vel.z += 1.0f;
            else if (action == SDL_RELEASED) camera.vel.z += -1.0f;
            break;
        case SDLK_s:
            if (action == SDL_PRESSED) camera.vel.z += -1.0f;
            else if (action == SDL_RELEASED) camera.vel.z += 1.0f;
            break;
        case SDLK_c:
            if (action == SDL_PRESSED) camera.vel.y += -1.0f;
            else if (action == SDL_RELEASED) camera.vel.y += 1.0f;
            break;
        case SDLK_SPACE:
            if (action == SDL_PRESSED) camera.vel.y += 1.0f;
            else if (action == SDL_RELEASED) camera.vel.y += -1.0f;
            break;
        case SDLK_LEFT:
            if (action == SDL_PRESSED) camera.yaw += 5.0f;
            break;
        case SDLK_RIGHT:
            if (action == SDL_PRESSED) camera.yaw -= 5.0f;
            break;
        case SDLK_UP:
            if (action == SDL_PRESSED) camera.pitch += 5.0f;
            break;
        case SDLK_DOWN:
            if (action == SDL_PRESSED) camera.pitch -= 5.0f;
            break;
        case SDLK_t:
            if (action == SDL_PRESSED) camera.position = glm::vec3{0.0f, 0.0f, 0.0f};
        case SDLK_g:
            Benchmarks::reset();
            break;
        default:
            break;
    }

}

void mouseMotion(const SDL_Event &event) {

    camera.yaw -= event.motion.xrel * 0.05f;
    camera.pitch -= event.motion.yrel * 0.05f;

    if (camera.yaw >= 360.0f) camera.yaw = camera.yaw - 360.0f;
    else if (camera.yaw <= 0.0f) camera.yaw = camera.yaw + 360.0f;
    if (camera.pitch >= 89.9f) camera.pitch = 89.9f;
    else if (camera.pitch <= -89.9f) camera.pitch = -89.9f;

}
