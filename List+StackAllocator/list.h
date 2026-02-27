#include <memory>

template <size_t N>
class StackStorage {
  char buffer_[N];
  char* ptr_;

public:
  StackStorage() : ptr_(buffer_) {}

  StackStorage(StackStorage&) = delete;

  StackStorage& operator=(StackStorage&) = delete;

  template <typename T, size_t M>
  friend class StackAllocator;
};

template <typename T, size_t N>
class StackAllocator {
  StackStorage<N>* storage_;

public:
  using value_type = T;

  template<typename U>
  StackAllocator(const StackAllocator<U, N>& other) : storage_(other.storage_) {}

  StackAllocator(StackStorage<N>& storage) : storage_(&storage) {}

  T* allocate(size_t size) {
    void* p = reinterpret_cast<void*>(storage_->ptr_);
    size_t space = N - (storage_->ptr_ - storage_->buffer_);
    std::align(alignof(T), sizeof(T) * size, p, space);
    storage_->ptr_ = reinterpret_cast<char*>(p) + size * sizeof(T);
    return reinterpret_cast<T*>(p);
  }

  void deallocate(const T*, size_t) {}

  template <typename U>
  struct rebind {
    using other = StackAllocator<U, N>;
  };

  template <typename S, size_t M>
  bool operator==(StackAllocator<S, M>& other) {
    return storage_ == other.storage_;
  }

  template <typename S, size_t M>
  bool operator!=(StackAllocator<S, M>& other) {
    return !(*this == other);
  }

  StackAllocator select_on_container_copy_construction() { return *this; }

  using propagate_on_container_copy_assignment = std::true_type;

  template <typename U, size_t M>
  friend class StackAllocator;
};

template <typename T, typename Alloc = std::allocator<T>>
class List {

  struct BaseNode {
    BaseNode* prev = nullptr;
    BaseNode* next = nullptr;
  };

  struct Node: BaseNode {
    T value = T();

    Node() {}

    Node(const T& value) : value(value) {}

    Node(T&& value) : value(std::move(value)) {}
  };

  using NodeAlloc = typename std::allocator_traits<Alloc>:: template rebind_alloc<Node>;
  using NodeTraits = std::allocator_traits<NodeAlloc>;

  [[no_unique_address]] NodeAlloc alloc_;
  BaseNode fake_node_ = {&fake_node_, &fake_node_};
  size_t sz_ = 0;

public:
  List();

  List(size_t);

  List(size_t, const T&);

  List(const Alloc&);

  List(size_t, const Alloc&);

  List(size_t, const T&, const Alloc&);

  List(const List&);

  List(List&&);

  List& operator=(const List&);

  List& operator=(List&&);

  ~List();

  Alloc get_allocator() const;

  size_t size() const;

  void push_back(const T&);

  void push_back(T&&);

  void push_front(const T&);

  void push_front(T&&);

  void pop_back();

  void pop_front();

  template <bool is_const>
  class common_iterator {
  public:
    using value_type = T;
    using difference_type = int;
    using reference = std::conditional_t<is_const, const T&, T&>;
    using pointer = std::conditional_t<is_const, const T*, T*>;
    using iterator_category = std::bidirectional_iterator_tag;

  private:
    using basenode_pointer = std::conditional_t<is_const, const BaseNode*, BaseNode*>;
    using node_pointer = std::conditional_t<is_const, const Node*, Node*>;

    basenode_pointer ptr_;

  public:
    common_iterator() = default;

    common_iterator(const common_iterator &other) : ptr_(other.ptr_) {}

    common_iterator(basenode_pointer ptr) : ptr_(ptr) {}

    common_iterator& operator=(const common_iterator& other) = default;

    operator common_iterator<true>() const {
      return common_iterator<true>(ptr_);
    }

    common_iterator& operator++();

    common_iterator operator++(int);

    common_iterator& operator--();

    common_iterator operator--(int);

    bool operator==(const common_iterator&) const;

    bool operator!=(const common_iterator&) const;

    reference operator*() const;

    pointer operator->() const;

    friend class List;
  };

  using iterator = common_iterator<false>;
  using const_iterator = common_iterator<true>;

  iterator begin();

  const_iterator begin() const;

  const_iterator cbegin() const;

  iterator end();

  const_iterator end() const;

  const_iterator cend() const;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  reverse_iterator rbegin();

  const_reverse_iterator rbegin() const;

  const_reverse_iterator crbegin() const;

  reverse_iterator rend();

  const_reverse_iterator rend() const;

  const_reverse_iterator crend() const;

  iterator insert(const_iterator, const T&);

  iterator insert(const_iterator, T&&);

  iterator erase(const_iterator);

private:
  template <typename... Args>
  iterator emplace(const_iterator, Args&&...);

  void swap(List&);

  void swap(List&&);
};

