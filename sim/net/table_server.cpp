#include "table_server.hpp"
#include <cstdio>

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")
  using socklen_t = int;
  static int  close_sock(std::uintptr_t s) { return closesocket((SOCKET)s); }
  static const std::uintptr_t INVALID_SOCK = (std::uintptr_t)INVALID_SOCKET;
  static const int SEND_FLAGS = 0;
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  static int  close_sock(std::uintptr_t s) { return ::close((int)s); }
  static const std::uintptr_t INVALID_SOCK = (std::uintptr_t)(~0);
  #ifdef MSG_NOSIGNAL
    static const int SEND_FLAGS = MSG_NOSIGNAL;
  #else
    static const int SEND_FLAGS = 0;
  #endif
#endif

namespace {
bool ensureWsa() {
#ifdef _WIN32
    static bool inited = false;
    if (!inited) {
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return false;
        inited = true;
    }
#endif
    return true;
}
} // namespace

TableServer::~TableServer() { stop(); }

bool TableServer::start(int port) {
    if (running_) return true;
    if (!ensureWsa()) return false;
    port_ = port;

    std::uintptr_t s = (std::uintptr_t)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCK) return false;

    int yes = 1;
    setsockopt((SOCKET)s, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons((unsigned short)port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (bind((SOCKET)s, (sockaddr*)&addr, sizeof(addr)) != 0) { close_sock(s); return false; }
    if (listen((SOCKET)s, 8) != 0)                            { close_sock(s); return false; }

    listenSock_ = s;
    running_    = true;
    acceptThread_ = std::thread(&TableServer::acceptLoop, this);
    return true;
}

void TableServer::acceptLoop() {
    while (running_) {
        std::uintptr_t c = (std::uintptr_t)accept((SOCKET)listenSock_, nullptr, nullptr);
        if (c == INVALID_SOCK) {
            if (!running_) break;
            continue;
        }
        std::lock_guard<std::mutex> lk(clientsMutex_);
        clients_.push_back(c);
    }
}

void TableServer::broadcast(const std::string& frame) {
    std::lock_guard<std::mutex> lk(clientsMutex_);
    for (size_t i = 0; i < clients_.size();) {
        int sent = send((SOCKET)clients_[i], frame.data(), (int)frame.size(), SEND_FLAGS);
        if (sent < 0) {
            close_sock(clients_[i]);
            clients_[i] = clients_.back();
            clients_.pop_back();
        } else {
            ++i;
        }
    }
}

int TableServer::clientCount() {
    std::lock_guard<std::mutex> lk(clientsMutex_);
    return (int)clients_.size();
}

void TableServer::stop() {
    if (!running_) return;
    running_ = false;
    if (listenSock_ != INVALID_SOCK) {
        close_sock(listenSock_);
        listenSock_ = INVALID_SOCK;
    }
    if (acceptThread_.joinable()) acceptThread_.join();
    std::lock_guard<std::mutex> lk(clientsMutex_);
    for (auto c : clients_) close_sock(c);
    clients_.clear();
}
