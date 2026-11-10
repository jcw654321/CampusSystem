#pragma once

#include "BankAccount.h"

class SavingAccount : public BankAccount {
public:
    double interestRate;
    SavingAccount(const std::string& accountId, const std::string& studentId, double interestRate, std::int64_t balance = 0);
    double getInterestRate();
    void writeTo(std::ostream& out) const override;
};
