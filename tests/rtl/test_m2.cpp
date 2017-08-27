//
// Created by cbw on 6/20/16.
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


pthread_rwlock_t rwlock;
int g_count1 = 0;
int g_count2 = 0;
static const int PT_NUM = 10;

void* t1(void*) {
    __tsan_func_entry(__builtin_return_address(0));
    int a;
    __tsan_read4(&g_count1);
    a = g_count1;
    printf("\r\nR%d\r\n", a);
    __tsan_func_exit();
    return NULL;
}

void dummy() {
    __tsan_func_entry(__builtin_return_address(0));
    __tsan_func_exit();
}
void* t2(void* tm) {
    __tsan_func_entry(__builtin_return_address(0));
    int a;
    pthread_t pt1;
    int b = *(int*)tm;

    int pnext = ++b;
    usleep((unsigned int) (2 + b));
//    usleep((unsigned int)2);
//    dummy();
    if (b < PT_NUM) {
        a = pthread_create(&pt1, NULL, &t2, &pnext);
    }
    pthread_rwlock_wrlock(&rwlock);
    __tsan_read4(&g_count1);
    a = g_count1;
    pthread_rwlock_unlock(&rwlock);
    printf("r%d", a);
    pthread_join(pt1, NULL);
    __tsan_func_exit();
    return NULL;
}

void* t3(void*) {
    __tsan_func_entry(__builtin_return_address(0));
    usleep(150);
    int aa = 5;
    pthread_rwlock_wrlock(&rwlock);
//    __tsan_read4(&g_count1);
//    aa = g_count1;
    __tsan_write4(&g_count1);
    g_count1 = 5;
    pthread_rwlock_unlock(&rwlock);
    printf("\r\nW\r\n");
    __tsan_func_exit();
    return NULL;
}

int t_main() {
    __tsan_func_entry(__builtin_return_address(0));
    pthread_t pt1, pt3;
//    pthread_t pts[PT_NUM];
    pthread_t pts;
//    int pids[PT_NUM];
    int pstart = 1;
    int ret, i;
    pthread_rwlock_init(&rwlock, NULL);
    setbuf(stdout, NULL);

    ret = pthread_create(&pt1, NULL, &t1, NULL);
    if (ret != 0) {
        printf("!Rfc%d", 1);
    }

    sleep(1);
//    for (i = 0; i < PT_NUM; ++i) {
//        pids[i] = i;
        ret = pthread_create(&pts, NULL, &t2, &pstart);
        if (ret != 0) {
            printf("!fc%d", i);
        }
//    }

    sleep(1);
    ret = pthread_create(&pt3, NULL, &t3, NULL);
    if (ret != 0) {
        printf("!Wfc%d", 2);
    }
//    for (i = 0; i < PT_NUM; ++i) {
//        ret = pthread_join(pts[i], NULL);
//        if (ret != 0) {
//            printf("!j%d", i);
//        }
//    }
//    for (i = 0; i < PT_NUM; ++i) {
//        pids[i] = i;
//        ret = pthread_create((pthread_t *) (pts + i), NULL, &t2, &pids[i]);
//        if (ret != 0) {
//            printf("!fc%d", i);
//        }
//    }

    sleep(1);

    pthread_join(pt1, NULL);
    pthread_join(pt3, NULL);
//    for (i = 0; i < PT_NUM; ++i) {
        pthread_join(pts, NULL);
//    }
    __tsan_func_exit();
    return 0;
}

TEST(MyTest, RWR_1) {
__tsan_init();
//__tsan_func_entry(__builtin_return_address(0));

u64* ptr = (u64*)MemToShadow((uptr)&g_count1);
u64* ptr2 = (u64*)MemToShadow((uptr)&g_count2);
uptr addr_tgt[2];
addr_tgt[0] = (uptr)&g_count1;
addr_tgt[1] = (uptr)&g_count2;

__tsan::_M_set_addr_len(2);
__tsan::_M_set_addrs(addr_tgt);

cout << "??????????????????????????????????????????????????????????????????????????????????????????" << endl << flush;
Printf("addr=%p  shadow=%p", &g_count1, ptr);
t_main();
cout << "??????????????????????????????????????????????????????????????????????????????????????????" << endl << flush;
//__tsan_func_exit();


}
