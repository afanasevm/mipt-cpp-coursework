#include <vector>
#include <stdexcept>



template <typename T>
class Deque {
private:
  static const size_t size_of_cluster = 16;

  std::vector<T*> arr_;
  size_t index_start_ = 0;
  size_t index_end_ = 0;



public:
  Deque();

  Deque(const Deque&);

  Deque(size_t);

  Deque(size_t, const T&);

  ~Deque();

  Deque& operator=(Deque);



  size_t size() const;

  T& operator[](size_t);

  const T& operator[](size_t) const;

  T& at(size_t);

  const T& at(size_t) const;



  void push_back(const T&);

  void pop_back();

  void push_front(const T&);

  void pop_front();





  template<bool is_const>
  class common_iterator {
  public:
    using value_type = T;
    using difference_type = int;
    using reference = std::conditional_t<is_const, const T&, T&>;
    using pointer = std::conditional_t<is_const, const T*, T*>;
    using iterator_category = std::random_access_iterator_tag;



  private:
    using container = std::conditional_t<is_const, T*const*, T**>;
    container ptr_;
    mutable pointer current_;
    int index_;
    mutable bool is_current_;



  public:
    common_iterator() = default;

    common_iterator(const common_iterator &other) :  ptr_(other.ptr_), current_(other.current_), index_(other.index_), is_current_(other.is_current_) {};

    common_iterator(container ptr, int index) : ptr_(ptr), index_(index), is_current_(false) {};

    common_iterator& operator=(const common_iterator& other) = default;

    operator common_iterator<true>() {
      return common_iterator<true>(ptr_, index_);
    }



    common_iterator& operator++();

    common_iterator operator++(int);

    common_iterator& operator--();

    common_iterator operator--(int);

    common_iterator operator+(int) const;

    common_iterator operator-(int) const;

    common_iterator& operator+=(int);

    common_iterator& operator-=(int);



    bool operator<(const common_iterator&) const;

    bool operator>(const common_iterator&) const;

    bool operator<=(const common_iterator&) const;

    bool operator>=(const common_iterator&) const;

    bool operator==(const common_iterator&) const;

    bool operator!=(const common_iterator&) const;



    int operator-(const common_iterator&) const;

    reference operator*() const;

    pointer operator->() const;
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



  template<bool is_const>
  common_iterator<is_const> insert(common_iterator<is_const>, const T&);

  template<bool is_const>
  common_iterator<is_const> erase(common_iterator<is_const>);

