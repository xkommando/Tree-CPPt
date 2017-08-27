//
// Created by cbw on 6/27/16.
//


#include "tsan_interface.h"
#include "tsan_rtl.h"
#include "tsan_test_util.h"
#include "gtest/gtest.h"

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
using std::cout;
using std::endl;
using std::flush;


using namespace __tsan;
const static u64 _SZ = (1<<15);
long g_arr[_SZ];

TEST(MyTest, Shadow_Mem) {

TestMutexBeforeInit();  // Mutexes must be usable before __tsan_init();
__tsan::_M_set_addr_len(2);
u64 steps = (_SZ / 20);
for (u64 i = 0; i < _SZ - 5; i+= steps) {
    uptr pga = (uptr)(g_arr + i);
    uptr pgasm = MemToShadow((uptr)(g_arr + i));
    Printf("%p -> %p\r\n", pga, pgasm);

pga = (uptr)(g_arr + i + 1);
pgasm = MemToShadow((uptr)(g_arr + i + 1));
Printf("%p -> %p\r\n", pga, pgasm);

pga = (uptr)(g_arr + i + 2);
pgasm = MemToShadow((uptr)(g_arr + i + 2));
Printf("%p -> %p\r\n", pga, pgasm);

pga = (uptr)(g_arr + i + 3);
pgasm = MemToShadow((uptr)(g_arr + i + 3));
Printf("%p -> %p\r\n", pga, pgasm);

pga = (uptr)(g_arr + i + 4);
pgasm = MemToShadow((uptr)(g_arr + i + 4));
Printf("%p -> %p\r\n", pga, pgasm);

Printf("\r\n");
}

__tsan_init();
__tsan_func_entry(__builtin_return_address(0));


__tsan_func_exit();
}