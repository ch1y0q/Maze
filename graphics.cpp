#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include "graphics.h"
#include "macro.h"
#include "maths.h"
#include "gamelogic.h"

#include <iostream>
#include <algorithm>
using namespace std;

/* framebuffer management */

framebuffer_t *framebuffer_create(int width, int height)
{
    vec4_t default_color = {0, 0, 0, 1};
    int num_elems = width * height;
    framebuffer_t *framebuffer;

    assert(width > 0 && height > 0);

    framebuffer = (framebuffer_t*)malloc(sizeof(framebuffer_t));
    framebuffer->width = width;
    framebuffer->height = height;
    framebuffer->colorbuffer = (vec4_t*)malloc(sizeof(vec4_t) * num_elems);

    framebuffer_clear_color(framebuffer, default_color);

    return framebuffer;
}

void framebuffer_release(framebuffer_t *framebuffer)
{
    free(framebuffer->colorbuffer);
    free(framebuffer);
}

void framebuffer_clear_color(framebuffer_t *framebuffer, vec4_t color)
{
    int num_elems = framebuffer->width * framebuffer->height;
    for (int i = 0; i < num_elems; ++i) {
        framebuffer->colorbuffer[i] = color;
    }
}

void framebuffer_copy(framebuffer_t *a, const framebuffer_t *b, ivec4_t range)
{
    for (int y = range.z; y <= range.w; y++)
        for (int x = range.x; x <= range.y; x++)
            a->colorbuffer[x + y * WINDOW_WIDTH] = b->colorbuffer[x + y * WINDOW_WIDTH];
}

void set_pixel(framebuffer_t *framebuffer, int x, int y, float r, float g, float b)
{
    int iter = x + (y - 1) * framebuffer->width;
    framebuffer->colorbuffer[iter].x = r;
    framebuffer->colorbuffer[iter].y = g;
    framebuffer->colorbuffer[iter].z = b;
    framebuffer->colorbuffer[iter].w = 1;
}

void set_pixel(framebuffer_t *framebuffer, int x, int y, vec3_t color)
{
    set_pixel(framebuffer, x, y, color.x, color.y, color.z);
}

void alpha_blend(framebuffer_t *framebuffer, int x, int y, float alpha, float r, float g, float b)
{
    int iter = x + (y - 1) * framebuffer->width;
    vec4_t *colorbuffer = framebuffer->colorbuffer + iter;
    colorbuffer->x = colorbuffer->x * (1 - alpha) + r * alpha;
    colorbuffer->y = colorbuffer->y * (1 - alpha) + g * alpha;
    colorbuffer->z = colorbuffer->z * (1 - alpha) + b * alpha;
    
}


/* graphics drawing */

void setpixel(int x, int y, float r, float g, float b)
{
    set_pixel(framebuffer, x, y, r, g, b);
}

void setpixel(int x, int y, vec3_t color)
{
    set_pixel(framebuffer, x, y, color.x, color.y, color.z);
}

void alphablend(int x, int y, float alpha, float r, float g, float b)
{
    alpha_blend(framebuffer, x, y, alpha, r, g, b);
}

//LINE
float capsuleSDF(float px, float py, float ax, float ay, float bx, float by, float r)
{
    float pax = px - ax, pay = py - ay, bax = bx - ax, bay = by - ay;
    float h = fmaxf(fminf((pax * bax + pay * bay) / (bax * bax + bay * bay), 1.0f), 0.0f);
    float dx = pax - bax * h, dy = pay - bay * h;
    return sqrtf(dx * dx + dy * dy) - r;
}

ivec4_t capsuleAABB(float ax, float ay, float bx, float by, float r)
{
    int x0 = max((int)floorf(fminf(ax, bx) - r), 0);
    int x1 = min((int) ceilf(fmaxf(ax, bx) + r), WINDOW_WIDTH - 1);
    int y0 = max((int)floorf(fminf(ay, by) - r), 0);
    int y1 = min((int) ceilf(fmaxf(ay, by) + r), WINDOW_HEIGHT - 1);
    return ivec4_new(x0, x1, y0, y1);
}

