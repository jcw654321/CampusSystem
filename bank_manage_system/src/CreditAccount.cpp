#include "CreditAccount.h"

using namespace std;

CreditAccount::CreditAccount(const string &accountId, const string &ownerUserId, int64_t balance)
    : BankAccount(accountId, ownerUserId, balance)
{
}

int64_t CreditAccount::getCreditLimit() const
{
    return FixedCreditLimit;
}

bool CreditAccount::withdraw(int64_t amount)
{
    if (amount <= 0)
    {
        return false;
    }
    // 目标规则是：balance - amount >= -FixedCreditLimit。
    // 移项后先比较 balance >= amount - FixedCreditLimit，避免比较前就扣款。
    if (getBalance() < amount - FixedCreditLimit)
    {
        return false;
    }
    return reduceBalance(amount);
}

void CreditAccount::writeTo(ostream &out) const
{
    out << "CREDIT " << getAccountId() << ' ' << getOwnerUserId() << ' ' << getBalance() << ' '
        << FixedCreditLimit << '\n';
}
