#include <memory>


template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

template <typename T>
class EnableSharedFromThis;



struct BaseControlBlock {
  size_t shared_count;
  size_t weak_count;


  BaseControlBlock() : shared_count(1), weak_count(0) {}

  virtual ~BaseControlBlock() = default;


  virtual void DeleteValue() = 0;

  virtual void Deallocate() = 0;


  void AddShared() { ++shared_count; }

  void AddWeak() { ++weak_count; }

  void DecreaseShared() {
    --shared_count;
    if (shared_count == 0) {
      DeleteValue();
      if (weak_count == 0) {
        Deallocate();
      }
    }
  }

  void DecreaseWeak() {
    --weak_count;
    if (shared_count == 0 && weak_count == 0) {
      Deallocate();
    }
  }
};



template <typename T>
class SharedPtr {
private:
  template <typename N, typename Deleter, typename Alloc>
  struct ControlBlock: BaseControlBlock {
    using Block_Alloc = typename std::allocator_traits<Alloc>:: template rebind_alloc<ControlBlock<N, Deleter, Alloc>>;
    using Block_Traits = typename std::allocator_traits<Alloc>:: template rebind_traits<ControlBlock<N, Deleter, Alloc>>;


    N* value;
    [[no_unique_address]] Deleter del;
    [[no_unique_address]] Alloc alloc;


    ControlBlock(N* val) : value(val) {}

    ControlBlock(N* val, Deleter deleter) : value(val) {
      try {
        del = deleter;
      } catch (...) {
        try {
          del = std::move(deleter);
        }
        catch (...) {
          throw;
        }
      }
    }

    ControlBlock(N* val, Deleter deleter, Alloc alloc) : value(val), alloc(alloc) {
      try {
        del = deleter;
      } catch (...) {
        try {
          del = std::move(deleter);
        }
        catch (...) {
          throw;
        }
      }
    }


    virtual ~ControlBlock() override = default;


    virtual void DeleteValue() override { del(value); }

    virtual void Deallocate() override {
      Block_Alloc block_alloc(alloc);
      Block_Traits::deallocate(block_alloc, this, 1);
    };
  };



  template <typename N, typename Alloc>
  struct ControlBlockMakeShared: BaseControlBlock {
    using Block_Alloc = typename std::allocator_traits<Alloc>:: template rebind_alloc<ControlBlockMakeShared<N, Alloc>>;
    using Block_Traits = typename std::allocator_traits<Alloc>:: template rebind_traits<ControlBlockMakeShared<N, Alloc>>;


    char value[sizeof(N)];
    Alloc alloc;


    template <typename... Args>
    ControlBlockMakeShared(Alloc alloc, Args&&... args) : alloc(alloc) {
      std::allocator_traits<Alloc>::construct(alloc, reinterpret_cast<N*>(value), std::forward<Args>(args)...);
    }


    virtual void DeleteValue() override { std::allocator_traits<Alloc>::destroy(alloc, reinterpret_cast<N*>(value)); }

    virtual void Deallocate() override {
      Block_Alloc block_alloc(alloc);
      Block_Traits::deallocate(block_alloc, this, 1);
    };
  };



  T* ptr_;
  BaseControlBlock* block_;


  SharedPtr(const WeakPtr<T>& weak);

public:
  SharedPtr();

  template <typename N>
  SharedPtr(N* ptr);

  template <typename N, typename Deleter>
  SharedPtr(N* ptr, Deleter del);

  template <typename N, typename Deleter, typename Alloc>
  SharedPtr(N* ptr, Deleter del, Alloc alloc);


  SharedPtr(const SharedPtr& other);

  template <typename N>
  SharedPtr(const SharedPtr<N>& other);

  SharedPtr(const SharedPtr&& other);

  template <typename N>
  SharedPtr(SharedPtr<N>&& other);


  template <typename N>
  SharedPtr(const SharedPtr<N>& other, T* ptr);

  template <typename N>
  SharedPtr(SharedPtr<N>&& other, T* ptr);

  SharedPtr& operator=(const SharedPtr& other);

  template <typename N>
  SharedPtr& operator=(const SharedPtr<N>& other);

