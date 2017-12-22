#pragma once

int screen_width = 800, screen_height = 800;

bool init_resources();
void onDisplay();
void onReshape(int width, int height);
void onIdle();
void free_resources();