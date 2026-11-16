#include "user_manager.h"

void UserManager::addUser(const User& user) {
    users.push_back(user);
}

void UserManager::removeUser(const User& user) {
    auto it = std::remove_if(users.begin(), users.end(),
        [&user](const User& u) { return u.getUserid() == user.getUserid(); });
    users.erase(it, users.end());
}

User* UserManager::findUser(const std::string& username) {
    for (auto& user : users) {
        if (user.getUserid() == username) {
            return &user;
        }
    }
    return nullptr;
}

bool UserManager::authenticateUser(const std::string& username, const std::string& password) {
    User* user = findUser(username);
    if (user) {
        return user->getPassword() == password;
    }
    return false;
}