  SharedPtr& operator=(const SharedPtr&& other);

  template <typename N>
  SharedPtr& operator=(SharedPtr<N>&& other);


  ~SharedPtr();



  T* get() const;

  T& operator*() const;

  T* operator->() const;


  long use_count() const;

  void reset();

  template <typename N>
  void reset(N* ptr);

  template <typename N, typename Deleter>
  void reset(N* ptr, Deleter del);

  template <typename N, typename Deleter, typename Alloc>
  void reset(N* ptr, Deleter del, Alloc alloc);

  void swap(SharedPtr& other);



  template <typename N, typename... Args>
  friend SharedPtr<N> makeShared(Args&&... args);

  template <typename N, typename Alloc, typename... Args>
  friend SharedPtr<N> allocateShared(const Alloc& alloc, Args&&... args);

  template <typename N>
  friend class SharedPtr;

  template <typename N>
  friend class WeakPtr;
};



template <typename T>
class WeakPtr {
private:
  T* ptr_;
  BaseControlBlock* block_;


public:
  WeakPtr();

  template <typename N>
  WeakPtr(const SharedPtr<N>& other);


  WeakPtr(const WeakPtr& other);

  template <typename N>
  WeakPtr(const WeakPtr<N>& other);

  WeakPtr(WeakPtr&& other);

  template <typename N>
  WeakPtr(WeakPtr<N>&& other);


  WeakPtr& operator=(const WeakPtr& other);

  template <typename N>
  WeakPtr& operator=(const WeakPtr<N>& other);

  WeakPtr& operator=(WeakPtr&& other);

  template <typename N>
  WeakPtr& operator=(WeakPtr<N>&& other);


  ~WeakPtr();



  void swap(WeakPtr& other);

  long use_count() const;

  bool expired() const;

  SharedPtr<T> lock() const;


  template <typename N>
  friend class SharedPtr;

  template <typename N>
  friend class WeakPtr;
};



template <typename T>
class EnableSharedFromThis {
  WeakPtr<T> sptr_;


public:
  SharedPtr<T> shared_from_this() const;


  template <typename N>
  friend class SharedPtr;
};



template <typename T>
SharedPtr<T>::SharedPtr() : ptr_(nullptr), block_(nullptr) {}

template <typename T>
template <typename N>
SharedPtr<T>::SharedPtr(N* ptr) : ptr_(ptr),  block_(new ControlBlock<N, std::default_delete<N>, std::allocator<N>>(ptr)) {
  if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
    ptr_->sptr_ = *this;
  }
}

template <typename T>
template <typename N, typename Deleter>
SharedPtr<T>::SharedPtr(N* ptr, Deleter del) : ptr_(ptr),  block_(new ControlBlock<N, Deleter, std::allocator<N>>(ptr, del)) {
  if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
    ptr_->sptr_ = *this;
  }
}

template <typename T>
template <typename N, typename Deleter, typename Alloc>
SharedPtr<T>::SharedPtr(N* ptr, Deleter del, Alloc alloc) : ptr_(ptr) {
  using Block_Alloc = typename std::allocator_traits<Alloc>:: template rebind_alloc<ControlBlock<N, Deleter, Alloc>>;
  using Block_Traits = typename std::allocator_traits<Alloc>:: template rebind_traits<ControlBlock<N, Deleter, Alloc>>;
  using Block = typename SharedPtr<T>:: template ControlBlock<N, Deleter, Alloc>;
  Block_Alloc block_alloc(alloc);
  Block* block(Block_Traits::allocate(block_alloc, 1));
  new (block) typename SharedPtr<T>:: template ControlBlock<N, Deleter, Alloc>(ptr, del, alloc);
  block_ = block;
  if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
    ptr_->sptr_ = *this;
  }
}


template <typename T>
SharedPtr<T>::SharedPtr(const SharedPtr& other) : ptr_(other.ptr_), block_(reinterpret_cast<BaseControlBlock*>(other.block_)) {
  if (block_ != nullptr) {
    block_->AddShared();
  }
  if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
    ptr_->sptr_ = *this;
  }
}

