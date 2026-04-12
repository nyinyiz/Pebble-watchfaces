#include <pebble.h>
#include <string.h>

#include "timezones.h"

#define PREF_KEY_TZ_INDEX 0
#define PREF_KEY_STYLE    1

#define COLOR_BG          GColorBlack
#define COLOR_MAIN        GColorWhite
#define COLOR_ACCENT      PBL_IF_COLOR_ELSE(GColorCyan, GColorWhite)
#define COLOR_DIM         PBL_IF_COLOR_ELSE(GColorLightGray, GColorWhite)
#define COLOR_BATT_FRAME  PBL_IF_COLOR_ELSE(GColorDarkGray, GColorWhite)
#define COLOR_BATT_HI     PBL_IF_COLOR_ELSE(GColorGreen, GColorWhite)
#define COLOR_BATT_MID    PBL_IF_COLOR_ELSE(GColorYellow, GColorWhite)
#define COLOR_BATT_LO     PBL_IF_COLOR_ELSE(GColorRed, GColorWhite)
#define COLOR_BATT_CHG    PBL_IF_COLOR_ELSE(GColorCyan, GColorWhite)

typedef enum {
  FACE_STYLE_DIGITAL = 0,
  FACE_STYLE_ANALOG  = 1,
} FaceStyle;

typedef enum {
  PROFILE_SMALL_RECT = 0,
  PROFILE_LARGE_RECT = 1,
  PROFILE_SMALL_ROUND = 2,
  PROFILE_LARGE_ROUND = 3,
} FaceProfile;

typedef struct {
  GRect battery_rect;

  GRect local_label_rect;
  GRect local_time_rect;
  GRect local_date_rect;
  GRect divider_rect;
  GRect remote_label_rect;
  GRect remote_time_rect;
  GRect remote_offset_rect;

  GRect analog_date_rect;
  GRect analog_city_rect;
  GRect analog_offset_rect;
  GPoint analog_main_center;
  int16_t analog_main_radius;
  GPoint analog_sub_center;
  int16_t analog_sub_radius;

  const char *time_font_key;
  const char *label_font_key;
  const char *date_font_key;
} FaceLayout;

static Window *s_main_window;
static Layer *s_face_layer;
static Window *s_picker_window;
static MenuLayer *s_menu_layer;

static int32_t s_tz_index = 0;
static FaceStyle s_face_style = FACE_STYLE_DIGITAL;
static uint8_t s_battery_pct = 100;
static bool s_charging = false;

static void str_toupper(char *s) {
  for (; *s; ++s) {
    if (*s >= 'a' && *s <= 'z') {
      *s -= 32;
    }
  }
}

static void trim_leading_zero(char *s) {
  if (s[0] == '0') {
    memmove(s, s + 1, strlen(s));
  }
}

static void format_time_string(const struct tm *time_info, bool is_24h, char *buf, size_t size) {
  strftime(buf, size, is_24h ? "%H:%M" : "%I:%M", time_info);
  if (!is_24h) {
    trim_leading_zero(buf);
  }
}

static void format_date_string(const struct tm *time_info, char *buf, size_t size) {
  strftime(buf, size, "%a %d %b", time_info);
  str_toupper(buf);
}

static void format_offset(int16_t offset_minutes, char *buf, size_t size) {
  bool neg = offset_minutes < 0;
  int abs_minutes = neg ? -offset_minutes : offset_minutes;
  int hours = abs_minutes / 60;
  int minutes = abs_minutes % 60;

  if (minutes == 0) {
    snprintf(buf, size, "UTC%s%d", neg ? "-" : "+", hours);
  } else {
    snprintf(buf, size, "UTC%s%d:%02d", neg ? "-" : "+", hours, minutes);
  }
}

static FaceProfile face_profile_for_bounds(GRect bounds) {
  if (bounds.size.w == 260) {
    return PROFILE_LARGE_ROUND;
  }
  if (bounds.size.w == 180) {
    return PROFILE_SMALL_ROUND;
  }
  if (bounds.size.w >= 200) {
    return PROFILE_LARGE_RECT;
  }
  return PROFILE_SMALL_RECT;
}

