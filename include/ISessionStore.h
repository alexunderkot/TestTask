#ifndef ISESSIONSTORE_H
#define ISESSIONSTORE_H

#include <string>
#include <stdexcept>

class ISessionStore {
public:
    virtual ~ISessionStore() = default;
    virtual void set(const std::string& user, const std::string& name, double value) = 0;
    virtual double get(const std::string& user, const std::string& name) = 0;
    virtual bool exists(const std::string& user, const std::string& name) = 0;
    virtual void clear(const std::string& user) = 0;
    virtual void clearAll() = 0;
    virtual std::string getEffectiveUser(const std::string& requestedUser) = 0;
};

#endif