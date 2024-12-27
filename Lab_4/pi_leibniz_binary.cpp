#include <cstdlib>
#include <string>
#include <algorithm>

extern "C" {

// Функция для вычисления числа Пи с использованием формулы Лейбница
float Pi(int K) {
    float pi = 0.0f;
    for (int i = 0; i < K; ++i) {
        if (i % 2 == 0) {
            pi += 4.0f / (2.0f * i + 1.0f);
        } else {
            pi -= 4.0f / (2.0f * i + 1.0f);
        }
    }
    return pi;
}

// Функция для перевода числа в двоичную систему
char* translation(long x) {
    if (x == 0) {
        char* result = new char[2];
        result[0] = '0';
        result[1] = '\0';
        return result;
    }

    std::string binary;
    long num = x;
    while (num > 0) {
        binary += (num % 2) ? '1' : '0';
        num /= 2;
    }
    std::reverse(binary.begin(), binary.end());

    char* result = new char[binary.size() + 1];
    std::copy(binary.begin(), binary.end(), result);
    result[binary.size()] = '\0';
    return result;
}

} // extern "C"
