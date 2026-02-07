#include <stdio.h>
#include <math.h>
#include "test_framework.h"
#include "filter.h"

int main() {
    TEST_SUITE("Filter Functions");
    
    // Test 1: Passthrough filter
    TEST_CASE_BEGIN("FIR filter passthrough (2-tap)");
    double in1[2] = {1.0, 2.0};
    double out1[2] = {0.0, 0.0};
    fir_filter(in1, out1, 2);
    if (fabs(out1[0] - 1.0) < 0.001) {
        TEST_PASS_STMT;
    } else {
        TEST_FAIL_STMT("First sample should be ~1.0");
    }
    
    // Test 2: Zero input
    TEST_CASE_BEGIN("FIR filter with zero input");
    double in2[3] = {0.0, 0.0, 0.0};
    double out2[3] = {1.0, 1.0, 1.0};
    fir_filter(in2, out2, 3);
    if (fabs(out2[0]) < 0.001 && fabs(out2[1]) < 0.001) {
        TEST_PASS_STMT;
    } else {
        TEST_FAIL_STMT("Output should be ~0.0");
    }
    
    // Test 3: Impulse response
    TEST_CASE_BEGIN("FIR filter impulse response");
    double in3[4] = {1.0, 0.0, 0.0, 0.0};
    double out3[4] = {0.0, 0.0, 0.0, 0.0};
    fir_filter(in3, out3, 4);
    if (fabs(out3[0] - 1.0) < 0.1) {
        TEST_PASS_STMT;
    } else {
        TEST_FAIL_STMT("Impulse response shape incorrect");
    }
    
    printf("\n=== Test Summary ===\n");
    printf("Total: %d, Passed: %d, Failed: %d\n", test_count, test_passed, test_failed);
    printf("Pass Rate: %.1f%%\n", test_count > 0 ? (100.0 * test_passed / test_count) : 0.0);
    return (test_failed == 0) ? 0 : 1;
}