static FaceLayout face_layout_for_bounds(GRect bounds) {
  switch (face_profile_for_bounds(bounds)) {
    case PROFILE_LARGE_ROUND:
      return (FaceLayout) {
        .battery_rect = GRect(40, 24, 180, 4),
        .local_label_rect = GRect(26, 36, 208, 20),
        .local_time_rect = GRect(26, 54, 208, 62),
        .local_date_rect = GRect(26, 114, 208, 22),
        .divider_rect = GRect(40, 142, 180, 2),
        .remote_label_rect = GRect(26, 148, 208, 20),
        .remote_time_rect = GRect(26, 168, 208, 62),
        .remote_offset_rect = GRect(26, 228, 208, 22),
        .analog_date_rect = GRect(26, 58, 208, 20),
        .analog_city_rect = GRect(26, 214, 208, 20),
        .analog_offset_rect = GRect(26, 232, 208, 20),
        .analog_main_center = GPoint(130, 132),
        .analog_main_radius = 76,
        .analog_sub_center = GPoint(186, 186),
        .analog_sub_radius = 30,
        .time_font_key = FONT_KEY_BITHAM_42_BOLD,
        .label_font_key = FONT_KEY_GOTHIC_18_BOLD,
        .date_font_key = FONT_KEY_GOTHIC_18_BOLD,
      };

    case PROFILE_SMALL_ROUND:
      return (FaceLayout) {
        .battery_rect = GRect(26, 16, 128, 3),
        .local_label_rect = GRect(18, 24, 144, 18),
        .local_time_rect = GRect(18, 40, 144, 46),
        .local_date_rect = GRect(18, 84, 144, 20),
        .divider_rect = GRect(28, 104, 124, 2),
        .remote_label_rect = GRect(18, 110, 144, 18),
        .remote_time_rect = GRect(18, 126, 144, 40),
        .remote_offset_rect = GRect(18, 158, 144, 18),
        .analog_date_rect = GRect(18, 44, 144, 18),
        .analog_city_rect = GRect(18, 148, 144, 16),
        .analog_offset_rect = GRect(18, 162, 144, 16),
        .analog_main_center = GPoint(90, 94),
        .analog_main_radius = 48,
        .analog_sub_center = GPoint(126, 130),
        .analog_sub_radius = 20,
        .time_font_key = FONT_KEY_BITHAM_34_MEDIUM_NUMBERS,
        .label_font_key = FONT_KEY_GOTHIC_14_BOLD,
        .date_font_key = FONT_KEY_GOTHIC_18_BOLD,
      };

    case PROFILE_LARGE_RECT:
      return (FaceLayout) {
        .battery_rect = GRect(10, 6, 180, 4),
        .local_label_rect = GRect(0, 16, 200, 18),
        .local_time_rect = GRect(0, 30, 200, 52),
        .local_date_rect = GRect(0, 82, 200, 22),
        .divider_rect = GRect(14, 110, 172, 2),
        .remote_label_rect = GRect(0, 116, 200, 18),
        .remote_time_rect = GRect(0, 130, 200, 52),
        .remote_offset_rect = GRect(0, 188, 200, 20),
        .analog_date_rect = GRect(0, 34, 200, 18),
        .analog_city_rect = GRect(0, 178, 200, 18),
        .analog_offset_rect = GRect(0, 196, 200, 18),
        .analog_main_center = GPoint(100, 108),
        .analog_main_radius = 58,
        .analog_sub_center = GPoint(148, 146),
        .analog_sub_radius = 24,
        .time_font_key = FONT_KEY_BITHAM_42_BOLD,
        .label_font_key = FONT_KEY_GOTHIC_14_BOLD,
        .date_font_key = FONT_KEY_GOTHIC_18_BOLD,
      };

    case PROFILE_SMALL_RECT:
    default:
      return (FaceLayout) {
        .battery_rect = GRect(12, 6, 120, 3),
        .local_label_rect = GRect(0, 12, 144, 16),
        .local_time_rect = GRect(0, 24, 144, 40),
        .local_date_rect = GRect(0, 62, 144, 18),
        .divider_rect = GRect(12, 84, 120, 2),
        .remote_label_rect = GRect(0, 88, 144, 16),
        .remote_time_rect = GRect(0, 100, 144, 40),
        .remote_offset_rect = GRect(0, 144, 144, 18),
        .analog_date_rect = GRect(0, 30, 144, 16),
        .analog_city_rect = GRect(0, 126, 144, 16),
        .analog_offset_rect = GRect(0, 140, 144, 16),
        .analog_main_center = GPoint(72, 80),
        .analog_main_radius = 40,
        .analog_sub_center = GPoint(108, 108),
        .analog_sub_radius = 16,
        .time_font_key = FONT_KEY_BITHAM_34_MEDIUM_NUMBERS,
        .label_font_key = FONT_KEY_GOTHIC_14_BOLD,
        .date_font_key = FONT_KEY_GOTHIC_14,
      };
  }
}

