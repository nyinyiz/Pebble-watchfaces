#include <pebble.h>
#include "timezones.h"

// ── Persistent storage key ─────────────────────────────────────────────────
#define PREF_KEY_TZ_INDEX 0

// ── Colors ─────────────────────────────────────────────────────────────────
#define COLOR_BG       GColorBlack
#define COLOR_LOCAL    GColorWhite
#define COLOR_OTHER    GColorCyan
#define COLOR_DIM      GColorLightGray
#define COLOR_DIVIDER  GColorCyan
#define COLOR_BATT_HI  GColorGreen
#define COLOR_BATT_MID GColorYellow
#define COLOR_BATT_LO  GColorRed
#define COLOR_BATT_CHG GColorCyan

// ── Watchface window state ─────────────────────────────────────────────────
static Window    *s_main_window;
static Layer     *s_battery_bar_layer;
static TextLayer *s_local_label_layer;
static TextLayer *s_local_time_layer;
static TextLayer *s_local_date_layer;
static Layer     *s_divider_layer;
static TextLayer *s_city_label_layer;
static TextLayer *s_other_time_layer;
static TextLayer *s_other_offset_layer;

// ── Picker window state ────────────────────────────────────────────────────
static Window    *s_picker_window;
static MenuLayer *s_menu_layer;

// ── App state ──────────────────────────────────────────────────────────────
static int32_t s_tz_index    = 0;
static uint8_t s_battery_pct = 100;
static bool    s_charging    = false;

// ── Text buffers ───────────────────────────────────────────────────────────
static char s_local_time_buf[8];     // "10:42"
static char s_local_date_buf[16];    // "SUN 12 APR"
static char s_other_time_buf[8];     // "05:42"
static char s_other_offset_buf[12];  // "UTC-5" or "UTC+5:30"

// ── Battery bar ────────────────────────────────────────────────────────────
static void battery_bar_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int bar_w = (bounds.size.w * s_battery_pct) / 100;

  GColor color;
  if (s_charging) {
    color = COLOR_BATT_CHG;
  } else if (s_battery_pct > 30) {
    color = COLOR_BATT_HI;
  } else if (s_battery_pct > 15) {
    color = COLOR_BATT_MID;
  } else {
    color = COLOR_BATT_LO;
  }

  graphics_context_set_fill_color(ctx, GColorDarkGray);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_fill_color(ctx, color);
  graphics_fill_rect(ctx, GRect(0, 0, bar_w, bounds.size.h), 0, GCornerNone);
}

// ── Divider line ───────────────────────────────────────────────────────────
static void divider_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, COLOR_DIVIDER);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
}

// ── Format UTC offset as "UTC+5:30" or "UTC-5" ────────────────────────────
static void format_offset(int16_t offset_minutes, char *buf, size_t size) {
  bool neg   = offset_minutes < 0;
  int  abs_m = neg ? -offset_minutes : offset_minutes;
  int  h     = abs_m / 60;
  int  m     = abs_m % 60;
  if (m == 0) {
    snprintf(buf, size, "UTC%s%d", neg ? "-" : "+", h);
  } else {
    snprintf(buf, size, "UTC%s%d:%02d", neg ? "-" : "+", h, m);
  }
}

// ── Uppercase a string in place ────────────────────────────────────────────
static void str_toupper(char *s) {
  for (; *s; s++) {
    if (*s >= 'a' && *s <= 'z') *s -= 32;
  }
}

// ── Refresh all text layers ────────────────────────────────────────────────
static void update_display(struct tm *local_tm, time_t utc_now) {
  bool is_24h = clock_is_24h_style();

  // Local time
  strftime(s_local_time_buf, sizeof(s_local_time_buf),
           is_24h ? "%H:%M" : "%I:%M", local_tm);
  text_layer_set_text(s_local_time_layer, s_local_time_buf);

  // Local date: "SUN 12 APR"
  strftime(s_local_date_buf, sizeof(s_local_date_buf), "%a %d %b", local_tm);
  str_toupper(s_local_date_buf);
  text_layer_set_text(s_local_date_layer, s_local_date_buf);

  // Other timezone time: add offset to UTC, render with gmtime_r
  int16_t off_min = TIMEZONES[s_tz_index].offset;
  time_t  other_t = utc_now + ((time_t)off_min * 60);
  struct tm other_tm = *gmtime(&other_t);  // copy out of static buffer
  strftime(s_other_time_buf, sizeof(s_other_time_buf),
           is_24h ? "%H:%M" : "%I:%M", &other_tm);
  text_layer_set_text(s_other_time_layer, s_other_time_buf);

  // City name
  text_layer_set_text(s_city_label_layer, TIMEZONES[s_tz_index].city);

  // UTC offset hint
  format_offset(off_min, s_other_offset_buf, sizeof(s_other_offset_buf));
  text_layer_set_text(s_other_offset_layer, s_other_offset_buf);
}

