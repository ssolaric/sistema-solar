#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <cstdio>
#include <vector>

#include <GL/glew.h>
#include <GL/freeglut.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include "shader_utils.h"

GLuint program;
GLint attribute_coord;
GLint attribute_color;
GLint uniform_mvp;

float alfa = 10.0f;
float beta = 20.0f;
float theta = 25.0f;
float phi = 20.0f;

int screen_width = 800, screen_height = 800;

struct Vertex {
    float x, y, z;
};

struct Triangle {
    unsigned int indices[3];
};

struct Mesh {
    //Informacion de estructura
    int numVertices;
    int numTriangles;
    std::vector<Vertex> vertices;
    std::vector<Triangle> triangles;

    //Informaci�n para transformaci�n inicial
    glm::vec3 centro;
    float scale;

    //Matriz de transformaci�n
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

Scene scene;
int numEdges;

void centrar(Mesh& mesh) {
    mesh.model_transform = mesh.model_transform * glm::scale(glm::mat4(1.0f), glm::vec3(mesh.scale, mesh.scale, mesh.scale)) *
        glm::translate(glm::mat4(1.0f), -mesh.centro);
}

void moverCentro(glm::mat4 transformacion, Mesh& mesh) {
    mesh.centro = glm::vec3(transformacion * glm::vec4(mesh.centro, 1.0f));
    //std::cout << glm::to_string(mesh.centro) << "\n";
}

Mesh leerOFF(const char* filename) {
    std::ifstream ifs(filename);

    //Leer formato
    std::string cad;
    ifs >> cad;

    if (cad != "OFF") {
        std::cout << "Error de formato\n";
        exit(EXIT_FAILURE);
    }

    int nverts, ntriang, nedges;
    ifs >> nverts >> ntriang >> nedges;
    std::cout << nverts << " " << ntriang << " " << nedges << "\n";

    Mesh mesh(nverts, ntriang);
    float cx = 0.0;
    float cy = 0.0;
    float cz = 0.0;

    int i;
    for (i = 0; i < nverts; i++) {
        ifs >> mesh.vertices[i].x >> mesh.vertices[i].y >> mesh.vertices[i].z;
        cx += mesh.vertices[i].x;
        cy += mesh.vertices[i].y;
        cz += mesh.vertices[i].z;
    }

    for (i = 0; i < ntriang; i++) {
        int nv;
        ifs >> nv >> mesh.triangles[i].indices[0] >> mesh.triangles[i].indices[1] >> mesh.triangles[i].indices[2];
    }

    cx /= nverts;
    cy /= nverts;
    cz /= nverts;
    mesh.centro = glm::vec3(cx, cy, cz);
    std::cout << cx << " " << cy << " " << cz << "\n";

    float maxx = -1.0e-10f, maxy = -1.0e-10f, maxz = -1.0e-10f;
    float minx = 1.0e10f, miny = 1.0e10f, minz = 1.0e10f;

    for (int i = 0; i < mesh.numVertices; i++) {
        minx = std::min(minx, mesh.vertices[i].x);
        maxx = std::max(maxx, mesh.vertices[i].x);
        miny = std::min(miny, mesh.vertices[i].y);
        maxy = std::max(maxy, mesh.vertices[i].y);
        minz = std::min(minz, mesh.vertices[i].z);
        maxz = std::max(maxz, mesh.vertices[i].z);
    }

    float diag = sqrt((maxx - minx)*(maxx - minx) + (maxy - miny)*(maxy - miny) + (maxz - minz)*(maxz - minz));
    mesh.scale = 2.0f / diag;

    mesh.model_transform = glm::mat4(1.0f);
    return mesh;
}

void init_buffers(Mesh& mesh) {
    mesh.object_vertices = std::vector<GLfloat>(mesh.numVertices * 3);
    mesh.object_color = std::vector<GLfloat>(mesh.numVertices * 3);
    mesh.object_indexes = std::vector<GLushort>(mesh.numTriangles * 3);

    int i;

    for (i = 0; i < mesh.numVertices; i++) {
        mesh.object_vertices[3 * i] = mesh.vertices[i].x;
        mesh.object_vertices[3 * i + 1] = mesh.vertices[i].y;
        mesh.object_vertices[3 * i + 2] = mesh.vertices[i].z;

        mesh.object_color[3 * i] = (1.0f * i) / mesh.numVertices;
        mesh.object_color[3 * i + 1] = (1.0f * i) / mesh.numVertices;
        mesh.object_color[3 * i + 2] = 0.8f;
    }

    for (i = 0; i < mesh.numTriangles; i++) {
        mesh.object_indexes[3 * i] = mesh.triangles[i].indices[0];
        mesh.object_indexes[3 * i + 1] = mesh.triangles[i].indices[1];
        mesh.object_indexes[3 * i + 2] = mesh.triangles[i].indices[2];
    }

    glGenBuffers(1, &mesh.vbo_object);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_object);
    glBufferData(GL_ARRAY_BUFFER, mesh.numVertices * 3 * sizeof(GLfloat), &mesh.object_vertices[0], GL_STATIC_DRAW);


