#include "Banksystem.h"
#include "SavingAccount.h"
#include "CreditAccount.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <utility>

using namespace std;

// 文件格式示例（金额都是分）：
// accounts.txt:     SAVING A001 S001 1250 0.05
//                   CREDIT A002 S002 -500 10000
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
        account->writeTo(out);
    }
    out.close();
    return !out.fail();
}

unique_ptr<BankAccount> Banksystem::parseAccountLine(const string &line) const
{
    istringstream in(line);
    string type, accountId, studentId;
    int64_t balance;

    if (!(in >> type >> accountId >> studentId >> balance))
    {
        return nullptr;
    }
    string extra; // 检查这一行是否还有不应出现的多余字段
    if (type == "SAVING")
    {
        double rate;
        if (!(in >> rate))
        {
            return nullptr;
        }
        if ((in >> extra) || rate < 0 || balance < 0)
        {
            return nullptr;
        }
        return make_unique<SavingAccount>(accountId, studentId, rate, balance);
    }
    if (type == "CREDIT")
    {
        int64_t limit;
        if (!(in >> limit))
        {
            return nullptr;
        }
        // 先排除负额度，再取负数检查余额下限。
        if ((in >> extra) || limit < 0 || balance < -limit)
        {
            return nullptr;
        }
        return make_unique<CreditAccount>(accountId, studentId, limit, balance);
    }
    return nullptr;
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
            if (!transaction.fromAccountId.empty() || !accountExists(transaction.toAccountId))
            {
                return false;
            }
        }
        else if (transaction.type == "取款")
        {
            if (!accountExists(transaction.fromAccountId) || !transaction.toAccountId.empty())
            {
                return false;
            }
        }
        else if (transaction.type == "转账")
        {
            if (!accountExists(transaction.fromAccountId) ||
                !accountExists(transaction.toAccountId) ||
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
