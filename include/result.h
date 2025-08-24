/*
MIT License
Copyright (c) 2025 András Broskó
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

/**
 * \file include/result.h
 * \brief Public interface for the result library. 
 * \details This file contains declarations and inline functions
 * necessary for using and testing the result library.
 * */

#ifndef RESULT_H
#define RESULT_H

#include <stddef.h>
#include <stdalign.h>
#include <stdlib.h>

#ifdef TEST
/** Flag for testing macros that call exit() */
extern int g_is_exit_called;
/** Flag for testing macros that return from the caller */
extern int g_is_return_called;
#endif

/** Shorthand for passing error information to functions */
#define ERRINFO\
	(res_err_info_t){__FILE__, __func__, __LINE__}

/** Type-alias wrapper for a uniform look
 * \param T The type of the result object. */
#define RES(T)\
	res_##T##_t

/** Creates a new result object with ERROR state.
 * \param T The type of what the OK value would be.
 * \param msg The error message.
 * \return The result object. */
#define ERR(T, msg)\
	res_##T##_err((msg), ERRINFO)

/** Creates a new result object with OK state.
 * \param T The type of the OK value.
 * \param value The OK value.
 * \return The result object. */
#define OK(T, value)\
	res_##T##_ok(value, ERRINFO)

#ifdef TEST
#define UNW_OR_RET(T, res, out_param, RT)\
	do {\
		if (res_##T##_get_ok((res), (out_param), ERRINFO) != 0) {\
			res_##RT##_t return_res = res_##RT##_err_from((res).id, ERRINFO);\
			res_##T##_del((res), ERRINFO);\
			g_is_return_called = 1;\
			return return_res;\
		}\
	} while(0)
#else
/** Attempts to return the OK value through an out parameter.
 * Returns from the caller on failure.
 * \param T The type of the OK value.
 * \param res The result object.
 * \param out_param Pointer to the variable to copy the OK value into.
 * \param RT The type of result the caller is expected to return.
 * */
#define UNW_OR_RET(T, res, out_param, RT)\
	do {\
		if (res_##T##_get_ok((res), (out_param), ERRINFO) != 0) {\
			res_##RT##_t return_res = res_##RT##_err_from((res).id, ERRINFO);\
			res_##T##_del((res), ERRINFO);\
			return return_res;\
		}\
	} while(0)
#endif

#ifdef TEST
#define UNW_OR_EXT(T, res, out_param)\
	do {\
		if (res_##T##_get_ok((res), (out_param), ERRINFO) != 0) {\
			res_##T##_print_err((res), ERRINFO);\
			g_is_exit_called = 1;\
		}\
	} while(0)
#else
/** Attempts to return the OK value through an out parameter.
 * Prints the error info and exits the program on failure.
 * \param T The type of the OK value.
 * \param res The result object.
 * \param out_param Pointer to the variable to copy the OK value into.
 * */
#define UNW_OR_EXT(T, res, out_param)\
	do {\
		if (res_##T##_get_ok((res), (out_param), ERRINFO) != 0) {\
			res_##T##_print_err((res), ERRINFO);\
			exit(1);\
		}\
	} while(0)
#endif

/** Creates a new void-type result object with OK state.
 * \return The result object. */
#define OK_VOID()\
	res_void_ok(ERRINFO)

#ifdef TEST
#define UNW_OR_RET_VOID(res, RT)\
	do {\
		if (res_void_get_ok((res), ERRINFO) != 0) {\
			res_##RT##_t return_res = res_##RT##_err_from((res).id, ERRINFO);\
			res_void_del((res));\
			g_is_return_called = 1;\
			return return_res;\
		}\
	} while(0)
#else
/** Returns from the caller if the result object is in ERROR sate.
 * \param res The result object.
 * \param RT The type of result the caller is expected to return.
 * */
#define UNW_OR_RET_VOID(res, RT)\
	do {\
		if (res_void_get_ok((res), ERRINFO) != 0) {\
			res_##RT##_t return_res = res_##RT##_err_from((res).id, ERRINFO);\
			res_void_del((res));\
			return return_res;\
		}\
	} while(0)
#endif

#ifdef TEST
#define UNW_OR_EXT_VOID(res)\
	do {\
		if (res_void_get_ok((res), ERRINFO) != 0) {\
			res_void_print_err((res), ERRINFO);\
			g_is_exit_called = 1;\
		}\
	} while(0)
#else
/** Prints the error information and exits the program if 
 * the result object is in ERROR state.
 * \param res The result object.
 * */
#define UNW_OR_EXT_VOID(res)\
	do {\
		if (res_void_get_ok((res), ERRINFO) != 0) {\
			res_void_print_err((res), ERRINFO);\
			exit(1);\
		}\
	} while(0)
#endif

/** Struct for storing error information. */
typedef struct res_err_info {
	const char *file;
	const char *func;
	int line;
} res_err_info_t;

/** \brief Generates a type-specific opaque handle and static inline functions 
 * for the desired result type. The functions are just type-safe
 * wrappers around the type generic functions filling out some type-specific fields
 * automatically. Please refer to the res_generic_* function documentation
 * for more details about the fundamental behaviour of each of these functions.
 * \param T The type of the result object.
 * */
