#include <pebble.h>

#include <stdio.h>
#include <string.h>

#include "aurora_logic.h"

#define AURORA_STAR_COUNT 22
#define AURORA_MAX_PEAKS 5
#define AURORA_MAX_WIDTH 260

typedef struct {
  int16_t center_x;
  int16_t peak_height;
  int16_t spread;
} AuroraPeak;

typedef struct {
  int16_t base_y;
  int16_t amplitude;
  int16_t half_height;
} AuroraBandSpec;

typedef struct {
  AuroraProfile profile;
  bool small;
  bool is_round;
  bool show_mountains;
  int16_t width;
  int16_t height;
  GPoint time_center;
  GPoint date_center;
  GPoint status_left_center;
  GPoint status_center;
  GPoint status_right_center;
  int16_t divider_y;
  int16_t aurora_top;
  int16_t aurora_bottom;
  int16_t meridiem_y_offset;
  GFont time_font;
  GFont date_font;
  GFont status_font;
  AuroraBandSpec bands[AURORA_BAND_COUNT];
  AuroraPeak peaks[AURORA_MAX_PEAKS];
  size_t peak_count;
  GPoint stars[AURORA_STAR_COUNT];
  uint8_t mountain_profile[AURORA_MAX_WIDTH];
} AuroraScene;

static const GPoint BASE_STARS[AURORA_STAR_COUNT] = {
  {8, 12}, {25, 7}, {50, 18}, {75, 5}, {98, 14},
  {125, 8}, {150, 20}, {178, 10}, {194, 17},
  {15, 30}, {40, 36}, {70, 26}, {100, 33}, {128, 40},
  {155, 28}, {184, 35},
  {10, 52}, {45, 58}, {85, 48}, {115, 55}, {148, 62},
  {180, 50},
};

static Window *s_main_window;
static Layer *s_face_layer;
static AuroraScene s_scene;
static uint8_t s_battery_percent;
static bool s_charging;
static bool s_connected;

static GColor color_bg(void) {
  return GColorBlack;
}

static GColor color_white(void) {
  return GColorWhite;
}

static GColor color_dim(void) {
  return PBL_IF_COLOR_ELSE(GColorFromRGB(100, 106, 128), GColorLightGray);
}

static GColor color_battery_rail(void) {
  return PBL_IF_COLOR_ELSE(GColorFromRGB(22, 22, 32), GColorDarkGray);
}

static GColor color_danger(void) {
  return PBL_IF_COLOR_ELSE(GColorFromRGB(235, 87, 87), GColorWhite);
}

static GColor color_snow(void) {
  return PBL_IF_COLOR_ELSE(GColorFromRGB(210, 230, 255), GColorWhite);
}

static GColor color_band(uint8_t index) {
  if (!PBL_IF_COLOR_ELSE(true, false)) {
    switch (index) {
      case 0:
        return GColorLightGray;
      case 1:
        return GColorWhite;
      default:
        return GColorDarkGray;
    }
  }

  switch (index) {
    case 0:
      return GColorFromRGB(98, 58, 205);
    case 1:
      return GColorFromRGB(50, 175, 200);
    default:
      return GColorFromRGB(50, 225, 115);
  }
}

static GColor color_accent(AuroraThemeId theme) {
  if (!PBL_IF_COLOR_ELSE(true, false)) {
    return GColorWhite;
  }

  switch (theme) {
    case AURORA_THEME_MORNING:
      return GColorFromRGB(248, 184, 79);
    case AURORA_THEME_DAY:
      return GColorFromRGB(95, 208, 255);
    case AURORA_THEME_NIGHT:
    default:
      return GColorFromRGB(128, 168, 245);
  }
}

static void fill_rect(GContext *ctx, GColor color, int16_t x, int16_t y, int16_t width, int16_t height) {
  graphics_context_set_fill_color(ctx, color);
  graphics_fill_rect(ctx, GRect(x, y, width, height), 0, GCornerNone);
}

