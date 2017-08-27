
#ifndef ASER_TREE_H
#define ASER_TREE_H

#include "../tsan_defs.h"
#include "../tsan_mman.h"
#include "../tsan_mutexset.h"
#include "../tsan_rtl.h"
#include "hashmap.h"
#include "tree_hash.h"

namespace __tsan {
namespace aser {
namespace tree {

static const u32 kHashTableSize = 32;
static const u32 kLockStateSize = 4;

struct _M_Stat {
  u64 total;
  u64 dropped;
  u64 skipped;
  _M_Stat()
      : total(0),
        dropped(0),
        skipped(0) {}

  void PrintMe() const {
    Printf(">>>total=%d, accepted=%d, dropped=%d, skipped=%d \r\n",
           total,
           (total - dropped - skipped),
           dropped,
           skipped);
  }
};

struct LockStack {
  u32 lockId_;
  u16 tid1_;
  u16 tid2_;
  LockStack()
      : lockId_(0),
        tid1_(0),
        tid2_(0) {}

  void PrintMe() const {
    Printf("lock=%d, tid1=%d, tid2=%d\r\n", lockId_, tid1_, tid2_);
  }
};


struct ContextTrieNode {
  typedef HashMap<u32, ContextTrieNode*, Jenkins_mix32> MapT;

  u64 tid_;
  MapT* children_; // lazy
  LockStack* stack_; // lazy ???

  ALWAYS_INLINE
  explicit ContextTrieNode(u32 tid)
      : tid_(tid),
        children_(nullptr),
        stack_(nullptr)    {}

  ALWAYS_INLINE
  void storeLockSet(const MutexSet& lockSet, u16 tid);

  void PrintMe() const {
    Printf("tid=%d", tid_);
    if (stack_ != nullptr) {
      for (u32 i = 0; i < kLockStateSize; ++i) {
        stack_[i].PrintMe();
      }
    }
  }
};


template <typename ContextTrieNode>
class ContextTrie {
public:
  typedef ContextTrieNode  TrieNodeT;
  typedef typename TrieNodeT::MapT   NodeMapT;

  ContextTrieNode root;
  u16 lastTid_;
  _M_Stat stat_;

  ALWAYS_INLINE
  ContextTrie()
    : root(0),
      lastTid_(0),
      stat_()     {
    void* ptr = internal_alloc(MBlockScopedBuf, sizeof(NodeMapT));
    root.children_ = new(ptr)NodeMapT(kHashTableSize);
  }

//  void PrintMe() {
//    TrieNodeT* pCurNode = &this->root;
//    Printf("root: %d\r\n", pCurNode->tid_);
//    if (pCurNode->children_ != nullptr) {
//      pCurNode->children_->Accept(&_M_PrintMap);
//    }
//  }

  ALWAYS_INLINE
  TrieNodeT* newBranch(const __tsan::Vector<u32>& tidHistory);

  ALWAYS_INLINE
  bool checkTree(ThreadState *thr, bool isWrite);
};

struct ContextTrieSimpleNode {

  typedef HashMap<u32, ContextTrieSimpleNode*, Jenkins_mix32> MapT;

  MapT* children_; // lazy
  LockStack stack_;

  ALWAYS_INLINE
  explicit ContextTrieSimpleNode(u32 tid)
    : children_(nullptr),
      stack_()     {}

};

class ContextSimpleTrie: public ContextTrie<ContextTrieSimpleNode> {
public:

  typedef ContextTrie<ContextSimpleTrie>  base_type;
  typedef ContextTrieSimpleNode  TrieNodeT;
  typedef typename TrieNodeT::MapT   NodeMapT;

//  ContextSimpleTrie()
//      : base_type() {}

  ALWAYS_INLINE
  bool checkTree(ThreadState *thr, bool isWrite);

};
//template <typename TrieNode, typename Ls, typename VT = LS::value_type>
//TrieNode* mergeToTrie(TrieNode* pCurNode, const Ls& vec, bool* added);

struct LocKey {
  u64 pc_;
  u64 addr_;
  LocKey(u64 pc, u64 addr)
    : pc_(pc), addr_(addr) {}

  ALWAYS_INLINE
  bool operator==(const LocKey& other) const {
    return pc_ == other.pc_ && addr_ == other.addr_;
  }

  ALWAYS_INLINE
  bool operator!=(const LocKey& other) const {
    return pc_ != other.pc_ || addr_ != other.addr_;
  }
};

struct LockKeyHash {
  ALWAYS_INLINE
  u32 operator()(const LocKey& key) const {
    return twang_32from64(hash_128_to_64(key.pc_, key.addr_));
  }
};
struct PCHash {
  ALWAYS_INLINE
  u32 operator()(const u64& pc) const {
    return twang_32from64(pc);
  }
};
// PC -> CTX
typedef HashMap<u64, ContextSimpleTrie*, PCHash> LocationCtx2T;

typedef HashMap<LocKey, ContextTrie<ContextTrieNode> *, LockKeyHash> LocationCtxT;
typedef ContextTrie<ContextTrieNode> CtxTrieT;
typedef ContextTrieNode TrieNodeT;

//extern LocationCtxT locationContexts;


// partial H (fork join), lock set
bool isTree(ThreadState *thr, uptr pc, uptr addr, bool isWrite);

// full H
bool isTree2(ThreadState *thr, uptr pc, uptr addr, bool isWrite);

// just check loc count, nothing else
bool isTree3(ThreadState *thr, uptr pc, uptr addr, bool isWrite, u64 idx);


_M_Stat& _M_getStat0();
_M_Stat _M_collectStat();

} //  tree
} //  aser
} // __tsan


#endif //ASER_TREE_H
