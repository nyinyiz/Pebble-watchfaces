#include <pebble.h>
#include "energy_logic.h"

static Window *s_main_window;
static Layer *s_canvas_layer;
static AppTimer *s_pulse_timer;
static AppTimer *s_reveal_timer;
static EnergyState s_state;

static void timer_callback(void *data) {
    s_state.pulse_anim_frame++;
    layer_mark_dirty(s_canvas_layer);
    s_pulse_timer = app_timer_register(50, timer_callback, NULL);
}

static void reveal_timer_callback(void *data) {
    s_state.show_time = false;
    layer_mark_dirty(s_canvas_layer);
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
    s_state.show_time = true;
    energy_logic_update(&s_state);
    layer_mark_dirty(s_canvas_layer);
    if (s_reveal_timer) app_timer_cancel(s_reveal_timer);
    s_reveal_timer = app_timer_register(3000, reveal_timer_callback, NULL);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    energy_logic_update(&s_state);
    layer_mark_dirty(s_canvas_layer);
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
    energy_logic_draw(ctx, layer_get_bounds(layer), &s_state);
}

static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    s_canvas_layer = layer_create(bounds);
    layer_set_update_proc(s_canvas_layer, canvas_update_proc);
    layer_add_child(window_layer, s_canvas_layer);

    energy_logic_init();
    energy_logic_update(&s_state);
}

static void main_window_unload(Window *window) {
    layer_destroy(s_canvas_layer);
}

static void init() {
    s_main_window = window_create();
    window_set_background_color(s_main_window, GColorBlack);
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });
    window_stack_push(s_main_window, true);

    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
    accel_tap_service_subscribe(accel_tap_handler);
    s_pulse_timer = app_timer_register(50, timer_callback, NULL);
}

static void deinit() {
    window_destroy(s_main_window);
    tick_timer_service_unsubscribe();
    accel_tap_service_unsubscribe();
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
