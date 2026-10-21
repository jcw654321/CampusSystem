#include<windows.h>
#include<book.h>
#include<loanrecord.h>

using namespace std;
//用于测试的cpp文件,不必在意
int main()
{
    SetConsoleOutputCP(CP_UTF8); // 设置控制台输出为UTF-8编码
    Book book("B001", "C++ Programming", "John Doe", "Programming", BookStatus::ON_SHELF, 5);
    book.displayBookInfo();
    LoanRecord loan("L001","student001","B001","2026-10-21","","2026-11-21","operator001");
    loan.displayLoanRecordInfo();
    if(loan.isoverdue("2026-11-20")){
        cout<<"The book is overdue."<<endl;
    }else{
        loan.markasreturned("2026-11-20");
        cout<<"The book has been returned."<<endl;
    }
}