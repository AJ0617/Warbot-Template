#include "table_client.hpp"
#include "protocol.hpp"
#include <chrono>

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")
  static int  close_sock(std::uintptr_t s) { return closesocket((SOCKET)s); }
  static const std::uintptr_t INVALID_SOCK = (std::uintptr_t)INVALID_SOCKET;
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  static int  close_sock(std::uintptr_t s) { return ::close((int)s); }
  static const std::uintptr_t INVALID_SOCK = (std::uintptr_t)(~0);
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

TableClient::~TableClient() { stop(); }

void TableClient::start(const std::string& host, int port) {
    if (running_) return;
    host_    = host;
    port_    = port;
    running_ = true;
    thread_  = std::thread(&TableClient::run, this);
}

void TableClient::run() {
    ensureWsa();
    while (running_) {
        std::uintptr_t s = (std::uintptr_t)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s == INVALID_SOCK) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port   = htons((unsigned short)port_);
        inet_pton(AF_INET, host_.c_str(), &addr.sin_addr);

        if (connect((SOCKET)s, (sockaddr*)&addr, sizeof(addr)) != 0) {
            close_sock(s);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }
        connected_ = true;
        sock_ = s;

        std::string buf;
        char chunk[4096];
        while (running_) {
            int n = recv((SOCKET)s, chunk, sizeof(chunk), 0);
            if (n <= 0) break;
            buf.append(chunk, n);

            // Split out complete frames terminated by a line containing only '~'.
            size_t pos;
            while ((pos = buf.find("~\n")) != std::string::npos) {
                std::string frame = buf.substr(0, pos);
                buf.erase(0, pos + 2);
                RenderState rs;
                if (simnet::parse(frame, rs)) {
                    std::lock_guard<std::mutex> lk(mutex_);
                    latest_  = std::move(rs);
                    hasData_ = true;
                }
            }
        }
        connected_ = false;
        sock_ = INVALID_SOCK;
        close_sock(s);
        if (running_) std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

bool TableClient::latest(RenderState& out) {
    if (!hasData_) return false;
    std::lock_guard<std::mutex> lk(mutex_);
    out = latest_;
    return true;
}

void TableClient::stop() {
    if (!running_) return;
    running_ = false;
    std::uintptr_t s = sock_.load();
    if (s != INVALID_SOCK) close_sock(s);  // interrupt a blocking recv
    if (thread_.joinable()) thread_.join();
}
