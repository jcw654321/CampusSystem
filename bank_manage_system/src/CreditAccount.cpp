#include "CreditAccount.h"

using namespace std;

CreditAccount::CreditAccount(const string& accountId, const string& studentId, int64_t creditLimit, int64_t balance) : BankAccount(accountId, studentId, balance) {
    this->creditLimit = creditLimit;
}

int64_t CreditAccount::getCreditLimit() {
    return creditLimit;
}

bool CreditAccount::withdraw(int64_t amount) {
    if (amount <= 0 || creditLimit < 0 || balance < amount - creditLimit) {
        return false;
    }
    balance -= amount;
    return true;
}

void CreditAccount::writeTo(ostream& out) const {
    out << "CREDIT " << accountId << ' ' << studentId << ' '
        << balance << ' ' << creditLimit << '\n';
}
