#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <vector>
#include <chrono>
#include <random>

#include "gamelogic.h"
#include "platform.h"
#include "graphics.h"
#include "macro.h"
#include "input.h"

#include <iostream>
using namespace std;

int WINDOW_WIDTH = 720;
int WINDOW_HEIGHT = 720;
int MAZE_WIDTH = 40;
int MAZE_HEIGHT = 20;

float bar_boundary_rate = 0.0;
float maze_margin_rate  = 0.04;

float mouse_moving_interval = 0.15;


float maze_margin_bottom = W_H * maze_margin_rate;
float maze_margin_top    = W_H - maze_margin_bottom;
float maze_margin_left   = maze_margin_bottom;
float maze_margin_right  = W_W * (1.0 - bar_boundary_rate) - maze_margin_bottom;
float maze_area_width  = maze_margin_right - maze_margin_left;
float maze_area_height = maze_margin_top - maze_margin_bottom;

framebuffer_t *framebuffer;
framebuffer_t *tempbuffer;

/* maze management
 *
 * "1" in mazemap stands for walls
 * "0" in mazemap stands for ground
 */

class maze_t
{
public:
    maze_t();
    ~maze_t();
    bool *mazemap;
    void refresh();
    void draw();
private:
    void initialize();
    void randomize();
    
};

maze_t::maze_t()
{
    int count = RMW * RMH;
    this->mazemap = (bool*)malloc(sizeof(bool) * count);
    
}

maze_t::~maze_t()
{
    free(this->mazemap);
}

void maze_t::refresh()
{
    initialize();
    randomize();
}

void maze_t::initialize()
{
    int iter = 0;
    for (int i = 0; i < RMH; ++i) {
        for (int j = 0; j < RMW; ++j, ++iter) {
            if (i % 2 == 0 || j % 2 == 0) {
                this->mazemap[iter] = 1;
            }
            else {
                this->mazemap[iter] = 0;
            }
        }
    }
}

/* Inside-Out Algorithm */
void create_random_permutation(vector<int>& res)
{
    uint32_t seed = (uint32_t)std::chrono::system_clock::now().time_since_epoch().count();
    mt19937 rand_num(seed);
    
    vector<int> t1(res.size());
    for (int i = 0; i < res.size(); ++i) {
        t1[i] = i;
    }
    
    res.assign(t1.size(),0);
    copy(t1.begin(),t1.end(),res.begin());
    
    int k;
    for (int i = 0; i < t1.size(); ++i) {
        uniform_int_distribution<int> dist(0, i);
        k = dist(rand_num);
        res[i] = res[k];
        res[k] = t1[i];
    }
}

/* go          up,          down,       left, right */
int offa[4] = {MAZE_WIDTH, -MAZE_WIDTH, -1,   1};   //offset in vector isaccessed
int offr[4] = {1,          -1,           0,   0};   //offset of row     or y
int offc[4] = {0,           0,          -1,   1};   //offset of column  or x

void maze_t::randomize()
{
    int count = MAZE_WIDTH * MAZE_HEIGHT;
    /* initiallize random number generators */
    uint32_t seed = (uint32_t)std::chrono::system_clock::now().time_since_epoch().count();
    mt19937 rand_num(seed);
    uniform_int_distribution<int> dist(0, count - 1);   //random number within [0, count - 1]
    uniform_int_distribution<int> dist_0_3(0, 3);       //random number within [0, 3]
    int cur = dist(rand_num);   //starting point
    
    vector<int> accessed;               //the points have accessed will be pushed in
    vector<int> isaccessed(count, 0);   //0 for not accessed already
    accessed.push_back(cur);
    isaccessed[cur] = 1;

    for (;accessed.size() < count;) {
        int tx = cur % MAZE_WIDTH;      //abscissa of current point
        int ty = cur / MAZE_WIDTH;      //ordinate of current point
        int offs = -1;
        for (int j = 0; j < 5; ++j) {   //for four directions
            int around = dist_0_3(rand_num);
            int nx = tx + offc[around]; //abscissa of new point
            int ny = ty + offr[around]; //ordinate of new point
            /* if the new point is in the boundary and not accessed */
            if (ny >= 0 && nx >= 0 && ny < MAZE_HEIGHT && nx < MAZE_WIDTH && !isaccessed[cur + offa[around]]) {
                offs = around;
                break;
            }
        }
        /* if each dirctions have accessed */
        if (offs < 0) {
            /* choose an accessed point as current */
            uniform_int_distribution<int> dist(0, (int)accessed.size() - 1);
            int tcur = dist(rand_num);
            cur = accessed[tcur];
        }
        else {
            /* choose a nearby point as current */
            tx = tx * 2 + 1 + offc[offs];
            ty = ty * 2 + 1 + offr[offs];
            this->mazemap[ty * (MAZE_WIDTH * 2  + 1) + tx] = 0;
            cur += offa[offs];
            isaccessed[cur] = 1;
            accessed.push_back(cur);
        }
    }
}

float box_length_rate = 0.8;
float filleted_rate = 0.2;
float ele_size;
float spx, spy;

void maze_t::draw()
{
    int rmw = RMW, rwh = RMH;
    /* if maze_width is much longer */
    if ((float)rmw / rwh > MA_W / MA_H) {
        ele_size = MA_W / rmw;
        spx = ele_size / 2;
        spy = (MA_H - ele_size * rwh) / 2 + spx;
    }
    else {
        ele_size = MA_H / rwh;
        spy = ele_size / 2;
        spx = (MA_W - ele_size * rmw) / 2 + spy;
    }
    
    float box_length = ele_size * box_length_rate;
    float fl = filleted_rate * box_length;
    for (int i = 0; i < rwh; ++i) {
        for (int j = 0; j < rmw; ++j) {
            if (this->mazemap[i * rmw + j])
                draw_filleted_box(MM_L + spx + j * ele_size, MM_B + spy + i * ele_size, 0, box_length, box_length, fl, vec3_new(0.4, 0.4, 0.4));
        }
    }
}

