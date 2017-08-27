
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <atomic>

//#include "tsan_interface.h"
//#include "tsan_rtl.h"
//#include "tsan_test_util.h"
//#include "gtest/gtest.h"

#include <stdlib.h>
//using namespace __tsan;

//std::atomic<int>
int g_x;
int g_y;
int g_z;
//void * get_pc () { return __builtin_return_address(0); }
//int get_counter1()
//{
//    __asm__ ("lea (%eip), %eax ") ;
//}
//
//int get_counter2()
//{
//    int x = 0;
//    __asm__ ("lea (%eip), %eax") ;
//}
void *t1(void *pr) {
//    if (g_x > 0) {
//    printf("\r\n1: %d\r\n", get_pc());
    for (int i = 0; i < (1 << 10); ++i) {
        for (int j = 0; j < (1 << 16); ++j) {
            g_y++;
        }
        printf("%d", g_y);
    }
    return NULL;
}


int main() {

    pthread_t pt1;
    int ret;

    g_x = 0;
    g_y = 0;

    t1(0);
//    printf("\r\n3: %d\r\n", get_pc());
    ret = pthread_create(&pt1, NULL, &t1, NULL);
    if (ret != 0) {
        printf("failed C");
    }
    t1(0);
    {
//        __tsan_write4(&g_x);
        g_x = 1;
//        __tsan_write4(&g_y);
        g_y = 2;
    }

    pthread_join(pt1, NULL);
    printf("\r\n\r\n%d  %d\r\n", g_x, g_y);
    return 0;
}