    glGenBuffers(1, &mesh.vbo_color);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_color);
    glBufferData(GL_ARRAY_BUFFER, mesh.numVertices * 3 * sizeof(GLfloat), &mesh.object_color[0], GL_STATIC_DRAW);

    glGenBuffers(1, &mesh.ibo_object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.numTriangles * 3 * sizeof(GLushort), &mesh.object_indexes[0], GL_STATIC_DRAW);
}


bool init_resources() {
    Mesh esfera = leerOFF("sphere.off");
    scene.meshes.push_back(esfera);
    scene.meshes.push_back(esfera);
    scene.meshes.push_back(esfera);

    scene.meshes[1].model_transform = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 0.0f))
        * glm::scale(glm::mat4(1.0f), glm::vec3(0.4f, 0.4f, 0.4f));

    scene.meshes[2].model_transform = glm::translate(glm::mat4(1.0f), glm::vec3(6.0f, 0.0f, 0.0f)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));

    // Aqu� aplico la transformaci�n de mover el objeto al centro y escalarlo ense�ada en clase.
    // La hago fuera del onDisplay porque solo es necesaria al inicio.
    centrar(scene.meshes[0]);
    centrar(scene.meshes[1]);
    centrar(scene.meshes[2]);

    // Por cada transformaci�n o conjunto de transformaciones hechas a los objetos, tengo que mover el centro hacia el centro actual del objeto.
    // Esta transformaci�n no siempre ser� la misma que se le aplic� al objeto.
    // En este caso en el que a�n no hay animaci�n, s� transformamos el centro igual que el objeto.
    moverCentro(scene.meshes[1].model_transform, scene.meshes[1]);
    moverCentro(scene.meshes[1].model_transform, scene.meshes[2]);


    init_buffers(scene.meshes[0]);
    init_buffers(scene.meshes[1]);
    init_buffers(scene.meshes[2]);

    GLint link_ok = GL_FALSE;
    GLuint vs, fs;
    if ((vs = create_shader("basic3.v.glsl", GL_VERTEX_SHADER)) == 0) return false;
    if ((fs = create_shader("basic3.f.glsl", GL_FRAGMENT_SHADER)) == 0) return false;

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        std::cerr << "Problemas con el Shader\n";
        return false;
    }

    attribute_coord = glGetAttribLocation(program, "coord3d");
    if (attribute_coord == -1) {
        std::cerr << "No se puede asociar el atributo coord\n";
        return false;
    }

    attribute_color = glGetAttribLocation(program, "color");
    if (attribute_color == -1) {
        std::cerr << "No se puede asociar el atributo color\n";
        return false;
    }

    uniform_mvp = glGetUniformLocation(program, "mvp");
    if (uniform_mvp == -1) {
        std::cerr << "No se puede asociar el uniform mvp\n";
        return false;
    }

    return true;
}



void graficarObjeto(const Mesh& mesh) {
    //Creamos matrices de modelo, vista y proyeccion
    glm::mat4 model = mesh.model_transform;

    glm::mat4 view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::ortho<float>(-6.0, 6.0, -6.0, 6.0, -6.0, 6.0);
    glm::mat4 mvp = projection * view * model;

    glUseProgram(program);

    //Enviamos la matriz que debe ser usada para cada vertice
    glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));

    glEnableVertexAttribArray(attribute_coord);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_object);

    glVertexAttribPointer(attribute_coord, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(attribute_color);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_color);

    glVertexAttribPointer(attribute_color, 3, GL_FLOAT, GL_FALSE, 0, 0);

    //Dibujar las primitivas
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo_object);
    int size;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

    //Dibujar los tri�nglos
    glDrawElements(GL_TRIANGLES, size / sizeof(GLushort), GL_UNSIGNED_SHORT, 0);

    glDisableVertexAttribArray(attribute_coord);
    glDisableVertexAttribArray(attribute_color);
}

