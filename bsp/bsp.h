//
// Created by Patricio Villalobos on 6/23/15.
//

#ifndef TESSELLATION_BSP_HPP
#define TESSELLATION_BSP_HPP

#define GLFW_INCLUDE_GLCOREARB

#include <OpenGL/OpenGL.h>

#include <memory>
#include <fstream>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "../common.h"
#include "Bezier.h"


using namespace std;

class BSP;

using BSP_P = std::unique_ptr<BSP> ;

enum Lumbs {
    ENTITIES = 0,
    TEXTURES = 1,
    PLANES = 2,
    NODES = 3,
    LEAFS = 4,
    LEAFFACES = 5,
    LEAFBRUSHES = 6,
    MODELS = 7,
    BRUSHES = 8,
    BRUSHSIDES = 9,
    VERTEXES = 10,
    MESHVERTS = 11,
    EFFECTS = 12,
    FACES = 13,
    LIGHTMAPS = 14,
    LIGHTVOLS = 15,
    VISDATA = 16
};

struct DirEntry {
    int32_t offset;
    int32_t length;
};

struct Header {
    char magic[5];
    int32_t version;
    DirEntry direntry[17];
};

struct Texture {
    static constexpr Lumbs value = TEXTURES;
    char name[64];
    int32_t flags;
    int32_t contents;
};

struct Planes {
    static constexpr Lumbs value = PLANES;
    float normal[3];
    float dist;
};

struct Node {
    static constexpr Lumbs value = NODES;
    int32_t plane;
    int32_t children[2];
    int32_t mins[3];
    int32_t maxs[3];
};

struct Leaf {
    static constexpr Lumbs value = LEAFS;
    int32_t cluster;
    int32_t area;
    int32_t mins[3];
    int32_t maxs[3];
    int32_t leafface;
    int32_t n_leaffaces;
    int32_t leafbrush;
    int32_t n_leafbrushes;
};

struct LeafFace {
    static constexpr Lumbs value = LEAFFACES;
    int32_t face;
};

struct LeafBrush {
    static constexpr Lumbs value = LEAFBRUSHES;
    int32_t brush;
};

struct Models {
    static constexpr Lumbs value = MODELS;
    float mins[3];
    float maxs[3];
    int32_t face;
    int32_t n_faces;
    int32_t brush;
    int32_t n_brushes;
};

struct Brush {
    static constexpr Lumbs value = BRUSHES;
    int32_t brushside;
    int32_t n_brushside;
    int32_t texture;
};

struct BrushSide {
    static constexpr Lumbs value = BRUSHSIDES;
    int32_t plane;
    int32_t texture;
};

struct Vertex {
    static constexpr Lumbs value = VERTEXES;
    float position[3];
    float textcoor[2][2];
    float normal[3];
    uint8_t color[4];
};

struct MeshVert {
    static constexpr Lumbs value = MESHVERTS;
    int32_t offset;
};

struct Effect {
    static constexpr Lumbs value = EFFECTS;
    char name[64];
    int32_t brush;
    int32_t unk;
};

struct Face {
    static constexpr Lumbs value = FACES;
    int32_t texture;
    int32_t effect;
    int32_t type;
    int32_t vertex;
    int32_t n_vertexes;
    int32_t meshvert;
    int32_t n_meshverts;
    int32_t lm_index;
    int32_t lm_start[2];
    int32_t lm_size[2];
    int32_t lm_origin[3];
    float lm_vecs[2][3];
    float normal[3];
    int32_t size[2];
};

struct FaceInfo {

//    int32_t type;
    GLuint ebo;
    GLuint lm_tex;
    GLuint tex;
    unsigned long indexebo;
    int32_t ebo_size;
    char alpha;

    vector<Bezier> surfaces;

    bool operator < (const FaceInfo& str) const
    {
//        return ebo < str.ebo;
        return (std::tie(alpha, tex, lm_tex, ebo ) < std::tie(str.alpha, str.tex, str.lm_tex, str.ebo   ));
    }

    bool operator ==(const FaceInfo& str) const
    {
        return (ebo == str.ebo && tex == str.tex && lm_tex == str.lm_tex);
    }
};

struct Lightmap {
    static constexpr Lumbs value = LIGHTMAPS;
    uint8_t map[128][128][3];
};

struct Lightvols {
    static constexpr Lumbs value = LIGHTVOLS;
    uint8_t ambient[3];
    uint8_t directional[3];
    uint8_t dir[2];
};

struct Visdata {
    static constexpr Lumbs value = VISDATA;
    int32_t n_vecs;
    int32_t sz_vecs;
    vector<uint8_t> vecs;
};


struct LeafOp {
    vector<FaceInfo> faces;
    vector<FaceInfo> patches;
    Leaf leaf;
    glm::vec3 mins;
    glm::vec3 maxs;
};


struct  TexInfo {
    GLuint tex;
    char alpha;
};

class BSP {

public:

    BSP(const char *filename);

    void load();

    bool is_loaded() const { return _loaded; };

    vector<Lightmap> const get_lightmap_texture() { return lightmaps; }

    void _load_entities_string(ifstream &filestream);

    int32_t getCluster(glm::vec3 position) {
        int32_t index = 0;
        while(index >= 0) {
            const Node &n = nodes[index];
            const Planes &p = planes[n.plane];
            float distance = glm::dot<float>(position, glm::vec3 {p.normal[0], p.normal[2], p.normal[1]});
            distance -= p.dist;

            index = distance >= 0 ? n.children[0] : n.children[1];
        }

        if(index == 0) return -1;

        return leafs[-index - 1].cluster * visdata.sz_vecs;

    };

    std::vector<Texture> textures;
    std::vector<Planes> planes;
    std::vector<Node> nodes;
    std::vector<Effect> effects;
    std::vector<Vertex> vertexes;
    std::vector<MeshVert> meshverts;
    std::vector<Face> faces;
    std::vector<Lightmap> lightmaps;
    std::vector<LeafFace> leaffaces;
    std::vector<Leaf> leafs;
    vector<FaceInfo> faces_info;


    vector<LeafOp> leafsOp;
    vector<TexInfo> texturesInfo;


    Visdata visdata;

    static void loadBSP(const char* filename);
    static void drawMap();
    static void drawFace(const FaceInfo &f);
    static void drawFace2(const FaceInfo &f);

    GLuint vao;
    GLuint vboPatches;

    GLuint eboPatches;

private:

    Header header;

    bool _loaded;

    string filename;

    string entities;

    GLuint vbo;

    vector<float> vertexData;

    bool _debug;

    template<typename T>
    void _load(ifstream &filestream, T &vec) {
        Lumbs l = T::value_type::value;

        filestream.seekg(header.direntry[l].offset, std::ios::beg);

        {
            for (int i = 0; i < header.direntry[l].length / sizeof(typename T::value_type); i++) {
                typename T::value_type t;
                filestream.read((char *) &t, sizeof(typename T::value_type));
                vec.push_back(t);
            }
        }
    };

    void _load_visdata(ifstream &file);

    static bool visibleFromCluster(const Leaf &leaf, int32_t cluster);

    void loadFacesData();

    void loadVertexData();

    void loadPatch(const Face &face, FaceInfo &fi);

    void loadTextures();

    GLuint theebo;
};



#endif //TESSELLATION_BSP_HPP
