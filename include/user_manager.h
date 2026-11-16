#pragma once

#include "user.h"
#include <vector>
#include <iostream>

class UserManager {
public:
    void addUser(const User& user);   // 添加用户
    void removeUser(const User& user);   // 删除用户
    User* findUser(const std::string& username);    // 查找用户
    bool authenticateUser(const std::string& username, const std::string& password);   // 验证用户

private:
    std::vector<User> users;
};