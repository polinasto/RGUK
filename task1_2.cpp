#include <iostream>
#include <cstring>   // Для strlen, memcpy, memset
#include <climits>   // Для INT_MAX, INT_MIN
#include <cstdlib>   // Для abs

// Максимальное количество "цифр" (32-битных блоков).
// Достаточно для очень больших чисел, но можно увеличить при необходимости.
// 1024 * 32 бит = 32768 бит (около 9860 десятичных цифр)
static const size_t MAX_BIGINT_DIGITS = 1024;

class BigInteger {
private:
    int digits[MAX_BIGINT_DIGITS]; // Массив цифр (little endian, каждый int - 32 бита)
    size_t digits_size;            // Текущее количество используемых элементов digits
    bool is_negative;              // Знак числа
    bool is_small_value;           // Флаг оптимизации для малых значений
    int small_value;               // Значение для оптимизации (если is_small_value = true)

    // Вспомогательные функции для констант (не зависят от объекта)
    static int get_bits_per_int() { return sizeof(int) * 8; }
    static int get_max_small() { return INT_MAX; }
    static int get_min_small() { return INT_MIN; }

    // Проверка, помещается ли число в int
    bool fits_in_int() const {
        if (digits_size > 1) return false; // Более одного 32-битного блока - точно не int
        if (digits_size == 0) return true; // Число 0, оптимизировано как 0

        long long val = digits[0];
        if (is_negative) val = -val;
        return val >= get_min_small() && val <= get_max_small();
    }

    // Оптимизация для малых значений (переход в small_value формат)
    void try_optimize() {
        if (digits_size == 0) { // Если массив пуст (в результате normalize или другого), это 0
            is_small_value = true;
            small_value = 0;
            is_negative = false;
            return;
        }

        if (fits_in_int()) {
            int val = digits[0];
            if (is_negative && val != 0) val = -val; // Применяем знак, если не 0

            is_small_value = true;
            small_value = val;
            digits_size = 0; // "Очищаем" большой формат
            is_negative = false; // Знак уже учтен в small_value
        }
    }

    // Переход из small_value формата в big_integer формат
    void ensure_big_format() {
        if (is_small_value) {
            bool was_negative = (small_value < 0);
            unsigned int abs_val = (unsigned int)std::abs(small_value);

            // Инициализация digits
            memset(digits, 0, sizeof(digits)); // Очищаем весь массив

            if (abs_val == 0) {
                digits[0] = 0;
                digits_size = 1;
            } else {
                digits[0] = (int)abs_val;
                digits_size = 1;
            }

            is_negative = was_negative;
            is_small_value = false;
        }
    }

    // Нормализация (удаление ведущих нулей из digits)
    void normalize() {
        while (digits_size > 1 && digits[digits_size - 1] == 0) {
            digits_size--;
        }
        if (digits_size == 1 && digits[0] == 0) {
            is_negative = false;
        }
    }

    // Сравнение абсолютных значений (используется для арифметики и сравнения)
    //-1 если abs(this) < abs(other), 0 если равны, 1 если abs(this) > abs(other)
    int compare_abs(const BigInteger& other) const {
        BigInteger a_abs = *this; // Создаем временные копии
        BigInteger b_abs = other;
        a_abs.ensure_big_format();
        b_abs.ensure_big_format();

        // Сравниваем по размеру
        if (a_abs.digits_size < b_abs.digits_size) return -1;
        if (a_abs.digits_size > b_abs.digits_size) return 1;

        // Размеры одинаковы, сравниваем поблочно с конца
        for (int i = (int)a_abs.digits_size - 1; i >= 0; i--) {
            unsigned int ad = (unsigned int)a_abs.digits[i];
            unsigned int bd = (unsigned int)b_abs.digits[i];
            if (ad < bd) return -1;
            if (ad > bd) return 1;
        }
        return 0; // Абсолютные значения равны
    }

    // Вспомогательная функция для деления, возвращает частное и остаток
    void divide_internal(const BigInteger& divisor, BigInteger& quotient, BigInteger& remainder) const {
        BigInteger current_this = *this; // Работаем с копиями, чтобы не менять оригинал
        current_this.ensure_big_format();
        BigInteger current_divisor = divisor;
        current_divisor.ensure_big_format();

        // Если делитель равен нулю, деление невозможно
        if (current_divisor.digits_size == 1 && current_divisor.digits[0] == 0) {
            quotient = BigInteger(0); // Или выбросить исключение
            remainder = BigInteger(0);
            return;
        }

        // Если делимое меньше делителя, частное 0, остаток - делимое
        if (current_this.compare_abs(current_divisor) < 0) {
            quotient = BigInteger(0);
            remainder = current_this;
            return;
        }

        // Если делимое равно делителю, частное 1, остаток 0
        if (current_this.compare_abs(current_divisor) == 0) {
            quotient = BigInteger(1);
            remainder = BigInteger(0);
            return;
        }

        // Длинное деление (сложная реализация без оптимизаций)
        BigInteger temp_quotient(0);
        BigInteger temp_remainder = current_this;

        // Простой, неоптимизированный алгоритм:
        // Многократное вычитание делителя из остатка
        while (temp_remainder.compare_abs(current_divisor) >= 0) {
            temp_remainder -= current_divisor;
            temp_quotient += BigInteger(1);
        }

        quotient = temp_quotient;
        remainder = temp_remainder;
    }


public:
    // --- Конструкторы ---
    BigInteger() : digits_size(0), is_negative(false), is_small_value(true), small_value(0) {}

