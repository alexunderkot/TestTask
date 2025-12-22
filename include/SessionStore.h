#ifndef SESSIONSTORE_H
#define SESSIONSTORE_H

#include "ISessionStore.h"
#include <unordered_map>
#include <map>
#include <mutex>
#include <string>

class SessionStore : public ISessionStore {
private:
    std::unordered_map<std::string, std::map<std::string, double>> sessions;
    std::mutex mutex;
    static const std::string DEFAULT_USER;
    
    SessionStore() = default;
    
public:
    SessionStore(const SessionStore&) = delete;
    SessionStore& operator=(const SessionStore&) = delete;
    
    static SessionStore& instance();
    
    void set(const std::string& user, const std::string& name, double value) override;
    double get(const std::string& user, const std::string& name) override;
    bool exists(const std::string& user, const std::string& name) override;
    void clear(const std::string& user) override;
    void clearAll() override;
    std::string getEffectiveUser(const std::string& requestedUser) override;
    
    void list(const std::string& user);  // For debugging
};

#endif