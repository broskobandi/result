#include "test_utils.h"

typedef struct {
	char buff[OK_BUFF_SIZE * 2];
} obj;

TYPEDEF_RES(int);
TYPEDEF_RES(obj);

void test_res_int_ok() {
	reset_globals();
	{ // Happy path
		res_int_t res = res_int_ok(5, ERRINFO);
		ASSERT(g_res_count == 1);
		ASSERT(g_res_buff[res.id].state == RES_STATE_OK);
		ASSERT(*g_res_buff[res.id].ok == 5);
		reset_globals();
	}
	{ // Alloc into free list
		g_res_count = RES_BUFF_SIZE;
		g_free_buff[0] = 0;
		g_free_count = 1;
		res_int_t res = res_int_ok(5, ERRINFO);
		ASSERT(g_res_count == RES_BUFF_SIZE);
		ASSERT(g_res_buff[res.id].state == RES_STATE_OK);
		ASSERT(*g_res_buff[res.id].ok == 5);
		ASSERT(!g_free_count);
		reset_globals();
	}
	{ // Out of memory
		g_res_count = RES_BUFF_SIZE;
		g_free_count = 0;
		res_int_t res = res_int_ok(5, ERRINFO);
		int line = __LINE__ - 1;
		ASSERT(g_res_count == RES_BUFF_SIZE);
		ASSERT(!g_free_count);
		ASSERT(res.id == g_fallback_id);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		ASSERT(g_res_fallback.err.err_info.line == line);
		ASSERT(!strcmp(g_res_fallback.err.msg, "Not enough memory"));
		reset_globals();
	}
	{ // Type too big
		res_obj_t res = res_obj_ok((obj){0}, ERRINFO);
		int line = __LINE__ - 1;
		ASSERT(res.id == g_fallback_id);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		ASSERT(g_res_fallback.err.err_info.line == line);
		ASSERT(!strcmp(g_res_fallback.err.msg, "Not enough memory"));
		reset_globals();
	}
}

void test_res_int_err() {
	reset_globals();
	{ // Happy path
		res_int_t res = res_int_err("msg", ERRINFO);
		int line = __LINE__ - 1;
		ASSERT(g_res_count == 1);
		ASSERT(!res.id);
		ASSERT(g_res_buff[res.id].state == RES_STATE_ERR);
		ASSERT(!strcmp(g_res_buff[res.id].err.msg, "msg"));
		ASSERT(g_res_buff[res.id].err.err_info.line == line);
		ASSERT(!strcmp(g_res_buff[res.id].err.err_info.file, __FILE__));
		ASSERT(!strcmp(g_res_buff[res.id].err.err_info.func, __func__));
		reset_globals();
	}
	{ // Allocate in free list
		g_res_count = RES_BUFF_SIZE;
		g_free_buff[0] = 0;
		g_free_count = 1;
		res_int_t res = res_int_err("msg", ERRINFO);
		int line = __LINE__ - 1;
		ASSERT(g_res_count == RES_BUFF_SIZE);
		ASSERT(!g_free_count);
		ASSERT(!res.id);
		ASSERT(g_res_buff[res.id].state == RES_STATE_ERR);
		ASSERT(!strcmp(g_res_buff[res.id].err.msg, "msg"));
		ASSERT(g_res_buff[res.id].err.err_info.line == line);
		ASSERT(!strcmp(g_res_buff[res.id].err.err_info.file, __FILE__));
		ASSERT(!strcmp(g_res_buff[res.id].err.err_info.func, __func__));
		reset_globals();
	}
	{ // Not enough memory
		g_res_count = RES_BUFF_SIZE;
		g_free_count = 0;
		res_int_t res = res_int_err("msg", ERRINFO);
		int line = __LINE__ - 1;
		ASSERT(res.id == g_fallback_id);
		ASSERT(g_res_count == RES_BUFF_SIZE);
		ASSERT(!g_free_count);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		ASSERT(!strcmp(g_res_fallback.err.msg, "Not enough memory"));
		ASSERT(g_res_fallback.err.err_info.line == line);
		ASSERT(!strcmp(g_res_fallback.err.err_info.file, __FILE__));
		ASSERT(!strcmp(g_res_fallback.err.err_info.func, __func__));
		reset_globals();
	}
}