static GSize measure_text_box(const char *text, GFont font) {
  return graphics_text_layout_get_content_size(
    text,
    font,
    GRect(0, 0, s_scene.width, 80),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft
  );
}

static void draw_left_text(GContext *ctx, const char *text, GFont font, GColor color, int16_t x, int16_t y) {
  GSize text_size = measure_text_box(text, font);

  graphics_context_set_text_color(ctx, color);
  graphics_draw_text(
    ctx,
    text,
    font,
    GRect(x, y, text_size.w + 2, text_size.h + 4),
    GTextOverflowModeTrailingEllipsis,
    GTextAlignmentLeft,
    NULL
  );
}

static void draw_centered_text(GContext *ctx, const char *text, GFont font, GColor color, GPoint top_center) {
  GSize text_size = measure_text_box(text, font);
  int16_t origin_x = top_center.x - (text_size.w / 2);

  draw_left_text(ctx, text, font, color, origin_x, top_center.y);
}

static void build_stars(void) {
  for (int i = 0; i < AURORA_STAR_COUNT; ++i) {
    int16_t x = (int16_t)((BASE_STARS[i].x * s_scene.width) / 200);
    int16_t y = BASE_STARS[i].y;

    if (s_scene.profile == AURORA_PROFILE_LARGE_ROUND) {
      y += 5;
    } else if (s_scene.small) {
      y = (int16_t)((y * 72) / 100);
    }

    s_scene.stars[i] = GPoint(x, y);
  }
}

static void build_mountain_profile(void) {
  memset(s_scene.mountain_profile, 0, sizeof(s_scene.mountain_profile));

  if (!s_scene.show_mountains) {
    return;
  }

  for (int16_t x = 0; x < s_scene.width; ++x) {
    uint8_t height = 3;

    for (size_t i = 0; i < s_scene.peak_count; ++i) {
      int16_t distance = x - s_scene.peaks[i].center_x;
      int16_t spread = s_scene.peaks[i].spread;

      if (distance > -spread && distance < spread) {
        int32_t numerator = (int32_t)s_scene.peaks[i].peak_height *
                            ((spread * spread) - (distance * distance));
        int32_t candidate = numerator / (spread * spread);
        if (candidate > height) {
          height = (uint8_t)candidate;
        }
      }
    }

    s_scene.mountain_profile[x] = height;
  }
}

static void configure_large_round(void) {
  s_scene.time_center = GPoint(130, 110);
  s_scene.date_center = GPoint(130, 154);
  s_scene.status_left_center = GPoint(44, 192);
  s_scene.status_center = GPoint(130, 192);
  s_scene.status_right_center = GPoint(216, 192);
  s_scene.divider_y = 100;
  s_scene.aurora_top = 8;
  s_scene.aurora_bottom = 95;
  s_scene.meridiem_y_offset = 22;
  s_scene.time_font = fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS);
  s_scene.date_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  s_scene.status_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  s_scene.show_mountains = false;

  s_scene.bands[0] = (AuroraBandSpec){ .base_y = 78, .amplitude = 7, .half_height = 5 };
  s_scene.bands[1] = (AuroraBandSpec){ .base_y = 58, .amplitude = 10, .half_height = 7 };
  s_scene.bands[2] = (AuroraBandSpec){ .base_y = 35, .amplitude = 9, .half_height = 5 };
}

