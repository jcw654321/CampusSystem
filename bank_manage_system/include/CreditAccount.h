#pragma once

#include "BankAccount.h"

// 信用账户：允许余额为负数，但不能低于 -creditLimit。
// 信用额度固定为 100000 分（1000 元）。
class CreditAccount : public BankAccount
{
  public:
    static constexpr std::int64_t FixedCreditLimit = 100000;
    CreditAccount(const std::string &accountId, const std::string &ownerUserId,
                  std::int64_t balance = 0);
    std::int64_t getCreditLimit() const;
    bool withdraw(std::int64_t amount) override;
    void writeTo(std::ostream &out) const override;
};
