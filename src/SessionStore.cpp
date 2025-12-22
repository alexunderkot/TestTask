#include "SessionStore.h"
#include <iostream> 
#include <string>

const std::string SessionStore::DEFAULT_USER = "default";

SessionStore& SessionStore::instance() {
    static SessionStore instance;
    return instance;
}

void SessionStore::set(const std::string& user, const std::string& name, double value) {
    std::lock_guard<std::mutex> lock(mutex);
    sessions[user][name] = value;
}

double SessionStore::get(const std::string& user, const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex);
    auto userIt = sessions.find(user);
    if (userIt == sessions.end()) {
        throw std::runtime_error("Unknown variable '" + name + "'");
    }
    
    auto varIt = userIt->second.find(name);
    if (varIt == userIt->second.end()) {
        throw std::runtime_error("Unknown variable '" + name + "'");
    }
    return varIt->second;
}

bool SessionStore::exists(const std::string& user, const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex);
    auto userIt = sessions.find(user);
    if (userIt == sessions.end()) {
        return false;
    }
    return userIt->second.find(name) != userIt->second.end();
}

void SessionStore::clear(const std::string& user) {
    std::lock_guard<std::mutex> lock(mutex);
    sessions[user].clear();
}

void SessionStore::clearAll() {
    std::lock_guard<std::mutex> lock(mutex);
    sessions.clear();
}

std::string SessionStore::getEffectiveUser(const std::string& requestedUser) {
    if (requestedUser.empty()) {
        return DEFAULT_USER;
    }
    return requestedUser;
}

void SessionStore::list(const std::string& user) {
    std::lock_guard<std::mutex> lock(mutex);
    auto userIt = sessions.find(user);
    if (userIt != sessions.end()) {
        for (const auto& pair : userIt->second) {
            std::cout << pair.first << " = " << pair.second << std::endl;
        }
    }
}