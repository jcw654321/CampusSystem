#pragma once

#include "BankAccount.h"

class CreditAccount : public BankAccount {
public:
    std::int64_t creditLimit;
    CreditAccount(const std::string& accountId, const std::string& studentId, std::int64_t creditLimit, std::int64_t balance = 0);
    std::int64_t getCreditLimit();
    bool withdraw(std::int64_t amount) override;
    void writeTo(std::ostream& out) const override;
};