  void swap(Deque&);
};





template <typename T>
Deque<T>::Deque() {
  arr_ = std::vector<T*>(1);
  arr_[0] = reinterpret_cast<T*>(new char[size_of_cluster * sizeof(T)]);
}

template <typename T>
Deque<T>::Deque(const Deque<T>& other) : index_start_(other.index_start_), index_end_(other.index_end_) {
  size_t size_of_vector = other.arr_.size();
  arr_ = std::vector<T*>(size_of_vector);
  size_t i = 0;
  try {
    for (; i < size_of_vector; ++i) {
      arr_[i] = reinterpret_cast<T*>(new char[size_of_cluster * sizeof(T)]);
    }
  } catch (...) {
    for (size_t j = 0; j < i; ++j) {
      delete reinterpret_cast<char*>(arr_[j]);
    }
    throw;
  }
  i = index_start_;
  try {
    for (; i < index_end_; ++i) {
      new (arr_[i / size_of_cluster] + i % size_of_cluster) T(other.arr_[i / size_of_cluster][i % size_of_cluster]);
    }
  } catch (...) {
    for (size_t j = index_start_; j < i; ++j) {
      (arr_[j / size_of_cluster] + j % size_of_cluster)->~T();
    }
    for (size_t j = 0; j < size_of_vector; ++j) {
      delete reinterpret_cast<char*>(arr_[j]);
    }
    throw;
  }
}

template <typename T>
Deque<T>::Deque(size_t size) : Deque(size, T()) {}

template <typename T>
Deque<T>::Deque(size_t size, const T& value) {
  size_t size_of_vector = (size + size_of_cluster - 1) / size_of_cluster;
  arr_ = std::vector<T*>(size_of_vector);
  size_t i = 0;
  try {
    for (; i < size_of_vector; ++i) {
      arr_[i] = reinterpret_cast<T*>(new char[size_of_cluster * sizeof(T)]);
    }
  } catch (...) {
    for (size_t j = 0; j < i; ++j) {
      delete reinterpret_cast<char*>(arr_[j]);
    }
    throw;
  }
  i = 0;
  try {
    for (; i < size; ++i) {
      new (arr_[i / size_of_cluster] + i % size_of_cluster) T(value);
    }
    index_end_ = size;
  } catch (...) {
    for (size_t j = 0; j < i; ++j) {
      (arr_[j / size_of_cluster] + j % size_of_cluster)->~T();
    }
    for (size_t j = 0; j < size_of_vector; ++j) {
      delete reinterpret_cast<char*>(arr_[j]);
    }
    throw;
  }
}

template <typename T>
Deque<T>::~Deque<T>() {
  for (size_t i = index_start_; i < index_end_; ++i) {
    (arr_[i / size_of_cluster] + i % size_of_cluster)->~T();
  }
  for (size_t i = 0; i < arr_.size(); ++i) {
    delete reinterpret_cast<char*>(arr_[i]);
  }
}

template <typename T>
Deque<T>& Deque<T>::operator=(Deque<T> other) {
  swap(other);
  return *this;
}



template <typename T>
size_t Deque<T>::size() const {
  return index_end_ - index_start_;
}

template <typename T>
T& Deque<T>::operator[](size_t pos) {
  return arr_[(pos + index_start_) / size_of_cluster][(pos + index_start_) % size_of_cluster];
}

template <typename T>
const T& Deque<T>::operator[](size_t pos) const {
  return arr_[(pos + index_start_) / size_of_cluster][(pos + index_start_) % size_of_cluster];
}

template <typename T>
T& Deque<T>::at(size_t pos) {
  if (pos >= size()) {
    throw std::out_of_range("");
  }
  return arr_[(pos + index_start_) / size_of_cluster][(pos + index_start_) % size_of_cluster];
}

template <typename T>
const T& Deque<T>::at(size_t pos) const {
  if (pos >= size()) {
    throw std::out_of_range("");
  }
  return arr_[(pos + index_start_) / size_of_cluster][(pos + index_start_) % size_of_cluster];
}



template <typename T>
void Deque<T>::push_back(const T& value) {
  if (index_end_ == arr_.size() * size_of_cluster) {
    arr_.push_back(reinterpret_cast<T*>(new char[size_of_cluster * sizeof(T)]));
  }
  new (arr_[index_end_ / size_of_cluster] + index_end_ % size_of_cluster) T(value);
  ++index_end_;
}

template <typename T>
void Deque<T>::pop_back() {
  --index_end_;
  (arr_[index_end_ / size_of_cluster] + index_end_ % size_of_cluster)->~T();
}

template <typename T>
void Deque<T>::push_front(const T& value) {
  if (index_start_ == 0) {
    arr_.insert(arr_.begin(), reinterpret_cast<T*>(new char[size_of_cluster * sizeof(T)]));
    index_start_ += size_of_cluster;
    index_end_ += size_of_cluster;
  }
  index_start_ -= 1;
  new (arr_[index_start_ / size_of_cluster] + index_start_ % size_of_cluster) T(value);
}

template <typename T>
void Deque<T>::pop_front() {
  (arr_[index_start_ / size_of_cluster] + index_start_ % size_of_cluster)->~T();
  ++index_start_;
}





template <typename T>
template <bool is_const>
typename Deque<T>::template common_iterator<is_const>& Deque<T>::common_iterator<is_const>::operator++() {
  *this += 1;
  return *this;
}

template <typename T>
template <bool is_const>
typename Deque<T>::template common_iterator<is_const> Deque<T>::common_iterator<is_const>::operator++(int) {
  common_iterator other(*this);
  *this += 1;
  return other;
}

template <typename T>
template <bool is_const>
typename Deque<T>::template common_iterator<is_const>& Deque<T>::common_iterator<is_const>::operator--() {
  *this -= 1;
  return *this;
}

template <typename T>
template <bool is_const>
typename Deque<T>::template common_iterator<is_const> Deque<T>::common_iterator<is_const>::operator--(int) {
  common_iterator other(*this);
  *this -= 1;
  return other;
}

template <typename T>
template <bool is_const>
typename Deque<T>::template common_iterator<is_const> Deque<T>::common_iterator<is_const>::operator+(int count) const {
  common_iterator<is_const> other(*this);
  other += count;
  return other;
}

template <typename T>
template <bool is_const>
typename Deque<T>::template common_iterator<is_const> Deque<T>::common_iterator<is_const>::operator-(int count) const {
  common_iterator<is_const> other(*this);
  other -= count;
  return other;
}

template <typename T>
template <bool is_const>
typename Deque<T>::template common_iterator<is_const>& Deque<T>::common_iterator<is_const>::operator+=(int count) {
  index_ += count;
  if (index_ >= static_cast<int>(size_of_cluster)) {
    is_current_ = false;
    ptr_ += index_ / size_of_cluster;
    index_ %= size_of_cluster;
  }
  return *this;
}

template <typename T>
template <bool is_const>
typename Deque<T>::template common_iterator<is_const>& Deque<T>::common_iterator<is_const>::operator-=(int count) {
  index_ -= count;
  if (index_ < 0) {
    is_current_ = false;
    ptr_ -= (-index_ + size_of_cluster - 1) / size_of_cluster;
    index_ += (-index_ + size_of_cluster - 1) / size_of_cluster * size_of_cluster;
  }
  return *this;
}



template <typename T>
template <bool is_const>
bool Deque<T>::common_iterator<is_const>::operator<(const common_iterator& other) const{
  return (ptr_ < other.ptr_) || (ptr_ == other.ptr_ && index_ < other.index_);
}

template <typename T>
template <bool is_const>
bool Deque<T>::common_iterator<is_const>::operator>(const common_iterator& other) const{
  return other < *this;
}

template <typename T>
template <bool is_const>
bool Deque<T>::common_iterator<is_const>::operator<=(const common_iterator& other) const{
  return !(*this > other);
}

template <typename T>
template <bool is_const>
bool Deque<T>::common_iterator<is_const>::operator>=(const common_iterator& other) const{
  return !(*this < other);
}

template <typename T>
template <bool is_const>
bool Deque<T>::common_iterator<is_const>::operator==(const common_iterator& other) const{
  return (ptr_ == other.ptr_ && index_ == other.index_);
}

template <typename T>
template <bool is_const>
bool Deque<T>::common_iterator<is_const>::operator!=(const common_iterator& other) const{
  return !(*this == other);
}



template <typename T>
template <bool is_const>
int Deque<T>::common_iterator<is_const>::operator-(const common_iterator& other) const{
  return (ptr_ - other.ptr_) * size_of_cluster + index_ - other.index_;
}

template <typename T>
template <bool is_const>
typename Deque<T>:: template common_iterator<is_const>::reference Deque<T>::common_iterator<is_const>::operator*() const {
  if (!is_current_) {
    is_current_ = true;
    current_ = *ptr_;
  }
  return *(current_ + index_);
}

template <typename T>
template <bool is_const>
typename Deque<T>:: template common_iterator<is_const>::pointer Deque<T>::common_iterator<is_const>::operator->() const {
  if (!is_current_) {
    is_current_ = true;
    current_ = *ptr_;
  }
  return (current_ + index_);
}





template <typename T>
typename Deque<T>::iterator Deque<T>::begin() {
  return Deque<T>::iterator(&arr_[index_start_ / size_of_cluster], index_start_ % size_of_cluster);
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::begin() const{
  return Deque<T>::const_iterator(&arr_[index_start_ / size_of_cluster], index_start_ % size_of_cluster);
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::cbegin() const {
  return Deque<T>::const_iterator(&arr_[index_start_ / size_of_cluster], index_start_ % size_of_cluster);
}

template <typename T>
typename Deque<T>::iterator Deque<T>::end() {
  return Deque<T>::iterator(&arr_[index_end_ / size_of_cluster], index_end_ % size_of_cluster);
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::end() const {
  return Deque<T>::const_iterator(&arr_[index_end_ / size_of_cluster], index_end_ % size_of_cluster);
}

template <typename T>
typename Deque<T>::const_iterator Deque<T>::cend() const {
  return Deque<T>::const_iterator(&arr_[index_end_ / size_of_cluster], index_end_ % size_of_cluster);
}

template <typename T>
typename Deque<T>::reverse_iterator Deque<T>::rbegin() {
  return std::make_reverse_iterator(end());
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::rbegin() const{
  return std::make_reverse_iterator(end());
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::crbegin() const {
  return std::make_reverse_iterator(cend());
}

template <typename T>
typename Deque<T>::reverse_iterator Deque<T>::rend() {
  return std::make_reverse_iterator(begin());
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::rend() const {
  return std::make_reverse_iterator(begin());
}

template <typename T>
typename Deque<T>::const_reverse_iterator Deque<T>::crend() const {
  return std::make_reverse_iterator(cbegin());
}



template <typename T>
template <bool is_const>
typename Deque<T>:: template common_iterator<is_const> Deque<T>::insert(Deque<T>::common_iterator<is_const> iter, const T& value) {
  if (iter == end()) {
    push_back(value);
    return end();
  }
  int shift = iter - begin();
  T val = T(*(end() - 1));
  for (common_iterator<is_const> temp(end() - 1); temp > iter; --temp) {
    temp->~T();
    new (&*temp) T(*(temp - 1));
  }
  iter->~T();
  new (&*iter) T(value);
  push_back(val);
  return begin() + shift;
}

template <typename T>
template <bool is_const>
typename Deque<T>:: template common_iterator<is_const> Deque<T>::erase(Deque<T>::common_iterator<is_const> iter) {
  for (common_iterator<is_const> temp(iter); temp < end() - 1; ++temp) {
    temp->~T();
    new (&*temp) T(*(temp + 1));
  }
  pop_back();
  return iter;
}

template <typename T>
void Deque<T>::swap(Deque& other) {
  std::swap(arr_, other.arr_);
  std::swap(index_start_, other.index_start_);
  std::swap(index_end_, other.index_end_);
}
