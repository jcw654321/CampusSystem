#include "Banksystem.h"
#include "SavingAccount.h"
#include "CreditAccount.h"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <utility>

using namespace std;

// 这些函数仍属于 Banksystem 类，这里集中放置文本文件的读写实现。
bool Banksystem::saveAccounts() const {
    ofstream out("accounts.txt");
    if (!out) {
        return false;
    }
    for (const auto& account : accounts) {
        account->writeTo(out);
    }
    out.close();
    return !out.fail();
}

unique_ptr<BankAccount> Banksystem::parseAccountLine(const string& line) const {
    istringstream in(line);
    string type, accountId, studentId;
    int64_t balance;

    if (!(in >> type >> accountId >> studentId >> balance)) {
        return nullptr;
    }
    string extra;
    if (type == "SAVING") {
        double rate;
        if (!(in >> rate) || (in >> extra) || rate < 0 || balance < 0) {
            return nullptr;
        }
        return make_unique<SavingAccount>(accountId, studentId, rate, balance);
    }
    if (type == "CREDIT") {
        int64_t limit;
        if (!(in >> limit) || (in >> extra) || limit < 0 || balance < -limit) {
            return nullptr;
        }
        return make_unique<CreditAccount>(accountId, studentId, limit, balance);
    }
    return nullptr;
}

bool Banksystem::loadAccounts() {
    if (!accounts.empty()) {
        return false;
    }
    ifstream in("accounts.txt");
    if (!in) {
        return false;
    }
    vector<unique_ptr<BankAccount>> loaded;
    string line;
    while (getline(in, line)) {
        auto account = parseAccountLine(line);
        if (!account) {
            return false;
        }
        for (const auto& existing : loaded) {
            if (existing->getAccountId() == account->getAccountId()) {
                return false;
            }
        }
        loaded.push_back(move(account));
    }
    if (in.bad() || !in.eof()) {
        return false;
    }
    accounts.swap(loaded);
    return true;
}

bool Banksystem::saveTransactions() const {
    ofstream out("transactions.txt");
    if (!out) {
        return false;
    }
    for (const auto& t : transactions) {
        out << quoted(t.type) << ' '
            << t.amount << ' '
            << quoted(t.fromAccountId) << ' '
            << quoted(t.toAccountId) << '\n';
    }
    out.close();
    return !out.fail();
}

bool Banksystem::loadTransactions() {
    ifstream in("transactions.txt");
    if (!in) {
        return false;
    }
    vector<Transaction> loaded;
    string line;
    while (getline(in, line)) {
        istringstream row(line);
        Transaction t;
        if (!(row >> quoted(t.type) >> t.amount
                  >> quoted(t.fromAccountId) >> quoted(t.toAccountId))) {
            return false;
        }
        string extra;
        if (t.amount <= 0 || (row >> extra)) {
            return false;
        }
        if (t.type == "存款") {
            if (!t.fromAccountId.empty() || !accountExists(t.toAccountId))
                return false;
        } else if (t.type == "取款") {
            if (!accountExists(t.fromAccountId) || !t.toAccountId.empty())
                return false;
        } else if (t.type == "转账") {
            if (!accountExists(t.fromAccountId) || !accountExists(t.toAccountId)
                || t.fromAccountId == t.toAccountId)
                return false;
        } else {
            return false;
        }
        loaded.push_back(t);
    }
    if (in.bad() || !in.eof()) {
        return false;
    }
    transactions.swap(loaded);
    return true;
}
