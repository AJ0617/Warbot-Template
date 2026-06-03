#pragma once
// Host-side broadcast server: accepts viewer connections on 127.0.0.1:<port>
// and pushes serialized snapshot frames to all of them.
#include <cstdint>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>

class TableServer {
public:
    ~TableServer();

    // Bind + listen on 127.0.0.1:port and start accepting clients. Returns false on failure.
    bool start(int port);
    // Send one complete frame to every connected client; drops dead sockets.
    void broadcast(const std::string& frame);
    void stop();

    int clientCount();

private:
    void acceptLoop();

    std::uintptr_t      listenSock_ = ~std::uintptr_t(0); // INVALID_SOCKET
    std::thread         acceptThread_;
    std::mutex          clientsMutex_;
    std::vector<std::uintptr_t> clients_;
    std::atomic<bool>   running_{false};
    int                 port_ = 0;
};
