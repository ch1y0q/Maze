#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "maths.h"

typedef struct {
    int width, height;
    vec4_t *colorbuffer;
} framebuffer_t;


/* framebuffer management */
framebuffer_t *framebuffer_create(int width, int height);

void framebuffer_release(framebuffer_t *framebuffer);

void framebuffer_clear_color(framebuffer_t *framebuffer, vec4_t color);

void framebuffer_copy(framebuffer_t *a, const framebuffer_t *b, ivec4_t range);

void set_pixel(framebuffer_t *framebuffer, int x, int y, float r, float g, float b);

void set_pixel(framebuffer_t *framebuffer, int x, int y, vec3_t color);

void alpha_blend(framebuffer_t *framebuffer, int x, int y, float alpha, float r, float g, float b);

/* graphics drawing*/
void setpixel(int x, int y, float r, float g, float b);

void setpixel(int x, int y, vec3_t color);

void alphablend(int x, int y, float alpha, float r, float g, float b);

ivec4_t capsuleAABB(float ax, float ay, float bx, float by, float r);

void draw_line(float ax, float ay, float bx, float by, float r, vec3_t color);

ivec4_t circleAABB(float cx, float cy, float r);

void draw_circle(float cx, float cy, float r, vec3_t color);

ivec4_t boxAABB(float cx, float cy, float theta, float w, float h);

void draw_box(float cx, float cy, float theta, float w, float h, vec3_t color);

void draw_filleted_box(float cx, float cy, float theta, float w, float h, float r, vec3_t color);


#endif
