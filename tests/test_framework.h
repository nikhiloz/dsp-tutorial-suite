/**
 * Minimal C test framework - no external dependencies
 * Simple assertion macros for unit testing
 */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int test_count __attribute__((unused)) = 0;
static int test_passed __attribute__((unused)) = 0;
static int test_failed __attribute__((unused)) = 0;

#define TEST_SUITE(name) \
    printf("\n=== Test Suite: %s ===\n", name)

#define TEST_CASE_BEGIN(name) \
    printf("  [TEST] %s ... ", name); \
    fflush(stdout); \
    test_count++

#define TEST_PASS_STMT \
    test_passed++; \
    printf("PASS\n")

#define TEST_FAIL_STMT(msg) \
    test_failed++; \
    printf("FAIL: %s\n", msg)

#endif // TEST_FRAMEWORK_H
