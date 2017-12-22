#include "solucion.h"

void onReshape(int w, int h) {
    screen_width = w;
    screen_height = h;
    glViewport(0, 0, screen_width, screen_height);
}