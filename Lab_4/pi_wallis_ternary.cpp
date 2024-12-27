// pi_wallis_ternary.cpp
#include <cstdlib>
#include <string>
#include <algorithm>

// Вспомогательная функция для расчёта произведения по формуле Валлиса
float wallis_product(int K) {
    float product = 1.0f;
    for(int n = 1; n <= K; ++n) {
        product *= (4.0f * n * n) / (4.0f * n * n - 1.0f);
    }
    return product;
}

extern "C" {

// Функция расчёта числа Пи с использованием формулы Валлиса
float Pi(int K) {
    float pi_over_two = wallis_product(K);
    return pi_over_two * 2.0f;
}

// Функция перевода числа в троичную систему
char* translation(long x) {
    if (x == 0) {
        char* result = new char[2];
        result[0] = '0';
        result[1] = '\0';
        return result;
    }

    std::string ternary;
    long num = x;
    while (num > 0) {
        ternary += ('0' + (num % 3));
        num /= 3;
    }
    std::reverse(ternary.begin(), ternary.end());

    char* result = new char[ternary.size() + 1];
    std::copy(ternary.begin(), ternary.end(), result);
    result[ternary.size()] = '\0';
    return result;
}

} // extern "C"
