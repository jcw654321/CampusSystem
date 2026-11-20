#include "CreditAccount.h"

using namespace std;

CreditAccount::CreditAccount(const string &accountId, const string &studentId, int64_t creditLimit,
                             int64_t balance)
    : BankAccount(accountId, studentId, balance)
{
    this->creditLimit = creditLimit;
}

int64_t CreditAccount::getCreditLimit() const
{
    return creditLimit;
}

bool CreditAccount::withdraw(int64_t amount)
{
    if (amount <= 0 || creditLimit < 0)
    {
        return false;
    }
    // 目标规则是：balance - amount >= -creditLimit。
    // 移项后先比较 balance >= amount - creditLimit，避免比较前就扣款。
    if (balance < amount - creditLimit)
    {
        return false;
    }
    balance -= amount;
    return true;
}

void CreditAccount::writeTo(ostream &out) const
{
    out << "CREDIT " << accountId << ' ' << studentId << ' ' << balance << ' ' << creditLimit
        << '\n';
}
