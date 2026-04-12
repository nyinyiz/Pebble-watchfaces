#ifndef AURORA_LOGIC_H
#define AURORA_LOGIC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define AURORA_BAND_COUNT 3
#define AURORA_TRIG_MAX_ANGLE 0x10000

typedef enum {
  AURORA_PROFILE_SMALL_RECT = 0,
  AURORA_PROFILE_LARGE_RECT = 1,
  AURORA_PROFILE_SMALL_ROUND = 2,
  AURORA_PROFILE_LARGE_ROUND = 3,
} AuroraProfile;

typedef enum {
  AURORA_THEME_NIGHT = 0,
  AURORA_THEME_MORNING = 1,
  AURORA_THEME_DAY = 2,
} AuroraThemeId;

typedef struct {
  int16_t sweep_inset;
  int16_t sweep_half_width;
  int16_t sweep_x;
  uint8_t twinkle_offset;
  int32_t pulse_angle;
  int32_t band_base_phases[AURORA_BAND_COUNT];
  int32_t band_phase_steps[AURORA_BAND_COUNT];
} AuroraAnimation;

AuroraProfile aurora_profile_for_size(int16_t width, int16_t height);
AuroraThemeId aurora_theme_for_hour(int hour);

void aurora_format_time(int hour24, int minute, bool is_24h, char *buffer, size_t size);
void aurora_format_date(int weekday_index, int day_of_month, int month_index, char *buffer, size_t size);
void aurora_format_battery_label(uint8_t percent, bool charging, char *buffer, size_t size);
const char *aurora_connection_label(bool connected);

AuroraAnimation aurora_build_animation(uint8_t seconds, int16_t width, bool is_round);

#endif
