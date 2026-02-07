/**
 * @file test_fft.c
 * @brief Unit tests for the FFT implementation.
 *
 * Tests verify the Cooley-Tukey radix-2 FFT against known transforms.
 * Run with: make test
 */

#include <stdio.h>
#include <math.h>
#include "test_framework.h"
#include "fft.h"
#include "dsp_utils.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main() {
    TEST_SUITE("FFT Functions");

    /* ── Test 1: DC signal → all energy in bin 0 ─────────────── */
    TEST_CASE_BEGIN("DC component (constant signal)");
    {
        Complex x[4] = {{1,0}, {1,0}, {1,0}, {1,0}};
        fft(x, 4);
        /* DC bin = sum of all values = 4.0 */
        if (fabs(x[0].re - 4.0) < 0.001 &&
            fabs(x[1].re) < 0.001 &&
            fabs(x[2].re) < 0.001 &&
            fabs(x[3].re) < 0.001) {
            TEST_PASS_STMT;
        } else {
            TEST_FAIL_STMT("Expected DC=4.0, others≈0");
        }
    }

    /* ── Test 2: Single impulse → flat spectrum ──────────────── */
    TEST_CASE_BEGIN("Impulse gives flat spectrum");
    {
        Complex x[8] = {{1,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}};
        fft(x, 8);
        /* An impulse at t=0 should give magnitude=1 at all bins */
        int ok = 1;
        for (int k = 0; k < 8; k++) {
            if (fabs(complex_mag(x[k]) - 1.0) > 0.001) { ok = 0; break; }
        }
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("All bins should have magnitude 1.0"); }
    }

    /* ── Test 3: Alternating ±1 → energy at Nyquist ─────────── */
    TEST_CASE_BEGIN("Alternating signal → Nyquist bin");
    {
        Complex x[4] = {{1,0}, {-1,0}, {1,0}, {-1,0}};
        fft(x, 4);
        /* x[N/2] should be 4.0 (all energy at Nyquist), others ≈ 0 */
        if (fabs(x[0].re) < 0.001 && fabs(x[2].re - 4.0) < 0.001) {
            TEST_PASS_STMT;
        } else {
            TEST_FAIL_STMT("Expected bin[2] = 4.0");
        }
    }

    /* ── Test 4: FFT→IFFT round-trip ─────────────────────────── */
    TEST_CASE_BEGIN("FFT then IFFT recovers original");
    {
        Complex x[8] = {{1,0},{2,0},{3,0},{4,0},{5,0},{6,0},{7,0},{8,0}};
        Complex orig[8];
        for (int i = 0; i < 8; i++) orig[i] = x[i];

        fft(x, 8);
        ifft(x, 8);

        int ok = 1;
        for (int i = 0; i < 8; i++) {
            if (fabs(x[i].re - orig[i].re) > 0.001 ||
                fabs(x[i].im) > 0.001) {
                ok = 0; break;
            }
        }
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("Round-trip should recover original signal"); }
    }

    /* ── Test 5: Pure sine → two conjugate peaks ─────────────── */
    TEST_CASE_BEGIN("Pure sine wave: peaks at correct bin");
    {
        int n = 16;
        Complex x[16];
        /* Generate a sine at bin 2 (frequency = 2·fs/N) */
        for (int i = 0; i < n; i++) {
            x[i].re = sin(2.0 * M_PI * 2.0 * i / n);
            x[i].im = 0.0;
        }
        fft(x, n);

        /* Expect peaks at bin 2 and bin 14 (conjugate symmetry) */
        double mag2  = complex_mag(x[2]);
        double mag14 = complex_mag(x[14]);
        if (mag2 > 7.0 && mag14 > 7.0 && complex_mag(x[0]) < 0.001) {
            TEST_PASS_STMT;
        } else {
            TEST_FAIL_STMT("Sine at bin 2 should peak at bins 2 and N-2");
        }
    }

    /* ── Test 6: fft_real wrapper ────────────────────────────── */
    TEST_CASE_BEGIN("fft_real matches manual complex FFT");
    {
        double signal[4] = {1.0, 2.0, 3.0, 4.0};
        Complex out1[4], out2[4];

        /* Method 1: fft_real */
        fft_real(signal, out1, 4);

        /* Method 2: manual */
        for (int i = 0; i < 4; i++) { out2[i].re = signal[i]; out2[i].im = 0; }
        fft(out2, 4);

        int ok = 1;
        for (int i = 0; i < 4; i++) {
            if (fabs(out1[i].re - out2[i].re) > 0.001 ||
                fabs(out1[i].im - out2[i].im) > 0.001) { ok = 0; break; }
        }
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("fft_real should match manual conversion"); }
    }

    printf("\n=== Test Summary ===\n");
    printf("Total: %d, Passed: %d, Failed: %d\n",
           test_count, test_passed, test_failed);
    printf("Pass Rate: %.1f%%\n",
           test_count > 0 ? (100.0 * test_passed / test_count) : 0.0);
    return (test_failed == 0) ? 0 : 1;
}
