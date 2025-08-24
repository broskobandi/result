#include "../test_utils.h"

int g_asserts;
int g_passed;
int g_failed;

int main(void) {
	test_generic();
	test_typedef();
	test_void();
	integration_test();

	print_results();
	return 0;
}
