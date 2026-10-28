#include<loanrecord.h>
#include<iostream>
#include<string>
#include<dateutils.h>
using namespace std;
bool LoanRecord::isreturned() const {
    if(returnDate.empty()){
        return false;
    }
    return true;
}

bool LoanRecord::isoverdue(const string& currentDate) const {
   if(isreturned()){
    if(compareDate(returnDate,duedate)){
        return true;
    } else {
        return false;
    }
   }
   else{
    if(compareDate(currentDate,duedate)){
        return true;
    } else {
        return false;
    }
   }
}

bool LoanRecord::markasreturned(const string& returnDate) {
    if(isreturned()){
    return false;
    }
    if(compareDate(borrowDate,returnDate))
    {
        return false;
    }
    if(!isDateValid(returnDate))
    {
        return false;
    }
    this->returnDate = returnDate;
    return true;
}

void LoanRecord::displayLoanRecordInfo() const {
   if(isreturned()){
     cout<<"借阅记录编号: "<<loanId<<" "<<"学生编号: "<<studentId<<" "<<"图书编号: "<<bookId<<" "<<"借阅日期: "<<borrowDate<<" "<<"归还日期: "<<returnDate<<" "<<"归还截止日期: "<<duedate<<" "<<"操作员: "<<operatorId<<endl;   
   } else {
     cout<<"借阅记录编号: "<<loanId<<" "<<"学生编号: "<<studentId<<" "<<"图书编号: "<<bookId<<" "<<"借阅日期: "<<borrowDate<<" "<<"归还截止日期: "<<duedate<<" "<<"尚未归还! 操作员: "<<operatorId<<endl;
   }
}