#include "Banksystem.h"
#include "SavingAccount.h"
#include "CreditAccount.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <utility>

using namespace std;

// 文件格式示例（金额都是分）：
// accounts.txt:     SAVING SU001 U001 1250
//                   CREDIT CU002 U002 -500 100000
// 已销户账户在原行前加 CLOSED；旧储蓄记录末尾的利率字段会读取并忽略。
// 旧信用账户记录中的额度仍按文件读取格式校验，但加载后统一使用固定额度。
// transactions.txt: "转账" 100 "A001" "A002"
// quoted 保留流水中的空账号字段，例如存款的转出账号是 ""。
// 两个文件分开保存，并不构成原子事务；调用方要分别检查返回值。
bool Banksystem::saveAccounts() const
{
    ofstream out(dataDirectory / "accounts.txt");
    if (!out)
    {
        return false;
    }
    for (const auto &account : accounts)
    {
        if (account->isClosed()) out << "CLOSED ";
        account->writeTo(out);
    }
    out.close();
    return !out.fail();
}

unique_ptr<BankAccount> Banksystem::parseAccountLine(const string &line) const
{
    istringstream in(line);
    string type, accountId, ownerUserId;
    int64_t balance;

    bool closed = false;
    if (!(in >> type)) return nullptr;
    if (type == "CLOSED")
    {
        closed = true;
        if (!(in >> type)) return nullptr;
    }
    if (!(in >> accountId >> ownerUserId >> balance) || (closed && balance != 0)) return nullptr;
    unique_ptr<BankAccount> account;
    if (type == "SAVING")
    {
        if (accountId != "S" + ownerUserId) return nullptr;
        if (balance < 0) return nullptr;
        string legacyRate;
        if (in >> legacyRate)
        {
            istringstream rateInput(legacyRate);
            double ignoredRate;
            string rateExtra;
            if (!(rateInput >> ignoredRate) || ignoredRate < 0 || (rateInput >> rateExtra))
                return nullptr;
        }
        account = make_unique<SavingAccount>(accountId, ownerUserId, balance);
    }
    else if (type == "CREDIT")
    {
        if (accountId != "C" + ownerUserId) return nullptr;
        int64_t legacyLimit;
        if (!(in >> legacyLimit) || legacyLimit < 0 ||
            balance < -CreditAccount::FixedCreditLimit) return nullptr;
        account = make_unique<CreditAccount>(accountId, ownerUserId, balance);
    }
    else return nullptr;
    string extra;
    if (in >> extra) return nullptr;
    account->closed = closed;
    return account;
}

bool Banksystem::loadAccounts()
{
    if (!accounts.empty())
    {
        return false;
    }
    ifstream in(dataDirectory / "accounts.txt");
    if (!in)
    {
        return false;
    }
    // 先读到临时容器；所有行都通过检查后才替换正式数据。
    vector<unique_ptr<BankAccount>> loadedAccounts;
    string line;
    while (getline(in, line))
    {
        auto account = parseAccountLine(line);
        if (!account)
        {
            return false;
        }
        for (const auto &existing : loadedAccounts)
        {
            if (existing->getAccountId() == account->getAccountId())
            {
                return false;
            }
        }
        loadedAccounts.push_back(move(account));
    }
    if (in.bad() || !in.eof())
    {
        return false;
    }
    accounts.swap(loadedAccounts);
    return true;
}

bool Banksystem::saveTransactions() const
{
    ofstream out(dataDirectory / "transactions.txt");
    if (!out)
    {
        return false;
    }
    for (const auto &transaction : transactions)
    {
        out << quoted(transaction.type) << ' ' << transaction.amount << ' '
            << quoted(transaction.fromAccountId) << ' ' << quoted(transaction.toAccountId) << '\n';
    }
    out.close();
    return !out.fail();
}

bool Banksystem::loadTransactions()
{
    ifstream in(dataDirectory / "transactions.txt");
    if (!in)
    {
        return false;
    }
    // 账户已经加载完成，下面还要检查每条流水引用的账号。
    vector<Transaction> loadedTransactions;
    string line;
    while (getline(in, line))
    {
        istringstream row(line);
        Transaction transaction;
        if (!(row >> quoted(transaction.type) >> transaction.amount >>
              quoted(transaction.fromAccountId) >> quoted(transaction.toAccountId)))
        {
            return false;
        }
        string extra;
        if (transaction.amount <= 0 || (row >> extra))
        {
            return false;
        }
        if (transaction.type == "存款")
        {
            if (!transaction.fromAccountId.empty() || !findAccountInternal(transaction.toAccountId))
            {
                return false;
            }
        }
        else if (transaction.type == "取款")
        {
            if (!findAccountInternal(transaction.fromAccountId) || !transaction.toAccountId.empty())
            {
                return false;
            }
        }
        else if (transaction.type == "转账")
        {
            if (!findAccountInternal(transaction.fromAccountId) ||
                !findAccountInternal(transaction.toAccountId) ||
                transaction.fromAccountId == transaction.toAccountId)
            {
                return false;
            }
        }
        else
        {
            return false;
        }
        loadedTransactions.push_back(transaction);
    }
    if (in.bad() || !in.eof())
    {
        return false;
    }
    transactions.swap(loadedTransactions);
    return true;
}
