#pragma once
#include <pebble.h>

#define NUM_SEGMENTS 8
#define DEFAULT_WAKE_HOUR 7
#define DEFAULT_SLEEP_HOUR 23

#define PERSIST_KEY_WAKE 1
#define PERSIST_KEY_SLEEP 2

typedef struct {
    float energy_level;
    int filled_segments;
    int wake_hour;
    int sleep_hour;
    bool show_details;
    char time_str[8];
    char date_str[16];
    int battery_percent;
    int pulse_anim_frame;
    GColor segment_colors[NUM_SEGMENTS];
} EnergyState;

void energy_logic_init(EnergyState *state);
void energy_logic_update(EnergyState *state, int hour, int min, int battery_percent, bool is24h);
float energy_calc_level(int hour, int min, int wake_hour, int sleep_hour);
int energy_calc_segments(float energy_level);
void energy_calc_colors(GColor *colors, int filled_segments);
void energy_logic_draw(GContext *ctx, GRect bounds, EnergyState *state);
