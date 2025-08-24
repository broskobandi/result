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
 * \file src/result.c
 * \brief Implementation of the result library.
 * \details This file contains definitions of the functions
 * necessary for using and testing the result library.
 * */

#include "result_utils.h"

/** Flag for testing public macros that call exit() */
int g_is_exit_called;
/** Flag for testing public macros that return from the caller */
int g_is_return_called;
/** Buffer to store the generic result structs in. */
res_t g_res_buff[RES_BUFF_SIZE];
/** The number of currently active result objects. */
size_t g_res_count;
/** Buffer to store the id-s of result objects ready to be reused. */
size_t g_free_buff[FREE_BUFF_SIZE];
/** The number of currently active result objects ready to be reused. */
size_t g_free_count;
/** Fallback result object to be used when g_res_buff is full and g_free_count is 0. */
res_t g_res_fallback = {.state = RES_STATE_INVALID};
/** The id for the fallback result object. */
const size_t g_fallback_id = (size_t)-1;
/** Mutex object. */
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
#ifdef TEST
/** Flag for testing functions that print fallback error. */
int g_is_fallback_error_printed = 0;
/** Flag for testing functions that print normal error. */
int g_is_error_printed = 0;
#endif

/** Creates a new result object with OK state.
 * \param value Pointer to the OK value. Can take NULL if the result is of type void.
 * \param alignment The alignment of the data to be stored. It must be a power of 2.
 * \param size The size of the data to be stored.
 * \param err_info The error information to be used on failure. 
 * \return A unique id to initialize a new instance of a result struct with. */
size_t res_generic_ok(const void *value, size_t alignment, size_t size, res_err_info_t err_info) {
	err_t err = {.err_info = err_info};
	pthread_mutex_lock(&g_mutex);
	if (
		!alignment || !size || (alignment & (alignment - 1)) ||
		alignment > alignof(max_align_t)
	) {
		err.msg = "Invalid argument";
		g_res_fallback.err = err;
		g_res_fallback.state = RES_STATE_ERR;
		pthread_mutex_unlock(&g_mutex);
		return g_fallback_id;
	}
	if (size > OK_BUFF_SIZE || (g_res_count + 1 > RES_BUFF_SIZE && !g_free_count)) {
		err.msg = "Not enough memory";
		g_res_fallback.err = err;
		g_res_fallback.state = RES_STATE_ERR;
		pthread_mutex_unlock(&g_mutex);
		return g_fallback_id;
	}
	size_t id = set_id();
	if (value) memcpy(g_res_buff[id].ok, value, size);
	g_res_buff[id].state = RES_STATE_OK;
	pthread_mutex_unlock(&g_mutex);
	return id;
}

/** Creates a new result object with ERROR state.
 * \param msg The error message.
 * \param err_info Additional error information.
 * \return A unique id to initialize a new instance of a result struct with. */
size_t res_generic_err(const char *msg, res_err_info_t err_info) {
	err_t err = {.msg = msg, .err_info = err_info};
	pthread_mutex_lock(&g_mutex);
	if (g_res_count + 1 > RES_BUFF_SIZE && !g_free_count) {
		err.msg = "Not enough memory";
		g_res_fallback.err = err;
		g_res_fallback.state = RES_STATE_ERR;
		pthread_mutex_unlock(&g_mutex);
		return g_fallback_id;
	}
	size_t id = set_id();
	g_res_buff[id].state = RES_STATE_ERR;
	g_res_buff[id].err = err;
	pthread_mutex_unlock(&g_mutex);
	return id;
}

/** Checks the state of the result object. 
 * \param id The id of the result object.
 * \param value A pointer to the variable to copy the OK value into.
 * Can take NULL in case the result object is of type void. 
 * \param size The size of the OK value. 
 * \param err_info The error information to be used on failure. */
