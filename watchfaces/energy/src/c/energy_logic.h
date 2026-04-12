#pragma once
#include <pebble.h>

typedef struct {
    float energy_level; // 1.0 (Full) to 0.0 (Empty)
    bool show_time;
    char time_str[8];
    int pulse_anim_frame;
    GColor current_color;
} EnergyState;

void energy_logic_init();
void energy_logic_update(EnergyState *state);
void energy_logic_draw(GContext *ctx, GRect bounds, EnergyState *state);
