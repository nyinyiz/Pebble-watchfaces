#include "aurora_logic.h"

#include <stdio.h>
#include <string.h>

enum {
  AURORA_BAND_STEP_0 = 365,
  AURORA_BAND_STEP_1 = 261,
  AURORA_BAND_STEP_2 = 188,
  AURORA_PHASE_OFFSET_0 = 2086,
  AURORA_PHASE_OFFSET_1 = 11468,
  AURORA_PHASE_OFFSET_2 = 25035,
};

static const char *const WEEKDAYS[] = {
  "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT",
};

static const char *const MONTHS[] = {
  "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
  "JUL", "AUG", "SEP", "OCT", "NOV", "DEC",
};

AuroraProfile aurora_profile_for_size(int16_t width, int16_t height) {
  if (width == 260 && height == 260) {
    return AURORA_PROFILE_LARGE_ROUND;
  }

  if (width == height) {
    return AURORA_PROFILE_SMALL_ROUND;
  }

  if (width >= 200) {
    return AURORA_PROFILE_LARGE_RECT;
  }

  return AURORA_PROFILE_SMALL_RECT;
}

AuroraThemeId aurora_theme_for_hour(int hour) {
  int normalized = hour % 24;
  if (normalized < 0) {
    normalized += 24;
  }

  if (normalized < 6 || normalized >= 18) {
    return AURORA_THEME_NIGHT;
  }

  if (normalized < 12) {
    return AURORA_THEME_MORNING;
  }

  return AURORA_THEME_DAY;
}

void aurora_format_time(int hour24, int minute, bool is_24h, char *buffer, size_t size) {
  int display_hour = hour24;

  if (!is_24h) {
    display_hour = hour24 % 12;
    if (display_hour == 0) {
      display_hour = 12;
    }
  }

  snprintf(buffer, size, "%d:%02d", display_hour, minute);
}

void aurora_format_date(int weekday_index, int day_of_month, int month_index, char *buffer, size_t size) {
  const char *weekday = "UNK";
  const char *month = "UNK";

  if (weekday_index >= 0 && weekday_index < (int)(sizeof(WEEKDAYS) / sizeof(WEEKDAYS[0]))) {
    weekday = WEEKDAYS[weekday_index];
  }

  if (month_index >= 0 && month_index < (int)(sizeof(MONTHS) / sizeof(MONTHS[0]))) {
    month = MONTHS[month_index];
  }

  snprintf(buffer, size, "%s %d %s", weekday, day_of_month, month);
}

void aurora_format_battery_label(uint8_t percent, bool charging, char *buffer, size_t size) {
  if (charging) {
    snprintf(buffer, size, "%u%% CHG", percent);
    return;
  }

  if (percent <= 20) {
    snprintf(buffer, size, "%u%% LOW", percent);
    return;
  }

  snprintf(buffer, size, "%u%%", percent);
}

const char *aurora_connection_label(bool connected) {
  return connected ? "BT OK" : "BT OFF";
}

AuroraAnimation aurora_build_animation(uint8_t seconds, int16_t width, bool is_round) {
  AuroraAnimation animation;
  int32_t base_angle = ((int32_t)seconds * AURORA_TRIG_MAX_ANGLE) / 60;
  int32_t track_width;

  memset(&animation, 0, sizeof(animation));

  animation.sweep_inset = is_round ? 40 : 12;
  animation.sweep_half_width = is_round ? 9 : 7;
  track_width = width - (animation.sweep_inset * 2);
  if (track_width < 0) {
    track_width = 0;
  }

  animation.sweep_x = animation.sweep_inset + (int16_t)((track_width * seconds) / 60);
  animation.twinkle_offset = seconds % 5;
  animation.pulse_angle = base_angle % AURORA_TRIG_MAX_ANGLE;

  animation.band_phase_steps[0] = AURORA_BAND_STEP_0;
  animation.band_phase_steps[1] = AURORA_BAND_STEP_1;
  animation.band_phase_steps[2] = AURORA_BAND_STEP_2;

  animation.band_base_phases[0] =
    (int32_t)(((int64_t)base_angle * 85) / 100 + AURORA_PHASE_OFFSET_0) % AURORA_TRIG_MAX_ANGLE;
  animation.band_base_phases[1] =
    (int32_t)(((int64_t)base_angle * 125) / 100 + AURORA_PHASE_OFFSET_1) % AURORA_TRIG_MAX_ANGLE;
  animation.band_base_phases[2] =
    (int32_t)(((int64_t)base_angle * 165) / 100 + AURORA_PHASE_OFFSET_2) % AURORA_TRIG_MAX_ANGLE;

  return animation;
}
