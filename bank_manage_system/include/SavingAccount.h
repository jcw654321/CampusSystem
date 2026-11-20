#pragma once

#include "BankAccount.h"

// 储蓄账户：沿用 BankAccount 的取款规则，余额不能小于零。
// interestRate 仅保存利率信息，当前没有自动计息功能。
class SavingAccount : public BankAccount
{
  public:
    double interestRate;
    SavingAccount(const std::string &accountId, const std::string &studentId, double interestRate,
                  std::int64_t balance = 0);
    double getInterestRate() const;
    void writeTo(std::ostream &out) const override;
};
