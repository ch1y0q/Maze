#include "macro.h"
#include "input.h"
#include "maths.h"
#include "graphics.h"
#include "platform.h"
#include "gamelogic.h"

const float CLICK_DELAY = 0.25f;

static vec2_t get_pos_delta(vec2_t old_pos, vec2_t new_pos)
{
    vec2_t delta = vec2_sub(new_pos, old_pos);
    return vec2_div(delta, (float)WINDOW_HEIGHT);
}

static vec2_t get_cursor_pos(window_t *window)
{
    float xpos, ypos;
    input_query_cursor(window, &xpos, &ypos);
    return vec2_new(xpos, ypos);
}

void button_callback(window_t *window, button_t button, int pressed)
{
    record_t *record = (record_t*)window_get_userdata(window);
    vec2_t cursor_pos = get_cursor_pos(window);
    if (button == BUTTON_L) {
        float curr_time = platform_get_time();
        if (pressed) {
            record->is_orbiting = 1;
            record->orbit_pos = cursor_pos;
            record->press_time = curr_time;
            record->press_pos = cursor_pos;
        } else {
            float prev_time = record->release_time;
            vec2_t pos_delta = get_pos_delta(record->orbit_pos, cursor_pos);
            record->is_orbiting = 0;
            record->orbit_delta = vec2_add(record->orbit_delta, pos_delta);
            if (prev_time && curr_time - prev_time < CLICK_DELAY) {
                record->double_click = 1;
                record->release_time = 0;
            } else {
                record->release_time = curr_time;
                record->release_pos = cursor_pos;
            }
        }
    } else if (button == BUTTON_R) {
        if (pressed) {
            record->is_panning = 1;
            record->pan_pos = cursor_pos;
        } else {
            vec2_t pos_delta = get_pos_delta(record->pan_pos, cursor_pos);
            record->is_panning = 0;
            record->pan_delta = vec2_add(record->pan_delta, pos_delta);
        }
    }
}

void update_click(float curr_time, record_t *record) {
    float last_time = record->release_time;
    if (last_time && curr_time - last_time > CLICK_DELAY) {
        vec2_t pos_delta = vec2_sub(record->release_pos, record->press_pos);
        if (vec2_length(pos_delta) < 5) {
            record->single_click = 1;
        }
        record->release_time = 0;
    }
    if (record->single_click || record->double_click) {
        float click_x = record->release_pos.x / WINDOW_WIDTH;
        float click_y = record->release_pos.y / WINDOW_HEIGHT;
        record->click_pos = vec2_new(click_x, 1 - click_y);
    }
}

void update_key(window_t *window, float curr_time, record_t *record) {
    float last_time = record->release_time;
    if (1||(last_time && curr_time - last_time > CLICK_DELAY)) {
        if (input_key_pressed(window, KEY_SPACE)) {
            record->key[KEY_SPACE] = 1;
        }
        if (input_key_pressed(window, KEY_A)) {
            record->key[KEY_A] = 1;
        }
        if (input_key_pressed(window, KEY_D)) {
            record->key[KEY_D] = 1;
        }
        if (input_key_pressed(window, KEY_S)) {
            record->key[KEY_S] = 1;
        }
        if (input_key_pressed(window, KEY_W)) {
            record->key[KEY_W] = 1;
        }
        if (input_key_pressed(window, KEY_UP)) {
            record->key[KEY_UP] = 1;
        }
        if (input_key_pressed(window, KEY_LEFT)) {
            record->key[KEY_LEFT] = 1;
        }
        if (input_key_pressed(window, KEY_DOWN)) {
            record->key[KEY_DOWN] = 1;
        }
        if (input_key_pressed(window, KEY_RIGHT)) {
            record->key[KEY_RIGHT] = 1;
        }
        if (input_key_pressed(window, KEY_SHIFT)) {
            record->key[KEY_SHIFT] = 1;
        }
        if (input_key_pressed(window, KEY_ESCAPE)) {
            record->key[KEY_ESCAPE] = 1;
        }
    }
}
