#include "BankAccount.h"
#include <limits>

using namespace std;

BankAccount::BankAccount(const string &accountId, const string &ownerUserId, int64_t balance)
{
    this->accountId = accountId;
    this->ownerUserId = ownerUserId;
    this->balance = balance;
}

string BankAccount::getAccountId() const
{
    return accountId;
}

string BankAccount::getOwnerUserId() const
{
    return ownerUserId;
}

int64_t BankAccount::getBalance() const
{
    return balance;
}

bool BankAccount::isClosed() const
{
    return closed;
}

bool BankAccount::deposit(int64_t amount)
{
    if (closed || amount <= 0)
    {
        return false;
    }
    // 用“最大值减金额”检查上界，避免先相加导致溢出。
    if (balance > numeric_limits<int64_t>::max() - amount)
    {
        return false;
    }
    balance += amount;
    return true;
}

bool BankAccount::withdraw(int64_t amount)
{
    // 默认规则：只能取出已有余额，储蓄账户直接沿用此实现。
    if (closed || amount <= 0 || amount > balance)
    {
        return false;
    }
    return reduceBalance(amount);
}

bool BankAccount::reduceBalance(int64_t amount)
{
    if (closed || amount <= 0 || balance < numeric_limits<int64_t>::min() + amount)
    {
        return false;
    }
    balance -= amount;
    return true;
}