void draw_line(float ax, float ay, float bx, float by, float r, vec3_t color)
{
    ivec4_t AABB = capsuleAABB(ax, ay, bx, by, r);
    for (int y = AABB.z; y <= AABB.w; y++)
        for (int x = AABB.x; x <= AABB.y; x++)
            alphablend(x, y, fmaxf(fminf(0.5f - capsuleSDF(x, y, ax, ay, bx, by, r), 1.0f), 0.0f), color.x, color.y, color.z);
}

//CIRCLE
float circleSDF(float x, float y, float cx, float cy, float r)
{
    double ux = x - cx, uy = y - cy;
    return sqrtf(ux * ux + uy * uy) - r;
}

ivec4_t circleAABB(float cx, float cy, float r)
{
    int x0 = max((int)floorf(cx - r) - 1, 0);;
    int x1 = min((int) ceilf(cx + r) + 1, WINDOW_WIDTH - 1);;
    int y0 = max((int)floorf(cy - r) - 1, 0);;
    int y1 = min((int) ceilf(cy + r) + 1, WINDOW_HEIGHT - 1);;
    return ivec4_new(x0, x1, y0, y1);
}

void draw_circle(float cx, float cy, float r, vec3_t color)
{
    ivec4_t AABB = circleAABB(cx, cy, r);
    for (int y = AABB.z; y <= AABB.w; y++)
        for (int x = AABB.x; x <= AABB.y; x++)
            alphablend(x, y, fmaxf(fminf(0.5f - circleSDF(x, y, cx, cy, r), 1.0f), 0.0f), color.x, color.y, color.z);
}

//BOX
float boxSDF(float x, float y, float cx, float cy, float theta, float w, float h)
{
    w *= 0.5;
    h *= 0.5;
    float costheta = cosf(theta), sintheta = sinf(theta);
    float dx = fabs((x - cx) * costheta + (y - cy) * sintheta) - w;
    float dy = fabs((y - cy) * costheta - (x - cx) * sintheta) - h;
    float ax = fmaxf(dx, 0.0f), ay = fmaxf(dy, 0.0f);
    return fminf(fmaxf(dx, dy), 0.0f) + sqrtf(ax * ax + ay * ay);
}

ivec4_t boxAABB(float cx, float cy, float theta, float w, float h)
{
    w *= 0.5;
    h *= 0.5;
    float costheta = fabs(cosf(theta)), sintheta = fabs(sinf(theta));
    int x0 = max((int)floorf(cx - w * costheta - h * sintheta) - 1, 0);;
    int x1 = min((int) ceilf(cx + w * costheta + h * sintheta) + 1, WINDOW_WIDTH - 1);
    int y0 = max((int)floorf(cy - w * sintheta - h * costheta) - 1, 0);;
    int y1 = min((int) ceilf(cy + w * sintheta + h * costheta) + 1, WINDOW_HEIGHT - 1);
    return ivec4_new(x0, x1, y0, y1);
}
void draw_box(float cx, float cy, float theta, float w, float h, vec3_t color)
{
    ivec4_t AABB = boxAABB(cx, cy, theta, w, h);
    for (int y = AABB.z; y <= AABB.w; y++)
        for (int x = AABB.x; x <= AABB.y; x++)
            alphablend(x, y, fmaxf(fminf(0.5f - boxSDF(x, y, cx, cy, theta, w, h), 1.0f), 0.0f), color.x, color.y, color.z);
}

void draw_filleted_box(float cx, float cy, float theta, float w, float h, float r, vec3_t color)
{
    ivec4_t AABB = boxAABB(cx, cy, theta, w, h);
    w -= r * 2.0;
    h -= r * 2.0;
    for (int y = AABB.z; y <= AABB.w; y++)
        for (int x = AABB.x; x <= AABB.y; x++)
            alphablend(x, y, fmaxf(fminf(0.5f - boxSDF(x, y, cx, cy, theta, w, h) + r, 1.0f), 0.0f), color.x, color.y, color.z);
}