template <typename T, typename Alloc>
void List<T, Alloc>::swap(List& other) {
  std::swap(fake_node_.next->prev, other.fake_node_.next->prev);
  std::swap(fake_node_.prev->next, other.fake_node_.prev->next);
  std::swap(fake_node_, other.fake_node_);
  std::swap(sz_, other.sz_);
}

template <typename T, typename Alloc>
void List<T, Alloc>::swap(List&& other) {
  std::swap(fake_node_.next->prev, other.fake_node_.next->prev);
  std::swap(fake_node_.prev->next, other.fake_node_.prev->next);
  std::swap(fake_node_, other.fake_node_);
  std::swap(sz_, other.sz_);
}

template <typename T, typename Alloc>
List<T, Alloc>::List() : List(Alloc()) {}

template <typename T, typename Alloc>
List<T, Alloc>::List(size_t sz) : List(sz, Alloc()) {}

template <typename T, typename Alloc>
List<T, Alloc>::List(size_t sz, const T& value) : List(sz, value, Alloc()) {}

template <typename T, typename Alloc>
List<T, Alloc>::List(const Alloc& alloc) : alloc_(alloc) {}

template <typename T, typename Alloc>
List<T, Alloc>::List(size_t sz, const Alloc& alloc) : alloc_(alloc) {
  try {
    for (size_t i = 0 ; i < sz; i++) {
      emplace(end());
    }
  } catch (...) {
    while (sz_ != 0) {
      pop_back();
    }
    throw;
  }
}

template <typename T, typename Alloc>
List<T, Alloc>::List(size_t sz, const T& value, const Alloc& alloc ) : alloc_(alloc) {
  try {
    for (size_t i = 0; i < sz; i++) {
      insert(end(), value);
    }
  } catch (...) {
    while (sz_ != 0) {
      pop_back();
    }
    throw;
  }
}

template <typename T, typename Alloc>
List<T, Alloc>::List(const List& other) : alloc_(std::allocator_traits<Alloc>::select_on_container_copy_construction(other.get_allocator())) {
  try {
    for (const T& value : other) {
      insert(end(), value);
    }
  } catch (...) {
    while (sz_ != 0) {
      pop_back();
    }
    throw;
  }
}

template <typename T, typename Alloc>
List<T, Alloc>::List(List&& other) : alloc_(std::allocator_traits<Alloc>::select_on_container_copy_construction(other.get_allocator())){
  swap(std::move(other));
}

template <typename T, typename Alloc>
List<T, Alloc>& List<T, Alloc>::operator=(const List& other) {
  if (std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value) {
    alloc_ = other.alloc_;
  }

  List temp = other;
  swap(temp);
  return *this;
}

template <typename T, typename Alloc>
List<T, Alloc>& List<T, Alloc>::operator=(List&& other) {
  while (sz_ != 0) {
    pop_back();
  }
  if (std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value) {
    alloc_ = other.alloc_;
  }
  swap(std::move(other));
}

template <typename T, typename Alloc>
List<T, Alloc>::~List() {
  while (sz_ != 0) {
    pop_back();
  }
}

template <typename T, typename Alloc>
Alloc List<T, Alloc>::get_allocator() const { return typename NodeTraits:: template rebind_alloc<T>(alloc_); }

template <typename T, typename Alloc>
size_t List<T, Alloc>::size() const { return sz_; }

template <typename T, typename Alloc>
void List<T, Alloc>::push_back(const T& value) { insert(end(), value); }

template <typename T, typename Alloc>
void List<T, Alloc>::push_back(T&& value) { insert(end(), std::move(value)); }

template <typename T, typename Alloc>
void List<T, Alloc>::push_front(const T& value) { insert(begin(), value); }

template <typename T, typename Alloc>
void List<T, Alloc>::push_front(T&& value) { insert(begin(), std::move(value)); }

template <typename T, typename Alloc>
void List<T, Alloc>::pop_back() { erase(--end()); }

template <typename T, typename Alloc>
void List<T, Alloc>::pop_front() { erase(begin()); }

template <typename T, typename Alloc>
template <bool is_const>
typename List<T, Alloc>:: template common_iterator<is_const>& List<T, Alloc>::common_iterator<is_const>::operator++() {
  ptr_ = ptr_->next;
  return *this;
}

template <typename T, typename Alloc>
template <bool is_const>
typename List<T, Alloc>:: template common_iterator<is_const> List<T, Alloc>::common_iterator<is_const>::operator++(int) {
  common_iterator<is_const> temp = *this;
  ptr_ = ptr_->next;
  return temp;
}

template <typename T, typename Alloc>
template <bool is_const>
typename List<T, Alloc>:: template common_iterator<is_const>& List<T, Alloc>::common_iterator<is_const>::operator--() {
  ptr_ = ptr_->prev;
  return *this;
}

template <typename T, typename Alloc>
template <bool is_const>
typename List<T, Alloc>:: template common_iterator<is_const> List<T, Alloc>::common_iterator<is_const>::operator--(int) {
  common_iterator<is_const> temp = *this;
  ptr_ = ptr_->prev;
  return temp;
}