    BigInteger(int value) : digits_size(0), is_negative(false), is_small_value(true), small_value(value) {}

    // Конструктор от массива int (little endian)
    BigInteger(const int* digit_array, size_t array_size) : is_negative(false), is_small_value(false) {
        if (array_size == 0) {
            digits_size = 1;
            digits[0] = 0;
        } else {
            digits_size = array_size;
            // Проверяем, чтобы не выйти за MAX_BIGINT_DIGITS
            if (digits_size > MAX_BIGINT_DIGITS) {
                digits_size = MAX_BIGINT_DIGITS; // Обрезаем, или можно выбросить исключение
            }
            memcpy(digits, digit_array, digits_size * sizeof(int));
        }
        normalize();
        try_optimize();
    }

    // Конструктор от строки (big endian) с указанием основания
    BigInteger(const char* str, size_t base = 10) : is_negative(false), is_small_value(false) {
        // Инициализация нулем
        digits_size = 1;
        digits[0] = 0;
        is_negative = false;
        is_small_value = false;

        if (!str || strlen(str) == 0) {
            try_optimize();
            return;
        }

        size_t len = strlen(str);
        size_t start = 0;

        if (str[0] == '-') {
            is_negative = true;
            start = 1;
        } else if (str[0] == '+') {
            start = 1;
        }

        // Конвертируем из строки, умножая на основание и прибавляя цифры
        for (size_t i = start; i < len; i++) {
            char c = str[i];
            int digit_val;

            if (c >= '0' && c <= '9') {
                digit_val = c - '0';
            } else if (c >= 'A' && c <= 'Z') {
                digit_val = c - 'A' + 10;
            } else if (c >= 'a' && c <= 'z') {
                digit_val = c - 'a' + 10;
            } else {
                continue; // Пропускаем недопустимые символы
            }

            if ((size_t)digit_val >= base) continue; // Пропускаем цифры, превышающие основание

            *this *= BigInteger((int)base);   // Умножаем текущее число на основание
            *this += BigInteger(digit_val); // Добавляем текущую цифру
        }
        // Если число было отрицательным, но в итоге стало 0 (например, "-0"), сбросим знак
        if (digits_size == 1 && digits[0] == 0) {
            is_negative = false;
        } else {
            // Применяем знак, если он был установлен
            if (str[0] == '-') {
                is_negative = true;
            } else {
                is_negative = false;
            }
        }
        normalize();
        try_optimize();
    }

    // Копирующий конструктор
    BigInteger(const BigInteger& other) {
        is_negative = other.is_negative;
        is_small_value = other.is_small_value;
        small_value = other.small_value;
        digits_size = other.digits_size;
        if (!is_small_value) {
            memcpy(digits, other.digits, digits_size * sizeof(int));
        }
    }

    // Оператор присваивания
    BigInteger& operator=(const BigInteger& other) {
        if (this == &other) return *this; // Самоприсваивание

        is_negative = other.is_negative;
        is_small_value = other.is_small_value;
        small_value = other.small_value;
        digits_size = other.digits_size;
        if (!is_small_value) {
            memcpy(digits, other.digits, digits_size * sizeof(int));
        }
        return *this;
    }

    // --- Деструктор ---
    ~BigInteger() {
    }

    // --- Операторы сравнения ---
    bool operator==(const BigInteger& other) const {
        if (is_small_value && other.is_small_value) {
            return small_value == other.small_value;
        }

        BigInteger a_temp = *this;
        BigInteger b_temp = other;
        a_temp.ensure_big_format();
        b_temp.ensure_big_format();

        if (a_temp.is_negative != b_temp.is_negative) return false;
        if (a_temp.digits_size != b_temp.digits_size) return false;

        return memcmp(a_temp.digits, b_temp.digits, a_temp.digits_size * sizeof(int)) == 0;
    }

    bool operator!=(const BigInteger& other) const { return !(*this == other); }

    bool operator<(const BigInteger& other) const {
        if (is_small_value && other.is_small_value) {
            return small_value < other.small_value;
        }

        BigInteger a_temp = *this;
        BigInteger b_temp = other;
        a_temp.ensure_big_format();
        b_temp.ensure_big_format();

        if (a_temp.is_negative && !b_temp.is_negative) return true;  // Отрицательное < Положительного
        if (!a_temp.is_negative && b_temp.is_negative) return false; // Положительное > Отрицательного

        int cmp = a_temp.compare_abs(b_temp);
        if (a_temp.is_negative) { // Оба отрицательные
            return cmp > 0; // 
        } else { // Оба положительные
            return cmp < 0; 
        }
    }

    bool operator<=(const BigInteger& other) const { return *this < other || *this == other; }
    bool operator>(const BigInteger& other) const { return !(*this <= other); }
    bool operator>=(const BigInteger& other) const { return !(*this < other); }

    // --- Арифметические операторы ---