// ── Tick handler (every minute) ────────────────────────────────────────────
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_display(tick_time, time(NULL));
}

// ── Battery handler ────────────────────────────────────────────────────────
static void battery_handler(BatteryChargeState state) {
  s_battery_pct = state.charge_percent;
  s_charging    = state.is_charging;
  layer_mark_dirty(s_battery_bar_layer);
}

// ── MenuLayer callbacks (timezone picker) ──────────────────────────────────
static uint16_t menu_get_num_rows(MenuLayer *ml, uint16_t section, void *ctx) {
  return (uint16_t)NUM_TIMEZONES;
}

static void menu_draw_row(GContext *ctx, const Layer *cell_layer,
                          MenuIndex *cell_index, void *context) {
  menu_cell_basic_draw(ctx, cell_layer,
                       TIMEZONES[cell_index->row].label, NULL, NULL);
}

static void menu_select(MenuLayer *ml, MenuIndex *cell_index, void *context) {
  s_tz_index = (int32_t)cell_index->row;
  persist_write_int(PREF_KEY_TZ_INDEX, s_tz_index);

  // Update watchface immediately
  time_t     now      = time(NULL);
  struct tm *local_tm = localtime(&now);
  update_display(local_tm, now);

  // Return to watchface
  window_stack_pop(true);
}

// ── Picker window load/unload ──────────────────────────────────────────────
static void picker_window_load(Window *window) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Picker window loaded — %d timezones", NUM_TIMEZONES);
  Layer *root   = window_get_root_layer(window);
  GRect  bounds = layer_get_bounds(root);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_rows = menu_get_num_rows,
    .draw_row     = menu_draw_row,
    .select_click = menu_select,
  });
  menu_layer_set_click_config_onto_window(s_menu_layer, window);

  // Scroll to currently selected entry
  MenuIndex selected = { .section = 0, .row = (uint16_t)s_tz_index };
  menu_layer_set_selected_index(s_menu_layer, selected,
                                MenuRowAlignCenter, false);
  layer_add_child(root, menu_layer_get_layer(s_menu_layer));
}

static void picker_window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
}

// ── SELECT button → open picker ────────────────────────────────────────────
static void open_picker(void) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Opening picker");
  window_stack_push(s_picker_window, true);
}

static void select_long_handler(ClickRecognizerRef recognizer, void *ctx) {
  APP_LOG(APP_LOG_LEVEL_INFO, "SELECT long-press fired");
  open_picker();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *ctx) {
  APP_LOG(APP_LOG_LEVEL_INFO, "SELECT single-click fired");
  open_picker();
}

static void click_config_provider(void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "click_config_provider called");
  // Long press on SELECT: 700ms hold → open picker (not intercepted by OS)
  window_long_click_subscribe(BUTTON_ID_SELECT, 700,
                               select_long_handler, NULL);
  // Also try single click as fallback
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

