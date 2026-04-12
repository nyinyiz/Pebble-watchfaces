#include "energy_logic.h"
#include <pebble.h>
#include <math.h>

void energy_logic_init() {}

void energy_logic_update(EnergyState *state) {
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    // Calculate progress (1.0 at 00:00, 0.0 at 23:59)
    int seconds_passed = (tick_time->tm_hour * 3600) + (tick_time->tm_min * 60) + tick_time->tm_sec;
    state->progress = 1.0f - ((float)seconds_passed / 86400.0f);

    if (state->show_time) {
        strftime(state->time_str, sizeof(state->time_str), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
    }
}

void energy_logic_draw(GContext *ctx, GRect bounds, EnergyState *state) {
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_fill_color(ctx, GColorWhite);

    // Battery dimensions
    int w = bounds.size.w / 2;
    int h = bounds.size.h / 3;

    // Pulse effect
    float pulse = 1.0f + (0.05f * sinf(state->pulse_anim_frame * 0.1f));
    int pw = (int)(w * pulse);
    int ph = (int)(h * pulse);
    int px = (bounds.size.w - pw) / 2;
    int py = (bounds.size.h - ph) / 2;

    // Draw battery shell
    graphics_draw_rect(ctx, GRect(px, py, pw, ph));
    graphics_fill_rect(ctx, GRect(px + pw, py + ph/4, 4, ph/2), 0, GCornerNone);

    // Draw fill level
    int fill_w = (int)((pw - 4) * state->progress);
    if (fill_w < 0) fill_w = 0;
    graphics_fill_rect(ctx, GRect(px + 2, py + 2, fill_w, ph - 4), 0, GCornerNone);

    // Reveal time if requested
    if (state->show_time) {
        graphics_context_set_text_color(ctx, GColorWhite);
        graphics_draw_text(ctx, state->time_str, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), 
                           GRect(0, py + ph + 10, bounds.size.w, 30), 
                           GTextOverflowModeFill, GTextAlignmentCenter, NULL);
    }
}
