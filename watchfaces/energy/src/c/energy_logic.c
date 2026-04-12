#include "energy_logic.h"
#include <pebble.h>

void energy_logic_init() {}

void energy_logic_update(EnergyState *state) {
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    // Day Cycle: 07:00 (Full 1.0) -> 23:00 (Empty 0.0)
    // Total active seconds: 16 hours * 3600 = 57600
    int current_sec = (tick_time->tm_hour * 3600) + (tick_time->tm_min * 60) + tick_time->tm_sec;
    int start_sec = 7 * 3600;
    int end_sec = 23 * 3600;

    if (current_sec <= start_sec) {
        state->energy_level = 1.0f;
    } else if (current_sec >= end_sec) {
        state->energy_level = 0.0f;
    } else {
        state->energy_level = 1.0f - ((float)(current_sec - start_sec) / (float)(end_sec - start_sec));
    }

    // Color mapping (Green -> Yellow -> Red)
    if (state->energy_level > 0.6f) {
        state->current_color = GColorGreen;
    } else if (state->energy_level > 0.3f) {
        state->current_color = GColorYellow;
    } else {
        state->current_color = GColorRed;
    }

    if (state->show_time) {
        strftime(state->time_str, sizeof(state->time_str), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
    }
}

void energy_logic_draw(GContext *ctx, GRect bounds, EnergyState *state) {
    // Vertical Pillar Design (Human Energy Vibe)
    int pillar_w = bounds.size.w / 4;
    int pillar_h = (bounds.size.h * 2) / 3;
    int px = (bounds.size.w - pillar_w) / 2;
    int py = (bounds.size.h - pillar_h) / 2;

    // Draw border with rounded corners
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_stroke_width(ctx, 3);
    graphics_draw_round_rect(ctx, GRect(px, py, pillar_w, pillar_h), 8);

    // Draw energy level (fill from bottom)
    int margin = 6;
    int fill_max_h = pillar_h - (margin * 2);
    int fill_h = (int)(fill_max_h * state->energy_level);
    
    if (fill_h > 0) {
        graphics_context_set_fill_color(ctx, state->current_color);
        // Fill with rounded corners for the energy bar
        graphics_fill_rect(ctx, GRect(px + margin, py + margin + (fill_max_h - fill_h), pillar_w - (margin * 2), fill_h), 4, GCornersAll);
    }

    // Reveal time
    if (state->show_time) {
        graphics_context_set_text_color(ctx, GColorWhite);
        graphics_draw_text(ctx, state->time_str, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), 
                           GRect(0, py + pillar_h + 10, bounds.size.w, 40), 
                           GTextOverflowModeFill, GTextAlignmentCenter, NULL);
    }
}
