#include <iostream>
#include <string>
#include <cstdlib>

// Объявление внешних функций из библиотеки
extern "C" {
    float Pi(int K);
    char* translation(long x);
}

void handle_command_1() {
    // Обработка команды для расчета числа Пи
    int K;
    std::cout << "Введите количество итераций K для вычисления числа Пи: ";
    std::cin >> K;
    if (K <= 0) {
        std::cout << "K должно быть положительным числом." << std::endl;
        return;
    }
    float pi = Pi(K);
    std::cout << "Вычисленное число Пи: " << pi << std::endl;
}

void handle_command_2() {
    // Обработка команды для перевода числа в двоичную систему
    long x;
    std::cout << "Введите число для перевода в двоичную систему: ";
    std::cin >> x;
    char* result = translation(x);
    std::cout << "Результат перевода: " << result << std::endl;
    delete[] result;
}

int main() {
    std::string input;
    while (true) {
        std::cout << "Введите команду (1 для расчета числа Пи, 2 для перевода): ";
        std::getline(std::cin, input);
        if (input.empty()) continue;

        if (input[0] == '1') {
            handle_command_1();
        } else if (input[0] == '2') {
            handle_command_2();
        } else {
            std::cout << "Неизвестная команда." << std::endl;
        }
    }
    return 0;
}