void onIdle() {
    Mesh& sol = scene.meshes[0];
    Mesh& tierra = scene.meshes[1];
    Mesh& luna = scene.meshes[2];
    float tierraAlrededorSol = glm::radians(alfa) / 500.0f;
    float tierraAlrededorSuEje = glm::radians(beta) / 500.0f;
    float lunaAlrededorTierra = glm::radians(theta) / 500.0f;
    float lunaAlrededorSuEje = glm::radians(phi) / 500.0f;

    glm::mat4 rotarTierraAlrededorSol = glm::rotate(glm::mat4(), tierraAlrededorSol, glm::vec3(0.0f, 1.0f, 0.0f));

    // A diferencia de otras transformaciones, en donde la matriz que las representa es constante, en este caso la matriz depende
    // de la posici�n actual del centro de la Tierra. Por lo tanto, en cada "iteraci�n" (cada vez que se llama a onIdle), hay que
    // actualizar el centro de la Tierra, como se ver� m�s abajo.
    glm::mat4 rotarTierraAlrededorSuEje = glm::translate(glm::mat4(), tierra.centro) *
        glm::rotate(glm::mat4(), tierraAlrededorSuEje, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::translate(glm::mat4(), -tierra.centro);

    // NO OLVIDAR: la matriz de transformaci�n SIEMPRE VA ANTES QUE LA MATRIZ QUE REPRESENTA EL MODELO.
    glm::mat4 transformacionTierra = rotarTierraAlrededorSol * rotarTierraAlrededorSuEje;
    tierra.model_transform = transformacionTierra * tierra.model_transform;

    // Se le aplica al centro la misma transformaci�n que a la Tierra. NO ES LO MISMO que aplicarle el model_transform de la Tierra a su centro.
    moverCentro(transformacionTierra, tierra);

    glm::mat4 moverLunaAlOrigen = glm::translate(glm::mat4(), -luna.centro);
    glm::mat4 rotarLunaAlrededorSuEje = glm::rotate(glm::mat4(), lunaAlrededorSuEje, glm::vec3(0.0f, 1.0f, 0.0f));

    glm::vec3 direccionCentroTierra = glm::normalize(tierra.centro);

    glm::mat4 mover1HaciaTierra = glm::translate(glm::mat4(), 1.0f * direccionCentroTierra);
    glm::mat4 rotarAnguloLunaAlrededorTierra = glm::rotate(glm::mat4(), lunaAlrededorTierra, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 moverAPosicionFinal = glm::translate(glm::mat4(), 4.0f * direccionCentroTierra);

    glm::mat4 transformacionLuna = moverAPosicionFinal *
        rotarAnguloLunaAlrededorTierra *
        mover1HaciaTierra *
        rotarLunaAlrededorSuEje *
        moverLunaAlOrigen;

    luna.model_transform = transformacionLuna * luna.model_transform;

    moverCentro(transformacionLuna, luna);

    // glutPostRedisplay(): tells GLUT to schedule a call to your display function once all the events are processed and before your idle function is called
    glutPostRedisplay();
}

void onDisplay() {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (auto& mesh : scene.meshes) {
        graficarObjeto(mesh);
    }
    glutSwapBuffers();
}

void onReshape(int w, int h) {
    screen_width = w;
    screen_height = h;

    glViewport(0, 0, screen_width, screen_height);
}

void free_resources() {
    glDeleteProgram(program);
    for (auto& mesh : scene.meshes) {
        glDeleteBuffers(1, &mesh.vbo_object);
        glDeleteBuffers(1, &mesh.ibo_object);
        glDeleteBuffers(1, &mesh.vbo_color);
    }
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitContextVersion(2, 0);
    glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(screen_width, screen_height);
    glutCreateWindow("OpenGL");

    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK) {
        std::cout << "Error inicializando GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (!GLEW_VERSION_2_0) {
        std::cout << "Tu tarjeta grafica no soporta OpenGL 2.0" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (init_resources()) {
        glutDisplayFunc(onDisplay);
        glutReshapeFunc(onReshape);
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glutIdleFunc(onIdle);
        glutMainLoop();
    }

    free_resources();
    exit(EXIT_SUCCESS);
}
