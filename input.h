#ifndef input_hpp
#define input_hpp

#include <stdio.h>
#include "macro.h"
#include "input.h"
#include "maths.h"
#include "graphics.h"
#include "platform.h"
#include "gamelogic.h"

class record_t{
public:
    int is_orbiting;
    vec2_t orbit_pos;
    vec2_t orbit_delta;
    /* pan */
    int is_panning;
    vec2_t pan_pos;
    vec2_t pan_delta;
    /* zoom */
    float dolly_delta;
    /* click */
    float press_time;
    float release_time;
    vec2_t press_pos;
    vec2_t release_pos;
    int single_click;
    int double_click;
    vec2_t click_pos;
    /* key */
    int key[KEYS_USED];
};

void button_callback(window_t *window, button_t button, int pressed);
void update_click(float curr_time, record_t *record);
void update_key(window_t *window, float delta_time, record_t *record);

#endif /* input_hpp */
