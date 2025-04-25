#ifndef PUKSRENJK_INT_WRAPPER_H
#define PUKSRENJK_INT_WRAPPER_H

class int_wrapper final
{

private:

    int *_value_ptr;

public:

    explicit int_wrapper(int value=0){_value_ptr=new int(value)}

public:
    ~int_wrapper(){delete _value_ptr}
    
    int_wrapper(const int_wrapper& other) : _value_ptr(other._value_ptr) {}
    
    int_wrapper& operator=(const int_wrapper& other){
      if (this==&other){
        return *this;
      }
      delete _value_ptr;
      _value_ptr=new int(*(other._value_ptr));
      return *this;
    }
    
    int_wrapper(int_wrapper&&other) : _value_ptr(other._value_ptr) {}
    
    int_wrapper& operator=(const int_wrapper&& other){
      if (this==&other){
        return *this;
      }
      delete _value_ptr;
      _value_ptr=other._value_ptr;
      other._value_ptr=nullptr;
      return *this;
    }
    // TODO: rule of 5 implementation

public:
    int_wrapper operator+(const int_wrapper& other){ return *(_value_ptr)+*(other._value_ptr); }
    int_wrapper operator+=(const int_wrapper& other){ 
      *(_value_ptr)+=*(other._value_ptr)
      return *this;
    }
    int_wrapper operator-(const int_wrapper& other){ return *(_value_ptr)-*(other._value_ptr); }
    int_wrapper operator-=(const int_wrapper& other){ 
      *(_value_ptr)-=*(other._value_ptr)
      return *this;
    }
    int_wrapper operator*(const int_wrapper& other){ return *(_value_ptr)**(other._value_ptr); }
    int_wrapper operator*=(const int_wrapper& other){ 
      *(_value_ptr)*=*(other._value_ptr)
      return *this;
    }
    int_wrapper operator/(const int_wrapper& other){ return *(_value_ptr)/(*(other._value_ptr)); }
    int_wrapper operator/=(const int_wrapper& other){ 
      *(_value_ptr)/=*(other._value_ptr)
      return *this;
    }
    int_wrapper operator%(const int_wrapper& other){ return *(_value_ptr)%*(other._value_ptr); }
    int_wrapper operator%=(const int_wrapper& other){ 
      *(_value_ptr)%=*(other._value_ptr)
      return *this;
    }
    int_wrapper operator++(){
      *(_value_ptr)++;
      return *this;
    }
    int_wrapper operator++(int){
      int_wrapper temp(*this);
      ++(*_value_ptr);
      return *this;
    }
    int_wrapper operator--(){
      *(_value_ptr)--;
      return *this;
    }
    int_wrapper operator--(int){
      int_wrapper temp(*this);
      --(*_value_ptr);
      return *this;
    }
    // TODO: arithmetic operators
    // +=, +, -=, -, *=, *, /=, /, %=, %, ++v, v++, --v, v--
    int_wrapper operator<<=(int shift) const {
	  *(_value_ptr)<<=shift;
	  return *this;
    }
	int_wrapper operator<<(int shift) const {
      return (*_value_ptr)<<shift;
    }
	int_wrapper operator>>=(int shift) const {
      *(_value_ptr)>>=shift;
	  return *this;
    }
	int_wrapper operator>>(int shift) const {
      return (*_value_ptr)>>shift;
    }
	int_wrapper operator&=(const int_wrapper&other) const {
      *(_value_ptr)&=*(other._value_ptr);
	  return *this;
    }
	int_wrapper operator&(const int_wrapper&other){
      return *(_value_ptr)&(*other._value_ptr);
    }
	int_wrapper operator|=(const int_wrapper&other) const {
      *(_value_ptr)|=*(other._value_ptr);
	  return *this;
    }
	int_wrapper operator|(const int_wrapper&other){
      return *(_value_ptr)|(*other._value_ptr);
    }
    }
	int_wrapper operator^=(const int_wrapper&other) const {
      *(_value_ptr)^=*(other._value_ptr);
	  return *this;
    }
	int_wrapper operator^(const int_wrapper&other){
      return *(_value_ptr)^(*other._value_ptr);
    }
    // TODO: bitwise operators
    // <<=, <<, >>=, >>, &=, &, |=, |=, ^=, ^
	ostream& operator<<(ostream& os, const int_wrapper& val){
		os << val._value_ptr <<std::endl;
		return os;
	}
	istream& operator<<(istream& is, const int_wrapper& val){
		is >> val._value_ptr;
		return is;
	}

    // TODO: stream operator
    // <<, >>

};

#endif //PUKSRENJK_INT_WRAPPER_H