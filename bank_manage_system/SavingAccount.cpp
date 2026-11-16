#include "SavingAccount.h"

using namespace std;

SavingAccount::SavingAccount(const string& accountId, const string& studentId, double interestRate, int64_t balance) : BankAccount(accountId, studentId, balance) {
    this->interestRate = interestRate;
}

double SavingAccount::getInterestRate() {
    return interestRate;
}

void SavingAccount::writeTo(ostream& out) const {
    out << "SAVING " << accountId << ' ' << studentId << ' '
        << balance << ' ' << interestRate << '\n';
}
