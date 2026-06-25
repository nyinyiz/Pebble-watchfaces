#include "matrix_logic.h"
#include <pebble.h>

static MatrixDrop drops[NUM_DROPS];
static GRect s_bounds;
static GFont s_font;

void matrix_logic_init(GRect bounds) {
    s_bounds = bounds;
    s_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
    for (int i = 0; i < NUM_DROPS; i++) {
        drops[i].x = (i * COLUMN_WIDTH) % bounds.size.w;
        drops[i].y = -(rand() % 100);
        drops[i].speed = (rand() % 5) + 2;
        drops[i].length = (rand() % 5) + 3;
    }
}

void matrix_logic_update() {
    for (int i = 0; i < NUM_DROPS; i++) {
        drops[i].y += drops[i].speed;
        if (drops[i].y > s_bounds.size.h) {
            drops[i].y = -20;
            drops[i].x = (rand() % (s_bounds.size.w / COLUMN_WIDTH)) * COLUMN_WIDTH;
        }
    }
}

void matrix_logic_draw(GContext *ctx, MatrixStats *stats) {
    // Draw digital rain
    graphics_context_set_text_color(ctx, GColorGreen);
    for (int i = 0; i < NUM_DROPS; i++) {
        for (int j = 0; j < drops[i].length; j++) {
            char c = '0' + (rand() % 10);
            char str[2] = {c, 0};
            GRect rect = GRect(drops[i].x, drops[i].y - (j * FONT_HEIGHT), COLUMN_WIDTH, FONT_HEIGHT);
            graphics_draw_text(ctx, str, s_font, rect, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
        }
    }

    // Draw stats background for readability
    graphics_context_set_fill_color(ctx, GColorBlack);
    GRect stats_rect = GRect(0, s_bounds.size.h - 40, s_bounds.size.w, 40);
    graphics_fill_rect(ctx, stats_rect, 0, GCornerNone);

    // Draw stats
    graphics_context_set_text_color(ctx, GColorGreen);
    char stats_buffer[64];
    snprintf(stats_buffer, sizeof(stats_buffer), "BAT: %d%% %s | STEPS: %d", stats->battery_percent, stats->is_charging ? "+" : "-", stats->step_count);
    
    graphics_draw_text(ctx, stats->date_str, s_font, GRect(0, s_bounds.size.h - 38, s_bounds.size.w, 18), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    graphics_draw_text(ctx, stats_buffer, s_font, GRect(0, s_bounds.size.h - 22, s_bounds.size.w, 18), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}

void matrix_logic_deinit() {}
