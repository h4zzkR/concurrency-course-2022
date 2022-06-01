#include <twist/stdlike/atomic.hpp>
#include <exe/executors/execute.hpp>
#include <wheels/intrusive/forward_list.hpp>
#include <wheels/logging/logging.hpp>
//#include <wheels/intrusive/list.hpp>

using exe::executors::TaskBase;

class LFStack {
  //  struct Slot {
  //    Slot* next = nullptr;
  //    TaskBase* item = nullptr;
  //  };

  //  using SlotT = wheels::IntrusiveForwardListNode<T>;

 public:
  TaskBase* TakeAll() {
    auto head = head_.load();
    while (!head_.compare_exchange_strong(head, nullptr)) {
    }

    //    for (size_t i = 0; i < sz; ++i) {
    //      head = static_cast<TaskBase*>(head->next_);
    //    }
    //    LOG_SIMPLE(head)
    return head;
  }

  bool Empty() {
    return head_.load() == nullptr;
  }

  void Add(TaskBase* item) {
    auto head = head_.load();
    item->SetNext(head);
    while (!head_.compare_exchange_strong(head, item)) {
    }
  }

 private:
  //  IntrusiveForwardListNode<T>
  twist::stdlike::atomic<TaskBase*> head_{nullptr};
};
