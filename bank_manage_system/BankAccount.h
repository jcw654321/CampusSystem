#pragma once

#include <cstdint>
#include <ostream>
#include <string>

class BankAccount {
public:
    std::string accountId;
    std::string studentId;
    std::int64_t balance;
    BankAccount(const std::string& accountId, const std::string& studentId,std::int64_t balance = 0);
    std::string getAccountId();
    std::string getStudentId();
    std::int64_t getBalance();
    bool deposit(std::int64_t amount);
    virtual bool withdraw(std::int64_t amount);
    virtual void writeTo(std::ostream& out) const = 0;
    virtual ~BankAccount() = default;
};
