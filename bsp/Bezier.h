//
// Created by Patricio Villalobos on 7/12/15.
//

#ifndef TESSELLATION_BEZIER_H
#define TESSELLATION_BEZIER_H

#include <array>
#include "../common.h"

class Bezier {

public:

    Bezier(array<float, 27> controlPoints, array<float, 27> controlColors, array<float, 18> controlTex,
               array<float, 18> texs, GLuint lmt, GLuint t);

    void drawControlPoints() const;

    static void calcEbo(int level);

private:

    std::array<float, 27> _controlPoints;
    std::array<float, 27> _controlColors;

    GLuint vbo;

    GLuint vao;

    void _calcBezier();

    std::array<glm::vec3, 9> _controlPointsV;
    std::array<glm::vec3, 9> _controlColorsV;
    std::array<glm::vec3, 9> _controlTexV;
    std::array<glm::vec3, 9> _controlTexV2;
    GLuint vaoBezier;
    GLuint vboBezier;
    static GLuint eboBezier;
    static int _lvl;
    static int quads;

    static int n_elements;


    GLuint vboNormals;
    int _lvl2;
    unsigned long startVect;
    GLuint tex;
    GLuint rt;
};


#endif //TESSELLATION_BEZIER_H
