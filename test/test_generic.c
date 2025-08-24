#include "test_utils.h"

void test_reset_globals() {
	g_res_buff[RES_BUFF_SIZE / 2].state = RES_STATE_OK;
	g_res_count = RES_BUFF_SIZE;
	g_free_buff[FREE_BUFF_SIZE / 2] = 4;
	g_free_count = FREE_BUFF_SIZE;
	g_res_fallback.err.msg = "msg";
	g_res_fallback.err.err_info = ERRINFO;
	g_res_fallback.state = RES_STATE_ERR;
	g_is_fallback_error_printed = 1;
	g_is_error_printed = 1;
	reset_globals();
	ASSERT(!g_res_buff[RES_BUFF_SIZE / 2].state);
	ASSERT(!g_free_buff[RES_BUFF_SIZE / 2]);
	ASSERT(!g_res_fallback.err.msg);
	ASSERT(!g_res_fallback.err.err_info.file);
	ASSERT(!g_res_fallback.err.err_info.func);
	ASSERT(!g_res_fallback.err.err_info.line);
	ASSERT(!g_res_count);
	ASSERT(!g_free_count);
	ASSERT(g_res_fallback.state == RES_STATE_INVALID);
	ASSERT(!g_is_fallback_error_printed);
	ASSERT(!g_is_error_printed);
	reset_globals();
}

void test_set_id() {
	reset_globals();
	size_t id = set_id();
	ASSERT(id == 0);
	ASSERT(g_res_count == 1);
	ASSERT(g_free_count == 0);
	reset_globals();
	g_free_count = 2;
	g_free_buff[1] = 1;
	id = set_id();
	ASSERT(id == 1);
	ASSERT(g_free_count == 1);
	reset_globals();
}

void test_generic_ok() {
	reset_globals();
	{ // Happy path
		int value = 5;
		size_t id = res_generic_ok(&value, alignof(int), sizeof(int), ERRINFO);
		ASSERT(id == g_res_count - 1);
		ASSERT((int)*g_res_buff[id].ok == value);
		ASSERT(g_res_buff[id].state == RES_STATE_OK);
		ASSERT(g_res_fallback.state == RES_STATE_INVALID);
		reset_globals();
	}
	{ // No alignment
		size_t id = res_generic_ok(NULL, 0, sizeof(int), ERRINFO);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		ASSERT(id == g_fallback_id);
		ASSERT(g_res_count == 0);
		ASSERT(strcmp(g_res_fallback.err.msg, "Invalid argument") == 0);
		reset_globals();
	}
	{ // No size
		size_t id = res_generic_ok(NULL, alignof(int), 0, ERRINFO);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		ASSERT(id == g_fallback_id);
		ASSERT(g_res_count == 0);
		ASSERT(strcmp(g_res_fallback.err.msg, "Invalid argument") == 0);
		reset_globals();
	}
	{ // Alignment not power of 2
		size_t id = res_generic_ok(NULL, 3, sizeof(int), ERRINFO);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		ASSERT(id == g_fallback_id);
		ASSERT(g_res_count == 0);
		ASSERT(strcmp(g_res_fallback.err.msg, "Invalid argument") == 0);
		reset_globals();
	}
	{ // Alignment too big
		size_t id = res_generic_ok(NULL, alignof(max_align_t) * 2, sizeof(int), ERRINFO);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		ASSERT(id == g_fallback_id);
		ASSERT(g_res_count == 0);
		ASSERT(strcmp(g_res_fallback.err.msg, "Invalid argument") == 0);
		reset_globals();
	}
	{ // size too big
		size_t id = res_generic_ok(NULL, alignof(int), OK_BUFF_SIZE + 1, ERRINFO);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		ASSERT(id == g_fallback_id);
		ASSERT(g_res_count == 0);
		ASSERT(strcmp(g_res_fallback.err.msg, "Not enough memory") == 0);
		reset_globals();
	}
	{ // Not enough memory
		g_res_count = RES_BUFF_SIZE;
		size_t id = res_generic_ok(NULL, alignof(int), sizeof(int), ERRINFO);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		ASSERT(id == g_fallback_id);
		ASSERT(strcmp(g_res_fallback.err.msg, "Not enough memory") == 0);
		reset_globals();
	}
	{ // Allocate in free list
		g_res_count = RES_BUFF_SIZE;
		g_free_count = 1;
		size_t free_id = 13;
		g_free_buff[g_free_count - 1] = free_id;
		size_t id = res_generic_ok(NULL, alignof(int), sizeof(int), ERRINFO);
		ASSERT(id = free_id);
		ASSERT(g_free_count == 0);
		ASSERT(g_res_fallback.state == RES_STATE_INVALID);
		reset_globals();
	}
}

