



//#include "llvm/ADT/ArrayRef.h"
#include "../tsan_defs.h"
#include "../tsan_mutexset.h"
#include "../tsan_rtl.h"
#include "../tsan_mutex.h"

#include "sanitizer_common/sanitizer_placement_new.h"
#include "tsan_tree.h"

//#include <vector>
//#include <unordered_map>

namespace __tsan {
namespace aser {
namespace tree {


static _M_Stat G_stat[kMaxTid];

ALWAYS_INLINE
_M_Stat& _M_getStat0() {
  return G_stat[0];
}

_M_Stat _M_collectStat() {
  _M_Stat sum;
  Printf("\r\nPrint Collect\r\n");
  for (u32 i = 0; i < kMaxTid; ++i) {
//    if (G_stat[i].total != 0)
//      G_stat[i].PrintMe();
    sum.total += G_stat[i].total;
    sum.dropped += G_stat[i].dropped;
    sum.skipped += G_stat[i].skipped;
  }
  return sum;
}

static const u32 LocMapSize = 16777216;
static LocationCtxT locationContexts(128);
static LocationCtx2T LocMap2(LocMapSize);
static ContextSimpleTrie _M_Loc[100];

typedef HashMap<LocKey, ContextTrie<ContextTrieNode> *, LockKeyHash> LocationCtxT;

template <typename Node>
static inline void* _M_PrintMap(Node* node) {
  Printf("PC=%p  ", node->key_);
  node->value_->stat_.PrintMe();
  return nullptr;
}

struct _XXX {
  _XXX() {
  }
  ~_XXX() {
    if (locationContexts.size() > 0) {
      Printf("\r\nisTree()\r\n");
      locationContexts.Accept(&_M_PrintMap);
    } else if (LocMap2.size() > 0) {
      Printf("\r\nisTree2()\r\n");
      LocMap2.Accept(&_M_PrintMap);
    }
    _M_Stat stat = _M_collectStat();
    Printf("\r\n\r\n>>>total=%d, dropped=%d, skipped=%d\r\n", stat.total, stat.dropped, stat.skipped);
  }
};
static _XXX _xxx;
void ContextTrieNode::storeLockSet(const MutexSet& lockSet, u16 tid) {
  if (stack_ == nullptr) {
    const u32 sz = kLockStateSize * sizeof(LockStack);
    stack_ = static_cast<LockStack*>(internal_alloc(MBlockScopedBuf, sz));
    internal_memset(stack_, 0, sz);
  }
  LockStack* left = stack_;
  for (u32 i = 0; i < lockSet.Size(); ++i) {
    if (i >= kLockStateSize) {
      Printf("ERROR!!!, ContextTrieNode LockStack full!");
      return;
    }
    const u32 thrLockId = lockSet.Get(i).id;
    if (left->lockId_ == 0) {
      left->lockId_ = thrLockId;
      left->tid1_ = tid;
    }
    left++;
  }
}


template<typename TrieNode, typename VT>
ALWAYS_INLINE void _M_addNode(TrieNode** ppNode, VT tid) {
  void* ptr = internal_alloc(MBlockScopedBuf, sizeof(TrieNode));
  TrieNode* node = new(ptr)TrieNode(tid);
  (*ppNode)->children_->put(tid, node);
  *ppNode = node;
}


template <typename TrieNode, typename Ls, typename VT = typename Ls::value_type>
ALWAYS_INLINE TrieNode* mergeToTrie(TrieNode* pCurNode, const Ls& vec, bool* added) {

  *added = false;
  const u32 vecSz = vec.Size();
  for (u32 i = 0; i < vecSz; ++i) {
    const VT tid = vec[i];
    if (pCurNode->children_ == nullptr) {
      void* ptr = internal_alloc(MBlockScopedBuf, sizeof(typename TrieNode::MapT));
      pCurNode->children_ = new(ptr)typename TrieNode::MapT(kHashTableSize);
      _M_addNode(&pCurNode, tid);
      *added = true;
    } else {
      TrieNode** ppChild = pCurNode->children_->get(tid);
      if (ppChild == nullptr) {
        _M_addNode(&pCurNode, tid);
        *added = true;
      } else {
        pCurNode = *ppChild;
      }
    }
  }
  return pCurNode;
}


template <typename TN>
typename ContextTrie<TN>::TrieNodeT*
ContextTrie<TN>::newBranch(const __tsan::Vector<u32>& tidHistory) {
  bool added;
  return mergeToTrie(&this->root, tidHistory, &added);
}


template <typename TN>
bool ContextTrie<TN>::checkTree(ThreadState *thr, bool isWrite) {
  const __tsan::Vector<u32>& tidHistory = thr->tctx->threadHistory;
  bool added = false;
  TrieNodeT* pCurNode = mergeToTrie(&this->root, tidHistory, &added);
  const u16 thrTid = (u16)thr->tid;
  const MutexSet& lockSet = thr->mset;
  LockStack* stacks = pCurNode->stack_;

  if (added) { // new context, store the lock set
    for (u32 i = 0; i < lockSet.Size(); ++i) {
      u32 thrLockId = lockSet.Get(i).id;
      stacks[i].lockId_ = thrLockId;
      stacks[i].tid1_ = thrTid;
    }
    return false;

  } else { // existing context
    bool do_drop = true;
    for (u32 i = 0; i < lockSet.Size(); ++i) {
      const u32 thrLockId = lockSet.Get(i).id;
      for (u32 j = 0; j < kLockStateSize; ++j) {
        if (stacks[j].lockId_ == thrLockId) {

          // match locks
          if (stacks[j].tid2_ == 0) {

            if (stacks[j].tid1_ == 0) { // size = 0
              stacks[j].tid1_ = thrTid; ////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
              do_drop = false;
            } else { // size = 1, there is tid1
              if (isWrite && stacks->tid1_ != thrTid) {
                stacks[j].tid2_ = thrTid;////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
                do_drop = false;
              }
            }
          }
          break;
        }
      }
    }
    return do_drop;
  }
}


RWMutex _M_lock;

bool isTree(ThreadState *thr, uptr pc, uptr addr, bool isWrite) {
  const u16 tid = static_cast<u16>(thr->tid);
  _M_Stat& stat = G_stat[tid];
  stat.total++;
  LocKey key(pc, addr);
_M_lock.Lock();
  CtxTrieT** ctx = locationContexts.get(key);
_M_lock.Unlock();
  if (ctx == nullptr) {
    const MutexSet& mset = thr->mset;
    void* ptr = internal_alloc(MBlockScopedBuf, sizeof(CtxTrieT));
    CtxTrieT * nCtx = new(ptr)CtxTrieT();
    nCtx->lastTid_ = tid;
_M_lock.Lock();
    nCtx->newBranch(thr->tctx->threadHistory)->storeLockSet(mset, tid);
    locationContexts.put(key, nCtx);
    nCtx->stat_.total++;
_M_lock.Unlock();
    thr->sNotUpdated_ = true;
    return false;
  } else {
_M_lock.Lock();
    if ((*ctx)->lastTid_ == tid && thr->sNotUpdated_) {

      (*ctx)->stat_.total++;
      (*ctx)->stat_.skipped++;
_M_lock.Unlock();
      stat.skipped++;
      return true;
    } else {
      (*ctx)->lastTid_ = tid;
      thr->sNotUpdated_ = true;
      bool ret = (*ctx)->checkTree(thr, isWrite);
      (*ctx)->stat_.total++;
      (*ctx)->stat_.dropped += ret;
_M_lock.Unlock();
      stat.dropped += ret;
      return ret;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////


bool ContextSimpleTrie::checkTree(ThreadState *thr, bool isWrite) {
  const __tsan::Vector<u32>& tidHistory = thr->tctx->threadHistory;
  bool added = false;
  ContextTrieSimpleNode* pCurNode = mergeToTrie(&this->root, tidHistory, &added);
  if (added) {
    return false;
  } else {
    const u16 thrTid = static_cast<u16>(thr->tid);
    LockStack& stack = pCurNode->stack_;
    if (stack.tid2_ == 0) {
      if (stack.tid1_ == 0) { // size = 0
        stack.tid1_ = thrTid; ////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
        return false;
      } else { // size = 1, there is tid1
        if (isWrite && stack.tid1_ != thrTid) {
          stack.tid2_ = thrTid;////////////////////////////!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
          return false;
        }
      }
    }
    return true;
  }
}




bool isTree3(ThreadState *thr, uptr pc, uptr addr, bool isWrite, u64 index) {
  const u16 tid = static_cast<u16>(thr->tid);
  _M_Stat& stat = G_stat[tid];
  stat.total++;
  ContextSimpleTrie& ctx = _M_Loc[index];
  if (ctx.lastTid_ > 1) {
    stat.skipped++;
    return true;
  }else {
    ctx.lastTid_++;
    return false;
  }
}

bool isTree2(ThreadState *thr, uptr pc, uptr addr, bool isWrite) {
  const u16 tid = static_cast<u16>(thr->tid);
  _M_Stat& stat = G_stat[tid];
  stat.total++;
//return true;
//  LocKey key(pc, addr);
_M_lock.Lock();
//  ContextSimpleTrie** ctx = LocMap2.get(key);
  ContextSimpleTrie** ctx = LocMap2.get(pc);
  if (ctx == nullptr) {
    if (LocMap2.size() > LocMapSize * 0.75 ) {
      LocMap2.clear();
    }
    void* ptr = internal_alloc(MBlockScopedBuf, sizeof(ContextSimpleTrie));
    auto * nCtx = new(ptr)ContextSimpleTrie();
    auto* node = nCtx->newBranch(thr->tctx->threadHistory);
//    nCtx->lastTid_ = tid;
    node->stack_.tid1_ = tid;
    nCtx->lastTid_ = tid;
    nCtx->stat_.total++;
    LocMap2.put(pc, nCtx);
_M_lock.Unlock();
    thr->sNotUpdated_ = true;
    return false;
  } else {
//      const u16 counter = (*ctx)->lastTid_;
//      if (counter > 1)
//        return true;
//      else {
//        (*ctx)->lastTid_++;
//        return false;
//      }
      (*ctx)->stat_.total++;
      (*ctx)->lastTid_ = tid;
      if ((*ctx)->lastTid_ == tid && thr->sNotUpdated_) {
        (*ctx)->stat_.skipped++;
_M_lock.Unlock();
        stat.skipped++;
        return true;
      } else {
        thr->sNotUpdated_ = true;
        bool ret = (*ctx)->checkTree(thr, isWrite);
        (*ctx)->stat_.dropped += ret;
_M_lock.Unlock();
        stat.dropped += ret;
        return ret;
      }
  }
}


} //  tree
} //  aser
} // __tsan