    BigInteger& operator+=(const BigInteger& other) {
        if (is_small_value && other.is_small_value) {
            long long result = (long long)small_value + other.small_value;
            if (result >= get_min_small() && result <= get_max_small()) {
                small_value = (int)result;
                return *this;
            }
        }

        ensure_big_format();
        BigInteger b = other; // Копия, чтобы гарантировать BigFormat
        b.ensure_big_format();

        // Если знаки разные, то это вычитание
        if (is_negative != b.is_negative) {
            // Например, (5) + (-2) = (5) - (2)
            // (-5) + (2) = -( (5) - (2) )
            bool original_sign_this = is_negative;
            is_negative = false; 
            b.is_negative = false; 

            int cmp_abs = compare_abs(b);
            if (cmp_abs < 0) { // Если abs(this) < abs(other), меняем местами и меняем знак результата
                BigInteger temp = *this;
                *this = b;
                b = temp;
                is_negative = !original_sign_this; // Знак будет противоположный
            } else {
                is_negative = original_sign_this; // Знак остается таким же, как у большего по модулю
            }

            unsigned int borrow = 0;
            for (size_t i = 0; i < digits_size; ++i) {
                unsigned long long diff = (unsigned long long)digits[i] - borrow;
                if (i < b.digits_size) {
                    diff -= (unsigned int)b.digits[i];
                }
                if (diff > UINT_MAX) { // Произошло заимствование
                    diff += (1ULL << get_bits_per_int());
                    borrow = 1;
                } else {
                    borrow = 0;
                }
                digits[i] = (int)diff;
            }
        } else { // Знаки одинаковые, это сложение
            unsigned long long carry = 0;
            size_t max_len = (digits_size > b.digits_size ? digits_size : b.digits_size);
            for (size_t i = 0; i < max_len || carry; ++i) {
                if (i >= MAX_BIGINT_DIGITS) { // Переполнение массива
                    // Обработка ошибки или расширение массива (если он динамический)
                    break;
                }
                unsigned long long sum = (i < digits_size ? (unsigned int)digits[i] : 0ULL) +
                                         (i < b.digits_size ? (unsigned int)b.digits[i] : 0ULL) + carry;
                if (i >= digits_size) digits_size++; // Увеличиваем размер, если добавляем новый блок
                digits[i] = (int)sum;
                carry = sum >> get_bits_per_int();
            }
        }
        normalize();
        try_optimize();
        return *this;
    }

    BigInteger operator+(const BigInteger& other) const {
        BigInteger result = *this;
        result += other;
        return result;
    }

    BigInteger& operator-=(const BigInteger& other) {
        BigInteger neg_other = other;
        if (neg_other.is_small_value) {
            if (neg_other.small_value != INT_MIN) { // Избегаем переполнения для INT_MIN при -value
                neg_other.small_value = -neg_other.small_value;
            } else { // Если INT_MIN, то его нельзя просто так инвертировать в int, переводим в big_format
                neg_other.ensure_big_format();
                neg_other.is_negative = !neg_other.is_negative;
            }
        } else {
            neg_other.is_negative = !neg_other.is_negative; // Инвертируем знак для big_format
        }
        return *this += neg_other; // Вычитание = сложение с инвертированным знаком
    }

    BigInteger operator-(const BigInteger& other) const {
        BigInteger result = *this;
        result -= other;
        return result;
    }

    BigInteger& operator*=(const BigInteger& other) {
        if (is_small_value && other.is_small_value) {
            long long result = (long long)small_value * other.small_value;
            if (result >= get_min_small() && result <= get_max_small()) {
                small_value = (int)result;
                // При умножении 0 на что-либо, результат 0. Знак не важен.
                if (small_value == 0) is_negative = false;
                return *this;
            }
        }

        ensure_big_format();
        BigInteger b = other;
        b.ensure_big_format();

        bool result_negative = (is_negative != b.is_negative);

        // Временный массив для результата умножения (может быть больше суммы длин)
        int temp_result_digits[MAX_BIGINT_DIGITS * 2]; // Может потребоваться до 2*MAX_BIGINT_DIGITS
        memset(temp_result_digits, 0, sizeof(temp_result_digits)); // Инициализация нулями

        for (size_t i = 0; i < digits_size; ++i) {
            unsigned long long carry = 0;
            unsigned int current_digit_a = (unsigned int)digits[i];
            for (size_t j = 0; j < b.digits_size || carry; ++j) {
                // Убедимся, что не выходим за границы временного массива
                if (i + j >= MAX_BIGINT_DIGITS * 2) {
                    // Обработка переполнения (увеличить MAX_BIGINT_DIGITS или выбросить исключение)
                    break;
                }

                unsigned int current_digit_b = (j < b.digits_size ? (unsigned int)b.digits[j] : 0ULL);
                unsigned long long product = current_digit_a * current_digit_b +
                                             (unsigned int)temp_result_digits[i + j] + carry;

                temp_result_digits[i + j] = (int)product;
                carry = product >> get_bits_per_int();
            }
        }

        // Копируем результат из временного массива в digits
        digits_size = digits_size + b.digits_size;
        // После умножения возможны ведущие нули, которые normalize уберет.
        // Верхний предел digits_size может быть sum_of_sizes.
        if (digits_size > MAX_BIGINT_DIGITS) digits_size = MAX_BIGINT_DIGITS; // Обрезаем, если что

        // Копируем только актуальную часть, которая может быть меньше digits_size
        size_t actual_result_len = 0;
        // Находим реальный размер результата (без ведущих нулей во временном массиве)
        for (int i = (MAX_BIGINT_DIGITS * 2) - 1; i >= 0; --i) {
            if (temp_result_digits[i] != 0) {
                actual_result_len = i + 1;
                break;
            }
        }
        if (actual_result_len == 0) { // Если результат 0
            *this = BigInteger(0);
            return *this;
        }

        digits_size = actual_result_len;
        if (digits_size > MAX_BIGINT_DIGITS) { // Если реальный размер все еще слишком велик
            digits_size = MAX_BIGINT_DIGITS; // Обрезаем
        }
        memcpy(digits, temp_result_digits, digits_size * sizeof(int));


        is_negative = result_negative;

        // Если результат стал 0, сбрасываем знак
        if (digits_size == 1 && digits[0] == 0) {
            is_negative = false;
        }

        normalize();
        try_optimize();
        return *this;
    }

