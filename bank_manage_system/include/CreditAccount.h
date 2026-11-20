#pragma once

#include "BankAccount.h"

// 信用账户：允许余额为负数，但不能低于 -creditLimit。
// 例如额度为 10000 分时，最低余额是 -10000 分。
class CreditAccount : public BankAccount
{
  public:
    std::int64_t creditLimit;
    CreditAccount(const std::string &accountId, const std::string &studentId,
                  std::int64_t creditLimit, std::int64_t balance = 0);
    std::int64_t getCreditLimit();
    bool withdraw(std::int64_t amount) override;
    void writeTo(std::ostream &out) const override;
};
