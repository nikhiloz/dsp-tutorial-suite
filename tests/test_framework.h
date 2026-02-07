/**
 * @file test_framework.h
 * @brief Minimal C test framework — zero external dependencies.
 *
 * Provides simple assertion macros for unit testing DSP functions.
 * Usage pattern:
 *
 *   TEST_SUITE("My Tests");
 *   TEST_CASE_BEGIN("case name");
 *   ... assertions ...
 *   TEST_CASE_END();
 *   TEST_SUMMARY();
 *
 * Macros:
 *   TEST_SUITE(name)        — Print suite header, reset counters.
 *   TEST_CASE_BEGIN(name)   — Start a named test case.
 *   TEST_CASE_END()         — Mark current test as passed.
 *   TEST_ASSERT(cond)       — Fail if cond is false.
 *   TEST_ASSERT_NEAR(a,b,e) — Fail if |a-b| > epsilon.
 *   TEST_SUMMARY()          — Print pass/fail totals + exit code.
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
