//
// Created by Patricio Villalobos on 6/23/15.
//

#ifndef TESSELLATION_SHADER_SOURCES_H
#define TESSELLATION_SHADER_SOURCES_H
#define GLSL(src) "#version 410 core\n" #src

const char* vertex_shader_source = GLSL(

        in vec3 position;
        in vec3 color;
        in vec2 texcoord;
        in vec2 coord;

        out vec3 Color;
        out vec2 TexCoord;
        out vec2 Coord;

        uniform mat4 proj;
        uniform mat4 view;
        uniform mat4 model;

        void main() {
            Color = color;
            TexCoord = texcoord;
            Coord = coord;
            gl_Position =  proj * view * model *vec4(position, 1.0);
        }

);



const char* vertex_shader_source_2 = GLSL(

        in vec3 position;
        in vec3 color;
        in vec2 texcoord;

        out vec3 Color;
        out vec2 TexCoord;

        uniform mat4 proj;
        uniform mat4 view;
        uniform mat4 model;

        uniform vec3 col;

        void main() {
            Color = color;
            TexCoord = texcoord;
            gl_Position =  proj * view * model *vec4(position, 1.0);
        }

);

const char* fragment_shader_source = GLSL(

        in vec3 Color;
        in vec2 TexCoord;
        in vec2 Coord;

        out vec4 outColor;

        uniform bool lm_texture;
        uniform bool onecolor;
        uniform bool realtex;

        uniform sampler2D tex;
        uniform sampler2D realt;

        void main() {

            if(lm_texture && realtex) {
                vec4 lm = texture(tex, TexCoord);
                vec4 t = texture(realt, Coord);
                outColor = mix(t, lm, 0.5);
            } else if (lm_texture && !realtex) {
                vec4 lm = texture(tex, TexCoord);
                outColor = lm;
            } else if(!lm_texture && realtex) {
                vec4 t = texture(realt, Coord);
                outColor = t;
            } else {
                outColor = vec4(Color, 1.0f);
            }
        }

);

const char* tcs_shader_source = GLSL (

    layout(vertices = 9) out;

     in vec3 Color[];
     in vec2 TexCoord[];

     out vec3 ColorF[];
     out vec2 TexCoordF[];

    void main(void) {
        if(gl_InvocationID == 0) {
            gl_TessLevelInner[0] = 3;
            gl_TessLevelInner[1] = 3;

            gl_TessLevelOuter[0] = 5;
            gl_TessLevelOuter[1] = 5;
            gl_TessLevelOuter[2] = 5;
            gl_TessLevelOuter[3] = 5;
        }

        gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
        ColorF[gl_InvocationID] = Color[gl_InvocationID];
        TexCoordF[gl_InvocationID] = TexCoord[gl_InvocationID];

    }
);

const char* tes_shader_source_ = GLSL (
    layout (quads) in;

     in vec3 ColorF[];
     in vec2 TexCoordF[];

     out vec3 Color;
     out vec2 TexCoord;



    uniform mat4 proj;
    uniform mat4 view;
    uniform mat4 model;
    void main(void) {




        vec4 p1  = mix(gl_in[0].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
//        p1 = mix(p1,  gl_in[2].gl_Position, gl_TessCoord.x);

        vec4 p2  = mix(gl_in[3].gl_Position, gl_in[4].gl_Position, gl_TessCoord.x);
        p2 = mix(p2,  gl_in[5].gl_Position, gl_TessCoord.x);

        vec4 p3  = mix(gl_in[6].gl_Position, gl_in[8].gl_Position, gl_TessCoord.x);
//        p3 = mix(p3,  gl_in[8].gl_Position, gl_TessCoord.x);

        vec4 p = mix(p1, p3, gl_TessCoord.y);
//        p = mix(p, p3, gl_TessCoord.y);

        Color = mix(ColorF[0], ColorF[1] , gl_TessCoord.x);
        TexCoord = TexCoordF[0];
        gl_Position =  proj * view * model * p;

    }
);


const char* tes_shader_source = GLSL (
        layout (quads, equal_spacing, cw) in;

        in vec3 ColorF[];
        in vec2 TexCoordF[];

        out vec3 Color;
        out vec2 TexCoord;



        uniform mat4 proj;
        uniform mat4 view;
        uniform mat4 model;
        void main(void) {

            float u = gl_TessCoord.x;
            float v = gl_TessCoord.y;

            vec4 p00 = (pow(1-u, 2)*pow(1-v, 2)) * gl_in[8].gl_Position;
            vec4 p01 = ((2*u) * (1 - u) * pow(1-v, 2)) * gl_in[7].gl_Position;
            vec4 p02 = (pow(u, 2) * pow(1-v, 2)) * gl_in[6].gl_Position;
            vec4 p03 = (2*pow(1-u, 2) * v * (1-v)) * gl_in[5].gl_Position;
            vec4 p04 = (4*u*v*(1-u)*(1-v)) * gl_in[4].gl_Position;
            vec4 p05 = (2*pow(u,2)*v*(1-v))* gl_in[3].gl_Position;
            vec4 p06 = (pow(1-u, 2)*pow(v, 2))* gl_in[2].gl_Position;
            vec4 p07 = (2*u*(1-u)*pow(v, 2)) * gl_in[1].gl_Position;
            vec4 p08 = (u*u*v*v) * gl_in[0].gl_Position;


            vec4 p = p00 + p01 + p02 + p03 + p04 + p05 + p06 + p07 + p08;

            vec3 c1 = mix(ColorF[0], ColorF[1] , gl_TessCoord.x);
            c1 = mix(Color,ColorF[2], gl_TessCoord.x);

            vec3 c2 = mix(ColorF[3], ColorF[4] , gl_TessCoord.x);
            c2 = mix(Color,ColorF[5], gl_TessCoord.x);

            vec3 c3 = mix(ColorF[6], ColorF[7] , gl_TessCoord.x);
            c3 = mix(Color,ColorF[8], gl_TessCoord.x);

            Color = mix(c1, c2, gl_TessCoord.y);
            Color = mix(Color, c3, gl_TessCoord.y);

            TexCoord = TexCoordF[0];
            gl_Position =  proj * view * model * p;

        }
);
#endif //TESSELLATION_SHADER_SOURCES_H
