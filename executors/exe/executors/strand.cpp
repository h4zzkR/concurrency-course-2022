//#include <exe/executors/strand.hpp>
//#include <wheels/support/defer.hpp>
//#include <wheels/logging/logging.hpp>
//
// namespace exe::executors {
//
// void Strand::Execute(TaskBase* task) {
//  tasks_storage_.Add(task);
//  if (can_execute_.exchange(false)) {
//    ScheduleLoop();
//  }
//}
//
// void Strand::ScheduleLoop() {
//  std::lock_guard locker(blocker);
//  TaskBase* task = tasks_storage_.TakeAll();
//  if (task == nullptr) {
//    can_execute_.store(true);
//    return;
//  }
//
//  exe::executors::Execute(executor_, [this, task]() mutable {
//    auto reverse = [](TaskBase* head) {
//      TaskBase* current = nullptr;
//      while (head != nullptr) {
//        TaskBase* next = (head->next_ == nullptr) ? nullptr :
//        head->next_->AsItem(); head->SetNext(current); current = head; head =
//        next;
//      }
//      return current;
//    };
//    TaskBase* reversed = reverse(task), *task_next;
////    TaskBase* reversed = task, *task_next = nullptr;
//
//    while (reversed != nullptr) {
//      task_next = (reversed->next_ == nullptr) ? nullptr :
//      reversed->next_->AsItem(); reversed->Run(); reversed = task_next;
//    }
//    can_execute_.store(true);
//    if (can_execute_.exchange(false)) {
//      ScheduleLoop();
//    }
//
//  });
//}
//
//// void Strand::EventLoop() {
////   for (size_t epochs = 0;; ++epochs) {
////     std::unique_lock guard(batches_locker_);
////     can_reschedule_ = false;
////
////     if (batches_.empty()) {
////       can_reschedule_ = true;
////       guard.unlock();
////       return;
////     } else if (epochs == batch_grab_limit_) {
////       can_reschedule_ = false;
////       ScheduleLoop(guard);
////       return;
////     }
////
////     auto batch = std::move(batches_.front());
////     batches_.pop();
////
////     guard.unlock();
////
////     batch();
////   }
//// }
//
//}  // namespace exe::executors

#include <exe/executors/strand.hpp>
#include <wheels/logging/logging.hpp>

namespace exe::executors {

void Strand::Execute(TaskBase* task) {
  std::unique_lock guard(batches_locker_);
  if (batches_.empty() || !batches_.back().AddTask(task)) {
    PushBatch(std::move(task));
  }

  if (can_reschedule_) {
    can_reschedule_ = false;
    ScheduleLoop(guard);
  } else {
    guard.unlock();
  }
}

bool Strand::StrandBatch::AddTask(TaskBase* task) {
  if (clip.size() >= clip_size) {
    return false;
  } else {
    clip.push(std::move(task));
    return true;
  }
}

void Strand::StrandBatch::operator()() {
  while (!clip.empty()) {
    clip.front()->Run();
    clip.pop();
  }
}

void Strand::EventLoop() {
  for (size_t epochs = 0;; ++epochs) {
    std::unique_lock guard(batches_locker_);
    can_reschedule_ = false;

    if (batches_.empty()) {
      can_reschedule_ = true;
      guard.unlock();
      return;
    } else if (epochs == batch_grab_limit_) {
      can_reschedule_ = false;
      ScheduleLoop(guard);
      return;
    }

    auto batch = std::move(batches_.front());
    batches_.pop();

    guard.unlock();

    batch();
  }
}

template <typename Locker>
void Strand::ScheduleLoop(std::unique_lock<Locker>& guarder) {
  batch_grab_limit_ = static_cast<size_t>(batches_.size() / k_grab_rate_) + 1;
  guarder.unlock();

  exe::executors::Execute(executor_, [this]() {
    EventLoop();
  });
}

Strand::StrandBatch::StrandBatch(size_t clip_size) : clip_size(clip_size) {
}

Strand::StrandBatch::StrandBatch(size_t clip_size, TaskBase* task)
    : StrandBatch(clip_size) {
  clip.push(std::move(task));
}

}  // namespace exe::executors
