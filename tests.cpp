#include <gtest/gtest.h> // Include Google Test header
#include "groupthread_lib.h"

// Test case for adding threads to the group
TEST(ThreadGroupTest, AddThreadTest) {
	ThreadsGroup::ThreadGroup threadGroup;

	threadGroup.createGroup();

	// Add a thread to the group
	bool added = threadGroup.addThread([]() {
		// Thread function
		});

	EXPECT_TRUE(added);

	// Close the group
	threadGroup.closeGroup();
}

// Test case for waiting for all threads to exit
TEST(ThreadGroupTest, WaitForAllExitTest) {
	ThreadsGroup::ThreadGroup threadGroup;

	threadGroup.createGroup();

	// Add a thread to the group that sleeps for 2 seconds
	threadGroup.addThread([]() {
		std::this_thread::sleep_for(std::chrono::seconds(2));
		});

	// Wait for all threads to exit with a timeout of 3 seconds
	bool result = threadGroup.waitForAllExit(std::chrono::seconds(3));

	EXPECT_TRUE(result);

	// Close the group
	threadGroup.closeGroup();
}

// Test case for adding threads after closing the group
TEST(ThreadGroupTest, AddThreadAfterCloseTest) {
	ThreadsGroup::ThreadGroup threadGroup;

	threadGroup.createGroup();

	// Close the group
	threadGroup.closeGroup();

	// Attempt to add a thread after closing the group
	bool added = threadGroup.addThread([]() {
		// Thread function
		});

	EXPECT_FALSE(added);
}

// Test case for waiting for all threads to exit when some threads are not joinable
TEST(ThreadGroupTest, WaitForAllExitWithNonJoinableThreadsTest) {
	ThreadsGroup::ThreadGroup threadGroup;

	threadGroup.createGroup();

	// Add a thread to the group that is not joinable
	std::thread nonJoinableThread([]() {
		// Thread function
		});

	// Wait for all threads to exit with a timeout of 3 seconds
	bool result = threadGroup.waitForAllExit(std::chrono::seconds(3));

	EXPECT_TRUE(result);

	// Close the group
	threadGroup.closeGroup();

	// Clean up the non-joinable thread
	if (nonJoinableThread.joinable()) {
		nonJoinableThread.join();
	}
}
// Test case for creating a thread pool with 25 elements
TEST(ThreadPoolTest, CreateThreadPoolWith25Elements) {
	// Create a thread pool with 25 elements
	ThreadsGroup::ThreadGroup threadPool;

	threadPool.createGroup();

	// Attempt to add 25 threads to the pool
	for (int i = 0; i < 25; ++i) {
		bool added = threadPool.addThread([]() {
			// Thread function
			});

		EXPECT_TRUE(added);
	}

	// Close the pool
	threadPool.closeGroup();
}
TEST(ThreadPoolTest, AddThreadAfterCloseTest) {
	ThreadsGroup::ThreadGroup threadPool;
	threadPool.createGroup();
	threadPool.closeGroup();

	bool added = threadPool.addThread([]() {
		// Thread function
		});

	EXPECT_FALSE(added);
}

//TEST(ThreadPoolTest, ExceptionHandlingTest) {
//	ThreadsGroup::ThreadGroup threadPool;
//	threadPool.createGroup();
//
//	bool added = threadPool.addThread([]() {
//		throw std::runtime_error("Test exception");
//		});
//
//	EXPECT_FALSE(added); // Expecting thread addition to fail
//
//	// Close the pool
//	threadPool.closeGroup();
//}
//TEST(ThreadPoolTest, WaitForAllExitTimeoutTest) {
//	ThreadsGroup::ThreadGroup threadPool;
//	threadPool.createGroup();
//
//	// Add a thread to the pool that doesn't exit
//	threadPool.addThread([]() {
//		std::this_thread::sleep_for(std::chrono::seconds(10));
//		});
//
//	// Wait for all threads to exit with a timeout of 1 second
//	bool result = threadPool.waitForAllExit(std::chrono::seconds(1));
//
//	EXPECT_FALSE(result); // Expecting timeout
//
//	// Close the pool
//	threadPool.closeGroup();
//}
TEST(ThreadPoolTest, CloseEmptyThreadPoolTest) {
	ThreadsGroup::ThreadGroup threadPool;
	threadPool.createGroup();

	// Close the pool without adding any threads
	threadPool.closeGroup();

	// No assertion needed, just ensure there are no errors
}
TEST(ThreadPoolTest, LargeThreadPoolTest) {
	ThreadsGroup::ThreadGroup threadPool;
	threadPool.createGroup();

	// Add a large number of threads to the pool
	for (int i = 0; i < 100; ++i) {
		bool added = threadPool.addThread([]() {
			// Thread function
			});

		EXPECT_TRUE(added);
	}

	// Wait for all threads to exit
	bool result = threadPool.waitForAllExit();

	EXPECT_TRUE(result);

	// Close the pool
	threadPool.closeGroup();
}

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
