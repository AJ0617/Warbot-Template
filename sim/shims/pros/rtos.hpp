#pragma once
#include <thread>
#include <mutex>
#include <chrono>
#include <functional>
#include <atomic>
#include <cstdint>

namespace pros {

inline void delay(std::uint32_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline std::uint32_t millis() {
    static auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return static_cast<std::uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count()
    );
}

class Mutex {
    std::mutex m_;
public:
    bool take(std::uint32_t /*timeout_ms*/ = UINT32_MAX) {
        m_.lock();
        return true;
    }
    bool give() {
        m_.unlock();
        return true;
    }
};

class Task {
    std::thread thread_;
public:
    Task() = default;

    template<typename Fn>
    explicit Task(Fn&& fn) {
        thread_ = std::thread(std::forward<Fn>(fn));
        thread_.detach();
    }

    // Cannot reliably stop a detached thread; the thread will eventually exit
    // on its own once the sim closes or its loop exits.
    void remove() {}

    static void delay(std::uint32_t ms) { pros::delay(ms); }
};

} // namespace pros