static void configure_small_round(void) {
  s_scene.time_center = GPoint(90, 80);
  s_scene.date_center = GPoint(90, 112);
  s_scene.status_left_center = GPoint(30, 140);
  s_scene.status_center = GPoint(90, 140);
  s_scene.status_right_center = GPoint(150, 140);
  s_scene.divider_y = 72;
  s_scene.aurora_top = 8;
  s_scene.aurora_bottom = 68;
  s_scene.meridiem_y_offset = 14;
  s_scene.time_font = fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS);
  s_scene.date_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  s_scene.status_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  s_scene.show_mountains = true;
  s_scene.peak_count = AURORA_MAX_PEAKS;

  s_scene.bands[0] = (AuroraBandSpec){ .base_y = 58, .amplitude = 6, .half_height = 4 };
  s_scene.bands[1] = (AuroraBandSpec){ .base_y = 42, .amplitude = 9, .half_height = 6 };
  s_scene.bands[2] = (AuroraBandSpec){ .base_y = 24, .amplitude = 8, .half_height = 4 };

  s_scene.peaks[0] = (AuroraPeak){ .center_x = 35, .peak_height = 11, .spread = 17 };
  s_scene.peaks[1] = (AuroraPeak){ .center_x = 66, .peak_height = 9, .spread = 12 };
  s_scene.peaks[2] = (AuroraPeak){ .center_x = 91, .peak_height = 14, .spread = 21 };
  s_scene.peaks[3] = (AuroraPeak){ .center_x = 124, .peak_height = 9, .spread = 14 };
  s_scene.peaks[4] = (AuroraPeak){ .center_x = 159, .peak_height = 7, .spread = 11 };
}

static void configure_large_rect(void) {
  s_scene.time_center = GPoint(100, 96);
  s_scene.date_center = GPoint(100, 128);
  s_scene.status_left_center = GPoint(34, 176);
  s_scene.status_center = GPoint(100, 176);
  s_scene.status_right_center = GPoint(166, 176);
  s_scene.divider_y = 88;
  s_scene.aurora_top = 5;
  s_scene.aurora_bottom = 85;
  s_scene.meridiem_y_offset = 22;
  s_scene.time_font = fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS);
  s_scene.date_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  s_scene.status_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  s_scene.show_mountains = true;
  s_scene.peak_count = AURORA_MAX_PEAKS;

  s_scene.bands[0] = (AuroraBandSpec){ .base_y = 68, .amplitude = 7, .half_height = 5 };
  s_scene.bands[1] = (AuroraBandSpec){ .base_y = 48, .amplitude = 10, .half_height = 7 };
  s_scene.bands[2] = (AuroraBandSpec){ .base_y = 28, .amplitude = 9, .half_height = 5 };

  s_scene.peaks[0] = (AuroraPeak){ .center_x = 20, .peak_height = 12, .spread = 20 };
  s_scene.peaks[1] = (AuroraPeak){ .center_x = 58, .peak_height = 10, .spread = 16 };
  s_scene.peaks[2] = (AuroraPeak){ .center_x = 102, .peak_height = 17, .spread = 26 };
  s_scene.peaks[3] = (AuroraPeak){ .center_x = 148, .peak_height = 11, .spread = 18 };
  s_scene.peaks[4] = (AuroraPeak){ .center_x = 185, .peak_height = 8, .spread = 14 };
}

static void configure_small_rect(void) {
  s_scene.time_center = GPoint(72, 76);
  s_scene.date_center = GPoint(72, 106);
  s_scene.status_left_center = GPoint(24, 144);
  s_scene.status_center = GPoint(72, 144);
  s_scene.status_right_center = GPoint(120, 144);
  s_scene.divider_y = 70;
  s_scene.aurora_top = 5;
  s_scene.aurora_bottom = 62;
  s_scene.meridiem_y_offset = 14;
  s_scene.time_font = fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS);
  s_scene.date_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  s_scene.status_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  s_scene.show_mountains = true;
  s_scene.peak_count = AURORA_MAX_PEAKS;

  s_scene.bands[0] = (AuroraBandSpec){ .base_y = 54, .amplitude = 5, .half_height = 4 };
  s_scene.bands[1] = (AuroraBandSpec){ .base_y = 38, .amplitude = 8, .half_height = 6 };
  s_scene.bands[2] = (AuroraBandSpec){ .base_y = 22, .amplitude = 7, .half_height = 4 };

  s_scene.peaks[0] = (AuroraPeak){ .center_x = 14, .peak_height = 10, .spread = 14 };
  s_scene.peaks[1] = (AuroraPeak){ .center_x = 42, .peak_height = 8, .spread = 12 };
  s_scene.peaks[2] = (AuroraPeak){ .center_x = 73, .peak_height = 14, .spread = 19 };
  s_scene.peaks[3] = (AuroraPeak){ .center_x = 107, .peak_height = 9, .spread = 13 };
  s_scene.peaks[4] = (AuroraPeak){ .center_x = 133, .peak_height = 7, .spread = 10 };
}

