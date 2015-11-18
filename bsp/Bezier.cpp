//
// Created by Patricio Villalobos on 7/12/15.
//

#include "Bezier.h"
#include "../shaders.h"

extern ShaderInfo shaderInfo;


int Bezier::quads = 0;
int Bezier::n_elements = 0;
GLuint Bezier::eboBezier = 0;
int Bezier::_lvl = 0;


Bezier::Bezier(array<float, 27> controlPoints, array<float, 27> controlColors, array<float, 18> controlTex,
               array<float, 18> texs, GLuint i, GLuint realt) {

    if (_lvl == 0) return;
    _controlPoints = controlPoints;
    _controlColors = controlColors;

    tex = i;
    rt = realt;

    for (int x = 0; x < 9; x++) {
        _controlPointsV[x] = glm::vec3 {_controlPoints[x * 3], _controlPoints[(x * 3) + 1],
                                        _controlPoints[(x * 3) + 2]};

        _controlColorsV[x] = glm::vec3 {(_controlPoints[x * 3]), (_controlPoints[(x * 3) + 1]),
                                        (_controlPoints[(x * 3) + 2])};

        _controlTexV[x] = glm::vec3 {(controlTex[(x * 2)]), (controlTex[(x * 2) + 1]),
                                     1.0f};

        _controlTexV2[x] = glm::vec3 {(texs[(x * 2)]), (texs[(x * 2) + 1]),
                                      1.0f};
    }


    _calcBezier();

}

extern GLint currentRealTex;
extern GLint currentTex;

void Bezier::drawControlPoints() const {
    if (_lvl == 0) return;

//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//    glUniform1i(tessShaderInfo.onecolor_attrib, GL_TRUE);
//    glBindVertexArray(vaoBezier);
//    glDrawElements(GL_TRIANGLES, n_elements, GL_UNSIGNED_INT, 0);
//    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//    glUniform1i(tessShaderInfo.onecolor_attrib, GL_FALSE);

    auto lmTex = GL_FALSE;
    auto realTex = GL_FALSE;

    glBindVertexArray(vaoBezier);
    if ( tex < 9999 ) {
        lmTex = GL_TRUE;
        if(currentTex != tex) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex);
            currentTex = tex;
        }
    }

    if( rt < 9999  ) {
        realTex = GL_TRUE;
        if(currentRealTex != rt) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, rt);
            currentRealTex = rt;
        }
    }

    glUniform1i(shaderInfo.tex_lm_attrib, lmTex);
    glUniform1i(shaderInfo.realtexture, realTex);
    glDrawElements(GL_TRIANGLES, n_elements, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);

}

