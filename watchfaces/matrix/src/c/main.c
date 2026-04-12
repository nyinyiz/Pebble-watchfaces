#include <pebble.h>
#include "matrix_logic.h"

static Window *s_main_window;
static Layer *s_canvas_layer;
static TextLayer *s_time_layer;
static AppTimer *s_timer;
static MatrixStats s_stats;

static void update_time() {
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    
    static char s_time_buffer[16];
    strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ? "%H:%M:%S" : "%I:%M:%S", tick_time);
    text_layer_set_text(s_time_layer, s_time_buffer);

    strftime(s_stats.date_str, sizeof(s_stats.date_str), "%a, %d %b", tick_time);
}

static void update_battery(BatteryChargeState charge_state) {
    s_stats.battery_percent = charge_state.charge_percent;
    s_stats.is_charging = charge_state.is_charging;
}

static void update_health() {
#if defined(PBL_HEALTH)
    s_stats.step_count = (int)health_service_sum_today(HealthMetricStepCount);
#else
    s_stats.step_count = 0;
#endif
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time();
    if (units_changed & MINUTE_UNIT) {
        update_health();
    }
}

static void timer_callback(void *data) {
    matrix_logic_update();
    layer_mark_dirty(s_canvas_layer);
    s_timer = app_timer_register(100, timer_callback, NULL);
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
    matrix_logic_draw(ctx, &s_stats);
}

static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    s_canvas_layer = layer_create(bounds);
    layer_set_update_proc(s_canvas_layer, canvas_update_proc);
    layer_add_child(window_layer, s_canvas_layer);

    // Vertically and horizontally center the time layer
    int layer_height = 40;
    s_time_layer = text_layer_create(GRect(0, (bounds.size.h - layer_height) / 2, bounds.size.w, layer_height));
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, GColorGreen);
    text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

    matrix_logic_init(bounds);
    
    // Initial data capture
    update_time();
    update_battery(battery_state_service_peek());
    update_health();
}

static void main_window_unload(Window *window) {
    text_layer_destroy(s_time_layer);
    layer_destroy(s_canvas_layer);
    matrix_logic_deinit();
}

static void init() {
    s_main_window = window_create();
    window_set_background_color(s_main_window, GColorBlack);
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });
    window_stack_push(s_main_window, true);

    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    battery_state_service_subscribe(update_battery);
    
    s_timer = app_timer_register(100, timer_callback, NULL);
}

static void deinit() {
    window_destroy(s_main_window);
    battery_state_service_unsubscribe();
    tick_timer_service_unsubscribe();
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
