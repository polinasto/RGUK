#include <iostream>
#include "Fraction.h"
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <sstream>
#include <string>

// Удаление пробелов из строки
std::string Fraction::removeSpaces(const std::string& str) {
    std::string result = str;
    result.erase(std::remove_if(result.begin(), result.end(), ::isspace), result.end());
    return result;
}

// Нахождение НОД двух чисел
BigInteger Fraction::gcd(BigInteger a, BigInteger b) {
    if (a == 0 && b == 0) return BigInteger(1);
    if (a < BigInteger(0)) a = BigInteger(0) - a;
    if (b < BigInteger(0)) b = BigInteger(0) - b;
    while (b != BigInteger(0)) {
        BigInteger temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// Нормализация дроби
void Fraction::normalize() {
    if (denominator == BigInteger(0))
        throw std::runtime_error("Denominator cannot be zero");
    if (denominator < BigInteger(0)) {
        numerator = BigInteger(0) - numerator;
        denominator = BigInteger(0) - denominator;
    }
    BigInteger common_divisor = gcd(numerator, denominator);
    if (common_divisor > BigInteger(1)) {
        numerator = numerator / common_divisor;
        denominator = denominator / common_divisor;
    }
    if (numerator == BigInteger(0)) denominator = BigInteger(1);
}

// Конструкторы
Fraction::Fraction() : numerator(0), denominator(1) {}

Fraction::Fraction(const BigInteger& num, const BigInteger& denom)
    : numerator(num), denominator(denom) { normalize(); }

Fraction::Fraction(int num, int denom)
    : numerator(num), denominator(denom) { normalize(); }

Fraction::Fraction(int num)
    : numerator(num), denominator(1) {}

Fraction::Fraction(const std::string& str) {
    std::string clean_str = removeSpaces(str);
    size_t slash_pos = clean_str.find('/');
    if (slash_pos == std::string::npos) {
        numerator = BigInteger(clean_str);
        denominator = BigInteger(1);
    } else {
        numerator = BigInteger(clean_str.substr(0, slash_pos));
        denominator = BigInteger(clean_str.substr(slash_pos + 1));
    }
    normalize();
}

// Геттеры
const BigInteger& Fraction::getNumerator() const { return numerator; }
const BigInteger& Fraction::getDenominator() const { return denominator; }

// Проверка на ноль
bool Fraction::isZero() const { return numerator == BigInteger(0); }

// Получение знака дроби
int Fraction::sign() const {
    if (numerator == BigInteger(0)) return 0;
    return (numerator < BigInteger(0)) ? -1 : 1;
}

// Арифметические операции
Fraction Fraction::operator-() const {
    return Fraction(BigInteger(0) - numerator, denominator);
}

Fraction& Fraction::operator+=(const Fraction& other) {
    numerator = numerator * other.denominator + other.numerator * denominator;
    denominator = denominator * other.denominator;
    normalize();
    return *this;
}

Fraction Fraction::operator+(const Fraction& other) const {
    Fraction result = *this;
    result += other;
    return result;
}

Fraction& Fraction::operator-=(const Fraction& other) {
    numerator = numerator * other.denominator - other.numerator * denominator;
    denominator = denominator * other.denominator;
    normalize();
    return *this;
}

Fraction Fraction::operator-(const Fraction& other) const {
    Fraction result = *this;
    result -= other;
    return result;
}

Fraction& Fraction::operator*=(const Fraction& other) {
    numerator = numerator * other.numerator;
    denominator = denominator * other.denominator;
    normalize();
    return *this;
}

Fraction Fraction::operator*(const Fraction& other) const {
    Fraction result = *this;
    result *= other;
    return result;
}

Fraction& Fraction::operator/=(const Fraction& other) {
    if (other.numerator == BigInteger(0))
        throw std::runtime_error("Division by zero");
    numerator = numerator * other.denominator;
    denominator = denominator * other.numerator;
    normalize();
    return *this;
}

Fraction Fraction::operator/(const Fraction& other) const {
    Fraction result = *this;
    result /= other;
    return result;
}

// Операторы сравнения
bool Fraction::operator==(const Fraction& other) const {
    return numerator == other.numerator && denominator == other.denominator;
}

bool Fraction::operator!=(const Fraction& other) const {
    return !(*this == other);
}

bool Fraction::operator<(const Fraction& other) const {
    return (numerator * other.denominator) < (other.numerator * denominator);
}

bool Fraction::operator<=(const Fraction& other) const {
    return *this < other || *this == other;
}

bool Fraction::operator>(const Fraction& other) const {
    return !(*this <= other);
}

bool Fraction::operator>=(const Fraction& other) const {
    return !(*this < other);
}

// Возведение в степень
Fraction Fraction::pow(unsigned int exponent) const {
    Fraction result(1);
    for (unsigned int i = 0; i < exponent; ++i)
        result *= *this;
    return result;
}

// Модуль дроби
Fraction Fraction::abs() const {
    if (numerator < BigInteger(0))
        return Fraction(BigInteger(0) - numerator, denominator);
    return *this;
}

// Преобразование в double
double Fraction::toDouble() const {
    std::string num_str = numerator.toString();
    std::string den_str = denominator.toString();
    return std::stod(num_str) / std::stod(den_str);
}

// Преобразование в строку
std::string Fraction::toString() const {
    std::ostringstream oss;
    if (denominator == BigInteger(1)) {
        oss << numerator;
    } else {
        oss << numerator << "/" << denominator;
    }
    return oss.str();
}

// Операторы ввода/вывода
std::ostream& operator<<(std::ostream& os, const Fraction& f) {
    os << f.toString();
    return os;
}

std::istream& operator>>(std::istream& is, Fraction& f) {
    std::string str;
    is >> str;
    f = Fraction(str);
    return is;
}
int main() {
    try {
        Fraction f1(1, 2);
        Fraction f2(3, 4);

        std::cout << "f1 = " << f1 << std::endl;
        std::cout << "f2 = " << f2 << std::endl;

        Fraction sum = f1 + f2;
        Fraction diff = f1 - f2;
        Fraction product = f1 * f2;
        Fraction quotient = f1 / f2;

        std::cout << f1 << " + " << f2 << " = " << sum << std::endl;
        std::cout << f1 << " - " << f2 << " = " << diff << std::endl;
        std::cout << f1 << " * " << f2 << " = " << product << std::endl;
        std::cout << f1 << " / " << f2 << " = " << quotient << std::endl;

        Fraction f3("-5/6");
        Fraction f4("7/8");
        Fraction f5("9");

        std::cout << "f3 = " << f3 << std::endl;
        std::cout << "f4 = " << f4 << std::endl;
        std::cout << "f5 = " << f5 << std::endl;

        Fraction complex = (f1 + f2) * f3 - f4 / f5;
        std::cout << "(f1 + f2) * f3 - f4 / f5 = " << complex << std::endl;

        std::cout << "f3.abs() = " << f3.abs() << std::endl;
        std::cout << "f3.pow(3) = " << f3.pow(3) << std::endl;
        std::cout << "f4.toDouble() = " << f4.toDouble() << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
