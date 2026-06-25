#include <pebble.h>
#include "energy_logic.h"

extern uint32_t MESSAGE_KEY_wake_hour;
extern uint32_t MESSAGE_KEY_sleep_hour;

static Window *s_main_window;
static Layer *s_canvas_layer;
static AppTimer *s_pulse_timer;
static AppTimer *s_reveal_timer;
static EnergyState s_state;
static bool s_animating = false;

// --- Pulse animation (focus-aware) ---

static void start_animation();

static void pulse_timer_callback(void *data) {
    s_pulse_timer = NULL;
    if (!s_animating) return;
    s_state.pulse_anim_frame++;
    if (s_canvas_layer) {
        layer_mark_dirty(s_canvas_layer);
    }
    s_pulse_timer = app_timer_register(100, pulse_timer_callback, NULL);
}

static void start_animation() {
    if (s_animating) return;
    s_animating = true;
    if (!s_pulse_timer) {
        s_pulse_timer = app_timer_register(100, pulse_timer_callback, NULL);
    }
}

static void stop_animation() {
    s_animating = false;
    if (s_pulse_timer) {
        app_timer_cancel(s_pulse_timer);
        s_pulse_timer = NULL;
    }
}

static void focus_handler(bool in_focus) {
    if (in_focus) {
        start_animation();
    } else {
        stop_animation();
    }
}

// --- Tap reveal ---

static void reveal_timer_callback(void *data) {
    s_reveal_timer = NULL;
    s_state.show_details = false;
    if (s_canvas_layer) {
        layer_mark_dirty(s_canvas_layer);
    }
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
    s_state.show_details = true;

    time_t temp = time(NULL);
    struct tm *t = localtime(&temp);
    BatteryChargeState bat = battery_state_service_peek();
    energy_logic_update(&s_state, t->tm_hour, t->tm_min, bat.charge_percent, clock_is_24h_style());

    if (s_canvas_layer) {
        layer_mark_dirty(s_canvas_layer);
    }

    if (s_reveal_timer) app_timer_cancel(s_reveal_timer);
    s_reveal_timer = app_timer_register(3000, reveal_timer_callback, NULL);
}

// --- Tick handler ---

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    BatteryChargeState bat = battery_state_service_peek();
    energy_logic_update(&s_state, tick_time->tm_hour, tick_time->tm_min, bat.charge_percent, clock_is_24h_style());
    if (s_canvas_layer) {
        layer_mark_dirty(s_canvas_layer);
    }
}

// --- AppMessage (config) ---

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
    Tuple *wake_t = dict_find(iter, MESSAGE_KEY_wake_hour);
    Tuple *sleep_t = dict_find(iter, MESSAGE_KEY_sleep_hour);

    if (wake_t) {
        s_state.wake_hour = wake_t->value->int32;
        persist_write_int(PERSIST_KEY_WAKE, s_state.wake_hour);
    }
    if (sleep_t) {
        s_state.sleep_hour = sleep_t->value->int32;
        persist_write_int(PERSIST_KEY_SLEEP, s_state.sleep_hour);
    }

    time_t temp = time(NULL);
    struct tm *t = localtime(&temp);
    BatteryChargeState bat = battery_state_service_peek();
    energy_logic_update(&s_state, t->tm_hour, t->tm_min, bat.charge_percent, clock_is_24h_style());
    if (s_canvas_layer) {
        layer_mark_dirty(s_canvas_layer);
    }
}

// --- Canvas ---

static void canvas_update_proc(Layer *layer, GContext *ctx) {
    energy_logic_draw(ctx, layer_get_bounds(layer), &s_state);
}

// --- Window ---

static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    s_canvas_layer = layer_create(bounds);
    layer_set_update_proc(s_canvas_layer, canvas_update_proc);
    layer_add_child(window_layer, s_canvas_layer);

    energy_logic_init(&s_state);

    time_t temp = time(NULL);
    struct tm *t = localtime(&temp);
    BatteryChargeState bat = battery_state_service_peek();
    energy_logic_update(&s_state, t->tm_hour, t->tm_min, bat.charge_percent, clock_is_24h_style());
}

static void main_window_unload(Window *window) {
    stop_animation();
    if (s_reveal_timer) {
        app_timer_cancel(s_reveal_timer);
        s_reveal_timer = NULL;
    }
    layer_destroy(s_canvas_layer);
    s_canvas_layer = NULL;
}

// --- Init/Deinit ---

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
    app_focus_service_subscribe(focus_handler);

    app_message_register_inbox_received(inbox_received_handler);
    app_message_open(64, 64);

    start_animation();
}

static void deinit() {
    stop_animation();
    app_focus_service_unsubscribe();
    accel_tap_service_unsubscribe();
    tick_timer_service_unsubscribe();
    window_destroy(s_main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
