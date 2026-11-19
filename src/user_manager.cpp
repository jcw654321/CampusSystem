#include "user_manager.h"
#include<fstream>
#include<filesystem>
#include<sstream>
#include"user.h"

static vector<string> splitLine(const string &line, char delim)
{
    vector<string> result;
    string token;
    stringstream ss(line);
    while (getline(ss, token, delim))
    {
        result.push_back(token);
    }
    if (!line.empty() && line.back() == delim)
    {
        result.push_back(""); // 处理最后一个元素为分隔符的情况
    }
    return result;
}

UserManager::UserManager(std::string dataf):dataFile(dataf){}

void UserManager::loadData() 
{
    ifstream in(dataFile);
    vector<User> tmpUser;
    string line;
    getline(in,line);//跳过第一行
    while(getline(in,line))
    {
        if(line.empty())
        {
            continue;
        }
        vector<string> vec = splitLine(line,',');
        tmpUser.push_back(User(vec[1],vec[3],vec[0],vec[4],vec[2]));
    }
    users = tmpUser;
    return;
}

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

