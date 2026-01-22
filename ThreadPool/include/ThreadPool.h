#pragma once

#include <iostream>
#include <functional>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <exception>

class ThreadPool {

	std::vector<std::thread> workers; // наши потоки
	std::queue<std::function<void()>> tasks; // сюда задачки будут поступать в очередь 
	std::mutex queue_mutex; // мьютекс для очереди
	std::condition_variable condition; // ожидающая переменная 
	bool stop = false; //остановка треадпула

public:

	explicit ThreadPool(size_t ThreadCount);
	~ThreadPool();

	void submit(std::function<void()> task);

	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;
};

