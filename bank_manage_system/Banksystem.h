#pragma once

#include "BankAccount.h"
#include "Transaction.h"
#include <memory>
#include <vector>

class Banksystem {
public:
    std::vector<std::unique_ptr<BankAccount>> accounts;
    std::vector<Transaction> transactions;
    bool accountExists(const std::string& accountId) const;
    bool openSavingAccount(const std::string& accountId, const std::string& studentId);
    bool openCreditAccount(const std::string& accountId, const std::string& studentId, std::int64_t creditLimit);
    BankAccount* findAccount(const std::string& accountId);
    bool withdraw(const std::string& accountId, std::int64_t amount);
    bool deposit(const std::string& accountId, std::int64_t amount);
    bool Transfer(const std::string& fromAccountId, const std::string& toAccountId, std::int64_t amount);
    void showTransactions(const std::string& accountId);
    bool saveAccounts() const;
    std::unique_ptr<BankAccount> parseAccountLine(const std::string& line) const;
    bool loadAccounts();
    bool saveTransactions() const;
    bool loadTransactions();
};
