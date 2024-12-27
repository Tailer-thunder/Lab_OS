#include <iostream>
#include <string>
#include <vector>
#include <dlfcn.h>
#include <cstdlib>
#include <limits>

// Типы функций для динамических библиотек
typedef float (*Pi_func)(int);
typedef char* (*translation_func)(long);

int main() {
    // Пути к библиотекам
    const char* lib1_path = "./libleibniz_binary.so";
    const char* lib2_path = "./libwallis_ternary.so";

    // Загрузка библиотеки libleibniz_binary.so
    void* handle1 = dlopen(lib1_path, RTLD_LAZY);
    if (!handle1) {
        std::cerr << "Не удалось загрузить библиотеку " << lib1_path << ": " << dlerror() << std::endl;
        return 1;
    }

    // Загрузка библиотеки libwallis_ternary.so
    void* handle2 = dlopen(lib2_path, RTLD_LAZY);
    if (!handle2) {
        std::cerr << "Не удалось загрузить библиотеку " << lib2_path << ": " << dlerror() << std::endl;
        dlclose(handle1);
        return 1;
    }

    // Получаем указатели на функции Pi и translation из первой библиотеки
    void* current_handle = handle1;
    Pi_func Pi = (Pi_func) dlsym(current_handle, "Pi");
    translation_func translation = (translation_func) dlsym(current_handle, "translation");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "Ошибка получения символа: " << dlsym_error << std::endl;
        dlclose(handle1);
        dlclose(handle2);
        return 1;
    }

    std::string input;
    while (true) {
        std::cout << "Введите команду (0 для переключения, 1 для расчета числа Пи, 2 для перевода): ";
        std::getline(std::cin, input);
        if (input.empty()) continue;

        if (input[0] == '0') {
            // Переключаемся на другую библиотеку
            if (current_handle == handle1) {
                current_handle = handle2;
                std::cout << "Переключено на библиотеку wallis_ternary." << std::endl;
            } else {
                current_handle = handle1;
                std::cout << "Переключено на библиотеку leibniz_binary." << std::endl;
            }

            // Загружаем новые символы Pi и translation
            Pi = (Pi_func) dlsym(current_handle, "Pi");
            translation = (translation_func) dlsym(current_handle, "translation");
            dlsym_error = dlerror();
            if (dlsym_error) {
                std::cerr << "Ошибка загрузки символов после переключения: " << dlsym_error << std::endl;
                dlclose(handle1);
                dlclose(handle2);
                return 1;
            }
        } else if (input[0] == '1') {
            // Расчет числа Пи
            int K;
            std::cout << "Введите количество итераций K для вычисления числа Пи: ";
            std::cin >> K;
            if (K <= 0) {
                std::cout << "K должно быть положительным числом." << std::endl;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Очищаем ввод
                continue;
            }
            float pi = Pi(K);
            std::cout << "Вычисленное число Пи: " << pi << std::endl;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Очищаем ввод
        } else if (input[0] == '2') {
            // Перевод числа в двоичный формат
            long x;
            std::cout << "Введите число для перевода в двоичную систему: ";
            std::cin >> x;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Очищаем ввод
            char* result = translation(x);
            std::cout << "Результат перевода: " << result << std::endl;
            delete[] result;
        } else {
            std::cout << "Неизвестная команда." << std::endl;
        }
    }

    // Закрываем библиотеки
    dlclose(handle1);
    dlclose(handle2);
    return 0;
}
