#include "ActionFunctors.h"

class FunctorQueue {
 public:
  static FunctorQueue& GetInstance() {
  	static FunctorQueue instance;
    return instance;
  }

  void Push(Functor::Abstract* func) {
    queue_.push(func);
  }

  Functor::Abstract* Pop() {
    Functor::Abstract* top = queue_.front();
    queue_.pop();
    return top;
  }

  bool IsEmpty() {
    return queue_.empty();
  }

  ~FunctorQueue() = default;

 private:
 	std::queue<Functor::Abstract*> queue_;

  FunctorQueue() = default;

  FunctorQueue(const FunctorQueue&) = delete;
  FunctorQueue& operator=(const FunctorQueue&) = delete;
  FunctorQueue(FunctorQueue&&) = delete;
  FunctorQueue& operator=(FunctorQueue&&) = delete;
};