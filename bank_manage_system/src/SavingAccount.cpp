#include "SavingAccount.h"

using namespace std;

SavingAccount::SavingAccount(const string &accountId, const string &ownerUserId, int64_t balance)
    : BankAccount(accountId, ownerUserId, balance)
{
}

void SavingAccount::writeTo(ostream &out) const
{
    out << "SAVING " << getAccountId() << ' ' << getOwnerUserId() << ' ' << getBalance() << '\n';
}
