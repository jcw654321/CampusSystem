#pragma once
#include<iostream>
#include<vector>
#include<string>
using namespace std;

struct Date{
    int year,month,day;
};

bool parseDate(const string& date, Date& parsedDate);      // 解析日期字符串为Date结构体
bool compareDate(const string& date1,const string& date2); // 比较两个日期字符串的大小

class LoanRecord{
public:
    LoanRecord() = default;
    LoanRecord(string loanId, string studentId, string bookId, string borrowDate, string returnDate, string duedate, string operatorId)
        : loanId(loanId), studentId(studentId), bookId(bookId), borrowDate(borrowDate), returnDate(returnDate), duedate(duedate), operatorId(operatorId) {}

        bool isreturned() const;
        bool isoverdue(const string& currentDate) const;  // 判断是否逾期
        bool markasreturned(const string& returnDate);  // 标记为已归还
        void displayLoanRecordInfo() const;  // 显示借阅记录信息
        
        string getLoanId() const { return loanId; } 
        string getStudentId() const { return studentId; }
        string getBookId() const { return bookId; }
        string getBorrowDate() const { return borrowDate; }
        string getReturnDate() const { return returnDate; }
        string getDuedate() const { return duedate; }
        string getOperatorId() const { return operatorId; }
       

    private:
    string loanId;             // 借阅记录编号
    string studentId;          // 学生编号
    string bookId;             // 图书编号
    string borrowDate;         // 借阅日期
    string returnDate;         // 归还日期,未归还置空
    string duedate;            // 归还截止日期
    string operatorId;         // 操作员
};