    BigInteger operator*(const BigInteger& other) const {
        BigInteger result = *this;
        result *= other;
        return result;
    }

    BigInteger& operator/=(const BigInteger& other) {
        if (other == BigInteger(0)) {
            // Деление на ноль, можно выбросить исключение или установить в 0
            *this = BigInteger(0);
            return *this;
        }

        // Обработка small_value оптимизации
        if (is_small_value && other.is_small_value) {
            small_value /= other.small_value;
            // Знак уже учтен в int делении
            if (small_value == 0) is_negative = false;
            return *this;
        }

        // Преобразование в big_format для обеих сторон
        ensure_big_format();
        BigInteger divisor = other;
        divisor.ensure_big_format();

        bool result_negative = (is_negative != divisor.is_negative);

        // Внутреннее деление работает с абсолютными значениями
        is_negative = false;
        divisor.is_negative = false;

        BigInteger quotient;
        BigInteger remainder;
        divide_internal(divisor, quotient, remainder); // Используем внутреннюю функцию

        *this = quotient; // Присваиваем частное
        is_negative = result_negative; // Восстанавливаем знак

        if (*this == BigInteger(0)) { // Если результат 0, знак всегда положительный
            is_negative = false;
        }
        normalize();
        try_optimize();
        return *this;
    }

    BigInteger operator/(const BigInteger& other) const {
        BigInteger result = *this;
        result /= other;
        return result;
    }

    BigInteger& operator%=(const BigInteger& other) {
        if (other == BigInteger(0)) {
            // Модуль по нулю, можно выбросить исключение или вернуть 0
            *this = BigInteger(0);
            return *this;
        }

        // Обработка small_value оптимизации
        if (is_small_value && other.is_small_value) {
            small_value %= other.small_value;
            // Знак остатка должен соответствовать знаку делимого
            if (is_negative && small_value != 0) small_value = -small_value;
            if (small_value == 0) is_negative = false;
            return *this;
        }

        // Преобразование в big_format
        ensure_big_format();
        BigInteger divisor = other;
        divisor.ensure_big_format();

        bool original_sign = is_negative; // Сохраняем знак исходного числа

        // Внутреннее деление работает с абсолютными значениями
        is_negative = false;
        divisor.is_negative = false;

        BigInteger quotient;
        BigInteger remainder;
        divide_internal(divisor, quotient, remainder); // Используем внутреннюю функцию

        *this = remainder; // Присваиваем остаток
        is_negative = original_sign; // Восстанавливаем исходный знак

        if (*this == BigInteger(0)) { // Если остаток 0, знак всегда положительный
            is_negative = false;
        }
        normalize();
        try_optimize();
        return *this;
    }

    BigInteger operator%(const BigInteger& other) const {
        BigInteger result = *this;
        result %= other;
        return result;
    }

    // --- Унарные операторы ---
    BigInteger operator+() const {
        return *this; // Унарный плюс ничего не меняет
    }

    BigInteger operator-() const {
        BigInteger result = *this;
        if (result == BigInteger(0)) { // -0 это 0
            result.is_negative = false;
        } else {
            result.is_negative = !result.is_negative;
        }
        // Если это small_value, то инвертируем его. Внимание к INT_MIN
        if (result.is_small_value) {
            if (result.small_value != INT_MIN) {
                result.small_value = -result.small_value;
            } else { // INT_MIN не может быть просто инвертирован в int
                result.ensure_big_format();
                result.is_negative = !result.is_negative; // Это будет 2147483648
            }
        }
        return result;
    }

    // Префиксные инкремент/декремент
    BigInteger& operator++() {
        return *this += BigInteger(1);
    }

    BigInteger& operator--() {
        return *this -= BigInteger(1);
    }

    // Постфиксные инкремент/декремент
    BigInteger operator++(int) {
        BigInteger temp = *this;
        ++(*this);
        return temp;
    }

    BigInteger operator--(int) {
        BigInteger temp = *this;
        --(*this);
        return temp;
    }