void test_res_int_get_ok() {
	reset_globals();
	{ // Happy path
		res_int_t res = res_int_ok(5, ERRINFO);
		int ok = 0;
		ASSERT(!res_int_get_ok(res, &ok, ERRINFO));
		ASSERT(ok == 5);
		reset_globals();
	}
	{ // Invalid id
		res_int_t res = {.id = RES_BUFF_SIZE / 2};
		int ok = 0;
		ASSERT(res_int_get_ok(res, &ok, ERRINFO) == 2);
		int line = __LINE__ - 1;
		ASSERT(!g_res_count);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		ASSERT(!strcmp(g_res_fallback.err.msg, "Invalid argument"));
		ASSERT(!strcmp(g_res_fallback.err.err_info.file, __FILE__));
		ASSERT(!strcmp(g_res_fallback.err.err_info.func, __func__));
		ASSERT(g_res_fallback.err.err_info.line == line);
		reset_globals();
	}
	{ // Size too big
		res_obj_t res = {.id = RES_BUFF_SIZE / 2};
		obj ok = {0};
		ASSERT(res_obj_get_ok(res, &ok, ERRINFO) == 2);
		int line = __LINE__ - 1;
		ASSERT(!g_res_count);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		ASSERT(!strcmp(g_res_fallback.err.msg, "Invalid argument"));
		ASSERT(!strcmp(g_res_fallback.err.err_info.file, __FILE__));
		ASSERT(!strcmp(g_res_fallback.err.err_info.func, __func__));
		ASSERT(g_res_fallback.err.err_info.line == line);
		reset_globals();
	}
	{ // Res state not RES_STATE_OK
		res_int_t res = res_int_err("msg", ERRINFO);
		int ok = 0;
		ASSERT(res_int_get_ok(res, &ok, ERRINFO) == 1);
		int line = __LINE__ - 1;
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		ASSERT(!strcmp(g_res_fallback.err.msg, "Result state is not RES_STATE_OK"));
		ASSERT(!strcmp(g_res_fallback.err.err_info.file, __FILE__));
		ASSERT(!strcmp(g_res_fallback.err.err_info.func, __func__));
		ASSERT(g_res_fallback.err.err_info.line == line);
		reset_globals();
	}
}

void test_res_int_get_err_from() {
	reset_globals();
	{ // Happy path
		res_int_t res1 = res_int_err("msg", ERRINFO);
		res_int_t res2 = res_int_err_from(res1.id, ERRINFO);
		int line = __LINE__ - 2;
		ASSERT(g_res_buff[res2.id].state == RES_STATE_ERR);
		ASSERT(g_res_buff[res2.id].err.err_info.line == line);
		ASSERT(!strcmp(g_res_buff[res2.id].err.err_info.file, __FILE__));
		ASSERT(!strcmp(g_res_buff[res2.id].err.err_info.func, __func__));
		ASSERT(!strcmp(g_res_buff[res2.id].err.msg, "msg"));
		reset_globals();
	}
	{ // Invalid src id
		res_int_t res1 = {.id = RES_BUFF_SIZE / 2};
		res_int_t res2 = res_int_err_from(res1.id, ERRINFO);
		int line = __LINE__ - 1;
		ASSERT(res2.id == g_fallback_id);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		ASSERT(g_res_fallback.err.err_info.line == line);
		ASSERT(!strcmp(g_res_fallback.err.err_info.file, __FILE__));
		ASSERT(!strcmp(g_res_fallback.err.err_info.func, __func__));
		ASSERT(!strcmp(g_res_fallback.err.msg, "Invalid argument"));
		reset_globals();
	}
	{ // State is not RES_STATE_ERR
		res_int_t res1 = res_int_ok(5, ERRINFO);
		res_int_t res2 = res_int_err_from(res1.id, ERRINFO);
		int line = __LINE__ - 1;
		ASSERT(res2.id == g_fallback_id);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		ASSERT(g_res_fallback.err.err_info.line == line);
		ASSERT(!strcmp(g_res_fallback.err.err_info.file, __FILE__));
		ASSERT(!strcmp(g_res_fallback.err.err_info.func, __func__));
		ASSERT(!strcmp(g_res_fallback.err.msg, "Invalid argument"));
		reset_globals();
	}
	{ // Not enough memory
		res_int_t res1 = res_int_err("msg", ERRINFO);
		g_res_count = RES_BUFF_SIZE;
		g_free_count = 0;
		res_int_t res2 = res_int_err_from(res1.id, ERRINFO);
		int line = __LINE__ - 1;
		ASSERT(res2.id == g_fallback_id);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		ASSERT(g_res_fallback.err.err_info.line == line);
		ASSERT(!strcmp(g_res_fallback.err.err_info.file, __FILE__));
		ASSERT(!strcmp(g_res_fallback.err.err_info.func, __func__));
		ASSERT(!strcmp(g_res_fallback.err.msg, "Not enough memory"));
		reset_globals();
	}
	{ // Allocate in free list
		res_int_t res1 = res_int_err("msg", ERRINFO);
		int line = __LINE__ - 1;
		g_free_count = 1;
		g_free_buff[0] = 0;
		g_res_count = RES_BUFF_SIZE;
		res_int_t res2 = res_int_err_from(res1.id, ERRINFO);
		ASSERT(g_res_count == RES_BUFF_SIZE);
		ASSERT(!g_free_count);
		ASSERT(!res2.id);
		ASSERT(g_res_buff[res2.id].state == RES_STATE_ERR);
		ASSERT(g_res_buff[res2.id].err.err_info.line == line);
		ASSERT(!strcmp(g_res_buff[res2.id].err.err_info.file, __FILE__));
		ASSERT(!strcmp(g_res_buff[res2.id].err.err_info.func, __func__));
		ASSERT(!strcmp(g_res_buff[res2.id].err.msg, "msg"));
		reset_globals();
	}
}

