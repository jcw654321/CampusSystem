#include "Banksystem.h"
#include "SavingAccount.h"
#include "CreditAccount.h"
#include <fstream>
#include <sstream>
#include <utility>

using namespace std;

// 文件格式示例（金额都是分）：
// accounts.txt:     SAVING SU001 U001 1250
//                   CREDIT CU002 U002 -500 100000
// 已销户账户在原行前加 CLOSED；旧储蓄记录末尾的利率字段会读取并忽略。
// 旧信用账户记录中的额度仍按文件读取格式校验，但加载后统一使用固定额度。
// 银行只保存账户数据，不保存交易记录。
bool Banksystem::saveAccounts() const
{
    filesystem::create_directories(dataDirectory);

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
    // 文件按类型解析；任何字段缺失、矛盾或多余都会使整行无效。
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
        // 旧储蓄文件可能带利率字段；只校验格式，当前规则不再使用利率。
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
        // 旧文件仍要求额度字段格式有效，但新账户统一使用固定信用额度。
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
    // 先读到临时容器；所有行都通过检查后才替换正式数据，避免部分加载。
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
