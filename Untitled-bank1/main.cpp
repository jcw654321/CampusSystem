#include "Banksystem.h"
#include <filesystem>
#include <iostream>
#include <system_error>

using namespace std;

int main() {
    Banksystem bankSystem;
    error_code ec;
    bool hasFile = filesystem::exists("accounts.txt", ec);
    if (ec) {
        cout << "无法检查账户文件：" << ec.message() << endl;
        return 1;
    }
    if (hasFile) {
        if (!bankSystem.loadAccounts()) {
            cout << "账户文件读取失败，程序退出，原文件未修改。" << endl;
            return 1;
        }
        cout << "已恢复账户。" << endl;
    }
    bool hasTransactions = filesystem::exists("transactions.txt", ec);
    if (ec) {
        cout << "无法检查流水文件：" << ec.message() << endl;
        return 1;
    }

    if (hasTransactions) {
        if (!bankSystem.loadTransactions()) {
            cout << "流水文件读取失败，程序退出。" << endl;
            return 1;
        }
        cout << "已恢复交易流水。" << endl;
    }
    while(true){
        cout<<"0.退出   1.查询交易流水  2.开储蓄账户    3.开信用账户    4.存款  5.取款  6.转账  7.查询余额  8.保存账户"<<endl;
        int choice;
        cin>>choice;
        if(choice == 0){
            break;
        }
        else if(choice == 1){
            cout<<"请输入账号：";
            string accountId;
            cin>>accountId;
            bankSystem.showTransactions(accountId);
        }
        else if(choice == 2){
            cout<<"请输入账号：";
            string accountId;
            cin>>accountId;
            cout<<"请输入学号：";
            string studentId; 
            cin>>studentId;
            if(bankSystem.openSavingAccount(accountId, studentId)){
                cout<<"开户成功"<<endl;
            }
            else{
                cout<<"开户失败"<<endl;
            }
        }
        else if(choice == 3){
            cout<<"请输入账号：";
            string accountId;
            cin>>accountId;
            cout<<"请输入学号：";
            string studentId;
            cin>>studentId;
            cout<<"请输入信用额度：";
            int64_t creditLimit;
            cin>>creditLimit;
            if(bankSystem.openCreditAccount(accountId, studentId, creditLimit)){
                cout<<"开户成功"<<endl;
            }
            else{
                cout<<"开户失败"<<endl;
            }
        }
        else if(choice == 4){
            cout<<"请输入账号：";
            string accountId;
            cin>>accountId;
            cout<<"请输入存款金额：";
            int64_t amount;
            cin>>amount;
            if(bankSystem.deposit(accountId, amount)){
                cout<<"存款成功"<<endl;
            }
            else{
                cout<<"存款失败"<<endl;
            }
        }
        else if(choice == 5){
            cout<<"请输入账号：";
            string accountId;
            cin>>accountId;
            cout<<"请输入取款金额：";
            int64_t amount;
            cin>>amount;
            if(bankSystem.withdraw(accountId, amount)){
                cout<<"取款成功"<<endl;
            }
            else{
                cout<<"取款失败"<<endl;
            }
        }
        else if(choice == 6){
            cout<<"请输入转出账号：";
            string fromAccountId;
            cin>>fromAccountId;
            cout<<"请输入转入账号：";
            string toAccountId;
            cin>>toAccountId;
            cout<<"请输入转账金额：";
            int64_t amount;
            cin>>amount;
            if(bankSystem.Transfer(fromAccountId, toAccountId, amount)){
                cout<<"转账成功"<<endl;
            }
            else{
                cout<<"转账失败"<<endl;
            }
        }
        else if(choice == 7){
            cout<<"请输入账号：";
            string accountId;
            cin>>accountId;
            BankAccount* account = bankSystem.findAccount(accountId);
            if(account != nullptr){
                cout<<"余额为："<<account->getBalance()<<"分"<<endl;
            }
            else{
                cout<<"账号不存在"<<endl;
            }
        }
        else if(choice == 8){
            if(!bankSystem.saveAccounts()){
                cout<<"账户信息保存失败,本次未保存流水"<<endl;
            }
            else if(!bankSystem.saveTransactions()){
                cout<<"账户已保存，但流水保存失败"<<endl;
            }
            else{
                cout<<"账户信息保存成功"<<endl;
            }
        }
        else{
            cout<<"选项无效"<<endl;
        }
    }
}
