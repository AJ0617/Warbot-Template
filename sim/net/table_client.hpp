#pragma once
// Viewer-side subscriber: connects to the host's table server, reads snapshot
// frames on a background thread, and exposes the most recent RenderState.
#include "renderer.hpp"
#include <cstdint>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>

class TableClient {
public:
    ~TableClient();

    // Spawn the connect/reconnect + reader thread (non-blocking).
    void start(const std::string& host, int port);
    // Copy the latest snapshot into out. Returns false until the first frame arrives.
    bool latest(RenderState& out);
    bool connected() const { return connected_.load(); }
    void stop();

private:
    void run();

    std::string        host_;
    int                port_ = 0;
    std::thread        thread_;
    std::mutex         mutex_;
    RenderState        latest_;
    std::atomic<bool>  hasData_{false};
    std::atomic<bool>  connected_{false};
    std::atomic<bool>  running_{false};
    std::atomic<std::uintptr_t> sock_{~std::uintptr_t(0)};
};
