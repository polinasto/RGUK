#include <cstring>
#include <iostream>
#include <stdlib.h>
using namespace std;

class Encoder {
private:
  unsigned char *key;
  size_t key_length;

public:
  //конструктор, принимающий ключ шифрования в виде массив байтов типа unsigned char const * и размер этого массива
  Encoder(unsigned char const *key, size_t key_length) {
    if (key==nullptr||key_length==0){
      throw std::invalid_argument("Argument error!\n");
    }
    this->key = new unsigned char[key_length];
    this->key_length = key_length;
    memcpy(this->key, key, key_length);
  }

  //деструктор
  ~Encoder() {
    if (key!=nullptr){
      free(key);
    }
  }

  //mutator для значения ключа
  void SetKey(unsigned char const *new_key, size_t new_key_length) {
    if (new_key == nullptr || new_key_length == 0) {
      throw std::invalid_argument("Invalid key or key length");
    }
    if (key != nullptr) {
        delete[] key;
    }
    try {
        key = new unsigned char[new_key_length];
        std::memcpy(key, new_key, new_key_length);
        key_length = new_key_length;
    } catch (const std::bad_alloc& e) {
        throw std::runtime_error("Memory allocation failed");
    }
  }

  unsigned int encode(unsigned char const *data, size_t data_length) {
    unsigned char s[256];
    unsigned char* output = new unsigned char[data_length];
    int i;

    for (i = 0; i < 256; i++) {
      s[i] = i;
    }
    char swap;
    for (i = 0; i < 256; i++) {
      int j = (key + s[i] + i) % 256;
      swap = s[i];
      s[i] = s[j];
      s[j] = swap;
    }
    int i = 0;
    int j = 0;
    for (size_t k = 0; k < data_length; k++) {
      i = (i + 1) % 256;
      j = (j + s[i]) % 256;
      swap = s[i];
      s[i] = s[j];
      s[j] = swap;

      output[k] = data[k] ^ s[(s[i] + s[j]) % 256]; // XOR операция
    }

    return output;
  }
}; 