template <typename T>
template <typename N>
SharedPtr<T>::SharedPtr(const SharedPtr<N>& other) : ptr_(other.ptr_), block_(reinterpret_cast<BaseControlBlock*>(other.block_)) {
  if (block_ != nullptr) {
    block_->AddShared();
  }
  if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
    ptr_->sptr_ = *this;
  }
}

template <typename T>
SharedPtr<T>::SharedPtr(const SharedPtr&& other) : ptr_(other.ptr_), block_(reinterpret_cast<BaseControlBlock*>(other.block_)) {
  other.ptr_ = nullptr;
  other.block_ = nullptr;
  if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
    ptr_->sptr_ = *this;
  }
}

template <typename T>
template <typename N>
SharedPtr<T>::SharedPtr(SharedPtr<N>&& other) : ptr_(other.ptr_), block_(reinterpret_cast<BaseControlBlock*>(other.block_)) {
  other.ptr_ = nullptr;
  other.block_ = nullptr;
  if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
    ptr_->sptr_ = *this;
  }
}


template <typename T>
template <typename N>
SharedPtr<T>::SharedPtr(const SharedPtr<N>& other, T* ptr) : ptr_(ptr), block_(reinterpret_cast<BaseControlBlock*>(other.block_)) {
  if (block_ != nullptr) {
    block_->AddShared();
  }
  if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
    ptr_->sptr_ = *this;
  }
}

template <typename T>
template <typename N>
SharedPtr<T>::SharedPtr(SharedPtr<N>&& other, T* ptr) : ptr_(ptr), block_(reinterpret_cast<BaseControlBlock*>(other.block_)) {
  other.ptr_ = nullptr;
  other.block_ = nullptr;
  if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
    ptr_->sptr_ = *this;
  }
}


template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr& other) {
  SharedPtr<T> copy(other);
  swap(copy);
  return *this;
}

template <typename T>
template <typename N>
SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr<N>& other) {
  SharedPtr<T> copy(other);
  swap(copy);
  return *this;
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr&& other) {
  SharedPtr<T> copy(std::move(other));
  swap(copy);
  return *this;
}

template <typename T>
template <typename N>
SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr<N>&& other) {
  SharedPtr<T> copy(std::move(other));
  swap(copy);
  return *this;
}


template <typename T>
SharedPtr<T>::~SharedPtr() {
  if (block_ != nullptr) {
    block_->DecreaseShared();
  }
}


template <typename T>
T* SharedPtr<T>::get() const { return ptr_; }

template <typename T>
T& SharedPtr<T>::operator*() const { return *ptr_;}

template <typename T>
T* SharedPtr<T>::operator->() const { return ptr_; }


template <typename T>
long SharedPtr<T>::use_count() const {
  if (block_ == nullptr) {
    return 0;
  }
  return block_->shared_count;
}

template <typename T>
void SharedPtr<T>::reset() {
  *this = SharedPtr();
}

template <typename T>
template <typename N>
void SharedPtr<T>::reset(N* ptr) {
  *this = SharedPtr(ptr);
}

template <typename T>
template <typename N, typename Deleter>
void SharedPtr<T>::reset(N* ptr, Deleter del) {
  *this = SharedPtr(ptr, del);
}

template <typename T>
template <typename N, typename Deleter, typename Alloc>
void SharedPtr<T>::reset(N* ptr, Deleter del, Alloc alloc) {
  *this = SharedPtr(ptr, del, alloc);
}

template <typename T>
void SharedPtr<T>::swap(SharedPtr& other) {
  std::swap(block_, other.block_);
  std::swap(ptr_, other.ptr_);
}


template <typename T, typename Alloc, typename... Args>
SharedPtr<T> allocateShared(const Alloc& alloc, Args&&... args) {
  using Block_Alloc = typename std::allocator_traits<Alloc>:: template rebind_alloc<typename SharedPtr<T>:: template ControlBlockMakeShared<T, Alloc>>;
  using Block_Traits = typename std::allocator_traits<Alloc>:: template rebind_traits<typename SharedPtr<T>:: template ControlBlockMakeShared<T, Alloc>>;
  using Block = typename SharedPtr<T>:: template ControlBlockMakeShared<T, Alloc>;
  Block_Alloc block_alloc(alloc);
  Block* block(Block_Traits::allocate(block_alloc, 1));
  new (block) typename SharedPtr<T>:: template ControlBlockMakeShared<T, Alloc>(alloc, std::forward<Args>(args)...);
  SharedPtr<T> res;
  res.block_ = block;
  res.ptr_ = reinterpret_cast<T*>(block->value);
  return res;
}

