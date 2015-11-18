//

#include <fstream>
#include <random>
#include <glm/detail/type_vec.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <array>
#include <set>


#include "bsp.h"
#include "../shaders.h"
#include "Bezier.h"
#include "../soil/SOIL.h"
#include "../Benchmarks.h"
#include "../tga/tga.h"


using namespace std;

extern BSP_P bspMap;

extern bool boxInFrustum(const int32_t[3], const int32_t[3]);

extern bool boxInFrustum2(const glm::vec3 mins, const glm::vec3 maxs);

extern ShaderInfo shaderInfo;

GLuint currprogram = 0;
unsigned long maxFacesSize = 0;


static set<FaceInfo> visibleFaces;
static set<FaceInfo> visiblePatches;


extern GLint currentRealTex;
extern GLint currentTex;

BSP::BSP(const char *fn) : filename(fn) {
    _loaded = false;
    _debug = true;


}

void BSP::load() {

    ifstream filestream{filename, std::ifstream::in | std::ifstream::binary};


    filestream.read(header.magic, 4);
    filestream.read((char *) &header.version, 4);
    filestream.read((char *) header.direntry, 17 * sizeof(DirEntry));

    header.magic[4] = '\0';

    cout << "Header : " << header.magic << endl;
    cout << "Version: " << header.version << endl;
// Entities

    _load_entities_string(filestream);

    _load(filestream, textures);
    _load(filestream, planes);
    _load(filestream, nodes);
    _load(filestream, effects);
    _load(filestream, vertexes);
    _load(filestream, meshverts);
    _load(filestream, lightmaps);
    _load(filestream, leafs);
    _load(filestream, leaffaces);
    _load(filestream, faces);

    _load_visdata(filestream);

// Load up and arrange the data to de GPU

    loadVertexData();

    loadFacesData();


    cout << "Textures ----" << endl;
    for (Texture t: textures) {
        cout << t.name << endl;
    }
    cout << "---- End textures " << endl;

    _loaded = true;


}

void BSP::_load_entities_string(ifstream &filestream) {
    filestream.seekg(header.direntry[ENTITIES].offset, ios::beg);
    {
        char *tmp = new char[header.direntry[0].length + 1];
        filestream.read(tmp, header.direntry[0].length);
        tmp[header.direntry[0].length] = 0;
        entities = tmp;
        delete[] tmp;
    }
}

void BSP::_load_visdata(ifstream &file) {
    file.seekg(header.direntry[Visdata::value].offset, std::ios::beg);

    file.read((char *) &visdata.n_vecs, sizeof(int32_t));
    file.read((char *) &visdata.sz_vecs, sizeof(int32_t));


    uint8_t b[visdata.n_vecs * visdata.sz_vecs];
    file.read((char *) &b, visdata.n_vecs * visdata.sz_vecs);

    for (int i = 0; i < visdata.n_vecs * visdata.sz_vecs; i++) {
        visdata.vecs.push_back(b[i]);
    }

}

void BSP::loadBSP(const char *s) {

    bspMap.reset(new BSP{s});
    bspMap->load();

}

extern ShaderInfo shaderInfo;

extern Matrices matrices;


extern Entity camera;
extern bool showset;