static GColor current_battery_fill_color(void) {
  if (s_charging) {
    return COLOR_BATT_CHG;
  }
  if (s_battery_pct > 30) {
    return COLOR_BATT_HI;
  }
  if (s_battery_pct > 15) {
    return COLOR_BATT_MID;
  }
  return COLOR_BATT_LO;
}

static void draw_text_line(GContext *ctx, const char *text, const char *font_key,
                           GRect rect, GColor color, GTextAlignment alignment) {
  graphics_context_set_text_color(ctx, color);
  graphics_draw_text(ctx, text, fonts_get_system_font(font_key), rect,
                     GTextOverflowModeTrailingEllipsis, alignment, NULL);
}

static GPoint point_on_circle(GPoint center, int32_t angle, int16_t radius) {
  return GPoint(
    center.x + (int16_t)(sin_lookup(angle) * radius / TRIG_MAX_RATIO),
    center.y - (int16_t)(cos_lookup(angle) * radius / TRIG_MAX_RATIO)
  );
}

static void draw_hand(GContext *ctx, GPoint center, int32_t angle, int16_t length,
                      uint8_t stroke_width, GColor color) {
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_stroke_width(ctx, stroke_width);
  graphics_draw_line(ctx, center, point_on_circle(center, angle, length));
}

static void draw_dial(GContext *ctx, GPoint center, int16_t radius, const struct tm *time_info,
                      GColor ring_color, GColor hand_color) {
  graphics_context_set_stroke_color(ctx, ring_color);
  graphics_context_set_stroke_width(ctx, radius >= 60 ? 3 : 2);
  graphics_draw_circle(ctx, center, radius);

  for (int i = 0; i < 12; ++i) {
    int32_t angle = TRIG_MAX_ANGLE * i / 12;
    int tick_outer = radius - 2;
    int tick_inner = tick_outer - (radius >= 60 ? 8 : 5);
    graphics_draw_line(ctx,
                       point_on_circle(center, angle, tick_inner),
                       point_on_circle(center, angle, tick_outer));
  }

  int32_t minute_angle = TRIG_MAX_ANGLE * time_info->tm_min / 60;
  int32_t hour_angle = TRIG_MAX_ANGLE *
    (((time_info->tm_hour % 12) * 60) + time_info->tm_min) / (12 * 60);

  draw_hand(ctx, center, hour_angle, radius * 55 / 100,
            radius >= 60 ? 4 : 3, hand_color);
  draw_hand(ctx, center, minute_angle, radius * 78 / 100,
            radius >= 60 ? 3 : 2, hand_color);

  graphics_context_set_fill_color(ctx, hand_color);
  graphics_fill_circle(ctx, center, radius >= 60 ? 4 : 3);
}

static void draw_battery_bar(GContext *ctx, GRect rect) {
  graphics_context_set_stroke_color(ctx, COLOR_BATT_FRAME);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_rect(ctx, rect);

  GRect fill = rect;
  fill.origin.x += 1;
  fill.origin.y += 1;
  fill.size.w -= 2;
  fill.size.h -= 2;

  if (fill.size.w < 1 || fill.size.h < 1) {
    return;
  }

  graphics_context_set_fill_color(ctx, COLOR_BG);
  graphics_fill_rect(ctx, fill, 0, GCornerNone);

  fill.size.w = (fill.size.w * s_battery_pct) / 100;
  if (fill.size.w < 1) {
    return;
  }

  graphics_context_set_fill_color(ctx, current_battery_fill_color());
  graphics_fill_rect(ctx, fill, 0, GCornerNone);
}

static void build_other_time(struct tm *other_tm_out) {
  time_t now = time(NULL);
  int16_t offset_minutes = TIMEZONES[s_tz_index].offset;
  time_t other_epoch = now + ((time_t)offset_minutes * 60);
  *other_tm_out = *gmtime(&other_epoch);
}

