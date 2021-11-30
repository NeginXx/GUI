#pragma once
#include <initializer_list>

template <typename T>
class List {
 private:
  struct Node;

 public:
  List() {
    tail_ = new Node;
    head_ = tail_;
    size_ = 0;
  }

  ~List() {
    Node* cur_node = tail_;
    while (cur_node != nullptr) {
      Node* prev_node = cur_node->prev;
      delete cur_node;
      cur_node = prev_node;
    }
  }

  List(const std::initializer_list<T>& init_list) : List() {
    if (init_list.begin() != init_list.end()) {
      auto it = init_list.end();
      do { this->PushFront(*--it); ++size_; } while (it != init_list.begin());
    }
  }

  List(const List<T>& list) = delete;

  size_t GetSize() const {
    return size_;
  }

  void PushFront(const T& val) {
    Node* new_node = new Node;
    new_node->next = head_;
    new_node->val = val;
    new_node->prev = nullptr;
    head_->prev = new_node;
    head_ = new_node;
    ++size_;
  }

  void PopBack() {
    Node* node = tail_->prev;
    tail_->prev = node->prev;
    if (node->prev != nullptr) {
      node->prev->next = tail_;
    } else {
      head_ = tail_;
    }
    delete node;
    --size_;
  }

  class Iterator {
   public:
    Iterator() {
      node_ptr_ = nullptr;
    }
    
    Iterator(Node* node_ptr) {
      node_ptr_ = node_ptr;
    }

    Node* GetNode() const {
      return node_ptr_;
    }

    Iterator& operator++() {
      if (node_ptr_ != nullptr) {
        node_ptr_ = node_ptr_->next;
      }
      return *this;
    }

    Iterator& operator--() {
      if (node_ptr_ != nullptr) {
        node_ptr_ = node_ptr_->prev;
      }
      return *this;
    }

    Iterator operator++(int n) {
      Iterator copy = *this;
      ++*this;
      return copy;
    }

    Iterator Next() const {
      Iterator it(this->node_ptr_->next);
      return it;
    }

    Iterator Prev() const {
      Iterator it(this->node_ptr_->prev);
      return it;
    }

    T& operator*() {
      return node_ptr_->val;
    }

    bool operator==(const Iterator& it) const {
      return this->node_ptr_ == it.node_ptr_;
    }

    bool operator!=(const Iterator& it) const {
      return !(this->node_ptr_ == it.node_ptr_);
    }

    friend void List<T>::Pop(Iterator& it);

   private:
    Node* node_ptr_ = nullptr;
  };

  Iterator begin() const {
    return Iterator(head_);
  }

  Iterator end() const {
    return Iterator(tail_);
  }

  Iterator Find(const T& val) {
    Iterator it = begin();
    for (; it != end(); ++it) {
      if (*it == val) {
        break;
      }
    }
    return it;
  }

  void Pop(Iterator& it) {
    Node* node = it.node_ptr_;
    assert(node->next != nullptr && "Pop of list.end()!");
    node->next->prev = node->prev;
    if (node->prev != nullptr) {
      node->prev->next = node->next;
    } else {
      head_ = node->next;
    }
    delete node;
    it.node_ptr_ = nullptr;
    --size_;
  }

  void Dump() const {
    printf("List dump:\n");
    printf("---------------------\n");
    printf("size = %lu\n", size_);
    Node* cur_node = head_;
    while (cur_node != nullptr) {
      printf("prev = %p\n", cur_node->prev);
      printf("node = %p\n", cur_node);
      printf("val = %p\n", cur_node->val);
      printf("next = %p\n", cur_node->next);
      cur_node = cur_node->next;
      if (cur_node != nullptr) {
        printf("\n");
      }
    }
    printf("---------------------\n");
  }

 private:
  Node* head_;
  Node* tail_;
  size_t size_;

  struct Node {
    Node* next = nullptr;
    Node* prev = nullptr;
    T val;
  };
};