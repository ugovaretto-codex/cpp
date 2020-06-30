//#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <iostream>
#include <random>
#include <thread>
#include <vector>
//Note: GCC does not seem to support hardware_*_interference_size
//#include <memory>
#include <new>

// CHANGE TO ACTUAL VALUE
constexpr std::size_t hardware_destructive_interference_size = 64;
constexpr std::size_t hardware_constructive_interference_size = 64;

constexpr unsigned kTimingTrialsToComputeAverage = 100;
constexpr unsigned kInnerLoopTrials = 1000000;

typedef unsigned useless_result_t;
typedef double elapsed_secs_t;

//////// CODE TO BE SAMPLED:

// wraps an int, default alignment allows false-sharing
struct naive_int {
    int value;
};
static_assert(alignof(naive_int) < hardware_destructive_interference_size, "");

// wraps an int, cache alignment prevents false-sharing
struct cache_int {
    alignas(hardware_destructive_interference_size) int value;
};
static_assert(alignof(cache_int) == hardware_destructive_interference_size, "");

// wraps a pair of int, purposefully pushes them too far apart for true-sharing
struct bad_pair {
    int first;
    char padding[hardware_constructive_interference_size];
    int second;
};
static_assert(sizeof(bad_pair) > hardware_constructive_interference_size, "");

// wraps a pair of int, ensures they fit nicely together for true-sharing
struct good_pair {
    int first;
    int second;
};
static_assert(sizeof(good_pair) <= hardware_constructive_interference_size, "");

// accesses a specific array element many times
template <typename T, typename Latch>
useless_result_t sample_array_threadfunc(Latch& latch, unsigned thread_index,
                                         T& vec) {
    // prepare for computation
    std::random_device rd;
    std::mt19937 mt{rd()};
    std::uniform_int_distribution<int> dist{0, 4096};

    auto& element = vec[vec.size() / 2 + thread_index];

    latch.count_down_and_wait();

    // compute
    for (unsigned trial = 0; trial != kInnerLoopTrials; ++trial) {
        element.value = dist(mt);
    }

    return static_cast<useless_result_t>(element.value);
}

// accesses a pair's elements many times
template <typename T, typename Latch>
useless_result_t sample_pair_threadfunc(Latch& latch, unsigned thread_index,
                                        T& pair) {
    // prepare for computation
    std::random_device rd;
    std::mt19937 mt{rd()};
    std::uniform_int_distribution<int> dist{0, 4096};

    latch.count_down_and_wait();

    // compute
    for (unsigned trial = 0; trial != kInnerLoopTrials; ++trial) {
        pair.first = dist(mt);
        pair.second = dist(mt);
    }

    return static_cast<useless_result_t>(pair.first) +
           static_cast<useless_result_t>(pair.second);
}

//////// UTILITIES:

// utility: allow threads to wait until everyone is ready
class threadlatch {
   public:
    explicit threadlatch(const std::size_t count) : count_{count} {}

    void count_down_and_wait() {
        std::unique_lock<std::mutex> lock{mutex_};
        if (--count_ == 0) {
            cv_.notify_all();
        } else {
            cv_.wait(lock, [&] { return count_ == 0; });
        }
    }

   private:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::size_t count_;
};

// utility: runs a given function in N threads
std::tuple<useless_result_t, elapsed_secs_t> run_threads(
    const std::function<useless_result_t(threadlatch&, unsigned)>& func,
    const unsigned num_threads) {
    threadlatch latch{num_threads + 1};

    std::vector<std::future<useless_result_t>> futures;
    std::vector<std::thread> threads;
    for (unsigned thread_index = 0; thread_index != num_threads;
         ++thread_index) {
        std::packaged_task<useless_result_t()> task{
            std::bind(func, std::ref(latch), thread_index)};

        futures.push_back(task.get_future());
        threads.push_back(std::thread(std::move(task)));
    }

    const auto starttime = std::chrono::high_resolution_clock::now();

    latch.count_down_and_wait();
    for (auto& thread : threads) {
        thread.join();
    }

    const auto endtime = std::chrono::high_resolution_clock::now();
    const auto elapsed =
        std::chrono::duration_cast<std::chrono::duration<double>>(endtime -
                                                                  starttime)
            .count();

    useless_result_t result = 0;
    for (auto& future : futures) {
        result += future.get();
    }

    return std::make_tuple(result, elapsed);
}

// utility: sample the time it takes to run func on N threads
void run_tests(
    const std::function<useless_result_t(threadlatch&, unsigned)>& func,
    const unsigned num_threads) {
    useless_result_t final_result = 0;
    double avgtime = 0.0;
    for (unsigned trial = 0; trial != kTimingTrialsToComputeAverage; ++trial) {
        const auto result_and_elapsed = run_threads(func, num_threads);
        const auto result = std::get<useless_result_t>(result_and_elapsed);
        const auto elapsed = std::get<elapsed_secs_t>(result_and_elapsed);

        final_result += result;
        avgtime = (avgtime * trial + elapsed) / (trial + 1);
    }

    std::cout << "Average time: " << avgtime
              << " seconds, useless result: " << final_result << std::endl;
}

int main() {
    const auto cores = std::thread::hardware_concurrency();
    std::cout << "Hardware concurrency: " << cores << std::endl;

    std::cout << "sizeof(naive_int): " << sizeof(naive_int) << std::endl;
    std::cout << "alignof(naive_int): " << alignof(naive_int) << std::endl;
    std::cout << "sizeof(cache_int): " << sizeof(cache_int) << std::endl;
    std::cout << "alignof(cache_int): " << alignof(cache_int) << std::endl;
    std::cout << "sizeof(bad_pair): " << sizeof(bad_pair) << std::endl;
    std::cout << "alignof(bad_pair): " << alignof(bad_pair) << std::endl;
    std::cout << "sizeof(good_pair): " << sizeof(good_pair) << std::endl;
    std::cout << "alignof(good_pair): " << alignof(good_pair) << std::endl;

    {
        std::cout << "Running naive_int test." << std::endl;

        std::vector<naive_int> vec;
        vec.resize((1u << 28) / sizeof(naive_int));  // allocate 256 mibibytes

        run_tests(
            [&](threadlatch& latch, unsigned thread_index) {
                return sample_array_threadfunc(latch, thread_index, vec);
            },
            cores);
    }
    {
        std::cout << "Running cache_int test." << std::endl;

        std::vector<cache_int> vec;
        vec.resize((1u << 28) / sizeof(cache_int));  // allocate 256 mibibytes

        run_tests(
            [&](threadlatch& latch, unsigned thread_index) {
                return sample_array_threadfunc(latch, thread_index, vec);
            },
            cores);
    }
    {
        std::cout << "Running bad_pair test." << std::endl;

        bad_pair p;

        run_tests(
            [&](threadlatch& latch, unsigned thread_index) {
                return sample_pair_threadfunc(latch, thread_index, p);
            },
            cores);
    }
    {
        std::cout << "Running good_pair test." << std::endl;

        good_pair p;

        run_tests(
            [&](threadlatch& latch, unsigned thread_index) {
                return sample_pair_threadfunc(latch, thread_index, p);
            },
            cores);
    }
}