    // --- Битовые операторы (упрощенная реализация, побитовые операции над 32-битными блоками) ---
    // Для правильных битовых операций (например, отрицательных чисел) нужен двухкомпонентный формат
    // Эта реализация работает на "сырых" 32-битных блоках, что может быть не то, что ожидается
    // для отрицательных чисел в двухкомпонентном дополнении.
    // Если требуется точная реализация двухкомпонентного дополнения, это значительно сложнее.

    // Вспомогательная функция для расширения чисел до одинаковой длины для побитовых операций
    // Возвращает указатель на новый массив и его размер
    void get_twos_complement_representation(int* buffer, size_t buffer_size, bool& is_neg_out) const {
        BigInteger temp = *this;
        temp.ensure_big_format();

        is_neg_out = temp.is_negative;

        // Копируем абсолютное значение
        memset(buffer, 0, buffer_size * sizeof(int));
        size_t copy_len = temp.digits_size;
        if (copy_len > buffer_size) copy_len = buffer_size;
        memcpy(buffer, temp.digits, copy_len * sizeof(int));

        if (is_neg_out) {
            // Инвертируем биты
            for (size_t i = 0; i < buffer_size; ++i) {
                buffer[i] = ~buffer[i];
            }
            // Добавляем 1
            unsigned int carry = 1;
            for (size_t i = 0; i < buffer_size && carry; ++i) {
                unsigned long long sum = (unsigned int)buffer[i] + carry;
                buffer[i] = (int)sum;
                carry = sum >> get_bits_per_int();
            }
        }
    }

    // Вспомогательная функция для создания BigInteger из двухкомпонентного представления
    static BigInteger from_twos_complement_representation(const int* buffer, size_t buffer_size) {
        if (buffer_size == 0) return BigInteger(0);

        // Определяем знак по старшему биту самого старшего блока
        bool is_neg = ((unsigned int)buffer[buffer_size - 1] >> (get_bits_per_int() - 1)) != 0;

        int temp_buffer[MAX_BIGINT_DIGITS * 2]; // Временный буфер для модификации
        memcpy(temp_buffer, buffer, buffer_size * sizeof(int));

        if (is_neg) {
            // Вычитаем 1
            unsigned int borrow = 1;
            for (size_t i = 0; i < buffer_size && borrow; ++i) {
                if ((unsigned int)temp_buffer[i] >= borrow) {
                    temp_buffer[i] = (int)((unsigned int)temp_buffer[i] - borrow);
                    borrow = 0;
                } else {
                    temp_buffer[i] = (int)((1ULL << get_bits_per_int()) - borrow + (unsigned int)temp_buffer[i]);
                }
            }
            // Инвертируем биты
            for (size_t i = 0; i < buffer_size; ++i) {
                temp_buffer[i] = ~temp_buffer[i];
            }
        }

        BigInteger result;
        result.is_small_value = false;
        result.digits_size = buffer_size;
        memcpy(result.digits, temp_buffer, buffer_size * sizeof(int));
        result.is_negative = is_neg;

        result.normalize();
        result.try_optimize();
        return result;
    }


    BigInteger& operator&=(const BigInteger& other) {
        ensure_big_format();
        BigInteger b = other;
        b.ensure_big_format();

        size_t common_size = (digits_size > b.digits_size ? digits_size : b.digits_size);
        if (common_size > MAX_BIGINT_DIGITS) common_size = MAX_BIGINT_DIGITS;

        int a_twos[MAX_BIGINT_DIGITS * 2]; // Достаточно большие буферы для двухкомпонентного
        int b_twos[MAX_BIGINT_DIGITS * 2];
        bool a_neg, b_neg;

        get_twos_complement_representation(a_twos, common_size, a_neg);
        b.get_twos_complement_representation(b_twos, common_size, b_neg);

        for (size_t i = 0; i < common_size; ++i) {
            a_twos[i] &= b_twos[i];
        }

        *this = from_twos_complement_representation(a_twos, common_size);
        normalize();
        try_optimize();
        return *this;
    }

    BigInteger operator&(const BigInteger& other) const {
        BigInteger result = *this;
        result &= other;
        return result;
    }

    BigInteger& operator|=(const BigInteger& other) {
        ensure_big_format();
        BigInteger b = other;
        b.ensure_big_format();

        size_t common_size = (digits_size > b.digits_size ? digits_size : b.digits_size);
        if (common_size > MAX_BIGINT_DIGITS) common_size = MAX_BIGINT_DIGITS;

        int a_twos[MAX_BIGINT_DIGITS * 2];
        int b_twos[MAX_BIGINT_DIGITS * 2];
        bool a_neg, b_neg;

        get_twos_complement_representation(a_twos, common_size, a_neg);
        b.get_twos_complement_representation(b_twos, common_size, b_neg);

        for (size_t i = 0; i < common_size; ++i) {
            a_twos[i] |= b_twos[i];
        }

        *this = from_twos_complement_representation(a_twos, common_size);
        normalize();
        try_optimize();
        return *this;
    }

    BigInteger operator|(const BigInteger& other) const {
        BigInteger result = *this;
        result |= other;
        return result;
    }

