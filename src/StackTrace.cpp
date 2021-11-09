// signal(SIGSEGV, PrintStackInfoAndExit);
#include "../include/StackTrace.h"

struct CallStack {
  struct FuncLine* ptr;
  size_t sz;
  size_t capacity;
  size_t mincapacity;
  size_t maxcapacity;
  size_t incfac;
} funcs_called = {NULL, 0, 100, 100, 1000, 2};

size_t CallStackExpand() {
  if (funcs_called.ptr == NULL) {
    return 1;
  }
  if (funcs_called.capacity * funcs_called.incfac > kCallStackMaxsize) {
    printf("kCallStackMaxsize reached\n");
    return 1;
  }
  funcs_called.ptr = (struct FuncLine*)realloc(funcs_called.ptr, funcs_called.capacity * funcs_called.incfac * sizeof(struct FuncLine));
  if (funcs_called.ptr == NULL) {
    printf ("No more memory for called functions stack\n");
    return 1;
  }
  funcs_called.capacity *= funcs_called.incfac;
  return 0;
}

size_t CallStackPushback(struct FuncLine func_line) {
  if (funcs_called.ptr == NULL) {
    return 0;
  }
  if (funcs_called.sz >= kCallStackMaxsize) {
    fprintf(stderr, "call stack maxsize reached\n");
    return 0;
  }
  if (funcs_called.sz == funcs_called.capacity) {
    if (CallStackExpand() != 0) {
      return 0;
    }
  }
  funcs_called.ptr[funcs_called.sz++] = func_line;
  return 1;
}

void FuncsCalledPush(struct FuncLine func_line) {
  static int cnt = 0;
  if (cnt == 0) {
    funcs_called.ptr = (struct FuncLine*)calloc(funcs_called.capacity, sizeof(struct FuncLine));
    if (funcs_called.ptr == NULL) {
      printf ("Can not use stack trace, no more memory for stack\n");
    }
  }
  CallStackPushback(func_line);
  ++cnt;
}

struct FuncLine* FuncsCalledGetValues(size_t* num) {
  if (funcs_called.ptr == NULL) {
    return NULL;
  }
  size_t sz = funcs_called.sz;
  if (*num > sz) {
    *num = sz;
  }
  struct FuncLine* arr = (struct FuncLine*)calloc(*num, sizeof(int));
  assert(arr != NULL && "Calloc failed in StackTrace.cpp:FuncsCalledGetValues");
  for (size_t i = 0; i < *num; i++) {
    arr[i] = FuncLine{(funcs_called.ptr[i]).func_name_, (funcs_called.ptr[i]).line_};
  }
  return arr;
}

void CallStackDestroy() {
  if (funcs_called.ptr == NULL) {
    return;
  }
  free(funcs_called.ptr);
}

void PrintStackInfoAndExit(int signum) {
  static bool called_first_time = true;
  if (!called_first_time) {
    exit(199);
  }
  called_first_time = false;

  switch(signum) {
    case 11:
      printf ("caught segv :^(, error = 11\n");
      break;
    case 6:
      printf ("error = 6\n");
      break;
    default:
      printf("can not recognize signum\n");
      printf("signum: %d\n", signum);
  }

  size_t funcs_in_stack_num = funcs_called.maxcapacity;
  struct FuncLine* arr = FuncsCalledGetValues(&funcs_in_stack_num);
  if (arr == NULL) {
    printf("0 called functions in stack\n");
    exit(199);
  }

  printf ("%lu called functions in stack:\n", funcs_in_stack_num);
  for (size_t i = 0; i < funcs_in_stack_num; i++) {
    printf("%lu line: %lu, function name: %s\n", i + 1, arr[i].line_, arr[i].func_name_);
  }

  CallStackDestroy();
  exit(199);
}

void CallStackShrink() {
  if (funcs_called.ptr == NULL) {
    return;
  }
  funcs_called.ptr = (struct FuncLine*)realloc(funcs_called.ptr, funcs_called.capacity / funcs_called.incfac * sizeof(struct FuncLine));
  funcs_called.capacity /= funcs_called.incfac;
}

void CallStackPopback() {
  if (funcs_called.sz == 0 || funcs_called.ptr == NULL) {
    fprintf(stderr, "CallStackPopback from empty stack!!!\n");
    return;
  }
  funcs_called.ptr[--funcs_called.sz] = FuncLine{NULL, 0};
  if (funcs_called.sz > funcs_called.mincapacity &&
      funcs_called.sz <= funcs_called.capacity / (funcs_called.incfac * funcs_called.incfac)) {
    CallStackShrink();
  }
}