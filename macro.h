#ifndef MACRO_H
#define MACRO_H

#define EPSILON 1e-5f
#define PI 3.1415927f
#define TWO_PI 6.28318530718

#define N 64
#define MAX_STEP 10
#define MAX_DISTANCE 2.0

#define TO_RADIANS(degrees) ((PI / 180) * (degrees))
#define TO_DEGREES(radians) ((180 / PI) * (radians))

#define LINE_SIZE 1024
#define PATH_SIZE 2048

#define UNUSED_VAR(x) ((void)(x))
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

#define W_W WINDOW_WIDTH
#define W_H WINDOW_HEIGHT
#define M_W MAZE_WIDTH
#define M_H MAZE_HEIGHT

#define MM_B maze_margin_bottom
#define MM_T maze_margin_top
#define MM_L maze_margin_left
#define MM_R maze_margin_right
#define MA_W maze_area_width
#define MA_H maze_area_height

#define RMW (MAZE_WIDTH * 2 + 1)
#define RMH (MAZE_HEIGHT * 2 + 1)

#define KEYS_USED 12

#endif
