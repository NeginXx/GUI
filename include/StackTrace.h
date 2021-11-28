#pragma once
#include <signal.h>
#include <initializer_list>
#define $ StackPush(Info{__func__, __LINE__})
#define $$ StackPop()
#define RETURN $$; return

struct Info {
  const char* func_name;
  size_t line;
};

void StackPush(const Info& info);
void StackPop();
// signums - numbers of signals to be caught, for example {SIGSEGV, SIGABRT}
// max_size - maximum number of lines to be in stack
// max_size of 10k or more is not recommended since the stack dump won't even fit on screen
void InitStackTrace(std::initializer_list<int> signums, size_t max_size);
void FreeStackTrace();