#include<loanrecord.h>
#include<iostream>
#include<string>
using namespace std;
bool parseDate(const string& date,Date& parsedDate){
    int y,m,d,usedstr;
    if(sscanf(date.c_str(),"%d-%d-%d%n",&y,&m,&d,&usedstr)!=3||usedstr!=date.length())
    return false;
   if(y<0||m<1||m>12||d<1)
   return false;
   int dayofmonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};
   int maxday = dayofmonth[m-1];
   if(m==2&&(y%4==0&&y%100!=0||y%400==0))
   maxday=29;
   if(d>maxday)
   return false;
   parsedDate.year=y;
   parsedDate.month=m;
   parsedDate.day=d;
   return true;
}

bool compareDate(const string& date1,const string& date2){//如果date1>date2返回true，否则返回false
    Date d1,d2;
    if(!parseDate(date1,d1)||!parseDate(date2,d2))
    {
        cout<<"Invalid date format!"<<endl;
        return false;
    }
    if(d1.year>d2.year)
        return true;
    if(d1.year<d2.year)
        return false;
    if(d1.month>d2.month)
        return true;
    if(d1.month<d2.month)
        return false;
    return d1.day>d2.day;
}

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
    cout<<"This book has already been returned!"<<endl;
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