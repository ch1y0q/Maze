#ifndef gamelogic_h
#define gamelogic_h


#include "graphics.h"
extern framebuffer_t *framebuffer;
extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;
// MAZE_WIDTH, MAZE_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT
const ivec4_t difficulty_list[] = {
        ivec4_new(2, 2, 300, 300),
        ivec4_new(10, 5, 500, 300),
        ivec4_new(20, 10, 600, 400),
        ivec4_new(20, 20, 600, 600),
        ivec4_new(30, 30, 700, 700),
        ivec4_new(40, 20, 800, 600),
        ivec4_new(40, 20, 1000, 800),
        ivec4_new(40, 40, 1000, 800),
        ivec4_new(45, 45, 1000, 800),
        ivec4_new(50, 50, 1200, 1200)
};

const vec4_t color_accent_list_bg[] = {
        vec4_new(0.02,0.05,0.06,1),
        vec4_new(0.10,0.08,0.06,0.80),
        vec4_new(0.5,0.5,0.5,0.60),
        vec4_new(0.10,0.08,0.06,0.80),
        vec4_new(0.10,0.08,0.06,0.80),
        vec4_new(0.02,0.05,0.06,1),
        vec4_new(0.10,0.08,0.06,0.80),
        vec4_new(0.02,0.05,0.06,1),
        vec4_new(0.10,0.08,0.06,0.80),
        vec4_new(0.10,0.08,0.06,0.80)
};

const vec3_t color_accent_list_maze[] = {
        vec3_new(0.4, 0.4, 0.4),
        vec3_new(0.6, 0.7, 0.45),
        vec3_new(0.4, 0.4, 1.4),
        vec3_new(0.6, 0.7, 0.1),
        vec3_new(0.4, 0.4, 0.8),
        vec3_new(0.6, 0.7, 0.45),
        vec3_new(0.4, 0.8, 0.4),
        vec3_new(1.6, 0.7, 0.45),
        vec3_new(0.6, 1.6, 0.6),
        vec3_new(1.6, 1.7, 1.8)
};

const vec3_t color_accent_list_mouse[] = {
        vec3_new(0.9, 0.9, 0.9),
        vec3_new(0.6, 0.2, 0.4),
        vec3_new(0.9, 0.9, 0.9),
        vec3_new(0.6, 0.2, 0.4),
        vec3_new(0.9, 0.9, 0.9),
        vec3_new(0.6, 0.2, 0.4),
        vec3_new(0.9, 0.9, 0.9),
        vec3_new(0.9, 0.2, 0.4),
        vec3_new(0.9, 0.9, 0.9),
        vec3_new(0.6, 0.2, 0.4)
};

const vec3_t color_accent_list_box[] = {
        vec3_new(0.1,0.12,0.15),
        vec3_new(0.1,0.12,0.15),
        vec3_new(0.1,0.12,0.15),
        vec3_new(0.1,0.12,0.15),
        vec3_new(0.1,0.12,0.15),
        vec3_new(0.1,0.12,0.15),
        vec3_new(0.1,0.12,0.15),
        vec3_new(0.1,0.12,0.15),
        vec3_new(0.1,0.12,0.15),
        vec3_new(0.1,0.12,0.15)
};

const vec3_t color_accent_list_hint[] = {
        vec3_new(0.4,0.12,0.15),
        vec3_new(0.4,0.12,0.15),
        vec3_new(0.4,0.12,0.15),
        vec3_new(0.4,0.12,0.15),
        vec3_new(0.4,0.12,0.15),
        vec3_new(0.4,0.12,0.15),
        vec3_new(0.4,0.12,0.15),
        vec3_new(0.4,0.12,0.15),
        vec3_new(0.4,0.12,0.15),
        vec3_new(0.4,0.12,0.15)
};

void main_loop(int difficulty = 0 , int color_accent = 0, int players = 1, int timing = 0);

#endif /* gamelogic_hpp */
