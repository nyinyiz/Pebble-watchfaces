#include <stdio.h>
#include <assert.h>
#include <math.h>

#define NUM_SEGMENTS 8

float energy_calc_level(int hour, int min, int wake_hour, int sleep_hour) {
    int current_sec = (hour * 3600) + (min * 60);

    if (sleep_hour > wake_hour) {
        int start_sec = wake_hour * 3600;
        int end_sec = sleep_hour * 3600;
        if (current_sec <= start_sec) return 1.0f;
        if (current_sec >= end_sec) return 0.0f;
        return 1.0f - ((float)(current_sec - start_sec) / (float)(end_sec - start_sec));
    } else {
        int total_sec = (24 - wake_hour + sleep_hour) * 3600;
        int start_sec = wake_hour * 3600;
        int end_sec = sleep_hour * 3600;

        if (current_sec >= start_sec) {
            int elapsed = current_sec - start_sec;
            return 1.0f - ((float)elapsed / (float)total_sec);
        } else if (current_sec <= end_sec) {
            int elapsed = (24 * 3600 - start_sec) + current_sec;
            return 1.0f - ((float)elapsed / (float)total_sec);
        } else {
            return 1.0f;
        }
    }
}

int energy_calc_segments(float energy_level) {
    if (energy_level <= 0.0f) return 0;
    if (energy_level >= 1.0f) return NUM_SEGMENTS;
    int seg = (int)(energy_level * NUM_SEGMENTS + 0.5f);
    if (seg > NUM_SEGMENTS) seg = NUM_SEGMENTS;
    if (seg < 0) seg = 0;
    return seg;
}

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    tests_run++; \
    printf("  %s... ", #name);

#define PASS() \
    tests_passed++; \
    printf("PASS\n"); \
    } while(0)

#define ASSERT_FLOAT_EQ(a, b, eps) \
    assert(fabs((a) - (b)) < (eps))

void test_standard_schedule() {
    TEST(full_at_wake) {
        float level = energy_calc_level(7, 0, 7, 23);
        ASSERT_FLOAT_EQ(level, 1.0f, 0.01f);
    } PASS();

    TEST(empty_at_sleep) {
        float level = energy_calc_level(23, 0, 7, 23);
        ASSERT_FLOAT_EQ(level, 0.0f, 0.01f);
    } PASS();

    TEST(half_at_midday) {
        float level = energy_calc_level(15, 0, 7, 23);
        ASSERT_FLOAT_EQ(level, 0.5f, 0.01f);
    } PASS();

    TEST(full_before_wake) {
        float level = energy_calc_level(5, 0, 7, 23);
        ASSERT_FLOAT_EQ(level, 1.0f, 0.01f);
    } PASS();

    TEST(empty_after_sleep) {
        float level = energy_calc_level(23, 30, 7, 23);
        ASSERT_FLOAT_EQ(level, 0.0f, 0.01f);
    } PASS();

    TEST(quarter_drain) {
        float level = energy_calc_level(11, 0, 7, 23);
        ASSERT_FLOAT_EQ(level, 0.75f, 0.01f);
    } PASS();
}

void test_night_owl_schedule() {
    TEST(full_at_wake) {
        float level = energy_calc_level(10, 0, 10, 2);
        ASSERT_FLOAT_EQ(level, 1.0f, 0.01f);
    } PASS();

    TEST(empty_at_sleep) {
        float level = energy_calc_level(2, 0, 10, 2);
        ASSERT_FLOAT_EQ(level, 0.0f, 0.01f);
    } PASS();

    TEST(full_during_rest) {
        float level = energy_calc_level(6, 0, 10, 2);
        ASSERT_FLOAT_EQ(level, 1.0f, 0.01f);
    } PASS();

    TEST(half_at_midpoint) {
        float level = energy_calc_level(18, 0, 10, 2);
        ASSERT_FLOAT_EQ(level, 0.5f, 0.01f);
    } PASS();

    TEST(after_midnight_draining) {
        float level = energy_calc_level(1, 0, 10, 2);
        ASSERT_FLOAT_EQ(level, 1.0f - (15.0f/16.0f), 0.01f);
    } PASS();
}

void test_early_bird_schedule() {
    TEST(full_at_wake) {
        float level = energy_calc_level(5, 0, 5, 21);
        ASSERT_FLOAT_EQ(level, 1.0f, 0.01f);
    } PASS();

    TEST(empty_at_sleep) {
        float level = energy_calc_level(21, 0, 5, 21);
        ASSERT_FLOAT_EQ(level, 0.0f, 0.01f);
    } PASS();

    TEST(half_at_midday) {
        float level = energy_calc_level(13, 0, 5, 21);
        ASSERT_FLOAT_EQ(level, 0.5f, 0.01f);
    } PASS();
}

void test_segments() {
    TEST(full_energy_8_segments) {
        assert(energy_calc_segments(1.0f) == 8);
    } PASS();

    TEST(empty_energy_0_segments) {
        assert(energy_calc_segments(0.0f) == 0);
    } PASS();

    TEST(half_energy_4_segments) {
        assert(energy_calc_segments(0.5f) == 4);
    } PASS();

    TEST(low_energy_2_segments) {
        assert(energy_calc_segments(0.20f) == 2);
    } PASS();

    TEST(high_energy_7_segments) {
        assert(energy_calc_segments(0.87f) == 7);
    } PASS();

    TEST(clamp_above_1) {
        assert(energy_calc_segments(1.5f) == 8);
    } PASS();

    TEST(clamp_below_0) {
        assert(energy_calc_segments(-0.5f) == 0);
    } PASS();
}

int main() {
    printf("=== Energy Logic Tests ===\n\n");

    printf("Standard schedule (7-23):\n");
    test_standard_schedule();

    printf("\nNight Owl schedule (10-02):\n");
    test_night_owl_schedule();

    printf("\nEarly Bird schedule (5-21):\n");
    test_early_bird_schedule();

    printf("\nSegment calculations:\n");
    test_segments();

    printf("\n=== Results: %d/%d passed ===\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}
