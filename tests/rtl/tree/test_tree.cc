

#include "gtest/gtest.h"
#include "tsan_interface.h"
#include "../tsan_test_util.h"
#include "tree/tsan_tree.h"
#include "tree/hashmap.h"
#include <vector>
#include <string>
#include <iostream>

#include <pthread.h>
#include <atomic>
#include <unistd.h>
#include <stdlib.h>
#include <unordered_map>

using std::cout;
using std::endl;
using std::flush;

struct Hasher {
  unsigned int operator()(int key) {
    key += (key << 12);  // key *= (1 + (1 << 12))
    key ^= (key >> 22);
    key += (key << 4);   // key *= (1 + (1 << 4))
    key ^= (key >> 9);
    key += (key << 10);  // key *= (1 + (1 << 10))
    key ^= (key >> 2);
    // key *= (1 + (1 << 7)) * (1 + (1 << 12))
    key += (key << 7);
    key += (key << 12);
    return key;
  }
};



TEST(ThreadSanitizer, TREE_LockState) {

  using namespace __tsan::aser::tree;

  using std::string;
string s2 = "2";
string s3 = "3";
string s4 = "4";
string s5 = "5";

HashMap<int, std::string*, Hasher> map(64);
std::unordered_map<int, std::string> stdmap;

map.put(1, &s2);
map.put(1, &s3);
map.put(2, &s4);
map.put(3, &s5);
map.put(4, &s5);
EXPECT_EQ(map.size(), 4u);
map.remove(2);
EXPECT_EQ(map.size(), 3u);
map.remove(2);
EXPECT_EQ(map.size(), 3u);

EXPECT_EQ(**map.get(1), s3);
EXPECT_EQ(map.get(2), nullptr);
EXPECT_EQ(**map.get(3), s5);
EXPECT_EQ(**map.get(4), s5);
//map.PrintMe();
//  LockStates ls1(5, 9, 89);
//  EXPECT_EQ(ls1.lockId(), 5ull);
//  EXPECT_EQ(ls1.wTid1(), 9ull);
//  EXPECT_EQ(ls1.wTid2(), 89ull);


}

using namespace __tsan;
using namespace __tsan::aser::tree;

void print_thread_info() {
  ThreadState* cur_thread_state = cur_thread();
  printf("\r\n!!! thread info %d\r\n", cur_thread_state->tid);
  for (unsigned long i = 0; i < cur_thread_state->mset.Size(); ++i) {
    MutexSet::Desc desc = cur_thread_state->mset.Get(i);
    printf("mutex id=%d \r\n", desc.id);
  }
}

std::atomic<int> count(10);

void printThreadHistory() {
  ThreadState* thr = cur_thread();
//  ThreadContextBase* tCtx = thr->tctx;

  cout << "\r\n-------------------History" << endl;
  cout << "cur tid: " << thr->tid << endl;
//  u32 ptid = tCtx->parent_tid;
//  cout << "parent tid: " << ptid << "  ";
//
//  while (ptid != 0) {
//    ThreadContextBase *pctx = __tsan::ctx->thread_registry->threads_[ptid];
//    if (pctx == NULL) {
//      cout << "\r\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!ERROR NULL parent ThreadContextBase" << endl;
//    }
//    ptid = pctx->parent_tid;
//    cout << ptid << "   ";
//  }
  cout << "\r\n from vector: ";
  const __tsan::Vector<u32>& pids = thr->tctx->threadHistory;
  for (unsigned int i = 0; i < pids.Size(); ++i) {
    cout << "   " << pids[i];
  }
  cout << "\r\n-------------------History" << endl;
  cout << std::flush;
}

void* t1(void* pr) {
  printThreadHistory();
  count--;
  if (count.load() > 0) {
    pthread_t pt1;
    int ret;
    ret = pthread_create(&pt1, NULL, &t1, NULL);
    if (ret != 0) {
      printf("failed C");
    }
  }
//  if (count.load() < 2) {
//  }
  return NULL;
}

TEST(DISABLED_ThreadSanitizer, TREE_ThreadHistory) {

  pthread_t pt1;
  int ret;
printThreadHistory();
  ret = pthread_create(&pt1, NULL, &t1, NULL);
  if (ret != 0) {
    printf("failed C");
  }

}

u64 getPC() {
  return ((u64)__builtin_return_address(0));
}


void* t3(void*) {
  ThreadState* cur_thread_state = cur_thread();
  u64 pc = getPC();
  int a;
//bool isTree(ThreadState *thr, uptr pc, uptr addr, bool isWrite) {

  bool ret = aser::tree::isTree(cur_thread_state, pc, (uptr)&a, true);
  Printf(">>> test is_tree  %d\r\n\r\n", ret);

  ret = aser::tree::isTree(cur_thread_state, pc, (uptr)&a, true);
  Printf(">>> test is_tree  %d\r\n\r\n", ret);

  ret = aser::tree::isTree(cur_thread_state, pc, (uptr)&a, false);
  Printf(">>> test is_tree  %d\r\n\r\n", ret);

  return nullptr;
}


void* t2(void*) {
  pthread_t pt1;
  int ret;
  ret = pthread_create(&pt1, NULL, &t3, NULL);
  if (ret != 0) {
    printf("failed C");
  }
  pthread_join(pt1, NULL);
  return nullptr;
}

TEST(ThreadSanitizer, TREE_Vec) {
  __tsan::Vector<u32> vec(MBlockScopedBuf);
vec.PushBack(15);
vec.PushBack(16);
vec.Remove(6);
vec.PushBack(17);
vec.PushBack(18);
vec.Remove(6);
vec.Remove(16);
vec.PrintMe();
Printf("--------------------\r\n");
vec.PushBack(20);
vec.Remove(17);
vec.PrintMe();
Printf("--------------------\r\n");
for (const auto& a : vec) {
printf("%d\r\n", a);
}
}


//TEST(ThreadSanitizer, TREE_tree) {
//
//pthread_t pt1;
//int ret;
//ret = pthread_create(&pt1, NULL, &t2, NULL);
//if (ret != 0) {
//printf("failed C");
//}
//
//
//pthread_join(pt1, NULL);
//
//}
//
//TEST(ThreadSanitizer, TREE_context) {
//ContextTrie trie;
//
//Vector<u32> tids1(MBlockScopedBuf);
//
//tids1.PushBack(1);
//tids1.PushBack(2);
//tids1.PushBack(3);
//tids1.PushBack(5);
//tids1.PushBack(7);
//tids1.PushBack(9);
//
//trie.search(tids1);
//
//Vector<u32> tids2(MBlockScopedBuf);
//
//tids2.PushBack(1);
//tids2.PushBack(2);
//tids2.PushBack(3);
//
//trie.search(tids2);
//
//tids2.PushBack(6);
//tids2.PushBack(7);
//tids2.PushBack(99);
//
//trie.search(tids2);
////trie.PrintMe();
//
//}

























