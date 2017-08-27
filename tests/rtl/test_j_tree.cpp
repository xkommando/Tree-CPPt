//
// Created by cbw on 6/20/16.
//

//
// Created by cbw on 6/20/16.
//


#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <atomic>

#include "tsan_interface.h"
#include "tsan_rtl.h"
#include "tsan_test_util.h"
#include "gtest/gtest.h"

#include <stdlib.h>
#include <iostream>
using std::cout;
using std::endl;
using std::flush;

using namespace __tsan;

//std::atomic<int>
static int g_x;
static int g_y;

void* t1_tree(void* pr) {
    __tsan_func_entry(__builtin_return_address(0));
//    __tsan_read4(&g_x);
//    if (g_x > 0) {
    bool checked1 = false;
    bool checked2 = false;
    for (int i = 0; i < (1<<2); ++i) {
        for (int j = 0; j < (1<<3); ++j) {
            if (!checked1) {
                __tsan_write4(&g_y);
                checked1 = true;
            }
            g_y++;
        }
        if (!checked2) {
            __tsan_read4(&g_y);
            checked2 = true;
        }
        printf("%d", g_y);
    }
//    }
    __tsan_func_exit();
    return NULL;
}

int t_main_tree() {

    __tsan_func_entry(__builtin_return_address(0));
    pthread_t pt1;
    int ret;

//    __tsan_write4(&g_x);
    g_x = 0;
//    __tsan_write4(&g_y);
    g_y = 0;

    ret = pthread_create(&pt1, NULL, &t1_tree, NULL);
    if (ret != 0) {
        printf("failed C");
    }

    {
        __tsan_write4(&g_x);
        g_x = 1;
        __tsan_write4(&g_y);
        g_y = 2;
    }
    pthread_join(pt1, NULL);
    __tsan_func_exit();
    t1_tree(0);
    return 0;
}

TEST(MyTest, RWR_TREE) {
__tsan_init();
__tsan_func_entry(__builtin_return_address(0));

u64* ptr_x = (u64*)MemToShadow((uptr)&g_x);
u64* ptr_y = (u64*)MemToShadow((uptr)&g_y);

uptr addr_tgt[2];
addr_tgt[0] = (uptr)&g_x;
addr_tgt[1] = (uptr)&g_y;

__tsan::_M_set_addr_len(2);
__tsan::_M_set_addrs(addr_tgt);

cout << "??????????????????????????????????????????????????????????????????????????????????????????" << endl << flush;
Printf("x addr=%p  shadow=%p", &g_x, ptr_x);
Printf("y addr=%p  shadow=%p", &g_y, ptr_y);
t_main_tree();
cout << "??????????????????????????????????????????????????????????????????????????????????????????" << endl << flush;
__tsan_func_exit();

Printf("\r\n\r\nx=%d  y=%d\r\n", &g_x, g_y);

}