/* mouse management */
class mouse_t
{
public:
    int x = M_W - (!(M_W & 1));
    int y = M_H + (!(M_H & 1));
    float p_x;
    float p_y;
    bool is_moving = 0;
    int to_move = -1;
    ivec4_t AABB();
    
    void draw();
    void draw(float dt);
    void move(int dx, int dy);
private:

};

ivec4_t mouse_t::AABB()
{
    return circleAABB(p_x, p_y, ele_size);
}

void mouse_t::draw()
{
    draw_circle(p_x, p_y, ele_size / 2.2, vec3_new(0.9, 0.9, 0.9));
}

void mouse_t::draw(float dt)
{
    if (this->to_move == 0) {
        this->p_y = MM_B + spy + y * ele_size + quadratic_smooth(dt, mouse_moving_interval, ele_size);
    }
    if (this->to_move == 1) {
        this->p_y = MM_B + spy + y * ele_size - quadratic_smooth(dt, mouse_moving_interval, ele_size);
    }
    if (this->to_move == 2) {
        this->p_x = MM_L + spx + x * ele_size - quadratic_smooth(dt, mouse_moving_interval, ele_size);
    }
    if (this->to_move == 3) {
        this->p_x = MM_L + spx + x * ele_size + quadratic_smooth(dt, mouse_moving_interval, ele_size);
    }
    draw();
}

void mouse_t::move(int dx, int dy)
{
    this->x += dx;
    this->y += dy;
    this->p_x = MM_L + spx + x * ele_size;
    this->p_y = MM_B + spy + y * ele_size;
}



int in_game_loop(window_t *window)
{
    maze_t maze;
    mouse_t mouse;
    
    cout << RMW << " " << RMW << endl;
    /* show maze area */
    draw_box((MM_R + MM_L)/2, (MM_T + MM_B)/2, 0, MM_R - MM_L, MM_T - MM_B, vec3_new(0.1,0.12,0.15));
    maze.refresh();
    maze.draw();
    
    record_t record;
    memset(&record, 0, sizeof(record_t));
    callbacks_t callbacks;
    memset(&callbacks, 0, sizeof(callbacks_t));
    callbacks.button_callback = button_callback;
    window_set_userdata(window, &record);
    input_set_callbacks(window, callbacks);
    
    bool acc_key = 1;
    float prev_time = platform_get_time();
    float print_time = prev_time;
    
    mouse.move(0,0);
    framebuffer_copy(tempbuffer, framebuffer, mouse.AABB());
    mouse.draw();
    window_draw_buffer(window, framebuffer);
    framebuffer_copy(framebuffer, tempbuffer, mouse.AABB());
    
    while (!window_should_close(window)) {
        
        float curr_time = platform_get_time();
        float delta_time = curr_time - prev_time;
        
        update_click(curr_time, &record);
        update_key(window, curr_time, &record);
        
        if (record.single_click == 1 || record.double_click == 1) {
            cout << 1 << endl;
        }
        if (delta_time > 0.1) {
            acc_key = 1;
        }
        if (!mouse.is_moving && acc_key) {
            bool is_acc = 0;
            if (record.key[KEY_A] && !maze.mazemap[mouse.y * RMW + mouse.x - 1]) {
                mouse.to_move = 2;
                is_acc = 1;
            }
            else if (record.key[KEY_S] && !maze.mazemap[(mouse.y - 1) * RMW + mouse.x]) {
                mouse.to_move = 1;
                is_acc = 1;
            }
            else if (record.key[KEY_D] && !maze.mazemap[mouse.y * RMW + mouse.x + 1]) {
                mouse.to_move = 3;
                is_acc = 1;
            }
            else if (record.key[KEY_W] && !maze.mazemap[(mouse.y + 1) * RMW + mouse.x]) {
                mouse.to_move = 0;
                is_acc = 1;
            }

            if (is_acc) {
                mouse.is_moving = 1;
                prev_time = curr_time;
                print_time = curr_time;
                acc_key = 0;
            }
        }
        if (mouse.is_moving) {
            if (curr_time - print_time >= mouse_moving_interval) {
                mouse.is_moving = 0;
                mouse.move(offc[mouse.to_move], offr[mouse.to_move]);
                cout << mouse.x << " " << mouse.y << endl;
            }
            else {
                ivec4_t AABB = mouse.AABB();
                framebuffer_copy(tempbuffer, framebuffer, AABB);
                mouse.draw(curr_time - print_time);
                window_draw_buffer(window, framebuffer);
                framebuffer_copy(framebuffer, tempbuffer, AABB);
            }
        }
        /* if reaches end */
        if (mouse.x + 2 == RMW && mouse.y - 1 == 0) {
            cout << " win " << endl;
            return 1;
        }
        record.single_click = 0;
        record.double_click = 0;
        memset(record.key, 0, sizeof(record.key));
        input_poll_events();
    }
    return 0;
}

void main_loop()
{
    window_t *window;
    window = window_create("Maze", W_W, W_H);
    framebuffer = framebuffer_create(W_W, W_H);
    tempbuffer  = framebuffer_create(W_W, W_H);
    
    for (int i = 0; i < W_W * W_H ; ++i)
        framebuffer->colorbuffer[i] = vec4_new(0.02,0.05,0.06,1);
    
    while (in_game_loop(window)) {
        cout << " restart " << endl;
    }
    window_destroy(window);
}






