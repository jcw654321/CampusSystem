#pragma once

#include "BankAccount.h"

// 储蓄账户：沿用 BankAccount 的取款规则，余额不能小于零。
class SavingAccount : public BankAccount
{
  public:
    SavingAccount(const std::string &accountId, const std::string &ownerUserId,
                  std::int64_t balance = 0);
    void writeTo(std::ostream &out) const override;
};
