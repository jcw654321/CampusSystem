#include "Banksystem.h"
#include "SavingAccount.h"
#include "CreditAccount.h"
#include <iostream>
#include <limits>

using namespace std;

bool Banksystem::accountExists(const string& accountId) const {
    for (const auto& account : accounts) {
        if (account->getAccountId() == accountId) {
            return true;
        }
    }
    return false;
}

bool Banksystem::openSavingAccount(const string& accountId, const string& studentId) {
    if(accountExists(accountId)) {
        return false;
    }
    SavingAccount* newAccount = new SavingAccount(accountId, studentId, 0.05);
    accounts.push_back(unique_ptr<BankAccount>(newAccount));
    return true;
}

bool Banksystem::openCreditAccount(const string& accountId, const string& studentId, int64_t creditLimit) {
    if(creditLimit < 0 || accountExists(accountId)) {
        return false;
    }
    CreditAccount* newAccount = new CreditAccount(accountId, studentId, creditLimit);
    accounts.push_back(unique_ptr<BankAccount>(newAccount));
    return true;
}

BankAccount* Banksystem::findAccount(const string& accountId) {
    for (const auto& account : accounts) {
        if (account->getAccountId() == accountId) {
            return account.get();
        }
    }
    return nullptr;
}

bool Banksystem::withdraw(const string& accountId, int64_t amount) {
    BankAccount* account = findAccount(accountId);
    if (account == nullptr) {
        return false;
    }
    if (!account->withdraw(amount)) {
        return false;
    }
    transactions.push_back({ "取款", amount, accountId, "" });
    return true;
}

bool Banksystem::deposit(const string& accountId, int64_t amount) {
    BankAccount* account = findAccount(accountId);
    if (account == nullptr) {
        return false;
    }
    if (!account->deposit(amount)) {
        return false;
    }
    transactions.push_back({ "存款", amount, "", accountId });
    return true;
}

bool Banksystem::Transfer(const string& fromAccountId, const string& toAccountId, int64_t amount) {
    BankAccount* fromAccount = findAccount(fromAccountId);
    BankAccount* toAccount = findAccount(toAccountId);
    if (fromAccount == nullptr || toAccount == nullptr) {
        return false;
    }
    if (fromAccountId == toAccountId) {
        return false;
    }
    if(amount <= 0 || toAccount->getBalance() > numeric_limits<int64_t>::max() - amount) {
        return false;
    }
    if(!fromAccount->withdraw(amount)) {
        return false;
    }
    if(!toAccount->deposit(amount)) {
        fromAccount->deposit(amount);
        return false;
    }
    transactions.push_back({ "转账", amount, fromAccountId, toAccountId });
    return true;
}

void Banksystem::showTransactions(const string& accountId) {
    if (!accountExists(accountId)) {
        cout << "账号不存在" << endl;
        return;
    }
    bool found = false;
    for(const auto& transaction : transactions) {
        if (transaction.fromAccountId == accountId || transaction.toAccountId == accountId) {
            found = true;
            cout << "交易类型: " << transaction.type << ", 金额: " << transaction.amount;
            if (!transaction.fromAccountId.empty()) {
                cout << ", 转出账号: " << transaction.fromAccountId;
            }
            if (!transaction.toAccountId.empty()) {
                cout << ", 转入账号: " << transaction.toAccountId;
            }
            cout << endl;
        }
    }
    if (!found) {
        cout << "该账户暂无交易记录" << endl;
    }
}
