#include <cassert>
#include <cstdio>
#include <stack>
#include <cstdlib>
#include "../include/StackTrace.h"

const int kExitCode = 200;

void PrintStackInfoAndExit(int signum);

class StackTrace {
 public:
  StackTrace() = delete;

  void PrintStackInfoAndExit(int signum) {
    static bool is_called_first_time = true;
    if (!is_called_first_time) {
      _Exit(kExitCode);
    }
    is_called_first_time = false;

    switch(signum) {
      case 6:
        printf("caught abrt :^(, error = %d\n", signum);
        break;
      case 11:
        printf("caught segv :^(, error = %d\n", signum);
        break;
      default:
        printf("caught signal :^(, signum = %d\n", signum);
        break;
    }

    if (stack_.empty()) {
      printf("0 registered functions on stack\n");
      _Exit(kExitCode);
    }

    size_t sz = stack_.size();
    if (sz == 1) {
      printf("%lu registered function on stack\n", sz);
    } else {
      printf("%lu registered functions on stack\n", sz);
    }
    printf("first %lu of those:\n", sz < max_size_ ? sz : max_size_);
    for (size_t i = 0; i < sz && i < max_size_; i++) {
      Info info = stack_.top();
      stack_.pop();
      printf("%lu: line %lu, function name %s\n", i + 1, info.line, info.func_name);
    }
    printf("\n");

    _Exit(kExitCode);
  }

  StackTrace(std::initializer_list<int> signums, size_t max_size)
  : segv_stack{}, max_size_(max_size)
  {
    segv_stack.ss_size = 4096;
    segv_stack.ss_sp = valloc(segv_stack.ss_size);
    sigaltstack(&segv_stack, NULL);
  
    struct sigaction action = {};
    action.sa_handler = ::PrintStackInfoAndExit;
    action.sa_flags = SA_ONSTACK;

    for (auto signum : signums) {
      sigaction(signum, &action, NULL);
    }
    max_size_ = max_size; 
  }

  ~StackTrace() {
    free(segv_stack.ss_sp);
  }

  void Push(const Info& info) {
    stack_.push(info);
  }

  void Pop() {
    if (stack_.empty()) {
      printf("Pop from empty StackTrace!!!\n");
      return;
    }
    stack_.pop();
  }

 private:
  std::stack<Info> stack_;
  stack_t segv_stack;
  size_t max_size_;
};

StackTrace* tracer = nullptr;

void InitStackTrace(std::initializer_list<int> signums, size_t max_size) {
  tracer = new StackTrace(signums, max_size);
}

void FreeStackTrace() {
  delete tracer;
  tracer = nullptr;
}

void PrintStackInfoAndExit(int signum) {
  tracer->PrintStackInfoAndExit(signum);
}

void StackPush(const Info& info) {
  assert(tracer != nullptr && "Use of uninitialized StackTrace!");
  tracer->Push(info);
}

void StackPop() {
  assert(tracer != nullptr && "Use of uninitialized StackTrace!");
  tracer->Pop();
}