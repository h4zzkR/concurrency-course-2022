#include <exe/executors/inline.hpp>

namespace exe::executors {

class InlineExecutor : public IExecutor {
 public:
  // IExecutor
  void Execute(TaskBase* task) override {
    task->Run();
  }
};

IExecutor& GetInlineExecutor() {
  static InlineExecutor instance;
  return instance;
}

}  // namespace exe::executors
