#pragma once

#include <cstdint>
#include <ostream>
#include <string>

// 账户基类：保存账号、所属学号和余额。金额、余额、额度的单位都是“分”。
// deposit 为两种账户共用；withdraw 为虚函数，信用账户可以改写取款规则。
// writeTo 为纯虚函数：每种账户负责写出自己的类型和附加字段。
class BankAccount
{
  public:
    std::string accountId;
    std::string studentId;
    std::int64_t balance;
    BankAccount(const std::string &accountId, const std::string &studentId,
                std::int64_t balance = 0);
    std::string getAccountId() const;
    std::string getStudentId() const;
    std::int64_t getBalance() const;
    bool deposit(std::int64_t amount);
    virtual bool withdraw(std::int64_t amount);
    virtual void writeTo(std::ostream &out) const = 0;
    virtual ~BankAccount() = default;
};