static void rebuild_scene(GRect bounds) {
  memset(&s_scene, 0, sizeof(s_scene));
  s_scene.width = bounds.size.w;
  s_scene.height = bounds.size.h;
  s_scene.is_round = bounds.size.w == bounds.size.h;
  s_scene.profile = aurora_profile_for_size(bounds.size.w, bounds.size.h);
  s_scene.small = s_scene.profile == AURORA_PROFILE_SMALL_RECT ||
                  s_scene.profile == AURORA_PROFILE_SMALL_ROUND;

  switch (s_scene.profile) {
    case AURORA_PROFILE_LARGE_ROUND:
      configure_large_round();
      break;
    case AURORA_PROFILE_SMALL_ROUND:
      configure_small_round();
      break;
    case AURORA_PROFILE_LARGE_RECT:
      configure_large_rect();
      break;
    case AURORA_PROFILE_SMALL_RECT:
    default:
      configure_small_rect();
      break;
  }

  build_stars();
  build_mountain_profile();
}

static void draw_stars(GContext *ctx, const AuroraAnimation *animation) {
  for (int i = 0; i < AURORA_STAR_COUNT; ++i) {
    bool dimmed = ((i + animation->twinkle_offset) % 5) == 0;
    GColor color = dimmed ? color_band(1) : color_white();
    int16_t size = (!dimmed && (i % 4) == 1) ? 2 : 1;

    fill_rect(ctx, color, s_scene.stars[i].x, s_scene.stars[i].y, size, size);
  }
}

static void draw_aurora(GContext *ctx, const AuroraAnimation *animation) {
  const int16_t step = 2;

  for (uint8_t band_index = 0; band_index < AURORA_BAND_COUNT; ++band_index) {
    int32_t pulse_ratio = sin_lookup(animation->pulse_angle);
    int16_t pulse_delta = (int16_t)((pulse_ratio * 2) / TRIG_MAX_RATIO);
    int16_t half_height = s_scene.bands[band_index].half_height;

    if (band_index == 1) {
      half_height = (int16_t)(half_height + pulse_delta);
      if (half_height < 1) {
        half_height = 1;
      }
    }

    for (int16_t x = 0; x < s_scene.width; x += step) {
      int16_t edge = x < 15 ? x : (s_scene.width - 1 - x < 15 ? s_scene.width - 1 - x : 15);
      int16_t column_half_height = (int16_t)((half_height * edge) / 15);
      int32_t phase;
      int32_t sine;
      int16_t center_y;
      int16_t top_y;
      int16_t bottom_y;

      if (column_half_height <= 0) {
        continue;
      }

      phase = (animation->band_base_phases[band_index] +
              (x * animation->band_phase_steps[band_index])) % TRIG_MAX_ANGLE;
      sine = sin_lookup((uint16_t)phase);
      center_y = (int16_t)(s_scene.bands[band_index].base_y +
                  ((sine * s_scene.bands[band_index].amplitude) / TRIG_MAX_RATIO));
      top_y = center_y - column_half_height;
      bottom_y = center_y + column_half_height;

      if (top_y < s_scene.aurora_top) {
        top_y = s_scene.aurora_top;
      }
      if (bottom_y > s_scene.aurora_bottom) {
        bottom_y = s_scene.aurora_bottom;
      }

      if (bottom_y > top_y) {
        fill_rect(ctx, color_band(band_index), x, top_y,
                  (s_scene.width - x) < step ? (s_scene.width - x) : step,
                  bottom_y - top_y);
      }
    }
  }
}

