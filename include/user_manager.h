#pragma once

#include "user.h"
#include <vector>
#include <iostream>

class UserManager {
public:
    UserManager(string data_file);
    void addUser(const User& user);   // 添加用户
    void removeUser(const User& user);   // 删除用户
    User* findUser(const std::string& username);    // 查找用户
    bool authenticateUser(const std::string& username, const std::string& password);   // 验证用户
    void loadData(); //从文件中加载用户
    bool SaveUser(const User& user, const std::string& previousUserid = ""); //保存或更新用户
    bool DeleteUser(const std::string& userid); //从文件中删除用户

private:
    std::string dataFile;
    std::vector<User> users;
};