template <typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args) {
  return SharedPtr<T>(allocateShared<T>(std::allocator<T>(), std::forward<Args>(args)...));
}


template <typename T>
SharedPtr<T>::SharedPtr(const WeakPtr<T>& weak) : ptr_(weak.ptr_), block_(reinterpret_cast<BaseControlBlock*>(weak.block_)) {
  if (block_ != nullptr) {
    block_->AddShared();
  }
}



template <typename T>
WeakPtr<T>::WeakPtr() : ptr_(nullptr), block_(nullptr) {}

template <typename T>
template <typename N>
WeakPtr<T>::WeakPtr(const SharedPtr<N>& other) : ptr_(other.ptr_), block_(reinterpret_cast<BaseControlBlock*>(other.block_)) {
  if (block_ != nullptr) {
    block_->AddWeak();
  }
}


template <typename T>
WeakPtr<T>::WeakPtr(const WeakPtr& other) : ptr_(other.ptr_), block_(reinterpret_cast<BaseControlBlock*>(other.block_)) {
  if (block_ != nullptr) {
    block_->AddWeak();
  }
}

template <typename T>
template <typename N>
WeakPtr<T>::WeakPtr(const WeakPtr<N>& other) : ptr_(other.ptr_), block_(reinterpret_cast<BaseControlBlock*>(other.block_)) {
  if (block_ != nullptr) {
    block_->AddWeak();
  }
}

template <typename T>
WeakPtr<T>::WeakPtr(WeakPtr&& other) : ptr_(other.ptr_), block_(reinterpret_cast<BaseControlBlock*>(other.block_)) {
  other.ptr_ = nullptr;
  other.block_ = nullptr;
}

template <typename T>
template <typename N>
WeakPtr<T>::WeakPtr(WeakPtr<N>&& other) : ptr_(other.ptr_), block_(reinterpret_cast<BaseControlBlock*>(other.block_)) {
  other.ptr_ = nullptr;
  other.block_ = nullptr;
}


template <typename T>
WeakPtr<T>& WeakPtr<T>::operator=(const WeakPtr& other) {
  WeakPtr<T> copy(other);
  swap(copy);
  return *this;
}

template <typename T>
template <typename N>
WeakPtr<T>& WeakPtr<T>::operator=(const WeakPtr<N>& other) {
  WeakPtr<T> copy(other);
  swap(copy);
  return *this;
}

template <typename T>
WeakPtr<T>& WeakPtr<T>::operator=(WeakPtr&& other) {
  WeakPtr<T> copy(std::move(other));
  swap(copy);
  return *this;
}

template <typename T>
template <typename N>
WeakPtr<T>& WeakPtr<T>::operator=(WeakPtr<N>&& other) {
  WeakPtr<T> copy(std::move(other));
  swap(copy);
  return *this;
}


template <typename T>
WeakPtr<T>::~WeakPtr() {
  if (block_ != nullptr) {
    block_->DecreaseWeak();
  }
}


template <typename T>
void WeakPtr<T>::swap(WeakPtr& other) { std::swap(block_, other.block_); }

template <typename T>
long WeakPtr<T>::use_count() const { return block_->shared_count; }

template <typename T>
bool WeakPtr<T>::expired() const { return (block_->shared_count == 0); }

template <typename T>
SharedPtr<T> WeakPtr<T>::lock() const {
  if (expired()) {
    return SharedPtr<T>();
  }
  return SharedPtr<T>(*this);
}


template <typename T>
SharedPtr<T> EnableSharedFromThis<T>::shared_from_this() const {
  if (sptr_.expired()) {
    return std::bad_weak_ptr();
  }
  return sptr_.lock();
}