static void draw_mountains(GContext *ctx) {
  if (!s_scene.show_mountains) {
    return;
  }

  for (int16_t x = 0; x < s_scene.width; x += 2) {
    uint8_t height = s_scene.mountain_profile[x];
    if ((x + 1) < s_scene.width && s_scene.mountain_profile[x + 1] > height) {
      height = s_scene.mountain_profile[x + 1];
    }

    fill_rect(
      ctx,
      color_bg(),
      x,
      s_scene.aurora_bottom - height,
      (s_scene.width - x) < 2 ? (s_scene.width - x) : 2,
      s_scene.divider_y - s_scene.aurora_bottom + height
    );
  }

  for (int16_t x = 1; x < (s_scene.width - 1); ++x) {
    uint8_t height = s_scene.mountain_profile[x];
    if (height > s_scene.mountain_profile[x - 1] &&
        height >= s_scene.mountain_profile[x + 1] &&
        height >= 9) {
      fill_rect(ctx, color_snow(), x - 1, s_scene.aurora_bottom - height - 1, 3, 2);
    }
  }
}

static void draw_second_sweep(GContext *ctx, const AuroraAnimation *animation, GColor accent) {
  int16_t pulse_left = animation->sweep_x - animation->sweep_half_width;
  int16_t pulse_right = animation->sweep_x + animation->sweep_half_width;
  int16_t pulse_width;

  if (pulse_left < animation->sweep_inset) {
    pulse_left = animation->sweep_inset;
  }
  if (pulse_right > (s_scene.width - animation->sweep_inset)) {
    pulse_right = s_scene.width - animation->sweep_inset;
  }

  pulse_width = pulse_right - pulse_left;
  if (pulse_width < 1) {
    pulse_width = 1;
  }

  fill_rect(ctx, accent, pulse_left, s_scene.divider_y - 1, pulse_width, 3);

  if (pulse_left > animation->sweep_inset) {
    graphics_context_set_stroke_color(ctx, color_dim());
    graphics_draw_line(
      ctx,
      GPoint(animation->sweep_inset, s_scene.divider_y),
      GPoint(pulse_left, s_scene.divider_y)
    );
  }
}

static GColor battery_bar_color(GColor accent) {
  if (s_charging) {
    return color_band(1);
  }
  if (s_battery_percent > 30) {
    return color_band(2);
  }
  if (s_battery_percent > 15) {
    return accent;
  }
  return color_danger();
}

static void draw_battery_bar(GContext *ctx, GColor accent) {
  int16_t bar_width = (int16_t)((s_scene.width * s_battery_percent) / 100);
  if (bar_width < 2) {
    bar_width = 2;
  }

  fill_rect(ctx, color_battery_rail(), 0, 0, s_scene.width, 3);
  fill_rect(ctx, battery_bar_color(accent), 0, 0, bar_width, 3);
}

