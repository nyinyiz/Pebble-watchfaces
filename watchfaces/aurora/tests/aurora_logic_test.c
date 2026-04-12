#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../src/c/aurora_logic.h"

static void test_profile_mapping(void) {
  assert(aurora_profile_for_size(144, 168) == AURORA_PROFILE_SMALL_RECT);
  assert(aurora_profile_for_size(200, 228) == AURORA_PROFILE_LARGE_RECT);
  assert(aurora_profile_for_size(180, 180) == AURORA_PROFILE_SMALL_ROUND);
  assert(aurora_profile_for_size(260, 260) == AURORA_PROFILE_LARGE_ROUND);
}

static void test_theme_mapping(void) {
  assert(aurora_theme_for_hour(5) == AURORA_THEME_NIGHT);
  assert(aurora_theme_for_hour(9) == AURORA_THEME_MORNING);
  assert(aurora_theme_for_hour(14) == AURORA_THEME_DAY);
  assert(aurora_theme_for_hour(21) == AURORA_THEME_NIGHT);
}

static void test_label_formatting(void) {
  char buf[24];
  aurora_format_time(9, 5, true, buf, sizeof(buf));
  assert(strcmp(buf, "9:05") == 0);

  aurora_format_time(21, 7, false, buf, sizeof(buf));
  assert(strcmp(buf, "9:07") == 0);

  aurora_format_date(6, 11, 3, buf, sizeof(buf));
  assert(strcmp(buf, "SAT 11 APR") == 0);

  aurora_format_battery_label(58, true, buf, sizeof(buf));
  assert(strcmp(buf, "58% CHG") == 0);

  aurora_format_battery_label(14, false, buf, sizeof(buf));
  assert(strcmp(buf, "14% LOW") == 0);

  assert(strcmp(aurora_connection_label(true), "BT OK") == 0);
  assert(strcmp(aurora_connection_label(false), "BT OFF") == 0);
}

static void test_animation_mapping(void) {
  AuroraAnimation rect = aurora_build_animation(15, 200, false);
  assert(rect.sweep_inset == 12);
  assert(rect.sweep_half_width == 7);
  assert(rect.sweep_x == 56);
  assert(rect.twinkle_offset == 0);
  assert(rect.band_phase_steps[0] > rect.band_phase_steps[1]);
  assert(rect.band_phase_steps[1] > rect.band_phase_steps[2]);

  AuroraAnimation round = aurora_build_animation(30, 180, true);
  assert(round.sweep_inset == 40);
  assert(round.sweep_half_width == 9);
  assert(round.sweep_x == 90);
  assert(round.twinkle_offset == 0);
}

int main(void) {
  test_profile_mapping();
  test_theme_mapping();
  test_label_formatting();
  test_animation_mapping();

  puts("aurora_logic_test: ok");
  return 0;
}