static void draw_digital_face(GContext *ctx, const FaceLayout *layout,
                              const struct tm *local_tm, const struct tm *other_tm) {
  char local_time_buf[8];
  char local_date_buf[16];
  char other_time_buf[8];
  char offset_buf[12];
  bool is_24h = clock_is_24h_style();

  format_time_string(local_tm, is_24h, local_time_buf, sizeof(local_time_buf));
  format_date_string(local_tm, local_date_buf, sizeof(local_date_buf));
  format_time_string(other_tm, is_24h, other_time_buf, sizeof(other_time_buf));
  format_offset(TIMEZONES[s_tz_index].offset, offset_buf, sizeof(offset_buf));

  draw_text_line(ctx, "LOCAL", layout->label_font_key, layout->local_label_rect,
                 COLOR_DIM, GTextAlignmentCenter);
  draw_text_line(ctx, local_time_buf, layout->time_font_key, layout->local_time_rect,
                 COLOR_MAIN, GTextAlignmentCenter);
  draw_text_line(ctx, local_date_buf, layout->date_font_key, layout->local_date_rect,
                 COLOR_DIM, GTextAlignmentCenter);

  graphics_context_set_fill_color(ctx, COLOR_ACCENT);
  graphics_fill_rect(ctx, layout->divider_rect, 0, GCornerNone);

  draw_text_line(ctx, TIMEZONES[s_tz_index].city, layout->label_font_key,
                 layout->remote_label_rect, COLOR_ACCENT, GTextAlignmentCenter);
  draw_text_line(ctx, other_time_buf, layout->time_font_key, layout->remote_time_rect,
                 COLOR_ACCENT, GTextAlignmentCenter);
  draw_text_line(ctx, offset_buf, layout->label_font_key, layout->remote_offset_rect,
                 COLOR_DIM, GTextAlignmentCenter);
}

static void draw_analog_face(GContext *ctx, const FaceLayout *layout,
                             const struct tm *local_tm, const struct tm *other_tm) {
  char local_date_buf[16];
  char offset_buf[12];

  format_date_string(local_tm, local_date_buf, sizeof(local_date_buf));
  format_offset(TIMEZONES[s_tz_index].offset, offset_buf, sizeof(offset_buf));

  draw_text_line(ctx, "LOCAL", layout->label_font_key, layout->local_label_rect,
                 COLOR_DIM, GTextAlignmentCenter);
  draw_text_line(ctx, local_date_buf, layout->date_font_key, layout->analog_date_rect,
                 COLOR_DIM, GTextAlignmentCenter);

  draw_dial(ctx, layout->analog_main_center, layout->analog_main_radius,
            local_tm, COLOR_MAIN, COLOR_MAIN);
  draw_dial(ctx, layout->analog_sub_center, layout->analog_sub_radius,
            other_tm, COLOR_ACCENT, COLOR_ACCENT);

  draw_text_line(ctx, TIMEZONES[s_tz_index].city, layout->label_font_key,
                 layout->analog_city_rect, COLOR_ACCENT, GTextAlignmentCenter);
  draw_text_line(ctx, offset_buf, layout->label_font_key, layout->analog_offset_rect,
                 COLOR_DIM, GTextAlignmentCenter);
}

static void face_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  FaceLayout layout = face_layout_for_bounds(bounds);
  time_t now = time(NULL);
  struct tm *local_tm = localtime(&now);
  struct tm other_tm;

  build_other_time(&other_tm);

  graphics_context_set_fill_color(ctx, COLOR_BG);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  draw_battery_bar(ctx, layout.battery_rect);

  if (s_face_style == FACE_STYLE_ANALOG) {
    draw_analog_face(ctx, &layout, local_tm, &other_tm);
  } else {
    draw_digital_face(ctx, &layout, local_tm, &other_tm);
  }
}

static void refresh_face(void) {
  if (s_face_layer) {
    layer_mark_dirty(s_face_layer);
  }
}

// ── Timezone / settings picker ──────────────────────────────────────────────

static uint16_t picker_get_num_sections(MenuLayer *ml, void *ctx) {
  return 2;
}

static uint16_t picker_get_num_rows(MenuLayer *ml, uint16_t section, void *ctx) {
  return section == 0 ? 1 : (uint16_t)NUM_TIMEZONES;
}

static int16_t picker_get_header_height(MenuLayer *ml, uint16_t section, void *ctx) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void picker_draw_header(GContext *ctx, const Layer *cell_layer,
                               uint16_t section, void *ctx2) {
  menu_cell_basic_header_draw(ctx, cell_layer, section == 0 ? "DISPLAY" : "TIMEZONE");
}