    BigInteger& operator^=(const BigInteger& other) {
        ensure_big_format();
        BigInteger b = other;
        b.ensure_big_format();

        size_t common_size = (digits_size > b.digits_size ? digits_size : b.digits_size);
        if (common_size > MAX_BIGINT_DIGITS) common_size = MAX_BIGINT_DIGITS;

        int a_twos[MAX_BIGINT_DIGITS * 2];
        int b_twos[MAX_BIGINT_DIGITS * 2];
        bool a_neg, b_neg;

        get_twos_complement_representation(a_twos, common_size, a_neg);
        b.get_twos_complement_representation(b_twos, common_size, b_neg);

        for (size_t i = 0; i < common_size; ++i) {
            a_twos[i] ^= b_twos[i];
        }

        *this = from_twos_complement_representation(a_twos, common_size);
        normalize();
        try_optimize();
        return *this;
    }

    BigInteger operator^(const BigInteger& other) const {
        BigInteger result = *this;
        result ^= other;
        return result;
    }

    BigInteger operator~() const {
        BigInteger temp = *this;
        temp.ensure_big_format();

        // Расширим до достаточной длины, чтобы учесть потенциальное переполнение старшего бита
        size_t expanded_size = temp.digits_size + 1; // +1 для потенциального бита знака
        if (expanded_size > MAX_BIGINT_DIGITS * 2) expanded_size = MAX_BIGINT_DIGITS * 2; // Ограничение

        int twos_comp[MAX_BIGINT_DIGITS * 2];
        bool is_neg_ignored; // Нам тут не нужен этот флаг, так как мы инвертируем напрямую

        temp.get_twos_complement_representation(twos_comp, expanded_size, is_neg_ignored);

        for (size_t i = 0; i < expanded_size; ++i) {
            twos_comp[i] = ~twos_comp[i];
        }
        return from_twos_complement_representation(twos_comp, expanded_size);
    }


    BigInteger& operator<<=(int shift) {
        if (*this == BigInteger(0) || shift == 0) return *this;
        ensure_big_format();

        // Сдвиг на 0 не меняет число
        if (shift < 0) return *this >>= (-shift); // Для отрицательного сдвига - это правый сдвиг

        int bits_per_int = get_bits_per_int();
        size_t block_shift = shift / bits_per_int; // На сколько блоков сдвигаем
        int bit_shift = shift % bits_per_int;      // На сколько бит внутри блока сдвигаем

        // Проверка на переполнение
        if (digits_size + block_shift + (bit_shift > 0 ? 1 : 0) > MAX_BIGINT_DIGITS) {
            // Число стало слишком большим для MAX_BIGINT_DIGITS
            // Можно обрезать или выбросить исключение
            // В данном случае просто обрезаем, теряя старшие биты
            // Это не идеально, но соответствует ограничению массива
            digits_size = MAX_BIGINT_DIGITS;
            // Сбрасываем все старшие блоки
            for (size_t i = 0; i < MAX_BIGINT_DIGITS; ++i) digits[i] = 0; // Обнуляем
            is_negative = false; //Результат очень велик и возможно обрезался до 0.
            normalize();
            try_optimize();
            return *this;
        }

        // Выделение памяти для нового размера
        size_t new_size = digits_size + block_shift + (bit_shift > 0 ? 1 : 0);
        if (new_size == 0) new_size = 1; // Минимум 1 элемент для 0

        // Сдвиг блоков
        for (int i = (int)digits_size - 1; i >= 0; --i) {
            digits[i + block_shift] = digits[i];
        }
        // Заполнение младших блоков нулями
        for (size_t i = 0; i < block_shift; ++i) {
            digits[i] = 0;
        }
        digits_size += block_shift;


        // Сдвиг битов внутри блоков
        if (bit_shift > 0) {
            unsigned int carry = 0;
            for (size_t i = block_shift; i < digits_size; ++i) {
                unsigned int current_digit = (unsigned int)digits[i];
                digits[i] = (int)((current_digit << bit_shift) | carry);
                carry = current_digit >> (bits_per_int - bit_shift);
            }
            if (carry && digits_size < MAX_BIGINT_DIGITS) {
                digits[digits_size++] = (int)carry;
            }
        }
        normalize();
        try_optimize();
        return *this;
    }

    BigInteger operator<<(int shift) const {
        BigInteger result = *this;
        result <<= shift;
        return result;
    }

