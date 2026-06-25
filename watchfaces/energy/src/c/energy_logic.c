#include "energy_logic.h"
#include <pebble.h>

static GFont s_font_time;
static GFont s_font_detail;

void energy_logic_init(EnergyState *state) {
    state->wake_hour = DEFAULT_WAKE_HOUR;
    state->sleep_hour = DEFAULT_SLEEP_HOUR;
    state->energy_level = 1.0f;
    state->filled_segments = NUM_SEGMENTS;
    state->show_details = false;
    state->pulse_anim_frame = 0;
    state->battery_percent = 100;

    if (persist_exists(PERSIST_KEY_WAKE)) {
        state->wake_hour = persist_read_int(PERSIST_KEY_WAKE);
    }
    if (persist_exists(PERSIST_KEY_SLEEP)) {
        state->sleep_hour = persist_read_int(PERSIST_KEY_SLEEP);
    }

    s_font_time = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
    s_font_detail = fonts_get_system_font(FONT_KEY_GOTHIC_18);
}

float energy_calc_level(int hour, int min, int wake_hour, int sleep_hour) {
    int current_sec = (hour * 3600) + (min * 60);

    if (sleep_hour > wake_hour) {
        int start_sec = wake_hour * 3600;
        int end_sec = sleep_hour * 3600;
        if (current_sec <= start_sec) return 1.0f;
        if (current_sec >= end_sec) return 0.0f;
        return 1.0f - ((float)(current_sec - start_sec) / (float)(end_sec - start_sec));
    } else {
        int total_sec = (24 - wake_hour + sleep_hour) * 3600;
        int start_sec = wake_hour * 3600;
        int end_sec = sleep_hour * 3600;

        if (current_sec >= start_sec) {
            int elapsed = current_sec - start_sec;
            return 1.0f - ((float)elapsed / (float)total_sec);
        } else if (current_sec <= end_sec) {
            int elapsed = (24 * 3600 - start_sec) + current_sec;
            return 1.0f - ((float)elapsed / (float)total_sec);
        } else {
            return 1.0f;
        }
    }
}

int energy_calc_segments(float energy_level) {
    if (energy_level <= 0.0f) return 0;
    if (energy_level >= 1.0f) return NUM_SEGMENTS;
    int seg = (int)(energy_level * NUM_SEGMENTS + 0.5f);
    if (seg > NUM_SEGMENTS) seg = NUM_SEGMENTS;
    if (seg < 0) seg = 0;
    return seg;
}

void energy_calc_colors(GColor *colors, int filled_segments) {
    for (int i = 0; i < NUM_SEGMENTS; i++) {
        if (i < filled_segments) {
            int pos = i;
            if (pos >= 5) {
                colors[i] = PBL_IF_COLOR_ELSE(GColorGreen, GColorWhite);
            } else if (pos >= 2) {
                colors[i] = PBL_IF_COLOR_ELSE(GColorYellow, GColorWhite);
            } else {
                colors[i] = PBL_IF_COLOR_ELSE(GColorRed, GColorWhite);
            }
        } else {
            colors[i] = GColorBlack;
        }
    }
}

void energy_logic_update(EnergyState *state, int hour, int min, int battery_percent, bool is24h) {
    state->energy_level = energy_calc_level(hour, min, state->wake_hour, state->sleep_hour);
    state->filled_segments = energy_calc_segments(state->energy_level);
    energy_calc_colors(state->segment_colors, state->filled_segments);
    state->battery_percent = battery_percent;

    time_t temp = time(NULL);
    struct tm *t = localtime(&temp);
    strftime(state->time_str, sizeof(state->time_str), is24h ? "%H:%M" : "%I:%M", t);

    if (state->show_details) {
        strftime(state->date_str, sizeof(state->date_str), "%a %d %b", t);
    }
}

void energy_logic_draw(GContext *ctx, GRect bounds, EnergyState *state) {
    int pillar_w = bounds.size.w / 4;
    int pillar_h = (bounds.size.h * 2) / 3;
    int px = (bounds.size.w - pillar_w) / 2;
    int py = (bounds.size.h - pillar_h) / 2 - 10;

    // Pillar border
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_stroke_width(ctx, 2);
    graphics_draw_round_rect(ctx, GRect(px, py, pillar_w, pillar_h), 6);

    // Segments
    int margin = 5;
    int inner_w = pillar_w - (margin * 2);
    int inner_h = pillar_h - (margin * 2);
    int gap = 3;
    int seg_h = (inner_h - (gap * (NUM_SEGMENTS - 1))) / NUM_SEGMENTS;

    for (int i = 0; i < NUM_SEGMENTS; i++) {
        int seg_y = py + margin + inner_h - ((i + 1) * (seg_h + gap)) + gap;
        GColor color = state->segment_colors[i];

        // Pulse on topmost filled segment
        bool is_top_filled = (i == state->filled_segments - 1) && (state->filled_segments > 0);
        if (is_top_filled && !gcolor_equal(color, GColorBlack)) {
            int phase = (state->pulse_anim_frame / 10) % 2;
            if (phase == 1) {
                #ifdef PBL_COLOR
                if (gcolor_equal(color, GColorGreen)) color = GColorMintGreen;
                else if (gcolor_equal(color, GColorYellow)) color = GColorPastelYellow;
                else if (gcolor_equal(color, GColorRed)) color = GColorMelon;
                #endif
            }
        }

        if (!gcolor_equal(color, GColorBlack)) {
            graphics_context_set_fill_color(ctx, color);
            graphics_fill_rect(ctx, GRect(px + margin, seg_y, inner_w, seg_h), 2, GCornersAll);
        } else {
            graphics_context_set_stroke_color(ctx, PBL_IF_COLOR_ELSE(GColorDarkGray, GColorDarkGray));
            graphics_context_set_stroke_width(ctx, 1);
            graphics_draw_round_rect(ctx, GRect(px + margin, seg_y, inner_w, seg_h), 2);
        }
    }

    // Always-visible time
    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx, state->time_str, s_font_time,
                       GRect(0, py + pillar_h + 5, bounds.size.w, 30),
                       GTextOverflowModeFill, GTextAlignmentCenter, NULL);

    // Tap-to-reveal details
    if (state->show_details) {
        char perc_str[8];
        snprintf(perc_str, sizeof(perc_str), "%d%%", (int)(state->energy_level * 100));
        graphics_draw_text(ctx, perc_str, s_font_detail,
                           GRect(0, py - 25, bounds.size.w, 24),
                           GTextOverflowModeFill, GTextAlignmentCenter, NULL);

        graphics_draw_text(ctx, state->date_str, s_font_detail,
                           GRect(0, py + pillar_h + 32, bounds.size.w, 24),
                           GTextOverflowModeFill, GTextAlignmentCenter, NULL);

        char bat_str[16];
        snprintf(bat_str, sizeof(bat_str), "BAT %d%%", state->battery_percent);
        graphics_draw_text(ctx, bat_str, s_font_detail,
                           GRect(0, py + pillar_h + 52, bounds.size.w, 24),
                           GTextOverflowModeFill, GTextAlignmentCenter, NULL);
    }
}