static void face_update_proc(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *time_info = localtime(&now);
  AuroraAnimation animation = aurora_build_animation((uint8_t)time_info->tm_sec, s_scene.width, s_scene.is_round);
  AuroraThemeId theme = aurora_theme_for_hour(time_info->tm_hour);
  GColor accent = color_accent(theme);
  char time_text[8];
  char meridiem_text[3];
  char date_text[16];
  char battery_text[16];
  char seconds_text[8];
  const char *connection_text = aurora_connection_label(s_connected);
  bool is_24h = clock_is_24h_style();
  GSize time_size;
  int16_t time_x;

  (void)layer;

  aurora_format_time(time_info->tm_hour, time_info->tm_min, is_24h, time_text, sizeof(time_text));
  aurora_format_date(time_info->tm_wday, time_info->tm_mday, time_info->tm_mon, date_text, sizeof(date_text));
  aurora_format_battery_label(s_battery_percent, s_charging, battery_text, sizeof(battery_text));
  snprintf(seconds_text, sizeof(seconds_text), "SEC %02d", time_info->tm_sec);

  if (is_24h) {
    meridiem_text[0] = '\0';
  } else {
    snprintf(meridiem_text, sizeof(meridiem_text), "%s", time_info->tm_hour >= 12 ? "PM" : "AM");
  }

  fill_rect(ctx, color_bg(), 0, 0, s_scene.width, s_scene.height);
  draw_stars(ctx, &animation);
  draw_aurora(ctx, &animation);
  draw_mountains(ctx);

  graphics_context_set_stroke_color(ctx, accent);
  graphics_draw_line(
    ctx,
    GPoint(animation.sweep_inset, s_scene.divider_y),
    GPoint(s_scene.width - animation.sweep_inset, s_scene.divider_y)
  );
  draw_second_sweep(ctx, &animation, accent);

  time_size = measure_text_box(time_text, s_scene.time_font);
  time_x = s_scene.time_center.x - (time_size.w / 2);
  draw_left_text(ctx, time_text, s_scene.time_font, color_white(), time_x, s_scene.time_center.y);

  if (meridiem_text[0] != '\0') {
    GSize meridiem_size = measure_text_box(meridiem_text, s_scene.date_font);
    int16_t meridiem_x = time_x + time_size.w + 5;
    int16_t meridiem_y = s_scene.time_center.y + s_scene.meridiem_y_offset;

    if ((meridiem_x + meridiem_size.w) < (s_scene.width - 2)) {
      draw_left_text(ctx, meridiem_text, s_scene.date_font, accent, meridiem_x, meridiem_y);
    }
  }

  draw_centered_text(ctx, date_text, s_scene.date_font, accent, s_scene.date_center);
  draw_centered_text(
    ctx,
    battery_text,
    s_scene.status_font,
    s_battery_percent <= 20 ? color_danger() : color_dim(),
    s_scene.status_left_center
  );
  draw_centered_text(ctx, seconds_text, s_scene.status_font, accent, s_scene.status_center);
  draw_centered_text(
    ctx,
    connection_text,
    s_scene.status_font,
    s_connected ? color_dim() : color_danger(),
    s_scene.status_right_center
  );
  draw_battery_bar(ctx, accent);
}

static void request_redraw(void) {
  if (s_face_layer != NULL) {
    layer_mark_dirty(s_face_layer);
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  (void)tick_time;
  (void)units_changed;
  request_redraw();
}

static void battery_handler(BatteryChargeState state) {
  s_battery_percent = state.charge_percent;
  s_charging = state.is_charging;
  request_redraw();
}

static void bluetooth_handler(bool connected) {
  s_connected = connected;
  request_redraw();
}

static void init(void) {
  Layer *root_layer;
  GRect bounds;
  BatteryChargeState battery_state;

  s_main_window = window_create();
  window_set_background_color(s_main_window, color_bg());
  root_layer = window_get_root_layer(s_main_window);
  bounds = layer_get_bounds(root_layer);

  s_face_layer = layer_create(bounds);
  layer_set_update_proc(s_face_layer, face_update_proc);
  layer_add_child(root_layer, s_face_layer);

  rebuild_scene(bounds);

  battery_state = battery_state_service_peek();
  s_battery_percent = battery_state.charge_percent;
  s_charging = battery_state.is_charging;
  s_connected = connection_service_peek_pebble_app_connection();

  battery_state_service_subscribe(battery_handler);
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_handler,
  });
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

  window_stack_push(s_main_window, true);
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  connection_service_unsubscribe();

  if (s_face_layer != NULL) {
    layer_destroy(s_face_layer);
  }
  if (s_main_window != NULL) {
    window_destroy(s_main_window);
  }
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