    BigInteger& operator>>=(int shift) {
        if (*this == BigInteger(0) || shift == 0) return *this;
        ensure_big_format();

        if (shift < 0) return *this <<= (-shift); // Для отрицательного сдвига - это левый сдвиг

        int bits_per_int = get_bits_per_int();
        size_t block_shift = shift / bits_per_int; // На сколько блоков сдвигаем
        int bit_shift = shift % bits_per_int;      // На сколько бит внутри блока сдвигаем

        if (block_shift >= digits_size) { // Если сдвиг больше или равен текущему размеру
            *this = BigInteger(0); // Результат будет 0
            return *this;
        }

        // Сдвиг блоков
        for (size_t i = 0; i < digits_size - block_shift; ++i) {
            digits[i] = digits[i + block_shift];
        }
        digits_size -= block_shift;
        // Заполнение старших блоков нулями (хотя normalize это уберет)
        for (size_t i = digits_size; i < MAX_BIGINT_DIGITS; ++i) {
            digits[i] = 0;
        }

        // Сдвиг битов внутри блоков
        if (bit_shift > 0) {
            unsigned int carry = 0;
            for (int i = (int)digits_size - 1; i >= 0; --i) {
                unsigned int current_digit = (unsigned int)digits[i];
                unsigned int new_carry = current_digit << (bits_per_int - bit_shift); // Биты, которые уйдут в следующий блок
                digits[i] = (int)((current_digit >> bit_shift) | carry);
                carry = new_carry;
            }
        }

        // Для отрицательных чисел, правый сдвиг должен быть арифметическим
        // (сохранять знак, заполняя старшие биты единицами)
        if (is_negative && !(*this == BigInteger(0))) {
            // Это сложная часть. Для корректного арифметического правого сдвига отрицательных чисел
            // нужно работать с двухкомпонентным дополнением.
            // Простая побитовая операция выше является логическим сдвигом.
            // TODO: Для правильного арифметического сдвига отрицательных чисел,
            // нужно сначала преобразовать в двухкомпонентное дополнение, сдвинуть,
            // а затем преобразовать обратно. Это выходит за рамки простой адаптации
            // и требует более глубокой переработки битовых операторов.
            // Пока что, эта реализация ведет себя как логический сдвиг для отрицательных чисел.
        }

        normalize();
        try_optimize();
        return *this;
    }

    BigInteger operator>>(int shift) const {
        BigInteger result = *this;
        result >>= shift;
        return result;
    }

    // --- Вывод (нужен ostream) ---
    // Этот метод не использует STL, но требует ostream, который является частью стандартной библиотеки.
    // Если требуется абсолютно не использовать STL, то нужно будет выводить посимвольно в буфер char*
    // и возвращать его, или передавать ostream как параметр.
    friend std::ostream& operator<<(std::ostream& os, const BigInteger& num) {
        if (num.is_small_value) {
            os << num.small_value;
            return os;
        }

        // Если число равно 0
        if (num.digits_size == 1 && num.digits[0] == 0) {
            os << "0";
            return os;
        }

        char buffer[MAX_BIGINT_DIGITS * 10 + 2]; // Достаточно для всех цифр + знак + '\0'
                                                // (приблизительно 10 десятичных цифр на 32 бита)
        int idx = 0;
        BigInteger temp = num;
        bool print_negative = temp.is_negative;
        temp.is_negative = false; // Работаем с абсолютным значением

        if (temp.digits_size == 1 && temp.digits[0] == 0) { // Если 0
            buffer[idx++] = '0';
        } else {
            // Алгоритм деления на 10 для получения десятичных цифр
            while (temp.digits_size > 1 || temp.digits[0] != 0) {
                int remainder = 0;
                unsigned long long current_block_val = 0;

                // Ручное деление на 10
                for (int i = (int)temp.digits_size - 1; i >= 0; --i) {
                    current_block_val = (current_block_val << BigInteger::get_bits_per_int()) | (unsigned int)temp.digits[i];
                    temp.digits[i] = (int)(current_block_val / 10);
                    remainder = (int)(current_block_val % 10);
                }
                temp.normalize(); // Убрать ведущие нули после деления

                buffer[idx++] = (char)('0' + remainder);
                if (idx >= sizeof(buffer) - 1) { // Проверка на переполнение буфера
                    // Можно выбросить исключение или усечь число
                    break;
                }
            }
        }

        if (print_negative) {
            buffer[idx++] = '-';
        }
        buffer[idx] = '\0'; // Нулевой символ

        // Вывести в обратном порядке
        for (int i = idx - 1; i >= 0; --i) {
            os << buffer[i];
        }

        return os;
    }

