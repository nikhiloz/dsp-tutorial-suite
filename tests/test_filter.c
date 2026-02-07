/**
 * @file test_filter.c
 * @brief Unit tests for the FIR filter implementation.
 *
 * Run with: make test
 */

#include <stdio.h>
#include <math.h>
#include "test_framework.h"
#include "filter.h"
#include "dsp_utils.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main() {
    TEST_SUITE("Filter Functions");

    /* ── Test 1: Identity filter {1.0} → passthrough ─────────── */
    TEST_CASE_BEGIN("Identity filter (1-tap passthrough)");
    {
        double h[1] = {1.0};
        double in[4] = {1.0, 2.0, 3.0, 4.0};
        double out[4] = {0};
        fir_filter(in, out, 4, h, 1);
        int ok = 1;
        for (int i = 0; i < 4; i++) {
            if (fabs(out[i] - in[i]) > 0.001) { ok = 0; break; }
        }
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("1-tap {1.0} should pass input through"); }
    }

    /* ── Test 2: Zero input → zero output ────────────────────── */
    TEST_CASE_BEGIN("Zero input gives zero output");
    {
        double h[3] = {0.25, 0.5, 0.25};
        double in[4] = {0, 0, 0, 0};
        double out[4] = {99, 99, 99, 99};
        fir_filter(in, out, 4, h, 3);
        int ok = 1;
        for (int i = 0; i < 4; i++) {
            if (fabs(out[i]) > 0.001) { ok = 0; break; }
        }
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("Zero in → zero out"); }
    }

    /* ── Test 3: Impulse response equals coefficients ────────── */
    TEST_CASE_BEGIN("Impulse response matches coefficients");
    {
        double h[3] = {0.25, 0.5, 0.25};
        double in[5] = {1.0, 0, 0, 0, 0};  /* impulse at t=0 */
        double out[5] = {0};
        fir_filter(in, out, 5, h, 3);
        /* out should be {0.25, 0.5, 0.25, 0, 0} */
        if (fabs(out[0] - 0.25) < 0.001 &&
            fabs(out[1] - 0.50) < 0.001 &&
            fabs(out[2] - 0.25) < 0.001 &&
            fabs(out[3]) < 0.001) {
            TEST_PASS_STMT;
        } else {
            TEST_FAIL_STMT("Impulse response should equal coefficients");
        }
    }

    /* ── Test 4: Moving average smooths a step ───────────────── */
    TEST_CASE_BEGIN("Moving average smooths step input");
    {
        double h[4];
        fir_moving_average(h, 4);
        /* Verify coefficients: each should be 0.25 */
        int ok = 1;
        for (int i = 0; i < 4; i++) {
            if (fabs(h[i] - 0.25) > 0.001) { ok = 0; break; }
        }
        /* Apply to a step function */
        double step[8] = {0, 0, 0, 0, 1, 1, 1, 1};
        double out[8] = {0};
        fir_filter(step, out, 8, h, 4);
        /* After the step, output should ramp: 0, 0, 0, 0, 0.25, 0.50, 0.75, 1.00 */
        if (ok &&
            fabs(out[4] - 0.25) < 0.001 &&
            fabs(out[5] - 0.50) < 0.001 &&
            fabs(out[6] - 0.75) < 0.001 &&
            fabs(out[7] - 1.00) < 0.001) {
            TEST_PASS_STMT;
        } else {
            TEST_FAIL_STMT("Moving average should create a ramp on step input");
        }
    }

    /* ── Test 5: Lowpass filter design sanity ────────────────── */
    TEST_CASE_BEGIN("Lowpass filter coefficients sum to 1.0");
    {
        double h[31];
        fir_lowpass(h, 31, 0.1);  /* 10% of Nyquist */
        double sum = 0.0;
        for (int i = 0; i < 31; i++) sum += h[i];
        if (fabs(sum - 1.0) < 0.01) {
            TEST_PASS_STMT;
        } else {
            TEST_FAIL_STMT("Normalized lowpass coefficients should sum to 1.0");
        }
    }

    /* ── Test 6: Lowpass removes high frequency ──────────────── */
    TEST_CASE_BEGIN("Lowpass attenuates high frequency");
    {
        int n = 128;
        double in_signal[128], out_signal[128];
        double h[21];
        fir_lowpass(h, 21, 0.05);  /* Very low cutoff */

        /* Generate high-frequency signal (near Nyquist) */
        for (int i = 0; i < n; i++) {
            in_signal[i] = sin(2.0 * M_PI * 0.45 * i);  /* 90% of Nyquist */
        }
        fir_filter(in_signal, out_signal, n, h, 21);

        /* After filter settles, output should be much smaller */
        double rms_in  = rms(in_signal + 21, n - 21);
        double rms_out = rms(out_signal + 21, n - 21);
        double attenuation = rms_out / rms_in;

        if (attenuation < 0.1) {  /* > 20 dB attenuation */
            TEST_PASS_STMT;
        } else {
            TEST_FAIL_STMT("High-freq should be attenuated > 20 dB");
        }
    }

    printf("\n=== Test Summary ===\n");
    printf("Total: %d, Passed: %d, Failed: %d\n",
           test_count, test_passed, test_failed);
    printf("Pass Rate: %.1f%%\n",
           test_count > 0 ? (100.0 * test_passed / test_count) : 0.0);
    return (test_failed == 0) ? 0 : 1;
}
