#include "test_utils.h"

TYPEDEF_RES(float);

void test_void_ok() {
	reset_globals();
	{ // Happy path
		res_void_t res = res_void_ok(ERRINFO);
		ASSERT(res.id == g_res_count - 1);
		ASSERT(g_res_buff[res.id].state == RES_STATE_OK);
		reset_globals();
	}
}

void test_void_err() {
	reset_globals();
	{ // Happy path
		res_void_t res = res_void_err("msg", ERRINFO);
		int line = __LINE__ - 1;
		ASSERT(res.id == g_res_count - 1);
		ASSERT(g_res_buff[res.id].state == RES_STATE_ERR);
		ASSERT(!strcmp(g_res_buff[res.id].err.msg, "msg"));
		ASSERT(!strcmp(g_res_buff[res.id].err.err_info.file, __FILE__));
		ASSERT(!strcmp(g_res_buff[res.id].err.err_info.func, __func__));
		ASSERT(g_res_buff[res.id].err.err_info.line ==  line);
		reset_globals();
	}
}

void test_void_get_ok() {
	reset_globals();
	{ // Happy path
		res_void_t res = res_void_ok(ERRINFO);
		ASSERT(!res_void_get_ok(res, ERRINFO));
		reset_globals();
	}
}

void test_void_err_from() {
	reset_globals();
	{ // Happy path
		res_float_t src = res_float_err("msg", ERRINFO);
		int line = __LINE__ - 1;
		res_void_t dst = res_void_err_from(src.id, ERRINFO);
		ASSERT(g_res_buff[dst.id].state == RES_STATE_ERR);
		ASSERT(g_res_buff[dst.id].err.err_info.line == line);
		ASSERT(!strcmp(g_res_buff[dst.id].err.err_info.func, __func__));
		ASSERT(!strcmp(g_res_buff[dst.id].err.err_info.file, __FILE__));
		ASSERT(!strcmp(g_res_buff[dst.id].err.msg, "msg"));
		reset_globals();
	}
}

void test_void_del() {
	reset_globals();
	{ // Happy path
		res_void_t res = res_void_ok(ERRINFO);
		res_void_del(res, ERRINFO);
		ASSERT(g_res_fallback.state == RES_STATE_INVALID);
		ASSERT(g_res_buff[res.id].state == RES_STATE_INVALID);
		ASSERT(g_free_count == 1);
		ASSERT(g_free_buff[0] == res.id);
		reset_globals();
	}
}

void test_void_print_err() {
	reset_globals();
	{ // Happy path
		res_void_t res = res_void_err("msg", ERRINFO);
		res_void_print_err(res, ERRINFO);
		ASSERT(g_res_fallback.state == RES_STATE_INVALID);
		ASSERT(!g_is_fallback_error_printed);
		ASSERT(g_is_error_printed);
		reset_globals();
	}
}

void test_void() {
	test_void_ok();
	test_void_err();
	test_void_get_ok();
	test_void_err_from();
	test_void_del();
	test_void_print_err();
}
