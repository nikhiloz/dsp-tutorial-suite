#include <stdio.h>
#include <math.h>
#include "test_framework.h"
#include "fft.h"

int main() {
    TEST_SUITE("FFT Functions");
    
    // Test 1: Simple DC component
    TEST_CASE_BEGIN("DC component (constant signal)");
    double in1[2] = {1.0, 1.0};
    double out1[2] = {0.0, 0.0};
    fft(in1, out1, 2);
    if (fabs(out1[0] - 2.0) < 0.001 && fabs(out1[1]) < 0.001) {
        TEST_PASS_STMT;
    } else {
        TEST_FAIL_STMT("Expected [2.0, 0.0]");
    }
    
    // Test 2: Single frequency
    TEST_CASE_BEGIN("Single frequency component");
    double in2[4] = {1.0, 0.0, -1.0, 0.0};
    double out2[4] = {0.0, 0.0, 0.0, 0.0};
    fft(in2, out2, 4);
    if (fabs(out2[2] - 2.0) < 0.1) {
        TEST_PASS_STMT;
    } else {
        TEST_FAIL_STMT("Expected Nyquist component");
    }
    
    // Test 3: FFT of ones
    TEST_CASE_BEGIN("FFT of ones");
    double in3[8];
    for (int i = 0; i < 8; i++) in3[i] = 1.0;
    double out3[8] = {0};
    fft(in3, out3, 8);
    if (fabs(out3[0] - 8.0) < 0.001) {
        TEST_PASS_STMT;
    } else {
        TEST_FAIL_STMT("DC component incorrect");
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Total: %d, Passed: %d, Failed: %d\n", test_count, test_passed, test_failed);
    printf("Pass Rate: %.1f%%\n", test_count > 0 ? (100.0 * test_passed / test_count) : 0.0);
    return (test_failed == 0) ? 0 : 1;
}
