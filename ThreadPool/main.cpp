#include "ThreadPool.h"
#include <iostream>
#include <chrono>
#include <atomic>
#include <future>

inline std::mutex cout_mutex;

// Простой тест - выполнение задач в пуле потоков
void testBasicFunctionality() {
    std::cout << "=== Test 1: Basic Functionality ===" << std::endl;

    ThreadPool pool(4);
    std::atomic<int> counter{ 0 };

    for (int i = 0; i < 10; ++i) {
        pool.submit([i, &counter]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Task " << i << " executed in thread "
                << std::this_thread::get_id() << std::endl;
            ++counter;
            });
    }

    // Даем время на выполнение задач
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Counter value: " << counter.load() << " (expected: 10)" << std::endl;
}

// Тест с возвратом значений через promise/future
void testWithReturnValues() {
    std::cout << "\n=== Test 2: Tasks with Return Values ===" << std::endl;

    ThreadPool pool(2);
    std::vector<std::future<int>> futures;

    for (int i = 0; i < 5; ++i) {
        auto promise = std::make_shared<std::promise<int>>();
        futures.push_back(promise->get_future());

        pool.submit([i, promise]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            int result = i * i;
            promise->set_value(result);
            });
    }

    for (int i = 0; i < futures.size(); ++i) {
        int result = futures[i].get();
        std::cout << "Task " << i << " returned: " << result
            << " (expected: " << i * i << ")" << std::endl;
    }
}

// Тест обработки исключений
void testExceptionHandling() {
    std::cout << "\n=== Test 3: Exception Handling ===" << std::endl;

    ThreadPool pool(2);
    std::atomic<bool> exception_caught{ false };

    // Задача, которая бросает исключение
    pool.submit([]() {
        throw std::runtime_error("Test exception from thread pool task!");
        });

    // Нормальная задача
    pool.submit([]() {
        std::cout << "Normal task executed successfully" << std::endl;
        });

    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Note: Exceptions in tasks should be caught to avoid termination" << std::endl;
}

// Тест производительности - много мелких задач
void testPerformance() {
    std::cout << "\n=== Test 4: Performance Test ===" << std::endl;

    const int TASK_COUNT = 1000;
    ThreadPool pool(8);
    std::atomic<int> completed_tasks{ 0 };

    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < TASK_COUNT; ++i) {
        pool.submit([&completed_tasks]() {
            // Имитация небольшой работы
            std::this_thread::sleep_for(std::chrono::microseconds(10));
            ++completed_tasks;
            });
    }

    // Ждем завершения всех задач
    while (completed_tasks.load() < TASK_COUNT) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "Completed " << TASK_COUNT << " tasks in "
        << duration.count() << " ms" << std::endl;
    std::cout << "Throughput: " << (TASK_COUNT * 1000.0 / duration.count())
        << " tasks/second" << std::endl;
}

// Тест деструктора и остановки пула
void testDestructor() {
    std::cout << "\n=== Test 5: Destructor Test ===" << std::endl;

    std::atomic<int> tasks_completed{ 0 };

    {
        ThreadPool pool(4);

        // Отправляем задачи, которые выполняются долго
        for (int i = 0; i < 8; ++i) {
            pool.submit([i, &tasks_completed]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                std::cout << "Long task " << i << " completed" << std::endl;
                ++tasks_completed;
                });
        }

        // Отправляем еще задачи
        for (int i = 8; i < 12; ++i) {
            pool.submit([i, &tasks_completed]() {
                std::cout << "Quick task " << i << " completed" << std::endl;
                ++tasks_completed;
                });
        }

        std::cout << "Exiting scope, ThreadPool destructor should be called..." << std::endl;
    } // Здесь вызывается деструктор ThreadPool

    std::cout << "Tasks completed: " << tasks_completed.load()
        << " (some tasks might not complete due to destructor)" << std::endl;
}

// Тест с разным количеством потоков
void testDifferentThreadCounts() {
    std::cout << "\n=== Test 6: Different Thread Counts ===" << std::endl;

    std::vector<size_t> thread_counts = { 1, 2, 4, 8 };

    for (size_t thread_count : thread_counts) {
        std::cout << "\nTesting with " << thread_count << " threads:" << std::endl;

        ThreadPool pool(thread_count);
        std::atomic<int> counter{ 0 };
        const int TASKS = 20;

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < TASKS; ++i) {
            pool.submit([&counter]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                ++counter;
                });
        }

        // Ждем завершения всех задач
        while (counter.load() < TASKS) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        std::cout << "Time: " << duration.count() << " ms" << std::endl;
    }
}

// Главная функция с меню тестов
int main() {
    std::cout << "ThreadPool Testing Suite\n" << std::endl;

    try {
        // Запускаем все тесты
        testBasicFunctionality();
        testWithReturnValues();
        testExceptionHandling();
        testPerformance();
        testDestructor();
        testDifferentThreadCounts();

        std::cout << "\n=== All tests completed ===" << std::endl;

    }
    catch (const std::exception& e) {
        std::cerr << "Error during testing: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}