void BSP::drawMap() {


    vector<FaceInfo> vfaces;
    vector<FaceInfo> patches;

    vfaces.reserve(5000);
    patches.reserve(2000);

//    if (!bspMap->_loaded) return;
    currprogram = 0;


//

    int32_t cluster = bspMap->getCluster(camera.position);


    glUseProgram(shaderInfo.shader_program);
    glUniformMatrix4fv(shaderInfo.uniView, 1, GL_FALSE, glm::value_ptr(matrices.view));
    glUniformMatrix4fv(shaderInfo.uniModel, 1, GL_FALSE, glm::value_ptr(matrices.model));
    glUniformMatrix4fv(shaderInfo.uniProj, 1, GL_FALSE, glm::value_ptr(matrices.proj));
    glUniform1i(shaderInfo.tex, 0);
    glUniform1i(shaderInfo.realt, 1);


    glBindVertexArray(bspMap->vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bspMap->theebo);

    Benchmarks::start("1totalFrame");

    Benchmarks::start("2leafLoop");


    for (LeafOp &leafOp : bspMap->leafsOp) {
        if (!visibleFromCluster(leafOp.leaf, cluster) || !boxInFrustum2(leafOp.mins, leafOp.maxs)) continue;

        vfaces.insert(vfaces.end(), leafOp.faces.begin(), leafOp.faces.end());
        patches.insert(patches.end(), leafOp.patches.begin(), leafOp.patches.end());
//        vfaces.insert(leafOp.faces.begin(), leafOp.faces.end());
//        patches.insert(leafOp.patches.begin(), leafOp.patches.end());
    }

    if (vfaces.size() > 0) {
        sort(vfaces.begin(), vfaces.end());
        vfaces.erase(unique(vfaces.begin(), vfaces.end()), vfaces.end());
    }

    if (patches.size() > 0) {
        sort(patches.begin(), patches.end());
        patches.erase(unique(patches.begin(), patches.end()), patches.end());
    }

    Benchmarks::stop("2leafLoop");

    Benchmarks::start("3drawCall");
//
    Benchmarks::start("4faces");
    for (const FaceInfo &fii: vfaces) {
        BSP::drawFace(fii);
    }
    Benchmarks::stop("4faces");
    showset = false;
//
    Benchmarks::start("5patches");
    currentTex = 99999;
    currentRealTex = 99999;
//    for (const FaceInfo &fii: visiblePatches) {
    for (const FaceInfo &fii: patches) {
        BSP::drawFace2(fii);
    }
    Benchmarks::stop("5patches");

    Benchmarks::stop("3drawCall");

    Benchmarks::stop("1totalFrame");
}

void BSP::drawFace(const FaceInfo &f) {

    auto lmTex = GL_FALSE;
    auto realtex = GL_FALSE;

//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, f.ebo);
    if (f.lm_tex < 9999) {
        lmTex = GL_TRUE;
        if (currentTex != f.lm_tex) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, f.lm_tex);
            currentTex = f.lm_tex;
        }
    }

    if (f.tex < 9999) {
        realtex = GL_TRUE;
        if (currentRealTex != f.tex) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, f.tex);
            currentRealTex = f.tex;
        }
    }

    glUniform1i(shaderInfo.realtexture, realtex);
    glUniform1i(shaderInfo.tex_lm_attrib, lmTex);

    glDrawElements(GL_TRIANGLES, f.ebo_size, GL_UNSIGNED_INT, (void *)(sizeof(int) * f.indexebo));
}

void BSP::drawFace2(const FaceInfo &f) {
    for (const Bezier &b: f.surfaces) {
        b.drawControlPoints();
    }
}

inline bool BSP::visibleFromCluster(const Leaf &leaf, int32_t cluster) {
    return (bspMap->visdata.vecs[cluster + (leaf.cluster >> 3)] & (1 << (leaf.cluster & 7))) != 0;
}

void BSP::loadVertexData() {
    Bezier::calcEbo(10);

    for (Vertex &v : vertexes) {
        float vv[14] = {
                v.position[0], v.position[2], v.position[1],
                v.color[0] / 255.0f, v.color[1] / 255.0f, v.color[2] / 255.0f, v.color[3] / 255.0f,
                v.normal[0], v.normal[2], v.normal[1],
                v.textcoor[0][0], v.textcoor[0][1],
                v.textcoor[1][0], v.textcoor[1][1]
        };

        for (int i = 0; i < 14; ++i) {
            vertexData.push_back(vv[i]);
        }

    }

    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), (void *) vertexData.data(), GL_STATIC_DRAW);


    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glVertexAttribPointer(shaderInfo.pos_attrb, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), 0);
    glVertexAttribPointer(shaderInfo.col_attrb, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                          (void *) (3 * sizeof(float)));
    glVertexAttribPointer(shaderInfo.tex_attrib, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                          (void *) (10 * sizeof(float)));
    glVertexAttribPointer(shaderInfo.texcoord_attrib, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float),
                          (void *) (12 * sizeof(float)));

    glEnableVertexAttribArray(shaderInfo.pos_attrb);
    glEnableVertexAttribArray(shaderInfo.col_attrb);
    glEnableVertexAttribArray(shaderInfo.texcoord_attrib);
    glEnableVertexAttribArray(shaderInfo.tex_attrib);

    glBindVertexArray(0);

}