void test_generic_err() {
	reset_globals();
	{ // Happy path
		size_t id = res_generic_err("msg", ERRINFO);
		int line = __LINE__ - 1;
		ASSERT(id == g_res_count - 1);
		ASSERT(g_res_buff[id].state == RES_STATE_ERR);
		ASSERT(strcmp(g_res_buff[id].err.msg, "msg") == 0);
		ASSERT(strcmp(g_res_buff[id].err.err_info.file, __FILE__) == 0);
		ASSERT(strcmp(g_res_buff[id].err.err_info.func, __func__) == 0);
		ASSERT(g_res_buff[id].err.err_info.line == line);
		ASSERT(g_res_fallback.state == RES_STATE_INVALID);
		reset_globals();
	}
	{ // Not enough memory
		g_res_count = RES_BUFF_SIZE;
		g_free_count = 0;
		size_t id = res_generic_err("msg", ERRINFO);
		int line = __LINE__ - 1;
		ASSERT(id == g_fallback_id);
		ASSERT(strcmp(g_res_fallback.err.msg, "Not enough memory") == 0);
		ASSERT(strcmp(g_res_fallback.err.err_info.file, __FILE__) == 0);
		ASSERT(strcmp(g_res_fallback.err.err_info.func, __func__) == 0);
		ASSERT(g_res_fallback.err.err_info.line == line);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		reset_globals();
	}
}

void test_generic_get_ok() {
	reset_globals();
	{ // Happy path
		int value_in = 5;
		size_t id = res_generic_ok(&value_in, alignof(int), sizeof(int), ERRINFO);
		int value_out = 0;
		ASSERT(!res_generic_get_ok(id, &value_out, sizeof(int), ERRINFO));
		ASSERT(value_in == value_out);
		ASSERT(g_res_fallback.state == RES_STATE_INVALID);
		reset_globals();
	}
	{ // id too big
		ASSERT(res_generic_get_ok(g_res_count + 1, NULL, sizeof(int), ERRINFO) == 2);
		int line = __LINE__ - 1;
		ASSERT(strcmp(g_res_fallback.err.err_info.file, __FILE__) == 0);
		ASSERT(strcmp(g_res_fallback.err.err_info.func, __func__) == 0);
		ASSERT(g_res_fallback.err.err_info.line == line);
		ASSERT(strcmp(g_res_fallback.err.msg, "Invalid argument") == 0);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		reset_globals();
	}
	{ // Size too big
		g_res_count = 1;
		ASSERT(res_generic_get_ok(0, NULL, OK_BUFF_SIZE + 1, ERRINFO) == 2);
		int line = __LINE__ - 1;
		ASSERT(strcmp(g_res_fallback.err.err_info.file, __FILE__) == 0);
		ASSERT(strcmp(g_res_fallback.err.err_info.func, __func__) == 0);
		ASSERT(g_res_fallback.err.err_info.line == line);
		ASSERT(strcmp(g_res_fallback.err.msg, "Invalid argument") == 0);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		reset_globals();
	}
	{ // No sise
		g_res_count = 1;
		ASSERT(res_generic_get_ok(0, NULL, 0, ERRINFO) == 2);
		int line = __LINE__ - 1;
		ASSERT(strcmp(g_res_fallback.err.err_info.file, __FILE__) == 0);
		ASSERT(strcmp(g_res_fallback.err.err_info.func, __func__) == 0);
		ASSERT(g_res_fallback.err.err_info.line == line);
		ASSERT(strcmp(g_res_fallback.err.msg, "Invalid argument") == 0);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		reset_globals();
	}
	{ // State is not RES_STATE_OK
		g_res_count = 1;
		g_res_buff[0].state = RES_STATE_ERR;
		ASSERT(res_generic_get_ok(0, NULL, 4, ERRINFO) == 1);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		reset_globals();
	}
}

void test_generic_err_from() {
	reset_globals();
	{ // Happy path
		size_t src = res_generic_err("msg", ERRINFO);
		size_t dst = res_generic_err_from(src, ERRINFO);
		ASSERT(g_res_buff[dst].state == RES_STATE_ERR);
		ASSERT(memcmp(&g_res_buff[dst].err, &g_res_buff[src], sizeof(err_t)) == 0);
		ASSERT(g_res_count == 2);
		ASSERT(g_res_fallback.state == RES_STATE_INVALID);
		reset_globals();
	}
	{ // src id invalid
		size_t dst = res_generic_err_from(13, ERRINFO);
		int line = __LINE__ - 1;
		ASSERT(dst == g_fallback_id);
		ASSERT(!strcmp(g_res_fallback.err.err_info.file, __FILE__));
		ASSERT(!strcmp(g_res_fallback.err.err_info.func, __func__));
		ASSERT(!strcmp(g_res_fallback.err.msg, "Invalid argument"));
		ASSERT(g_res_fallback.err.err_info.line == line);
		ASSERT(g_res_count == 0);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		reset_globals();
	}
	{ // src state isn't RES_STATE_ERR
		size_t src = res_generic_ok(NULL, 2, 2, ERRINFO);
		ASSERT(g_res_buff[src].state == RES_STATE_OK);
		size_t dst = res_generic_err_from(src, ERRINFO);
		int line = __LINE__ - 1;
		ASSERT(dst == g_fallback_id);
		ASSERT(!strcmp(g_res_fallback.err.err_info.file, __FILE__));
		ASSERT(!strcmp(g_res_fallback.err.err_info.func, __func__));
		ASSERT(!strcmp(g_res_fallback.err.msg, "Invalid argument"));
		ASSERT(g_res_fallback.err.err_info.line == line);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		reset_globals();
	}
	{ // Not enough memory
		g_res_count = RES_BUFF_SIZE - 1;
		size_t src = res_generic_ok(NULL, 2, 2, ERRINFO);
		ASSERT(g_res_buff[src].state == RES_STATE_OK);
		ASSERT(g_res_count == RES_BUFF_SIZE);
		ASSERT(g_res_buff[src].state == RES_STATE_OK);
		g_res_buff[src].state = RES_STATE_ERR;
		size_t dst = res_generic_err_from(src, ERRINFO);
		int line = __LINE__ - 1;
		ASSERT(dst == g_fallback_id);
		ASSERT(!strcmp(g_res_fallback.err.err_info.file, __FILE__));
		ASSERT(!strcmp(g_res_fallback.err.err_info.func, __func__));
		ASSERT(!strcmp(g_res_fallback.err.msg, "Not enough memory"));
		ASSERT(g_res_fallback.err.err_info.line == line);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		reset_globals();
	}
}

