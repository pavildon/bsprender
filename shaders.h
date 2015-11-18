//
// Created by Patricio Villalobos on 6/21/15.
//

#ifndef TESSELLATION_SHADERS_H
#define TESSELLATION_SHADERS_H




struct ShaderInfo {
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint shader_program;
    GLint uniView;
    GLint uniModel;
    GLint uniProj;
    GLuint tcs_shader;
    GLuint tes_shader;
    GLuint current_shader;
    GLint pos_attrb;
    GLint col_attrb;
    GLint tex_lm_attrib;
    GLint texcoord_attrib;
    GLint onecolor_attrib;
    GLint tex_attrib;
    GLint realt;
    GLint tex;
    GLint realtexture;
};



#endif //TESSELLATION_SHADERS_H
