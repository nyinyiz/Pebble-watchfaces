#pragma once
#include <pebble.h>

typedef struct {
    float progress; // 0.0 to 1.0
    bool show_time; // true when tapped/shaken
    char time_str[8];
    int pulse_anim_frame;
} EnergyState;

void energy_logic_init();
void energy_logic_update(EnergyState *state);
void energy_logic_draw(GContext *ctx, GRect bounds, EnergyState *state);
