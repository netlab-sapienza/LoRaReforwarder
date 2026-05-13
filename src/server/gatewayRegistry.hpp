#ifndef LORAREFORWARDER_GATEWAY_REGISTRY_HPP
#define LORAREFORWARDER_GATEWAY_REGISTRY_HPP

#include <string>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <thread>
#include <atomic>
#include <functional>
#include <cstdint>

struct GatewayEntry {
    std::string gatewayIp;
    uint16_t gatewayPort;
    std::string serverIp;
    uint16_t serverPort;
    std::chrono::steady_clock::time_point lastSeen;
};

class GatewayRegistry {
public:
    using ResponseCallback = std::function<void(const uint8_t*, int, std::string, uint16_t, GatewayEntry)>;

    explicit GatewayRegistry(int ttlSeconds = 10, int cleanupIntervalMs = 5000)
        : ttl_(std::chrono::seconds(ttlSeconds)),
          cleanupInterval_(std::chrono::milliseconds(cleanupIntervalMs)),
          running_(true) {
        cleanupThread_ = std::thread(&GatewayRegistry::cleanupLoop, this);
    }

    ~GatewayRegistry() {
        running_ = false;
        if (cleanupThread_.joinable()) {
            cleanupThread_.join();
        }
    }

    void registerGateway(const std::string& key, const GatewayEntry& entry) {
        std::lock_guard<std::mutex> lock(mutex_);
        entries_[key] = entry;
    }

    GatewayEntry* lookupGateway(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = entries_.find(key);
        if (it != entries_.end()) {
            it->second.lastSeen = std::chrono::steady_clock::now();
            return &it->second;
        }
        return nullptr;
    }

    void removeGateway(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        entries_.erase(key);
    }

    void setResponseCallback(ResponseCallback cb) {
        responseCallback_ = cb;
    }

 private:
    std::unordered_map<std::string, GatewayEntry> entries_;
    std::mutex mutex_;
    std::atomic<bool> running_;
    std::thread cleanupThread_;
    ResponseCallback responseCallback_ = nullptr;
    const std::chrono::steady_clock::duration ttl_;
    const std::chrono::milliseconds cleanupInterval_;

    void cleanupLoop() {
        while (running_) {
            std::this_thread::sleep_for(cleanupInterval_);
            std::lock_guard<std::mutex> lock(mutex_);
            auto now = std::chrono::steady_clock::now();
            for (auto it = entries_.begin(); it != entries_.end(); ) {
                if (now - it->second.lastSeen > ttl_) {
                    it = entries_.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
};

#endif /* LORAREFORWARDER_GATEWAY_REGISTRY_HPP */
