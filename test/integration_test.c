#include "test_utils.h"

TYPEDEF_RES(float);

int g_is_return_called = 0;
int g_is_exit_called = 0;

RES(float) divide(int dividend, int divisor) {
	if (!divisor) return ERR(float, "Divisor mustn't be 0.");
	return OK(float, (float)dividend / (float)divisor);
}

RES(void) call_divide(int dividend, int divisor) {
	float quotient = 0;
	g_is_return_called = 0;
	TRY(float, divide(dividend, divisor), &quotient, void);
	return OK_VOID();
}

void integration_test() {
	reset_globals();
	g_is_exit_called = 0;
	UNW_VOID(call_divide(10, 5));
	ASSERT(!g_is_exit_called);

	reset_globals();
	g_is_exit_called = 0;
	UNW_VOID(call_divide(10, 0));
	ASSERT(g_is_exit_called);

	reset_globals();
	g_is_exit_called = 0;
	float quotient = 0.0f;
	UNW(float,  divide(10, 5), &quotient);
	ASSERT(!g_is_exit_called);

	reset_globals();
	g_is_exit_called = 0;
	quotient = 0.0f;
	UNW(float,  divide(10, 0), &quotient);
	ASSERT(g_is_exit_called);
}
