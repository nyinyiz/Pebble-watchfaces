#pragma once
#include <pebble.h>

#define NUM_DROPS 15
#define FONT_HEIGHT 16
#define COLUMN_WIDTH 10

typedef struct {
    int x;
    int y;
    int speed;
    char chars[10];
    int length;
} MatrixDrop;

typedef struct {
    int battery_percent;
    bool is_charging;
    int step_count;
    char date_str[32];
} MatrixStats;

void matrix_logic_init(GRect bounds);
void matrix_logic_update();
void matrix_logic_draw(GContext *ctx, MatrixStats *stats);
void matrix_logic_deinit();