    /*std::istream& operator>>(std::istream& is, BigInteger& n) {
        std::string s;
        is >> s;
        n = BigInteger(s.c_str());
        return is;
    }*/

};
int main() {
    std::cout << "=== Демонстрация класса BigInteger ===" << std::endl;

    // 1. Тестирование конструкторов
    std::cout << "\n1. Тестирование конструкторов:" << std::endl;

    // Конструктор от массива (little endian)
    int arr[] = {123, 456, 789};
    BigInteger a(arr, 3);
    std::cout << "Из массива: " << a << std::endl;

    // Конструктор от строки в разных системах счисления
    BigInteger b("1234567890987654321", 10);
    BigInteger c("1010101010101010101", 2);
    BigInteger d("1234567890123456789", 16);

    std::cout << "Из строки (10): " << b << std::endl;
    std::cout << "Из строки (2): " << c << std::endl;
    std::cout << "Из строки (16): " << d << std::endl;

    // 2. Тестирование арифметических операций
    std::cout << "\n2. Тестирование арифметических операций:" << std::endl;
    BigInteger x("123456789012345");
    BigInteger y("45678904567");

    std::cout << "x = " << x << ", y = " << y << std::endl;
    std::cout << "x + y = " << (x + y) << std::endl;
    std::cout << "x - y = " << (x - y) << std::endl;
    std::cout << "x * y = " << (x * y) << std::endl;
    std::cout << "x / y = " << (x / y) << std::endl;
    std::cout << "x % y = " << (x % y) << std::endl;

    // 3. Тестирование операций сравнения
    std::cout << "\n3. Тестирование операций сравнения:" << std::endl;
    std::cout << "x == y: " << (x == y ? "true" : "false") << std::endl;
    std::cout << "x != y: " << (x != y ? "true" : "false") << std::endl;
    std::cout << "x > y: " << (x > y ? "true" : "false") << std::endl;
    std::cout << "x < y: " << (x < y ? "true" : "false") << std::endl;
    std::cout << "x >= y: " << (x >= y ? "true" : "false") << std::endl;
    std::cout << "x <= y: " << (x <= y ? "true" : "false") << std::endl;

    // 4. Тестирование инкремента/декремента
    std::cout << "\n4. Тестирование инкремента/декремента:" << std::endl;
    BigInteger z("34567890987654");
    std::cout << "z = " << z << std::endl;
    std::cout << "++z = " << (++z) << std::endl;
    std::cout << "z++ = " << (z++) << std::endl;
    std::cout << "z = " << z << std::endl;
    std::cout << "--z = " << (--z) << std::endl;
    std::cout << "z-- = " << (z--) << std::endl;
    std::cout << "z = " << z << std::endl;

    // 5. Тестирование поразрядных операций
    std::cout << "\n5. Тестирование поразрядных операций:" << std::endl;
    BigInteger p("100000000000000000000");   // 1111 в двоичной15
    BigInteger q("100000000000000000001");    // 0111 в двоичной7
    BigInteger neg("-100000000000000000000"); // отрицательное число (-1000 в двоичной)-8

    std::cout << "p = " << p << std::endl;
    std::cout << "q = " << q << std::endl;
    std::cout << "neg = " << neg << std::endl;

    std::cout << "p & q = " << (p & q) << std::endl;
    std::cout << "p | q = " << (p | q) << std::endl;
    std::cout << "p ^ q = " << (p ^ q) << std::endl;
    std::cout << "~p = " << (~p) << std::endl;
    std::cout << "~neg = " << (~neg) << std::endl;
    std::cout << "p & neg = " << (p & neg) << std::endl;
    std::cout << "neg | p = " << (neg | p) << std::endl;
    std::cout << "neg ^ p = " << (neg ^ p) << std::endl;

    // 6. Тестирование битовых сдвигов
    std::cout << "\n6. Тестирование битовых сдвигов:" << std::endl;
    BigInteger s("100000000000000000000");
    BigInteger neg_s("-100000000000000000000");
    std::cout << "s = " << s << ", neg_s = " << neg_s << std::endl;
    std::cout << "s << 2 = " << (s << 2) << std::endl;
    std::cout << "s >> 1 = " << (s >> 1) << std::endl;
    std::cout << "neg_s << 2 = " << (neg_s << 2) << std::endl;
    std::cout << "neg_s >> 1 = " << (neg_s >> 1) << std::endl;

    BigInteger big_pos("100000000000000000000");
    BigInteger big_neg("100000000000000000001");
    std::cout << "big_pos >> 2 = " << (big_pos >> 2) << std::endl;
    std::cout << "big_neg >> 2 = " << (big_neg >> 2) << std::endl;

    // 7. Тестирование с отрицательными числами
    std::cout << "\n7. Тестирование с отрицательными числами:" << std::endl;
    BigInteger neg1("-12345678987654321");
    BigInteger neg2("45612345678905678");

    std::cout << "neg1 = " << neg1 << std::endl;
    std::cout << "neg2 = " << neg2 << std::endl;
    std::cout << "neg1 + neg2 = " << (neg1 + neg2) << std::endl;
    std::cout << "neg1 - neg2 = " << (neg1 - neg2) << std::endl;
    std::cout << "neg1 * neg2 = " << (neg1 * neg2) << std::endl;

    // 8. Тестирование ввода/вывода для обычных интов
    std::cout << "\n8. Тестирование ввода/вывода для обычных интов:" << std::endl;
    BigInteger small_pos(42);
    BigInteger small_neg(-42);
    BigInteger zero_val(0);
    BigInteger max_int(INT_MAX);
    BigInteger min_int(INT_MIN);

    std::cout << "Малое положительное: " << small_pos << std::endl;
    std::cout << "Малое отрицательное: " << small_neg << std::endl;
    std::cout << "Ноль: " << zero_val << std::endl;
    std::cout << "INT_MAX: " << max_int << std::endl;
    std::cout << "INT_MIN: " << min_int << std::endl;

    std::cout << "\nТестирование операций с малыми числами:" << std::endl;
    std::cout << "42 + (-42) = " << (small_pos + small_neg) << std::endl;
    std::cout << "42 * (-42) = " << (small_pos * small_neg) << std::endl;
    std::cout << "42 & (-42) = " << (small_pos & small_neg) << std::endl;
    std::cout << "42 | (-42) = " << (small_pos | small_neg) << std::endl;
    std::cout << "42 ^ (-42) = " << (small_pos ^ small_neg) << std::endl;

    /*std::cout << "\nВведите число для тестирования ввода: ";
    BigInteger input;
    std::cin >> input;
    std::cout << "Вы ввели: " << input << std::endl;
*/
    return 0;
}
