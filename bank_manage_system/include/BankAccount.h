#pragma once

#include <cstdint>
#include <ostream>
#include <string>

// 账户基类：保存账号、所属用户号和余额。金额、余额、额度的单位都是“分”。
// deposit 为两种账户共用；withdraw 为虚函数，信用账户可以改写取款规则。
// writeTo 为纯虚函数：每种账户负责写出自己的类型和附加字段。
class BankAccount
{
  public:
    BankAccount(const std::string &accountId, const std::string &ownerUserId,
                std::int64_t balance = 0);
    std::string getAccountId() const;
    std::string getOwnerUserId() const;
    std::int64_t getBalance() const;
    bool isClosed() const;
    bool deposit(std::int64_t amount);
    virtual bool withdraw(std::int64_t amount);
    virtual void writeTo(std::ostream &out) const = 0;
    virtual ~BankAccount() = default;

  protected:
    bool reduceBalance(std::int64_t amount);

  private:
    friend class Banksystem; // 只有银行业务层可以销户或恢复账户。
    bool closed = false;
    std::string accountId;
    std::string ownerUserId;
    std::int64_t balance;
};