#define TYPEDEF_RES(T)\
	typedef struct res_##T {\
		const size_t id;\
	} res_##T##_t;\
	__attribute__((unused))\
	static inline res_##T##_t res_##T##_ok(T value, res_err_info_t err_info) {\
		return (res_##T##_t){\
			.id = res_generic_ok(&value, alignof(T), sizeof(T), err_info)\
		};\
	}\
	__attribute__((unused))\
	static inline res_##T##_t res_##T##_err(const char *msg, res_err_info_t err_info) {\
		return (res_##T##_t){.id = res_generic_err(msg, err_info)};\
	}\
	__attribute__((unused))\
	static inline int res_##T##_get_ok(res_##T##_t res, T *value, res_err_info_t err_info) {\
		return res_generic_get_ok(res.id, value, sizeof(T), err_info);\
	}\
	__attribute__((unused))\
	static inline res_##T##_t res_##T##_err_from(size_t src_id, res_err_info_t err_info) {\
		return (res_##T##_t){.id = res_generic_err_from(src_id, err_info)};\
	}\
	__attribute__((unused))\
	static inline void res_##T##_del(res_##T##_t res, res_err_info_t err_info) {\
		res_generic_del(res.id, err_info);\
	}\
	__attribute__((unused))\
	static inline void res_##T##_print_err(res_##T##_t res, res_err_info_t err_info) {\
		res_generic_print_err(res.id, err_info);\
	}\

/** Creates a new result object with OK state.
 * \param value Pointer to the OK value. Can take NULL if the result is of type void.
 * \param alignment The alignment of the data to be stored. It must be a power of 2.
 * \param size The size of the data to be stored.
 * \param err_info The error information to be used on failure. 
 * \return A unique id to initialize a new instance of a result struct with. */
size_t res_generic_ok(const void *value, size_t alignment, size_t size, res_err_info_t err_info);
/** Creates a new result object with ERROR state.
 * \param msg The error message.
 * \param err_info Additional error information.
 * \return A unique id to initialize a new instance of a result struct with. */
size_t res_generic_err(const char *msg, res_err_info_t err_info);
/** Checks the state of the result object. 
 * \param id The id of the result object.
 * \param value A pointer to the variable to copy the OK value into.
 * Can take NULL in case the result object is of type void. 
 * \param size The size of the OK value. 
 * \param err_info The error information to be used on failure. */
int res_generic_get_ok(size_t id, void *value, size_t size, res_err_info_t err_info);
/** Creates a new result object with ERROR state and initializes it with the 
 * error information stored in another result object.
 * \param src_id The id of the source result object.
 * \err_info The error information to be used on failure. */
size_t res_generic_err_from(size_t src_id, res_err_info_t err_info);
/** Sets the state of the result object INVALID. Its memory in the buffer is marked 
 * to be reused. 
 * \param id The id of thet result object. 
 * \param err_info The error information to be used on failure. */
void res_generic_del(size_t id, res_err_info_t err_info);
/** Prints the error information stored in the result object.
 * \param id The id of the result object.
 * \param err_info The error information to be used on failure. */
void res_generic_print_err(size_t id, res_err_info_t err_info);

/** Opaque handle for the result object. */
typedef struct res_void {
	const size_t id;
} res_void_t;

/** Creates new result object with OK state.
 * \param err_info The error information to be used on failure. 
 * \return The result object. */
static inline res_void_t res_void_ok(res_err_info_t err_info) {
	return (res_void_t){.id = res_generic_ok(NULL, 2, 2, err_info)};
}
/** Creates a new result object with ERROR state.
 * \param msg The error message. 
 * \param err_info Additional error information. 
 * \return The result object. */
static inline res_void_t res_void_err(const char *msg, res_err_info_t err_info) {
	return (res_void_t){.id = res_generic_err(msg, err_info)};
}
/** Checks the state of the result object.
 * \param res The result object.
 * \param err_info The error information to be used on failure.
 * \return 0 if the result is OK, 1 if the result is not OK, 2 if any of the 
 * arguments are invalid. */
static inline int res_void_get_ok(res_void_t res, res_err_info_t err_info) {
	return res_generic_get_ok(res.id, NULL, 2, err_info);
}
/** Creates a new result object with ERROR state and initializes it with 
 * the error information of another result object. 
 * \param src_id The id of the source result object.
 * \param err_info The error information to be used on failure. 
 * \return The result object. */
static inline res_void_t res_void_err_from(size_t src_id, res_err_info_t err_info) {
	return (res_void_t){.id = res_generic_err_from(src_id, err_info)};
}
/** Deletes the result object.
 * \param res The result object.
 * \param err_info The error information to be used on failure. */
static inline void res_void_del(res_void_t res, res_err_info_t err_info) {
	res_generic_del(res.id, err_info);
}
/** Prints the error information stored in the result object. 
 * \param res The result object.
 * \param err_info The error information to be used on failure. */\
static inline void res_void_print_err(res_void_t res, res_err_info_t err_info) {
	res_generic_print_err(res.id, err_info);
}

#endif