template <typename T, typename Alloc>
template <bool is_const>
bool List<T, Alloc>::common_iterator<is_const>::operator==(const common_iterator& other) const {
  return ptr_ == other.ptr_;
}

template <typename T, typename Alloc>
template <bool is_const>
bool List<T, Alloc>::common_iterator<is_const>::operator!=(const common_iterator& other) const {
  return !(*this == other);
}

template <typename T, typename Alloc>
template <bool is_const>
typename List<T, Alloc>:: template common_iterator<is_const>::reference List<T, Alloc>::common_iterator<is_const>::operator*() const {
  return static_cast<node_pointer>(ptr_)->value;
}

template <typename T, typename Alloc>
template <bool is_const>
typename List<T, Alloc>:: template common_iterator<is_const>::pointer List<T, Alloc>::common_iterator<is_const>::operator->() const {
  return &(static_cast<node_pointer>(ptr_)->value);
}

template <typename T, typename Alloc>
typename List<T, Alloc>::iterator List<T, Alloc>::begin() {
  return iterator(static_cast<typename iterator::basenode_pointer>(fake_node_.next));
}

template <typename T, typename Alloc>
typename List<T, Alloc>::const_iterator List<T, Alloc>::begin() const {
  return const_iterator(static_cast<typename const_iterator::basenode_pointer>(fake_node_.next));
}

template <typename T, typename Alloc>
typename List<T, Alloc>::const_iterator List<T, Alloc>::cbegin() const {
  return const_iterator(static_cast<typename const_iterator::basenode_pointer>(fake_node_.next));
}

template <typename T, typename Alloc>
typename List<T, Alloc>::iterator List<T, Alloc>::end() {
  return iterator(static_cast<typename iterator::basenode_pointer>(&fake_node_));
}

template <typename T, typename Alloc>
typename List<T, Alloc>::const_iterator List<T, Alloc>::end() const {
  return const_iterator(static_cast<typename const_iterator::basenode_pointer>(&fake_node_));
}

template <typename T, typename Alloc>
typename List<T, Alloc>::const_iterator List<T, Alloc>::cend() const {
  return const_iterator(static_cast<typename const_iterator::basenode_pointer>(&fake_node_));
}

template <typename T, typename Alloc>
typename List<T, Alloc>::reverse_iterator List<T, Alloc>::rbegin() {
  return std::make_reverse_iterator(end());
}

template <typename T, typename Alloc>
typename List<T, Alloc>::const_reverse_iterator List<T, Alloc>::rbegin() const {
  return std::make_reverse_iterator(end());
}

template <typename T, typename Alloc>
typename List<T, Alloc>::const_reverse_iterator List<T, Alloc>::crbegin() const {
  return std::make_reverse_iterator(cend());
}

template <typename T, typename Alloc>
typename List<T, Alloc>::reverse_iterator List<T, Alloc>::rend() {
  return std::make_reverse_iterator(begin());
}

template <typename T, typename Alloc>
typename List<T, Alloc>::const_reverse_iterator List<T, Alloc>::rend() const {
  return std::make_reverse_iterator(begin());
}

template <typename T, typename Alloc>
typename List<T, Alloc>::const_reverse_iterator List<T, Alloc>::crend() const {
  return std::make_reverse_iterator(cbegin());
}

template <typename T, typename Alloc>
typename List<T, Alloc>::iterator List<T, Alloc>::insert(const_iterator iter, const T& value) {
  return emplace(iter, value);
}

template <typename T, typename Alloc>
typename List<T, Alloc>::iterator List<T, Alloc>::insert(const_iterator iter, T&& value) {
  return emplace(iter, std::move(value));
}

template <typename T, typename Alloc>
typename List<T, Alloc>::iterator List<T, Alloc>::erase(const_iterator iter) {
  BaseNode* next = iter.ptr_->next;
  BaseNode* prev = iter.ptr_->prev;
  Node* node = static_cast<Node*>(const_cast<BaseNode*>(iter.ptr_));
  NodeTraits::destroy(alloc_, node);
  NodeTraits::deallocate(alloc_, node, 1);
  prev->next = next;
  next->prev = prev;
  sz_--;
  return iterator(next);
}

template <typename T, typename Alloc>
template <typename... Args>
typename List<T, Alloc>::iterator List<T, Alloc>::emplace(const_iterator iter, Args&&... args) {
  BaseNode* node = const_cast<BaseNode*>(iter.ptr_);
  BaseNode* prev = node->prev;
  Node* new_node = NodeTraits::allocate(alloc_, 1);
  NodeTraits::construct(alloc_, new_node, std::forward<Args>(args)...);
  prev->next = static_cast<BaseNode*>(new_node);
  new_node->prev = prev;
  new_node->next = node;
  node->prev = static_cast<BaseNode*>(new_node);
  sz_++;
  return iterator(static_cast<BaseNode*>(new_node));
}
