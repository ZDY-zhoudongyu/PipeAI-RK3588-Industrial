#include "pipeline/blocking_queue.hpp"

#include <atomic>
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

int main() {
    using namespace std::chrono_literals;
    pipeai::BlockingQueue<int> q(4);

    std::vector<int> consumed;
    std::thread consumer([&] {
        while (auto value = q.pop()) {
            consumed.push_back(*value);
        }
    });

    for (int i = 0; i < 100; ++i) {
        assert(q.push(i));
    }
    q.close();
    consumer.join();

    assert(consumed.size() == 100);
    for (int i = 0; i < 100; ++i) {
        assert(consumed[static_cast<std::size_t>(i)] == i);
    }
    assert(!q.push(101));
    assert(!q.pop().has_value());

    pipeai::BlockingQueue<int> timeout_q(1);
    assert(timeout_q.push(1));
    const auto t0 = std::chrono::steady_clock::now();
    assert(!timeout_q.push_for(2, 20ms));
    const auto elapsed = std::chrono::steady_clock::now() - t0;
    assert(elapsed >= 15ms);
    timeout_q.close();

    std::cout << "blocking_queue tests passed\n";
    return 0;
}