void Bezier::_calcBezier() {
    _lvl2 = _lvl;
    quads = (_lvl - 1) * (_lvl2 - 1);
    n_elements = quads * 6;
    float factor = 1.0f / (_lvl - 1);
    float factor_ = 1.0f / (_lvl2 - 1);
    float data[_lvl * _lvl * 10];

    int i = 0;

    for (float v = 0.0f; v <= 1.0f; v += factor) {
        for (float u = 0.0f; u <= 1.0f; u += factor_) {
            float bu[3];
            float bv[3];
            bu[0] = (float) pow(1.0f - u, 2);
            bu[1] = (2.0f * u) * (1.0f - u);
            bu[2] = u * u;

            bv[0] = (float) pow(1.0f - v, 2);
            bv[1] = (2.0f * v) * (1.0f - v);
            bv[2] = v * v;

            auto p1 = bu[0] * bv[0] * _controlPointsV[0];
            auto p2 = bu[0] * bv[1] * _controlPointsV[1];
            auto p3 = bu[0] * bv[2] * _controlPointsV[2];

            auto p4 = bu[1] * bv[0] * _controlPointsV[3];
            auto p5 = bu[1] * bv[1] * _controlPointsV[4];
            auto p6 = bu[1] * bv[2] * _controlPointsV[5];

            auto p7 = bu[2] * bv[0] * _controlPointsV[6];
            auto p8 = bu[2] * bv[1] * _controlPointsV[7];
            auto p9 = bu[2] * bv[2] * _controlPointsV[8];

            auto p = p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;

            data[i] = p.x;
            data[i + 1] = p.y;
            data[i + 2] = p.z;

            auto c1 = bu[0] * bv[0] * _controlColorsV[0];
            auto c2 = bu[0] * bv[1] * _controlColorsV[1];
            auto c3 = bu[0] * bv[2] * _controlColorsV[2];

            auto c4 = bu[1] * bv[0] * _controlColorsV[3];
            auto c5 = bu[1] * bv[1] * _controlColorsV[4];
            auto c6 = bu[1] * bv[2] * _controlColorsV[5];

            auto c7 = bu[2] * bv[0] * _controlColorsV[6];
            auto c8 = bu[2] * bv[1] * _controlColorsV[7];
            auto c9 = bu[2] * bv[2] * _controlColorsV[8];

            auto c = c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8 + c9;

            data[i + 3] = c.x;
            data[i + 4] = c.y;
            data[i + 5] = c.z;

            {
                auto t1 = bu[0] * bv[0] * _controlTexV[0];
                auto t2 = bu[0] * bv[1] * _controlTexV[1];
                auto t3 = bu[0] * bv[2] * _controlTexV[2];

                auto t4 = bu[1] * bv[0] * _controlTexV[3];
                auto t5 = bu[1] * bv[1] * _controlTexV[4];
                auto t6 = bu[1] * bv[2] * _controlTexV[5];

                auto t7 = bu[2] * bv[0] * _controlTexV[6];
                auto t8 = bu[2] * bv[1] * _controlTexV[7];
                auto t9 = bu[2] * bv[2] * _controlTexV[8];

                auto t = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9;

                data[i + 6] = t.x;
                data[i + 7] = t.y;
            }

            {
                auto t1 = bu[0] * bv[0] * _controlTexV2[0];
                auto t2 = bu[0] * bv[1] * _controlTexV2[1];
                auto t3 = bu[0] * bv[2] * _controlTexV2[2];

                auto t4 = bu[1] * bv[0] * _controlTexV2[3];
                auto t5 = bu[1] * bv[1] * _controlTexV2[4];
                auto t6 = bu[1] * bv[2] * _controlTexV2[5];

                auto t7 = bu[2] * bv[0] * _controlTexV2[6];
                auto t8 = bu[2] * bv[1] * _controlTexV2[7];
                auto t9 = bu[2] * bv[2] * _controlTexV2[8];

                auto t = t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9;

                data[i + 8] = t.x;
                data[i + 9] = t.y;
            }

            i += 10;
        }
    }


    glGenVertexArrays(1, &vaoBezier);
    glBindVertexArray(vaoBezier);

    glGenBuffers(1, &vboBezier);
    glBindBuffer(GL_ARRAY_BUFFER, vboBezier);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    glVertexAttribPointer(shaderInfo.pos_attrb, 3, GL_FLOAT, GL_FALSE, (10 * sizeof(float)), 0);
    glEnableVertexAttribArray(shaderInfo.pos_attrb);
    glVertexAttribPointer(shaderInfo.col_attrb, 3, GL_FLOAT, GL_FALSE, (10 * sizeof(float)),
                          (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(shaderInfo.col_attrb);
    glVertexAttribPointer(shaderInfo.texcoord_attrib, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float),
                          (void *) (6 * sizeof(float)));
    glEnableVertexAttribArray(shaderInfo.texcoord_attrib);
    glVertexAttribPointer(shaderInfo.tex_attrib, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float),
                          (void *) (8 * sizeof(float)));
    glEnableVertexAttribArray(shaderInfo.tex_attrib);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboBezier);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void Bezier::calcEbo(int level) {
    quads = (level - 1) * (level - 1);
    n_elements = quads * 6;
    _lvl = level;
    int elements[quads * 6];
    int o = 0;

    for (int y = 0; y < level - 1; y++) {
        for (int x = 0; x < level - 1; x++) {
            elements[o++] = (y * level) + x;
            elements[o++] = (y * level) + x + 1;
            elements[o++] = (y * level) + x + level + 1;

            elements[o++] = (y * level) + x;
            elements[o++] = (y * level) + x + level + 1;
            elements[o++] = (y * level) + x + level;
        }
    }

    glGenBuffers(1, &eboBezier);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboBezier);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


