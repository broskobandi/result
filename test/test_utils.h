#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include "result.h"
#include "result_utils.h"
#include <stdio.h>
#include <string.h>

extern int g_asserts;
extern int g_passed;
extern int g_failed;

#define ASSERT(expr)\
	if (!(expr)) {\
		printf("\033[31m[ASSERT FAILED]\033[0m\n\t%s\n\tLine: %d\n", #expr, __LINE__);\
		g_failed++;\
	} else {\
		g_passed++;\
	}\
	g_asserts++;\

static inline void print_results() {
	if (!g_failed) {
		printf("\n\033[32m[%d ASSERTS PASSED]\033[0m\n", g_passed);\
	} else {
		printf("\n\033[32m[%d ASSERTS PASSED]\033[0m\n", g_passed);\
		printf("\033[31m[%d ASSERTS FAILED]\033[0m\n", g_failed);\
	}
}

void test_generic();
void test_typedef();
void test_void();
void integration_test();

#endif
