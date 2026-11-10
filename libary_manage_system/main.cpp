#include<windows.h>
#include"book.h"
#include"loanrecord.h"
#include"dateutils.h"

using namespace std;
//用于测试的cpp文件,不必在意
int main()
{
    SetConsoleOutputCP(CP_UTF8); // 设置控制台输出为UTF-8编码
    isDateValid("--");
    isDateValid("2026-13-45");
    
    Book book("B001", "C++ Programming", "John Doe", "Programming", BookStatus::ON_SHELF, 5);
    book.displayBookInfo();
    LoanRecord loan("L001","student001","B001","2026-10-21","","2026-11-21","operator001");
    loan.displayLoanRecordInfo();
    string returndate;
    int numl;
    cin>>returndate>>numl;
    
        if(!loan.markasreturned(returndate))
        {
            cout<<"日期不合法"<<endl;
        }
        else if(loan.isoverdue(returndate))
        {
            cout<<"overdue"<<endl;
        }
        else 
        {
            cout<<"ok"<<endl;
        }
    addDays(returndate,numl);
    cout<<returndate;
    
    
}