static void picker_draw_row(GContext *ctx, const Layer *cell_layer,
                            MenuIndex *idx, void *ctx2) {
  if (idx->section == 0) {
    const char *mode = s_face_style == FACE_STYLE_DIGITAL ? "Digital" : "Analog";
    menu_cell_basic_draw(ctx, cell_layer, "Clock Style", mode, NULL);
  } else {
    menu_cell_basic_draw(ctx, cell_layer,
                         TIMEZONES[idx->row].label,
                         TIMEZONES[idx->row].city, NULL);
  }
}

static void picker_select_click(MenuLayer *ml, MenuIndex *idx, void *ctx) {
  if (idx->section == 0) {
    s_face_style = (s_face_style == FACE_STYLE_DIGITAL)
                   ? FACE_STYLE_ANALOG : FACE_STYLE_DIGITAL;
    persist_write_int(PREF_KEY_STYLE, (int32_t)s_face_style);
    menu_layer_reload_data(s_menu_layer);
    vibes_short_pulse();
    refresh_face();
  } else {
    s_tz_index = (int32_t)idx->row;
    persist_write_int(PREF_KEY_TZ_INDEX, s_tz_index);
    refresh_face();
    window_stack_pop(true);
  }
}

static void picker_window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_sections  = picker_get_num_sections,
    .get_num_rows      = picker_get_num_rows,
    .get_header_height = picker_get_header_height,
    .draw_header       = picker_draw_header,
    .draw_row          = picker_draw_row,
    .select_click      = picker_select_click,
  });
  menu_layer_set_click_config_onto_window(s_menu_layer, window);

  MenuIndex cur = { .section = 1, .row = (uint16_t)s_tz_index };
  menu_layer_set_selected_index(s_menu_layer, cur, MenuRowAlignCenter, false);

  layer_add_child(root, menu_layer_get_layer(s_menu_layer));
}

static void picker_window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
  s_menu_layer = NULL;
  window_destroy(s_picker_window);
  s_picker_window = NULL;
}

static void open_picker(ClickRecognizerRef rec, void *ctx) {
  if (s_picker_window) {
    return;
  }
  s_picker_window = window_create();
  window_set_window_handlers(s_picker_window, (WindowHandlers) {
    .load   = picker_window_load,
    .unload = picker_window_unload,
  });
  window_stack_push(s_picker_window, true);
}

static void main_click_config_provider(void *ctx) {
  window_long_click_subscribe(BUTTON_ID_UP, 700, open_picker, NULL);
}

// ── Periodic handlers ────────────────────────────────────────────────────────

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  refresh_face();
}

static void battery_handler(BatteryChargeState state) {
  s_battery_pct = state.charge_percent;
  s_charging = state.is_charging;
  refresh_face();
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  s_face_style = s_face_style == FACE_STYLE_DIGITAL
    ? FACE_STYLE_ANALOG
    : FACE_STYLE_DIGITAL;
  persist_write_int(PREF_KEY_STYLE, (int32_t)s_face_style);
  vibes_short_pulse();
  refresh_face();
}

static void main_window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);

  s_face_layer = layer_create(bounds);
  layer_set_update_proc(s_face_layer, face_layer_update_proc);
  layer_add_child(root, s_face_layer);

  window_set_click_config_provider(window, main_click_config_provider);
}

static void main_window_unload(Window *window) {
  layer_destroy(s_face_layer);
  s_face_layer = NULL;
}

static void init(void) {
  if (persist_exists(PREF_KEY_TZ_INDEX)) {
    s_tz_index = persist_read_int(PREF_KEY_TZ_INDEX);
    if (s_tz_index < 0 || s_tz_index >= NUM_TIMEZONES) {
      s_tz_index = 0;
    }
  }

  if (persist_exists(PREF_KEY_STYLE)) {
    int32_t stored_style = persist_read_int(PREF_KEY_STYLE);
    if (stored_style == FACE_STYLE_ANALOG || stored_style == FACE_STYLE_DIGITAL) {
      s_face_style = (FaceStyle)stored_style;
    }
  }

  s_main_window = window_create();
  window_set_background_color(s_main_window, COLOR_BG);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(battery_handler);
  accel_tap_service_subscribe(accel_tap_handler);

  BatteryChargeState battery_state = battery_state_service_peek();
  s_battery_pct = battery_state.charge_percent;
  s_charging = battery_state.is_charging;
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  accel_tap_service_unsubscribe();

  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
