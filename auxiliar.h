#pragma once

struct Vertex {
    float x, y, z;
};

struct Triangle {
    unsigned int indices[3];
};

struct Mesh {
    //Información de estructura
    int numVertices;
    int numTriangles;
    std::vector<Vertex> vertices;
    std::vector<Triangle> triangles;

    //Información para transformación inicial
    glm::vec3 centro;
    float scale;

    //Matriz de transformación
    glm::mat4 model_transform;

    //Buffers para graficado
    std::vector<GLfloat> object_vertices;
    std::vector<GLfloat> object_color;
    std::vector<GLushort> object_indexes;

    //Ids para buffers
    GLuint vbo_object;
    GLuint vbo_color;
    GLuint ibo_object;

    Mesh(int nverts, int ntriang) : numVertices(nverts), numTriangles(ntriang), vertices(numVertices), triangles(numTriangles) {
    }
};

struct Scene {
    std::vector<Mesh> meshes;
};
