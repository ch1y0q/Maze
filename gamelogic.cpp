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
#include <iomanip>

// #define DEBUG
using namespace std;

// re-assigned in main_loop
int WINDOW_WIDTH = 720;
int WINDOW_HEIGHT = 720;
int MAZE_WIDTH = 40;
int MAZE_HEIGHT = 20;


float bar_boundary_rate = 0.0;
float maze_margin_rate = 0.04;

float mouse_moving_interval = 0.15;
float key_interval = 0.25;

float maze_margin_bottom;
float maze_margin_top;
float maze_margin_left;
float maze_margin_right;
float maze_area_width;
float maze_area_height;


framebuffer_t *framebuffer;
framebuffer_t *tempbuffer;

// re-assigned in main_loop
int color_accent = 0;
int players = 1;
int timing = 0;

/* maze management
 *
 * "1" in mazemap stands for walls
 * "0" in mazemap stands for ground
 */

class maze_t {
public:
    maze_t();
    maze_t(int, int);
    ~maze_t();

    void refresh();
    void draw();
    void draw_hint();

    int width;
    int height;
    bool *mazemap;

    void dfs(int);
    vector<int> hint;
    vector<int> path;
private:
    void initialize();
    void randomize();
};

maze_t::maze_t() : width(M_W), height(M_H) {
    int count = (width * 2 + 1) * (height * 2 + 1);
    this->mazemap = (bool *) malloc(sizeof(bool) * count);
    path.clear();
}

maze_t::maze_t(int w, int h) {
    width = w;
    height = h;
    M_W = width;
    M_H = height;
    maze_t();
}

maze_t::~maze_t() {
    free(this->mazemap);
}

void maze_t::refresh() {
    initialize();
    randomize();
}

void maze_t::initialize() {
    int iter = 0;
    for (int i = 0; i < (height * 2 + 1); ++i) {
        for (int j = 0; j < (width * 2 + 1); ++j, ++iter) {
            if (i % 2 == 0 || j % 2 == 0) {
                this->mazemap[iter] = 1;
            } else {
                this->mazemap[iter] = 0;
            }
        }
    }
}

/* go          up,          down,       left,       right */
int offa[4] = {MAZE_WIDTH, -MAZE_WIDTH,     -1,         1};     //offset in vector isaccessed
int offr[4] = {         1,          -1,      0,         0};     //offset of row     or y
int offc[4] = {         0,           0,     -1,         1};     //offset of column  or x

void maze_t::randomize() {
    int maze_size = width * height;
    /* initiallize random number generators */
    uint32_t seed = (uint32_t) std::chrono::system_clock::now().time_since_epoch().count();
    mt19937 rand_num(seed);
    /* use uniform_int_distribution to yield uniformly distributed random number in an interval */
    uniform_int_distribution<int> dist(0, maze_size - 1);   //random number within [0, count - 1]
    uniform_int_distribution<int> dist_0_3(0, 3);       //random number within [0, 3]
    int cur = dist(rand_num);   //starting point

    vector<int> accessed;               //the points have accessed will be pushed in
    vector<int> is_accessed(maze_size, 0);   //0 for not accessed yet
    accessed.push_back(cur);
    is_accessed[cur] = 1;

    while (accessed.size() < maze_size) {
        int tx = cur % width;      //abscissa of current point
        int ty = cur / width;      //ordinate of current point
        int offs = -1;
        for (int j = 0; j < 4; ++j) {   //for four directions
            int around = dist_0_3(rand_num);
            int nx = tx + offc[around]; //abscissa of new point
            int ny = ty + offr[around]; //ordinate of new point
            /* if the new point is within the boundary and not accessed */
            if (ny >= 0 && nx >= 0 && ny < height && nx < width && !is_accessed[cur + offa[around]]) {
                offs = around;
                break;
            }
        }
        /* if all directions have accessed */
        if (offs < 0) {
            /* choose an accessed point as current */
            uniform_int_distribution<int> dist(0, (int) accessed.size() - 1);
            int tcur = dist(rand_num);
            cur = accessed[tcur];
        } else {
            /* choose a nearby point as current */
            tx = tx * 2 + 1 + offc[offs];
            ty = ty * 2 + 1 + offr[offs];

            /* update mazemap*/
            this->mazemap[ty * (width * 2 + 1) + tx] = 0;

            /* update is_accssed and accessed */
            cur += offa[offs];
            is_accessed[cur] = 1;
            accessed.push_back(cur);
        }
    }
}

void maze_t::dfs(int pos) {
    if (pos == 4 * width) {
        hint.assign(path.begin(), path.end());
        return;
    }
    int y = pos / RMW;
    int x = pos % RMW;
    for (int i = 0; i < 4; ++i) {
        int nx = x + offc[i]; //abscissa of new point
        int ny = y + offr[i]; //ordinate of new point

        /* if the new point is within the boundary */
        if (ny >= 0 && nx >= 0 && ny < RMH && nx < RMW) {
            if (!mazemap[ny * RMW + nx] && (find(path.begin(), path.end(), ny * RMW + nx) == path.end())) {
                path.push_back(ny * RMW + nx);
                dfs(ny * RMW + nx);
                path.pop_back();
            }
        }
    }
    return;
}