void BSP::loadFacesData() {

    GLuint tex[lightmaps.size()];
    glGenTextures(lightmaps.size(), tex);
    int o = 0;

    for (GLuint tid: tex) {
        glBindTexture(GL_TEXTURE_2D, tid);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, lightmaps[o].map);
        glGenerateMipmap(GL_TEXTURE_2D);
        o++;
    }

    loadTessShaderProgram();
    loadTextures();

    vector<uint32_t> elementos;

    for (Face &f: faces) {
        FaceInfo fi;
        fi.lm_tex = f.lm_index >= 0 || texturesInfo[f.texture].alpha > 0 ? tex[f.lm_index] : 999999;
        fi.tex = f.texture >= 0 ? texturesInfo[f.texture].tex : 999999;
        fi.alpha = texturesInfo[f.texture].alpha;
        if (f.type != 2) {
//            glGenBuffers(1, &fi.ebo);
            uint32_t elements[f.n_meshverts];
            fi.indexebo = elementos.size();
            fi.ebo = elementos.size();
            for (int vn = 0; vn < f.n_meshverts; vn++) {
                elementos.push_back((const uint32_t) (f.vertex + meshverts[f.meshvert + vn].offset));
//                elements[vn] = (const uint32_t) (f.vertex + meshverts[f.meshvert + vn].offset);
            }


            fi.ebo_size = f.n_meshverts;
        } else {
            bspMap->loadPatch(f, fi);
        }
        faces_info.emplace_back(std::move(fi));

    }
    glGenBuffers(1, &theebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, theebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementos.size()* sizeof(int), elementos.data(), GL_STATIC_DRAW);

    for (Leaf &leaf : leafs) {
        LeafOp leafOp;
        leafOp.leaf = leaf;
        for (int i = leaf.leafface; i < leaf.leafface + leaf.n_leaffaces; i++) {
            int32_t lf = leaffaces[i].face;
            if (faces[lf].type != 2) {
                leafOp.faces.push_back(faces_info[lf]);
            } else {
                leafOp.patches.push_back(faces_info[lf]);
            }
        }
        glm::vec3 mins{leaf.mins[0], leaf.mins[2], leaf.mins[1]};
        glm::vec3 maxs{leaf.maxs[0], leaf.maxs[2], leaf.maxs[1]};
        leafOp.mins = mins;
        leafOp.maxs = maxs;

        std::sort(leafOp.faces.begin(), leafOp.faces.end());
        std::sort(leafOp.patches.begin(), leafOp.patches.end());

        leafsOp.emplace_back(leafOp);
    }

}

