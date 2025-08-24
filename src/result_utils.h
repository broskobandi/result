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
 * \file src/result_utils.h
 * \brief Private interface for the result library
 * \details This file contains declarations and inline functions
 * necessary for using and testing the result library.
 * */

#ifndef RESULT_UTILS_H
#define RESULT_UTILS_H

#include "result.h"
#include <pthread.h>
#include <string.h>
#include <stdio.h>

/** Size of the buffer to store the OK data in. */
#define OK_BUFF_SIZE 1024LU
/** Size of the buffer to store the result instances in. */
#define RES_BUFF_SIZE 32LU
/** Size of the buffer to store the id's of result objects
 * ready to be reused. */
#define FREE_BUFF_SIZE RES_BUFF_SIZE

/** Result states enum */
typedef enum res_state {
	RES_STATE_INVALID,
	RES_STATE_ERR,
	RES_STATE_OK
} res_state_t;

/** Error struct for storing all the error information. */
typedef struct err {
	const char *msg;
	res_err_info_t err_info;
} err_t;

/** Generic result struct. */
typedef struct res {
	union {
		alignas(max_align_t) unsigned char ok[OK_BUFF_SIZE];
		err_t err;
	};
	res_state_t state;
} res_t;

/** Buffer to store the generic result structs in. */
extern res_t g_res_buff[RES_BUFF_SIZE];
/** The number of currently active result objects. */
extern size_t g_res_count;
/** Buffer to store the id-s of result objects ready to be reused. */
extern size_t g_free_buff[FREE_BUFF_SIZE];
/** The number of currently active result objects ready to be reused. */
extern size_t g_free_count;
/** Fallback result object to be used when g_res_buff is full and g_free_count is 0. */
extern res_t g_res_fallback;
/** The id for the fallback result object. */
extern const size_t g_fallback_id;
/** Mutex object. */
extern pthread_mutex_t g_mutex;
#ifdef TEST
/** Flag for testing functions that print fallback error. */
extern int g_is_fallback_error_printed;
/** Flag for testing functions that print normal error. */
extern int g_is_error_printed;
#endif

/** Resets global variables to their default states. */
static inline void reset_globals() {
	memset(g_res_buff, 0, RES_BUFF_SIZE * sizeof(res_t));
	memset(g_free_buff, 0, FREE_BUFF_SIZE * sizeof(size_t));
	memset(&g_res_fallback, 0, sizeof(res_t));
	g_res_count = 0;
	g_free_count = 0;
	g_res_fallback.state = RES_STATE_INVALID;
#ifdef TEST
	g_is_fallback_error_printed = 0;
	g_is_error_printed = 0;
#endif
}

/** Creates a new result id and either increments g_res_count or 
 * decrements g_free_count by one.
 * \return The new result id.*/
static inline size_t set_id() {
	size_t id = g_fallback_id;
	if (g_free_count) {
		id = g_free_buff[g_free_count - 1];
		g_free_count--;
	} else {
		id = g_res_count;
		g_res_count++;
	}
	return id;
}

/** Prints the error information.
 * \param e The error struct whose content is to be printed. */
static inline void print_err(err_t e) {
	fprintf(stderr, "[ERROR]:\n\tMessage: %s\n\tFile: %s\n\tFunction: %s\n\tLine: %d\n", 
			e.msg, e.err_info.file, e.err_info.func, e.err_info.line);
}

#endif