float box_length_rate = 0.8;
float filleted_rate = 0.2;
float ele_size;
float spx, spy;

void maze_t::draw() {
    int rmw = RMW, rwh = RMH;
    /* if maze_width is much longer */
    if ((float) rmw / rwh > MA_W / MA_H) {
        ele_size = MA_W / rmw;
        spx = ele_size / 2;
        spy = (MA_H - ele_size * rwh) / 2 + spx;
    } else {
        ele_size = MA_H / rwh;
        spy = ele_size / 2;
        spx = (MA_W - ele_size * rmw) / 2 + spy;
    }

    float box_length = ele_size * box_length_rate;
    float fl = filleted_rate * box_length;
    for (int i = 0; i < rwh; ++i) {
        for (int j = 0; j < rmw; ++j) {
            if (this->mazemap[i * rmw + j])
                draw_filleted_box(MM_L + spx + j * ele_size, MM_B + spy + i * ele_size, 0, box_length, box_length, fl,
                                  color_accent_list_maze[color_accent]);
        }
    }
}

void maze_t::draw_hint() {
    int rmw = RMW, rwh = RMH;
    /* if maze_width is much longer */
    if ((float) rmw / rwh > MA_W / MA_H) {
        ele_size = MA_W / rmw;
        spx = ele_size / 2;
        spy = (MA_H - ele_size * rwh) / 2 + spx;
    } else {
        ele_size = MA_H / rwh;
        spy = ele_size / 2;
        spx = (MA_W - ele_size * rmw) / 2 + spy;
    }

    float box_length = ele_size * box_length_rate;
    float fl = filleted_rate * box_length;

    for (int it : hint) {
        int i = it / rmw;
        int j = it % rmw;
#ifdef DEBUG
        cout << "DRAW" << i << " " << j << endl;
#endif
        draw_filleted_box(MM_L + spx + j * ele_size, MM_B + spy + i * ele_size, 0, box_length, box_length, fl,
                          color_accent_list_hint[color_accent]);
    }
}

/* mouse management */
class mouse_t {
public:
    mouse_t();

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

    void set_color(int);

    void set_color(vec3_t);

private:
    vec3_t color;
};

mouse_t::mouse_t() {
    color = color_accent_list_mouse[color_accent];
}

ivec4_t mouse_t::AABB() {
    return circleAABB(p_x, p_y, ele_size);
}

void mouse_t::draw() {
    draw_circle(p_x, p_y, ele_size / 2.2, color);
}

