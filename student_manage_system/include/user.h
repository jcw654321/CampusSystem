#pragma once

#include<string>
using namespace std;

// 用户类，包含用户的基本信息和操作
class User {
public:
    User(string name, string email, string userid, string password)
        : name(name), email(email), userid(userid), password(password) {};
    string getName() const;                       // 获取用户名
    string getEmail() const;                      // 获取邮箱
    string getUserid() const;                     // 获取用户ID
    string getPassword() const;                   // 获取密码
    string getSex() const;                        // 获取性别
    void setName(const string& name);             // 设置用户名
    void setEmail(const string& email);           // 设置邮箱
    void setUserid(const string& userid);         // 设置用户ID
    void setPassword(const string& password);     // 设置密码
    void setSex(const string& sex);               // 设置性别

private:
    string userid;         // 用户ID
    string name;           // 用户名
    string email;          // 邮箱
    string password;       // 密码
    string sex;             // 性别
};


string User::getName() const {
    return name;
}


string User::getEmail() const {
    return email;
}

string User::getUserid() const {
    return userid;
}

string User::getPassword() const {
    return password;
}

void User::setName(const string& name) {
    this->name = name;
}

void User::setEmail(const string& email) {
    this->email = email;
}

void User::setUserid(const string& userid) {
    this->userid = userid;
}

void User::setPassword(const string& password) {
    this->password = password;
}

string User::getSex() const {
    return sex;
}

void User::setSex(const string& sex) {
    this->sex = sex;
}