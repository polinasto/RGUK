#include <iostream>
#include <cmath>
using namespace std;



class ComplexNum final{
    private:
        //поля, соответствующие действительной и мнимой части комплексного числа (типа double)
        double real,imag;
        const double PI=3.14159265358979323846;
        


    public:
        //конструктор, который принимает значения действительной и мнимой части (оба параметра по умолчанию равны 0)
        ComplexNum(double r=0,double i=0) : real(r),imag(i){}

        //double epsilon()
        //операторные методы, производящие операции сложения, вычитания, умножения и деления комплексных чисел (с модификацией и без модификации вызывающего объекта: +=/+, ...)
        ComplexNum operator+(const ComplexNum& other) const{
            return ComplexNum(real + other.real,imag+other.imag);
        }
        ComplexNum operator-(const ComplexNum& other) const{
            return ComplexNum(real-other.real,imag-other.imag);
        }
        ComplexNum operator*(const ComplexNum& other) const{
            return ComplexNum(real * other.real - imag * other.imag,real * other.imag + imag * other.real);
        }
        ComplexNum operator/(const ComplexNum& other) const{
            double denominator = other.real * other.real + other.imag * other.imag;
            if (denominator==0){
                ComplexNum(0,0);
            }
            return ComplexNum((real * other.real + imag * other.imag) / denominator,(imag * other.real - real * other.imag) / denominator);
        }
        ComplexNum operator+=(const ComplexNum& other){
            real+=other.real;
            imag+=other.imag;
            return *this;
        }
        ComplexNum operator-=(const ComplexNum& other){
            real-=other.real;
            imag-=other.imag;
            return *this;
        }
        ComplexNum operator*=(const ComplexNum& other){
            real=real * other.real - imag * other.imag;
            imag=real * other.imag + imag * other.real;
            return *this;
        }
        ComplexNum operator/=(const ComplexNum& other){
            double denominator = other.real * other.real + other.imag * other.imag;
            if (denominator==0){
                ComplexNum(0,0);
            }
            real=(real * other.real + imag * other.imag) / denominator;
            real=(imag * other.real - real * other.imag) / denominator;
            return *this;
        }
        //метод, возвращающий модуль комплексного числа
        double absCN() const{
            return sqrt(real*real+imag*imag);
        }
        //метод, возвращающий аргумент комплексного числа
        double argCN(double EPS){
            
            if (real + EPS > 0) {
                return std::atan2(imag, real);
            } else if (real + EPS < 0 && imag + EPS >= 0) {
                return std::atan2(imag, real);
            } else if (real + EPS < 0 && imag + EPS < 0) {
                return std::atan2(imag, real);
            } else if (std::abs(real) < EPS && imag > 0) {
                return PI / 2;
            } else if (std::abs(real) < EPS && imag < 0) {
                return -PI / 2;
            }else{
                throw std::runtime_error("Error with EPS\n");
            }
            
        }
        //перегруженный оператор вставки в поток
        friend std::ostream& operator<<(std::ostream& os, const ComplexNum& n){
            os<<n.real<<(n.imag>=0?"+":" ")<<n.imag<<"i";
            return os;
        }
        //перегруженный оператор выгрузки из потока
        friend std::istream& operator>>(std::istream& is, ComplexNum& n){
            char sign,i_char;

            if (!(is >>n.real)){
                return is;
            }
            if(!(is>>sign)||sign!='+'&&sign!='-'){
                return is;
            }
            if(!(is>>n.imag)){
                return is;
            }
            if (!(is>>i_char)||i_char!='i'){
                return is;
            }
            if(sign=='-'){
                n.imag=-1*n.imag;
            }
            return is;
        }
};
int main(){

    //EPS
    
    double EPS=1;
    float prev_eps;
    while ((1+EPS)!=1){
        prev_eps=EPS;
        EPS/=2;
    }
    //создание комплексных чисел
    ComplexNum num1(3,9),num2(7,2);

    //вывод
    std::cout<<"num1 = "<<num1<<std::endl;
    std::cout<<"num2 = "<<num2<<std::endl;

    //сложение
    ComplexNum sum=num1+num2;
    std::cout<<"num1 + num2 = "<<sum<<std::endl;
    
    //+=
    num1+=num2;
    std::cout<<"After using num1 += num2, num1 is now "<<num1<<std::endl;
    
    //вычитание
    ComplexNum sub=num1-num2;
    std::cout<<"num1 - num2 = "<<sub<<std::endl;
    
    //-=
    num2-=num1;
    std::cout<<"After using num2 -= num1, num2 is now "<<num2<<std::endl;
    
    //умножение
    ComplexNum mul=num1*num2;
    std::cout<<"num1 * num2 = "<<mul<<std::endl;
    
    //*=
    num2*=num1;
    std::cout<<"After using num2 *= num1, num2 is now "<<num2<<std::endl;
    
    //деление
    ComplexNum div=num1/num2;
    std::cout<<"num1 / num2 = "<<div<<std::endl;

    // /=
    num2/=num1;
    std::cout<<"After using num2 /= num1, num2 is now "<<num2<<std::endl;
    
    //модуль
    
    std::cout<<"|num1| = "<<num1.absCN()<<std::endl;

    //аргумент
    std::cout<<"arg(c1) = "<<num1.argCN(prev_eps)<<std::endl;

    //ввод комплекснaого числа
    ComplexNum c3;
    std::cout<<"Введите комплексное число:";
    std::cin>>c3;
    std::cout<<"Вы ввели "<<c3<<std::endl;
    return 0;
    
}
//развертка класса
//get
//cath throw try
