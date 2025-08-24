/* Include the library */
#include <result.h>

/* Generate code for the desired type */
TYPEDEF_RES(float);

/* Create functions that return the newly created type */
RES(float) divide(int dividend, int divisor) {
	/* Save the error message in the result object and return it
	 * in the event of failure...
	 * (More information, such as file name, function name, and line number
	 * are saved along with the error message.) */
	if (!divisor) return ERR(float, "Divisor mustn't be 0.");
	/* ... or return from the function with an OK value. */
	return OK(float, (float)dividend / (float)divisor);
}

/* The void type result is a built-in type, no need to TYPEDEF_RES it. */
RES(void) call_divide(int dividend, int divisor) {
	float quotient = 0;
	/* Unwrap the result value and save it in a
	 * variable through an out argument.
	 * Return from the current function with the appropriate 
	 * result type and error information on failure. */
	UNW_OR_RET(float, divide(dividend, divisor), &quotient, void);
	return OK_VOID();
}

int main(void) {
	/* Check if the function ran successfully or exit from the program 
	 * on failure printing the relevant error information automatically. */
	UNW_OR_EXT_VOID(call_divide(10, 5));

	float quotient = 0.0f;
	/* Save the return value directly into a variable 
	 * or exit the program after printing the relevant error value. */
	UNW_OR_EXT(float,  divide(10, 5), &quotient);
	return 0;
}
