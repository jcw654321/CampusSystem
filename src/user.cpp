#include "user.h"
using namespace std;

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