#pragma once
#include "auxiliar.h"

extern int screen_width;
extern int screen_height;

bool init_resources();
void onDisplay();
void onReshape(int width, int height);
void onIdle();
void free_resources();