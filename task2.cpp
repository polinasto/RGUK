#include <iostream>
using namespace std;

class logical_values_array final{
    private:
        //поле _value
        unsigned int _value;
    public:
        //аксессор
        unsigned int _value_acsessor() const { return _value; }

        //конструктор
        logical_values_array(unsigned int _val=0) : _value(_val) {};

        //методы
        //инверсия
        logical_values_array inversion() const{
            return logical_values_array(logical_values_array(~_value));
        }
        //конъюнкция
        logical_values_array conjuction(const logical_values_array &other) const{
            return logical_values_array( _value & other._value );
        }
        //дизъюнкция
        logical_values_array disjuntion(const logical_values_array &other) const{
            return logical_values_array( _value | other._value );
        }
        //имликация
        logical_values_array implication(const logical_values_array &other) const{
            return logical_values_array(  ~_value | other._value );
        }
        //коимпликация
        logical_values_array coimplicaton(const logical_values_array &other) const{
            return logical_values_array( _value | ~other._value );
        }
        //эквивалентность
        logical_values_array XOR(const logical_values_array &other) const{
            return logical_values_array( _value ^ other._value );
        }
        //стрелка пирса
        logical_values_array PIERCE(const logical_values_array& other) const {
            return logical_values_array(~(_value | other._value));
        }
        //штрих шеффера
        logical_values_array SHEFFER(const logical_values_array& other) const {
            return logical_values_array(~(_value & other._value));

        }
        //статический метод equals, сравнивающий два объекта по отношению эквивалентности
        static logical_values_array equals(const logical_values_array& other, const logical_values_array& another_other) {
            return logical_values_array(other._value == another_other._value ? 1 : 0);
        }

        //метод get_bit, который возвращает значение бита по его позиции (позиция является параметром типа size_t)

        bool get_bit(size_t position) const {
            if (position >= sizeof(_value) * 8) {
                throw std::out_of_range("Position is out of range.");
            }
            return(_value>>position)&1;
        }

        //перегруженный оператор [], делегирующий выполнение на метод get_bit
        logical_values_array operator[](size_t position) const{
            return get_bit(position);
        }

        //метод, принимающий значение типа char *; по значению адреса в параметре должно быть записано двоичное представление поля _value в виде строки в стиле языка программирования C
        void convert(char * string){
            for (int i = sizeof(_value) * 8 - 1; i >= 0; --i) {
                // Сдвигаем битовое представление вправо и берем младший бит
                string[i] = (_value & (1 << i)) ? '1' : '0';
            }
            string[sizeof(_value) * 8] = '\0'; // Завершаем строку нулевым символом
            
        }
        //вывод потока
        friend std::ostream& operator<<(std::ostream& os, logical_values_array& n){
            os<<n._value_acsessor();
            return os;
        }
        friend std::istream& operator>>(std::istream& is,logical_values_array& n){
            is>>n._value;
            return is;
        }
        
};

int main(){
    logical_values_array one(11);

    logical_values_array two=one.inversion();
    std::cout<<two<<std::endl;
    
    logical_values_array three;
    std::cin>>three;
    std::cout<<"Entered num is "<<three<<std::endl;

    return 0;
}