void BSP::loadPatch(const Face &face, FaceInfo &fi) {
    static GLuint eboid = 998872;

    fi.ebo = eboid++;
    for (int y = 0; y < (face.size[1] - 1) / 2; y++) {
        for (int x = 0; x < (face.size[0] - 1) / 2; x++) {
            std::array<float, 27> controlPoints{0};
            std::array<float, 27> controlColors{0};
            std::array<float, 18> controlTexs{0};
            std::array<float, 18> controlTexs2{0};

            int v = 0;
            int v2 = 0;
            int first_ = face.vertex + (2 * x) + (2 * y * face.size[0]);

            for (int yy = 0; yy < 3; yy++) {
                int first = first_ + (yy * face.size[0]);

                controlPoints[v] = vertexes[first].position[0];
                controlPoints[v + 1] = vertexes[first].position[2];
                controlPoints[v + 2] = vertexes[first].position[1];

                controlPoints[v + 3] = vertexes[first + 1].position[0];
                controlPoints[v + 4] = vertexes[first + 1].position[2];
                controlPoints[v + 5] = vertexes[first + 1].position[1];

                controlPoints[v + 6] = vertexes[first + 2].position[0];
                controlPoints[v + 7] = vertexes[first + 2].position[2];
                controlPoints[v + 8] = vertexes[first + 2].position[1];

                controlColors[v] = vertexes[first].color[0] / 255.0f;
                controlColors[v + 1] = vertexes[first].color[1] / 255.0f;
                controlColors[v + 2] = vertexes[first].color[2] / 255.0f;

                controlColors[v + 3] = vertexes[first + 1].color[0] / 255.0f;
                controlColors[v + 4] = vertexes[first + 1].color[1] / 255.0f;
                controlColors[v + 5] = vertexes[first + 1].color[2] / 255.0f;

                controlColors[v + 6] = vertexes[first + 2].color[0] / 255.0f;
                controlColors[v + 7] = vertexes[first + 2].color[1] / 255.0f;
                controlColors[v + 8] = vertexes[first + 2].color[2] / 255.0f;

                controlTexs[v2] = vertexes[first].textcoor[1][0];
                controlTexs[v2 + 1] = vertexes[first].textcoor[1][1];

                controlTexs[v2 + 2] = vertexes[first + 1].textcoor[1][0];
                controlTexs[v2 + 3] = vertexes[first + 1].textcoor[1][1];

                controlTexs[v2 + 4] = vertexes[first + 2].textcoor[1][0];
                controlTexs[v2 + 5] = vertexes[first + 2].textcoor[1][1];

                controlTexs2[v2] = vertexes[first].textcoor[0][0];
                controlTexs2[v2 + 1] = vertexes[first].textcoor[0][1];

                controlTexs2[v2 + 2] = vertexes[first + 1].textcoor[0][0];
                controlTexs2[v2 + 3] = vertexes[first + 1].textcoor[0][1];

                controlTexs2[v2 + 4] = vertexes[first + 2].textcoor[0][0];
                controlTexs2[v2 + 5] = vertexes[first + 2].textcoor[0][1];

                v += 9;
                v2 += 6;

            }
            fi.surfaces.emplace_back(
                    Bezier{controlPoints, controlColors, controlTexs, controlTexs2, fi.lm_tex, fi.tex});
        }
    }


}

void BSP::loadTextures() {

    GLuint maxtexture{0};

    for (Texture t: textures) {

        string name = string{t.name} + ".jpg";
        string name2 = string{t.name} + ".tga";
        cout << t.name << endl;
        int w = 0;
        int h = 0;
        auto imgtype = GL_RGB;

        unsigned char *img = SOIL_load_image(name.c_str(), &w, &h, 0, SOIL_LOAD_RGB);

        if (img == nullptr) {
            R_LoadTGA(name2.c_str(), &img, &w, &h);
            imgtype = GL_RGBA;
        }

        if (img != nullptr) {

            TexInfo ti;
            GLuint tex;

            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, imgtype,
                         GL_UNSIGNED_BYTE, img);

            glGenerateMipmap(GL_TEXTURE_2D);
            ti.tex = tex;
            ti.alpha = (char) (imgtype == GL_RGBA ? 1 : 0);
            texturesInfo.push_back(ti);
            cout << tex << " " << w << " x " << h << endl;
            SOIL_free_image_data(img);
            if (ti.tex > maxtexture) maxtexture = ti.tex;
        } else {
            texturesInfo.push_back(TexInfo{999999, 0});
            cout << "Error cargando : " << t.name << endl;
        }

    }
    cout << "Max Texture " << maxtexture << endl;
}