void test_generic_del() {
	reset_globals();
	{ // Happy path
		size_t id = res_generic_ok(NULL, 2, 2, ERRINFO);
		ASSERT(id == 0);
		ASSERT(g_res_count == 1);
		ASSERT(g_free_count == 0);
		res_generic_del(id, ERRINFO);
		ASSERT(g_res_count == 1);
		ASSERT(g_free_count == 1);
		ASSERT(g_res_fallback.state == RES_STATE_INVALID);
		reset_globals();
	}
	{ // Invalid id
		g_free_count = FREE_BUFF_SIZE;
		res_generic_del(RES_BUFF_SIZE + 1, ERRINFO);
		int line = __LINE__ - 1;
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		ASSERT(!strcmp(g_res_fallback.err.msg, "Invalid argument"));
		ASSERT(!strcmp(g_res_fallback.err.err_info.file, __FILE__));
		ASSERT(!strcmp(g_res_fallback.err.err_info.func, __func__));
		ASSERT(g_res_fallback.err.err_info.line == line);
		reset_globals();
	}
	{ // Free buff full
		size_t id = res_generic_ok(NULL, 2, 2, ERRINFO);
		g_free_count = FREE_BUFF_SIZE;
		res_generic_del(id, ERRINFO);
		int line = __LINE__ - 1;
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		ASSERT(!strcmp(g_res_fallback.err.msg, "Not enough memory"));
		ASSERT(!strcmp(g_res_fallback.err.err_info.file, __FILE__));
		ASSERT(!strcmp(g_res_fallback.err.err_info.func, __func__));
		ASSERT(g_res_fallback.err.err_info.line == line);
		reset_globals();
	}
	{ // Res invalid
		size_t id = 0;
		g_res_buff[id].state = RES_STATE_INVALID;
		res_generic_del(id, ERRINFO);
		int line = __LINE__ - 1;
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		ASSERT(!strcmp(g_res_fallback.err.msg, "Invalid argument"));
		ASSERT(!strcmp(g_res_fallback.err.err_info.file, __FILE__));
		ASSERT(!strcmp(g_res_fallback.err.err_info.func, __func__));
		ASSERT(g_res_fallback.err.err_info.line == line);
		reset_globals();
	}
}

void test_generic_print_err() {
	reset_globals();
	{ // Happy path
		size_t id = res_generic_err("msg", ERRINFO);
		ASSERT(!g_is_error_printed);
		ASSERT(!g_is_fallback_error_printed);
		res_generic_print_err(id, ERRINFO);
		ASSERT(g_res_fallback.state == RES_STATE_INVALID);
		ASSERT(g_is_error_printed);
		ASSERT(!g_is_fallback_error_printed);
		reset_globals();
	}
	{ // Invalid id
		res_generic_print_err(0, ERRINFO);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		ASSERT(!g_is_error_printed);
		ASSERT(g_is_fallback_error_printed);
		reset_globals();
	}
	{ // State not RES_STATE_ERR
		g_res_buff[0].state = RES_STATE_OK;
		res_generic_print_err(0, ERRINFO);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		ASSERT(!g_is_error_printed);
		ASSERT(g_is_fallback_error_printed);
		reset_globals();
	}
	{ // Id is g_fallback_id
		res_generic_print_err(g_fallback_id, ERRINFO);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		ASSERT(!g_is_error_printed);
		ASSERT(g_is_fallback_error_printed);
		reset_globals();
	}
	{ // Fallback state is RES_STATE_ERR
		g_res_fallback.state = RES_STATE_ERR;
		size_t id = res_generic_err("msg", ERRINFO);
		res_generic_print_err(id, ERRINFO);
		ASSERT(!g_is_error_printed);
		ASSERT(g_is_fallback_error_printed);
		reset_globals();
	}
}

void test_generic() {
	test_reset_globals();
	test_set_id();
	test_generic_ok();
	test_generic_err();
	test_generic_get_ok();
	test_generic_err_from();
	test_generic_del();
	test_generic_print_err();
}
