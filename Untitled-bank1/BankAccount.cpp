#include "BankAccount.h"
#include <limits>

using namespace std;

BankAccount::BankAccount(const string& accountId, const string& studentId,int64_t balance) {
    this->accountId = accountId;
    this->studentId = studentId;
    this->balance = balance;
}

string BankAccount::getAccountId() {
    return accountId;
}

string BankAccount::getStudentId() {
    return studentId;
}

int64_t BankAccount::getBalance() {
    return balance;
}

bool BankAccount::deposit(int64_t amount) {
    if (amount <= 0 ) {
        return false;
    }
    if(balance > numeric_limits<int64_t>::max() - amount) {
        return false;
    }
    balance += amount;
    return true;
}

bool BankAccount::withdraw(int64_t amount) {
    if (amount <= 0 || amount > balance) {
        return false;
    }
    balance -= amount;
    return true;
}
