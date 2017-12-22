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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "shader_utils.h"
#include "auxiliar.h"
#include "solucion.h"

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitContextVersion(2, 0);
    glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(screen_width, screen_height);
    glutCreateWindow("OpenGL");

    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK) {
        std::cout << "Error inicializando GLEW\n";
        exit(EXIT_FAILURE);
    }

    if (!GLEW_VERSION_2_0) {
        std::cout << "Tu tarjeta gráfica no soporta OpenGL 2.0\n";
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