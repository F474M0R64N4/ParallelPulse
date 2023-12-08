#ifndef THREADSGROUP_H
#define THREADSGROUP_H

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <stdexcept>
#include <future>

namespace ThreadsGroup {

	class ThreadGroup {
	public:
		ThreadGroup() = default;
		~ThreadGroup();

		void createGroup();
		bool addThread(std::function<void()> threadFunction);
		bool waitForAllExit(std::chrono::milliseconds timeout = std::chrono::milliseconds::zero());
		void closeGroup();

	private:
		std::vector<std::thread> threads_;
		std::mutex mutex_;
		std::condition_variable cv_;
		bool closed_ = false;

		void CloseTerminatedThreads();
	};

	ThreadGroup::~ThreadGroup() {
		closeGroup();
	}

	void ThreadGroup::createGroup() {
		std::unique_lock<std::mutex> lock(mutex_);
    		closed_ = false;
	}

	bool ThreadGroup::addThread(std::function<void()> threadFunction) {
		std::unique_lock<std::mutex> lock(mutex_);
		if (closed_) {
			return false; // Group is closed, cannot add more threads
		}

		try {
			threads_.emplace_back(threadFunction);
		}
		catch (const std::exception& e) {
			std::cerr << "Error adding thread: " << e.what() << std::endl;
			return false;
		}

		return true;
	}

	bool ThreadGroup::waitForAllExit(std::chrono::milliseconds timeout) {
		std::vector<std::shared_future<void>> futures;
		timeout = std::chrono::milliseconds::zero();
		{
			std::unique_lock<std::mutex> lock(mutex_);
			if (closed_) {
				return true; // Group is already closed
			}

			for (auto& thread : threads_) {
				if (thread.joinable()) {
					futures.emplace_back(std::async(std::launch::async, [&thread]() {
						thread.join();
						}));
				}
			}
		}

		for (auto& future : futures) {
			if (timeout != std::chrono::milliseconds::zero() &&
				future.wait_for(timeout) == std::future_status::timeout) {
				return false; // Timed out waiting for threads to finish
			}
		}

		return true;
	}

	void ThreadGroup::closeGroup() {
		std::unique_lock<std::mutex> lock(mutex_);
		if (closed_) {
			return; // Group is already closed
		}

		for (auto& thread : threads_) {
			if (thread.joinable()) {
				thread.join();
			}
		}

		threads_.clear();
		closed_ = true;
		cv_.notify_all();
	}

	void ThreadGroup::CloseTerminatedThreads() {
		std::unique_lock<std::mutex> lock(mutex_);
		auto it = std::remove_if(threads_.begin(), threads_.end(), [](const std::thread& thread) {
			return !thread.joinable() || thread.get_id() == std::thread::id();
			});

		threads_.erase(it, threads_.end());
		cv_.notify_all();
	}

}

#endif // THREADSGROUP_H