void mouse_t::draw(float dt) {
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

void mouse_t::move(int dx, int dy) {
    this->x += dx;
    this->y += dy;
    this->p_x = MM_L + spx + x * ele_size;
    this->p_y = MM_B + spy + y * ele_size;
}

void mouse_t::set_color(int in_color) {
    color = color_accent_list_mouse[in_color];
}

void mouse_t::set_color(vec3_t in_color) {
    color = in_color;
}

int in_game_loop(window_t *window) {
    maze_t maze;
    mouse_t mouse;

#ifdef DEBUG
    cout << RMW << " " << RMW << endl;
#endif

    /* show maze area */
    draw_box((MM_R + MM_L) / 2, (MM_T + MM_B) / 2, 0, MM_R - MM_L, MM_T - MM_B, vec3_new(0.1, 0.12, 0.15));
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
    bool is_hinted = false;

    mouse.move(0, 0);
    framebuffer_copy(tempbuffer, framebuffer, mouse.AABB());
    mouse.draw();
    window_draw_buffer(window, framebuffer);
    framebuffer_copy(framebuffer, tempbuffer, mouse.AABB());

    float start_time = platform_get_time();
    float hint_prev_time = platform_get_time();
    float new_prev_time = platform_get_time();
    while (!window_should_close(window)) {

        float curr_time = platform_get_time();
        float delta_time = curr_time - prev_time;


        update_click(curr_time, &record);
        update_key(window, curr_time, &record);

        /* single and double click not used
        if (record.single_click == 1 || record.double_click == 1) {
            cout << 1 << endl;
        }
        */

        if (delta_time > 0.1) {
            acc_key = 1;
        }

        // navigate with A, W, S, D or arrow keys
        if (!mouse.is_moving && acc_key) {
            bool is_acc = 0;
            if ((record.key[KEY_A] || record.key[KEY_LEFT]) && !maze.mazemap[mouse.y * RMW + mouse.x - 1]) {
                mouse.to_move = 2;
                is_acc = 1;
            } else if ((record.key[KEY_S] || record.key[KEY_DOWN]) && !maze.mazemap[(mouse.y - 1) * RMW + mouse.x]) {
                mouse.to_move = 1;
                is_acc = 1;
            } else if ((record.key[KEY_D] || record.key[KEY_RIGHT]) && !maze.mazemap[mouse.y * RMW + mouse.x + 1]) {
                mouse.to_move = 3;
                is_acc = 1;
            } else if ((record.key[KEY_W] || record.key[KEY_UP]) && !maze.mazemap[(mouse.y + 1) * RMW + mouse.x]) {
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
#ifdef DEBUG
                cout << mouse.x << " " << mouse.y << endl;
#endif
            } else {
                ivec4_t AABB = mouse.AABB();
                framebuffer_copy(tempbuffer, framebuffer, AABB);
                mouse.draw(curr_time - print_time);
                window_draw_buffer(window, framebuffer);
                framebuffer_copy(framebuffer, tempbuffer, AABB);
            }
        }

        /* return is pressed = new game */
        if (record.key[KEY_RETURN] && acc_key && curr_time - new_prev_time >= key_interval) {
            cout << " new game " << endl;
            draw_box((MM_R + MM_L) / 2, (MM_T + MM_B) / 2, 0, MM_R - MM_L, MM_T - MM_B, vec3_new(0.1, 0.12, 0.15));
            maze.refresh();
            maze.draw();

            memset(&record, 0, sizeof(record_t));
            memset(&callbacks, 0, sizeof(callbacks_t));
            callbacks.button_callback = button_callback;

            acc_key = 1;
            prev_time = platform_get_time();
            print_time = prev_time;
            is_hinted = false;

            mouse = mouse_t();          // move the mouse to center
            mouse.move(0, 0);
            framebuffer_copy(tempbuffer, framebuffer, mouse.AABB());
            mouse.draw();
            window_draw_buffer(window, framebuffer);
            framebuffer_copy(framebuffer, tempbuffer, mouse.AABB());

            start_time = platform_get_time();
            hint_prev_time = platform_get_time();
            new_prev_time = platform_get_time();
            continue;
        }

        /* esc is pressed = quit */
        if (record.key[KEY_ESCAPE]) {
            cout << " quit " << endl;
            return 0;
        }

        /* space is pressed = hint */
        if (record.key[KEY_SPACE] && acc_key && curr_time - hint_prev_time >= key_interval) {
            hint_prev_time = curr_time;
            is_hinted = !is_hinted;
            if (is_hinted) {
                cout << " hint " << endl;
                maze.path.clear();
                maze.path.push_back(mouse.y * RMW + mouse.x);
                maze.dfs(mouse.y * RMW + mouse.x);
#ifdef DEBUG
                for (auto it = maze.hint.begin(); it < maze.hint.end(); ++it) {
                    cout << *it << " ";
                }
                cout << endl;
#endif
                maze.draw_hint();

                ivec4_t AABB = mouse.AABB();
                framebuffer_copy(tempbuffer, framebuffer, AABB);
                mouse.draw();
                window_draw_buffer(window, framebuffer);
                framebuffer_copy(framebuffer, tempbuffer, AABB);
            } else {
                for (int i = 0; i < W_W * W_H; ++i)
                    framebuffer->colorbuffer[i] = color_accent_list_bg[color_accent];
                /* show maze area */
                draw_box((MM_R + MM_L) / 2, (MM_T + MM_B) / 2, 0, MM_R - MM_L, MM_T - MM_B,
                         color_accent_list_box[color_accent]);

                maze.draw();

                ivec4_t AABB = mouse.AABB();
                framebuffer_copy(tempbuffer, framebuffer, AABB);
                mouse.draw();
                window_draw_buffer(window, framebuffer);
                framebuffer_copy(framebuffer, tempbuffer, AABB);
            }
            acc_key = 0;
        }

        /* if reaches end */
        if (mouse.x + 2 == RMW && mouse.y - 1 == 0) {
            cout << " win " << endl;
            if (timing) cout << " " << fixed << setprecision(2) << curr_time - start_time << endl;
            return 1;
        }
        record.single_click = 0;
        record.double_click = 0;
        memset(record.key, 0, sizeof(record.key));
        input_poll_events();
    }
    return 0;
}


void main_loop(int difficulty, int color_accent, int players, int timing) {
    ::color_accent = color_accent;
    ::players = players;
    ::timing = timing;
    window_t *window;
    M_W = difficulty_list[difficulty].x;
    M_H = difficulty_list[difficulty].y;
    W_W = difficulty_list[difficulty].z;
    W_H = difficulty_list[difficulty].w;
    maze_margin_bottom = W_H * maze_margin_rate;
    maze_margin_top = W_H - maze_margin_bottom;
    maze_margin_left = maze_margin_bottom;
    maze_margin_right = W_W * (1.0 - bar_boundary_rate) - maze_margin_bottom;
    maze_area_width = maze_margin_right - maze_margin_left;
    maze_area_height = maze_margin_top - maze_margin_bottom;

    offa[0] = M_W;
    offa[1] = -M_W;

    window = window_create("Maze", W_W, W_H);
    framebuffer = framebuffer_create(W_W, W_H);
    tempbuffer = framebuffer_create(W_W, W_H);
    for (int i = 0; i < W_W * W_H; ++i)
        framebuffer->colorbuffer[i] = color_accent_list_bg[color_accent];

    while (in_game_loop(window)) {
        cout << " restart " << endl;
    }
    window_destroy(window);
}
