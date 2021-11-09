// signal(SIGSEGV, PrintStackInfoAndExit);
#pragma once
#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define $ FuncsCalledPush(FuncLine{__func__, __LINE__})
#define $$ CallStackPopback()
#define RETURN $$; return

struct FuncLine {
  const char* func_name_;
  size_t line_;
};

const size_t kCallStackMaxsize = (size_t)1e8;
void FuncsCalledPush(struct FuncLine func_line);
void PrintStackInfoAndExit(int signum);
void CallStackPopback();