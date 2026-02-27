#include <iostream>
#include <cstring>

class String {
 private:
  char* array = nullptr;
  size_t size_of_string = 0;
  size_t size_of_memory = 0;

  String(size_t length): array(new char[length + 1]), size_of_string(length), size_of_memory(length + 1) {
    array[size_of_string] = 0;
  }

 public:
  String(): array(new char[1]), size_of_string(0), size_of_memory(1) {}

  String(char symbol): array(new char[2]), size_of_string(1), size_of_memory(2) {
    array[0] = symbol;
    array[1] = 0;
  }

  String (const char* string): String(strlen(string)) {
    memcpy(array, string, size_of_string);
  }

  String (size_t length, char c): String(length) {
    memset(array, c, length);
  }

  String (const String& string): String(string.size_of_string) {
    memcpy(array, string.array, size_of_string);
  }

  ~String() {
    delete[] array;
  }

  String& operator=(String string) {
    swap(string);
    return *this;
  }

  void swap(String& string) {
    std::swap(array, string.array);
    std::swap(size_of_string, string.size_of_string);
    std::swap(size_of_memory, string.size_of_memory);
  }

  void ChangeMemory() {
    char* temp = new char[size_of_memory];
    memcpy(temp, array, size_of_string + 1);
    delete[] array;
    array = temp;
  }

  char& operator[](size_t index) {
    return array[index];
  }

  const char& operator[](size_t index) const {
    return array[index];
  }

  String& operator+=(char symbol) {
    push_back(symbol);
    return *this;
  }

  String& operator+=(const String& string) {
    if ((size_of_string + string.size_of_string + 1) > size_of_memory) {
      size_of_memory = size_of_string + string.size_of_string + 1;
      this->ChangeMemory();
    }
    memcpy(array + size_of_string, string.array, string.size_of_string + 1);
    size_of_string += string.size_of_string;
    return *this;
  }

  size_t length() const{
    return size_of_string;
  }

  size_t size() const{
    return size_of_string;
  }

  size_t capacity() const{
    return size_of_memory - 1;
  }

  void push_back(char symbol) {
    if ((size_of_string + 2) > size_of_memory) {
      size_of_memory = size_of_string * 2 + 1;
      this->ChangeMemory();
    }
    array[size_of_string++] = symbol;
    array[size_of_string] = 0;
  }

  void pop_back() {
    if (size_of_string > 0) {
      array[--size_of_string] = 0;
    }
  }

  char& front() {
    return array[0];
  }

  const char& front() const {
    return array[0];
  }

  char& back() {
    return array[size_of_string - 1];
  }

  const char& back() const{
    return array[size_of_string - 1];
  }

  size_t find(const String& substring) const{
    if (substring.size_of_string <= size_of_string) {
      for (size_t i = 0; i <= size_of_string - substring.size_of_string; i++) {
        if (strncmp(array + i, substring.array, substring.size_of_string) == 0) {
          return i;
        }
      }
    }
    return size_of_string;
  }

  size_t rfind(const String& substring) const{
    if (substring.size_of_string <= size_of_string) {
      for (int i = size_of_string - substring.size_of_string; i >= 0; i--) {
        if (strncmp(array + i, substring.array, substring.size_of_string) == 0) {
          return i;
        }
      }
    }
    return size_of_string;
  }

  String substr(size_t index, size_t length) const{
    if (index > size_of_string) {
      length = 0;
      index = 0;
    }
    if (length > size_of_string - index) {
      length = size_of_string - index;
    }
    String result(length);
    memcpy(result.array, array + index, length);
    return result;
  }

  bool empty() {
    return (size_of_string == 0);
  }

  void clear() {
    array[0] = 0;
    size_of_string = 0;
  }

  void shrink_to_fit() {
    size_of_memory = size_of_string + 1;
    this->ChangeMemory();
  }

  char* data() {
    return array;
  }

  const char* data() const {
    return array;
  }
};

bool operator==(const String& first, const String& second) {
  return (strcmp(first.data(), second.data()) == 0);
}

bool operator!=(const String& first, const String& second) {
  return (strcmp(first.data(), second.data()) != 0);
}

bool operator<(const String& first, const String& second) {
  return (strcmp(first.data(), second.data()) < 0);
}

bool operator>(const String& first, const String& second) {
  return (strcmp(first.data(), second.data()) > 0);
}

bool operator<=(const String& first, const String& second) {
  return (strcmp(first.data(), second.data()) <= 0);
}

bool operator>=(const String& first, const String& second) {
  return (strcmp(first.data(), second.data()) >= 0);
}

String operator+(const String& first, const String& second) {
  String result = first;
  result += second;
  return result;
}

std::ostream& operator<<(std::ostream& cout, const String& string) {
  cout << string.data();
  return cout;
}

std::istream& operator>>(std::istream& cin, String& string) {
  string.clear();
  char symbol;
  while (cin.get(symbol)) {
    if (isspace(symbol)) {
      break;
    }
    string.push_back(symbol);
  }
  return cin;
}