void test_res_int_del() {
	reset_globals();
	{ // Happy path
		res_int_t res = res_int_ok(5, ERRINFO);
		ASSERT(g_res_buff[res.id].state == RES_STATE_OK);
		res_int_del(res, ERRINFO);
		ASSERT(g_res_fallback.state == RES_STATE_INVALID);
		ASSERT(g_res_buff[res.id].state == RES_STATE_INVALID);
		ASSERT(g_free_count == 1);
		reset_globals();
	}
	{ // Invalid id
		res_int_del((res_int_t){.id = RES_BUFF_SIZE / 2}, ERRINFO);
		int line = __LINE__ - 1;
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		ASSERT(!g_res_count);
		ASSERT(g_res_fallback.err.err_info.line == line);
		ASSERT(!strcmp(g_res_fallback.err.err_info.func, __func__));
		ASSERT(!strcmp(g_res_fallback.err.err_info.file, __FILE__));
		ASSERT(!strcmp(g_res_fallback.err.msg, "Invalid argument"));
		reset_globals();
	}
	{ // Res has already been deleted
		res_int_t res = res_int_ok(5, ERRINFO);
		res_int_del(res, ERRINFO);
		res_int_del(res, ERRINFO);
		int line = __LINE__ - 1;
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		ASSERT(g_res_count == 1);
		ASSERT(g_res_fallback.err.err_info.line == line);
		ASSERT(!strcmp(g_res_fallback.err.err_info.func, __func__));
		ASSERT(!strcmp(g_res_fallback.err.err_info.file, __FILE__));
		ASSERT(!strcmp(g_res_fallback.err.msg, "Invalid argument"));
		reset_globals();
	}
	{ // Res has already been deleted
		res_int_t res = res_int_ok(5, ERRINFO);
		g_free_count = FREE_BUFF_SIZE;
		res_int_del(res, ERRINFO);
		int line = __LINE__ - 1;
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		ASSERT(g_res_count == 1);
		ASSERT(g_res_fallback.err.err_info.line == line);
		ASSERT(!strcmp(g_res_fallback.err.err_info.func, __func__));
		ASSERT(!strcmp(g_res_fallback.err.err_info.file, __FILE__));
		ASSERT(!strcmp(g_res_fallback.err.msg, "Not enough memory"));
		reset_globals();
	}
}

void test_res_int_print_err() {
	reset_globals();
	{ // Happy path
		res_int_t res = res_int_err("msg", ERRINFO);
		res_int_print_err(res, ERRINFO);
		ASSERT(g_is_error_printed);
		ASSERT(!g_is_fallback_error_printed);
		reset_globals();
	}
	{ // Invalid id
		res_int_print_err((res_int_t){.id = 0}, ERRINFO);
		int line = __LINE__ - 1;
		ASSERT(!g_is_error_printed);
		ASSERT(g_is_fallback_error_printed);
		ASSERT(g_res_fallback.err.err_info.line == line);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		ASSERT(!strcmp(g_res_fallback.err.err_info.file, __FILE__));
		ASSERT(!strcmp(g_res_fallback.err.err_info.func, __func__));
		ASSERT(!strcmp(g_res_fallback.err.msg, "Invalid argument"));
		reset_globals();
	}
	{ // State not RES_STATE_ERR
		res_int_t res = res_int_ok(5, ERRINFO);
		res_int_print_err(res, ERRINFO);
		int line = __LINE__ - 1;
		ASSERT(!g_is_error_printed);
		ASSERT(g_is_fallback_error_printed);
		ASSERT(g_res_fallback.err.err_info.line == line);
		ASSERT(g_res_fallback.state == RES_STATE_ERR);
		ASSERT(!strcmp(g_res_fallback.err.err_info.file, __FILE__));
		ASSERT(!strcmp(g_res_fallback.err.err_info.func, __func__));
		ASSERT(!strcmp(g_res_fallback.err.msg, "Invalid argument"));
		reset_globals();
	}
}

void test_typedef() {
	test_res_int_ok();
	test_res_int_err();
	test_res_int_get_ok();
	test_res_int_get_err_from();
	test_res_int_del();
	test_res_int_print_err();
}