int res_generic_get_ok(size_t id, void *value, size_t size, res_err_info_t err_info) {
	err_t err = {.err_info = err_info};
	pthread_mutex_lock(&g_mutex);
	if (id >= g_res_count || size > OK_BUFF_SIZE || !size) {
		err.msg = "Invalid argument";
		g_res_fallback.err = err;
		g_res_fallback.state = RES_STATE_ERR;
		pthread_mutex_unlock(&g_mutex);
		return 2;
	}
	if (g_res_buff[id].state != RES_STATE_OK) {
		err.msg = "Result state is not RES_STATE_OK";
		g_res_fallback.err = err;
		g_res_fallback.state = RES_STATE_ERR;
		pthread_mutex_unlock(&g_mutex);
		return 1;
	}
	if (value) memcpy(value, &g_res_buff[id].ok, size);
	pthread_mutex_unlock(&g_mutex);
	return 0;
}

/** Creates a new result object with ERROR state and initializes it with the 
 * error information stored in another result object.
 * \param src_id The id of the source result object.
 * \err_info The error information to be used on failure. */
size_t res_generic_err_from(size_t src_id, res_err_info_t err_info) {
	err_t err = {.err_info = err_info};
	pthread_mutex_lock(&g_mutex);
	if (src_id >= g_res_count || g_res_buff[src_id].state != RES_STATE_ERR) {
		err.msg = "Invalid argument";
		g_res_fallback.err = err;
		g_res_fallback.state = RES_STATE_ERR;
		pthread_mutex_unlock(&g_mutex);
		return g_fallback_id;
	}
	if (g_res_count + 1 > RES_BUFF_SIZE && !g_free_count) {
		err.msg = "Not enough memory";
		g_res_fallback.err = err;
		g_res_fallback.state = RES_STATE_ERR;
		pthread_mutex_unlock(&g_mutex);
		return g_fallback_id;
	}
	size_t id = set_id();
	g_res_buff[id].err = g_res_buff[src_id].err;
	g_res_buff[id].state = RES_STATE_ERR;
	pthread_mutex_unlock(&g_mutex);
	return id;
}

/** Sets the state of the result object INVALID. Its memory in the buffer is marked 
 * to be reused. 
 * \param id The id of thet result object. 
 * \param err_info The error information to be used on failure. */
void res_generic_del(size_t id, res_err_info_t err_info) {
	err_t err = {.err_info = err_info};
	pthread_mutex_lock(&g_mutex);
	if (id >= RES_BUFF_SIZE || g_res_buff[id].state == RES_STATE_INVALID) {
		err.msg = "Invalid argument";
		g_res_fallback.state = RES_STATE_ERR;
		g_res_fallback.err = err;
		pthread_mutex_unlock(&g_mutex);
		return;
	}
	if (g_free_count + 1 > FREE_BUFF_SIZE) {
		err.msg = "Not enough memory";
		g_res_fallback.state = RES_STATE_ERR;
		g_res_fallback.err = err;
		pthread_mutex_unlock(&g_mutex);
		return;
	}
	g_free_buff[g_free_count] = id;
	g_free_count++;
	g_res_buff[id].state = RES_STATE_INVALID;
	pthread_mutex_unlock(&g_mutex);
}

/** Prints the error information stored in the result object.
 * \param id The id of the result object.
 * \param err_info The error information to be used on failure. */
void res_generic_print_err(size_t id, res_err_info_t err_info) {
	err_t err = {.err_info = err_info};
	pthread_mutex_lock(&g_mutex);

	if (id >= RES_BUFF_SIZE || g_res_buff[id].state != RES_STATE_ERR) {
		err.msg = "Invalid argument";
		g_res_fallback.state = RES_STATE_ERR;
		g_res_fallback.err = err;
	}

	if (id == g_fallback_id || g_res_fallback.state == RES_STATE_ERR) {
#ifdef TEST
		g_is_fallback_error_printed = 1;
#else
		print_err(g_res_fallback.err);
#endif
		pthread_mutex_unlock(&g_mutex);
		return;
	}

#ifdef TEST
	g_is_error_printed = 1;
#else
	print_err(g_res_buff[id].err);
#endif

	pthread_mutex_unlock(&g_mutex);
}
