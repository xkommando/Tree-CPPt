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
#include "tsan_mutexset.h"
#include "tsan_test_util.h"
#include "gtest/gtest.h"
#include "tree/tsan_tree.h"
#include <stdlib.h>
#include <iostream>

#include "tsan_mutex.h"

using std::cout;
using std::endl;
using std::flush;

using namespace __tsan;

//std::atomic<int>
static int g_x;
static int g_y;
static int g_z;

#define TS_R4(addr, idx)\
          __tsan_read4(&g_x);

#define TS_W4(addr, idx)\
          __tsan_write4(&g_x);


pthread_rwlock_t rwlock;
pthread_mutex_t mutex;

void print_thread_lockset() {
  ThreadState *cur_thread_state = cur_thread();
  printf("\r\n!!! thread %d  lockset: ", cur_thread_state->tid);
  for (u32 i = 0; i < cur_thread_state->mset.Size(); ++i) {
    MutexSet::Desc desc = cur_thread_state->mset.Get(i);
    printf("mutex id=%d \r\n", desc.id);
  }
  printf("\r\n");
}

void print_thread_history() {
  ThreadState *curThr = cur_thread();
  printf("\r\n!!! thread %d history : ", curThr->tid);
  __tsan::Vector <u32> &vec = curThr->tctx->threadHistory;
  for (u32 i = 0; i < vec.Size(); ++i) {
    printf("%d   ", vec[i]);
  }
  printf("\r\n");
}

RWMutex _M_lock;
void *t1(void *pr) {
  __tsan_func_entry(__builtin_return_address(0));
//    __tsan_read4(&g_x);
//    if (g_x > 0) {
//    for (int i = 0; i < (1<<10); ++i) {
//    pthread_rwlock_rdlock(&rwlock);
//    print_thread_lockset();
//    print_thread_history();
//    pthread_rwlock_unlock(&rwlock);

  for (int i = 0; i < (1 << 6); ++i) {
//        pthread_rwlock_wrlock(&rwlock);
//    _M_lock.Lock();
    TS_R4(&g_x, 0);
    TS_R4(&g_z, 1);
    TS_R4(&g_y, 2);
    g_y += g_x * g_z;
    for (int j = 0; j < (1 << 23); ++j) {
      TS_R4(&g_y, 3);
      g_y++;
    }
    TS_R4(&g_y, 4);
    printf("%d", g_y);
//    _M_lock.Unlock();
//        pthread_rwlock_unlock(&rwlock);
  }
  printf("\r\n");
//    }
//    }
  __tsan_func_exit();
  return NULL;
}

void *t2(void *) {

//    print_thread_history();
  pthread_t pt1;
//    pthread_mutex_lock(&mutex);
//    pthread_rwlock_rdlock(&rwlock);
  int ret = pthread_create(&pt1, NULL, &t1, NULL);
//    pthread_mutex_unlock(&mutex);
//    pthread_rwlock_unlock(&rwlock);
  if (ret != 0) {
    printf("failed C");
  }
//    print_thread_history();
  pthread_join(pt1, NULL);
//    print_thread_history();
  return nullptr;
}

void *t3(void *) {
  pthread_t pt1;
//    pthread_mutex_lock(&mutex);
//    pthread_rwlock_rdlock(&rwlock);
//    print_thread_history();
  int ret = pthread_create(&pt1, NULL, &t2, NULL);
//    pthread_mutex_unlock(&mutex);
//    pthread_rwlock_unlock(&rwlock);

  if (ret != 0) {
    printf("failed C");
  }
  pthread_join(pt1, NULL);
  return nullptr;
}

int main() {

  __tsan_func_entry(__builtin_return_address(0));
  pthread_t pt1;
  pthread_t pt2;
  int ret;

//    __tsan_write4(&g_x);
  g_x = 0;
//    __tsan_write4(&g_y);
//    _M_tsan_write4(&g_y, 5);
  g_y = 0;

  ret = pthread_create(&pt1, NULL, &t3, NULL);
  if (ret != 0) {
    printf("failed C");
  }
  ret = pthread_create(&pt2, NULL, &t1, NULL);
  if (ret != 0) {
    printf("failed C");
  }
  {
//        pthread_rwlock_wrlock(&rwlock);
//        __tsan_write4(&g_x);
//        _M_tsan_write4(&g_x, 6);
    g_x = 1;
//        __tsan_write4(&g_y);
//        _M_tsan_write4(&g_y, 7);
    g_y = 2;
//        pthread_rwlock_unlock(&rwlock);
  }
  pthread_join(pt1, NULL);
  pthread_join(pt2, NULL);
  Printf("\r\n y=%d\r\n", g_y);
  Printf("g_x addr=%p  \r\n", &g_x);
  Printf("g_y addr=%p  \r\n", &g_y);
  Printf("g_z addr=%p  \r\n", &g_z);
  __tsan_func_exit();
  return 0;
}

//
//TEST(MyTest, RWR_TSAN) {
//__tsan_init();
//__tsan_func_entry(__builtin_return_address(0));
//
////u64* ptr_x = (u64*)MemToShadow((uptr)&g_x);
////u64* ptr_y = (u64*)MemToShadow((uptr)&g_y);
////u64* ptr_z = (u64*)MemToShadow((uptr)&g_z);
////
////uptr addr_tgt[2];
////addr_tgt[0] = (uptr)&g_x;
////addr_tgt[1] = (uptr)&g_y;
////
////__tsan::_M_set_addr_len(2);
////__tsan::_M_set_addrs(addr_tgt);
////
////cout << "??????????????????????????????????????????????????????????????????????????????????????????" << endl << flush;
////Printf("x addr=%p  shadow=%p\r\n", &g_x, ptr_x);
////Printf("y addr=%p  shadow=%p\r\n", &g_y, ptr_y);
////Printf("z addr=%p  shadow=%p\r\n", &g_z, ptr_z);
//t_main();
//cout << "??????????????????????????????????????????????????????????????????????????????????????????" << endl << flush;
//__tsan_func_exit();
//using namespace __tsan::aser::tree;
//_M_Stat stat = _M_getStat();
//
//Printf("\r\n\r\n>>>all %d, accepted:%d,  dropped: %d\r\n", stat.all, (stat.all - stat.dropped), stat.dropped);
//
////Printf("\r\n\r\nx=%d  y=%d\r\n", &g_x, g_y);
//
//}
