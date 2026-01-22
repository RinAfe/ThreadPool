#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t ThreadCount) {
	for (int i = 0; i < ThreadCount; i++) {
		workers.emplace_back(
			[this] {
				while (true) {
					std::function<void()> task;
					{
						std::unique_lock<std::mutex> lock(queue_mutex);
						condition.wait(lock, [this] {
							return stop || !tasks.empty();
							});

						if (stop && tasks.empty()) return;

						task = std::move(tasks.front());
						tasks.pop();
					}
					try {
						task();
					}
					catch (const std::exception& e) {
						std::lock_guard<std::mutex> lock(queue_mutex);
						std::cerr << "Exception in thread pool task: " << e.what() << std::endl;
					}
					catch (...) {
						std::lock_guard<std::mutex> lock(queue_mutex);
						std::cerr << "Unknown exception in thread pool task" << std::endl;
					}
				}
			}
		);
	}
}

void ThreadPool::submit(std::function<void()> task) {
	{
		std::lock_guard<std::mutex> lock(queue_mutex);

		if (stop) {
			throw std::runtime_error("ThreadPool is stopped!");
		}

		tasks.emplace(std::move(task));
	}

	condition.notify_one();
}

ThreadPool::~ThreadPool() {
	{
		std::lock_guard<std::mutex> lock(queue_mutex);
		stop = true;
	}

	condition.notify_all();
	for (std::thread& worker : workers)
		worker.join();
}