// ── Main window layout ─────────────────────────────────────────────────────
//
//  y=0    ▓▓▓ battery bar (h=3) ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
//  y=8      LOCAL                         GOTHIC_14 dim
//  y=26     10:42                         BITHAM_42_BOLD white
//  y=78     SUN 12 APR                    GOTHIC_18_BOLD dim
//  y=106  ──────────────────────────────  divider cyan (inset 10px)
//  y=112    New York                      GOTHIC_14 cyan
//  y=130    05:42                         BITHAM_42_BOLD cyan
//  y=184    UTC-5                         GOTHIC_14 dim
//
static void main_window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);
  int W = bounds.size.w;  // 200 on emery

  // Battery bar — full width top edge, 3px
  s_battery_bar_layer = layer_create(GRect(0, 0, W, 3));
  layer_set_update_proc(s_battery_bar_layer, battery_bar_update_proc);
  layer_add_child(root, s_battery_bar_layer);

  // "LOCAL" label
  s_local_label_layer = text_layer_create(GRect(0, 8, W, 18));
  text_layer_set_background_color(s_local_label_layer, GColorClear);
  text_layer_set_text_color(s_local_label_layer, COLOR_DIM);
  text_layer_set_font(s_local_label_layer,
                      fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_local_label_layer, GTextAlignmentCenter);
  text_layer_set_text(s_local_label_layer, "LOCAL");
  layer_add_child(root, text_layer_get_layer(s_local_label_layer));

  // Local time — large white
  s_local_time_layer = text_layer_create(GRect(0, 26, W, 52));
  text_layer_set_background_color(s_local_time_layer, GColorClear);
  text_layer_set_text_color(s_local_time_layer, COLOR_LOCAL);
  text_layer_set_font(s_local_time_layer,
                      fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_local_time_layer, GTextAlignmentCenter);
  layer_add_child(root, text_layer_get_layer(s_local_time_layer));

  // Local date
  s_local_date_layer = text_layer_create(GRect(0, 78, W, 22));
  text_layer_set_background_color(s_local_date_layer, GColorClear);
  text_layer_set_text_color(s_local_date_layer, COLOR_DIM);
  text_layer_set_font(s_local_date_layer,
                      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_local_date_layer, GTextAlignmentCenter);
  layer_add_child(root, text_layer_get_layer(s_local_date_layer));

  // Divider — inset 10px each side
  s_divider_layer = layer_create(GRect(10, 106, W - 20, 2));
  layer_set_update_proc(s_divider_layer, divider_update_proc);
  layer_add_child(root, s_divider_layer);

  // City label — cyan
  s_city_label_layer = text_layer_create(GRect(0, 112, W, 18));
  text_layer_set_background_color(s_city_label_layer, GColorClear);
  text_layer_set_text_color(s_city_label_layer, COLOR_OTHER);
  text_layer_set_font(s_city_label_layer,
                      fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_city_label_layer, GTextAlignmentCenter);
  layer_add_child(root, text_layer_get_layer(s_city_label_layer));

  // Other timezone time — large cyan
  s_other_time_layer = text_layer_create(GRect(0, 130, W, 52));
  text_layer_set_background_color(s_other_time_layer, GColorClear);
  text_layer_set_text_color(s_other_time_layer, COLOR_OTHER);
  text_layer_set_font(s_other_time_layer,
                      fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_other_time_layer, GTextAlignmentCenter);
  layer_add_child(root, text_layer_get_layer(s_other_time_layer));

  // UTC offset hint — dim, bottom
  s_other_offset_layer = text_layer_create(GRect(0, 184, W, 18));
  text_layer_set_background_color(s_other_offset_layer, GColorClear);
  text_layer_set_text_color(s_other_offset_layer, COLOR_DIM);
  text_layer_set_font(s_other_offset_layer,
                      fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_other_offset_layer, GTextAlignmentCenter);
  layer_add_child(root, text_layer_get_layer(s_other_offset_layer));
}

static void main_window_unload(Window *window) {
  layer_destroy(s_battery_bar_layer);
  text_layer_destroy(s_local_label_layer);
  text_layer_destroy(s_local_time_layer);
  text_layer_destroy(s_local_date_layer);
  layer_destroy(s_divider_layer);
  text_layer_destroy(s_city_label_layer);
  text_layer_destroy(s_other_time_layer);
  text_layer_destroy(s_other_offset_layer);
}

// ── App lifecycle ──────────────────────────────────────────────────────────
static void init(void) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Dual Time init — %d timezones available", NUM_TIMEZONES);
  // Load persisted timezone index; validate bounds
  if (persist_exists(PREF_KEY_TZ_INDEX)) {
    s_tz_index = persist_read_int(PREF_KEY_TZ_INDEX);
    if (s_tz_index < 0 || s_tz_index >= NUM_TIMEZONES) s_tz_index = 0;
  }

  // Main watchface window
  s_main_window = window_create();
  window_set_background_color(s_main_window, COLOR_BG);
  window_set_window_handlers(s_main_window, (WindowHandlers){
    .load   = main_window_load,
    .unload = main_window_unload,
  });
  window_set_click_config_provider(s_main_window, click_config_provider);
  window_stack_push(s_main_window, true);

  // Picker window — created once, pushed on SELECT
  s_picker_window = window_create();
  window_set_window_handlers(s_picker_window, (WindowHandlers){
    .load   = picker_window_load,
    .unload = picker_window_unload,
  });

  // Subscribe to time and battery events
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(battery_handler);

  // Seed initial battery state and display
  BatteryChargeState bat = battery_state_service_peek();
  s_battery_pct = bat.charge_percent;
  s_charging    = bat.is_charging;

  time_t     now      = time(NULL);
  struct tm *local_tm = localtime(&now);
  update_display(local_tm, now);
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  window_destroy(s_main_window);
  window_destroy(s_picker_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
