#pragma once
#include"book.h"
#include"loan_record.h"
#include"dateutils.h"
#include<vector>
#include<string>
using namespace std;
class LibrarySystem{
public:
explicit LibrarySystem(string dataFile);              //构造函数,只用来保存数据库的位置,具体的文件读写交给专门的成员函数

public:
static const int kLoanDays = 30;      //借书的天数限制
static const int kMaxUnreturnedPerStudent = 5; //每个学生最多未归还的书籍数量

public://图书管理相关成员函数
bool addBook(const Book& book,string& reason );       //reason用于记录增加书本失败的原因,即编号为空,名称重复,册数小于等于0等
int findBookIndex(const string& bookId)  const;     //寻找书本下标
int getBookCount() const;                             //获取书本数量
bool getBookAtIndex(int index,Book& outbook) const;   //通过书籍下表获取书本
void listBooks() const;                               //用于遍历打印(测试时方便使用)
vector<int> searchByTitle(const string& bookname) const;     //通过书名查找书籍
int searchByBookId(const string& bookId) const;      //通过id查找书籍
vector<int> searchByAuthor(const string& author) const;      //通过作者查找书籍
vector<int> searchByCategory(const string& category) const;  //通过种类查找书籍
bool updateBook(const string& bookId,const string& title,const string& author,const string& category,string& reason); //更新书籍信息,reason用于记录失败原因,字符串为空表示不修改该字段
bool adjustCopies(const string& bookId, int newTotal,string& reason); //调整书籍数量,reason用于记录失败原因
bool setBookStatus(const string& bookId,BookStatus newStatus,string& reason); //设置书籍状态为上架或者下架,下架前要求未归还书籍为0,reason用于记录失败原因
int unreturnedBooksCount(const string& bookId) const;                       //统计该书未归还的书籍数量
int availableCopies(const string& bookId) const;      //统计该书可借的书籍数量


public://借阅管理相关成员函数
bool borrowBook(const string& bookId,const string& studentId,const string& operatorId, const string& today,string& reason);//借书
bool returnBook(const string& loanId,const string& operatorId,const string& today,string& reason);//还书
int findLoanIndex(const string& loanId) const;//按照借阅编号查找借条,未找到返回-1
int getLoanCount() const;//记录数量
bool getLoanAtIndex(int index,LoanRecord& outLoan) const;//取出第index条借阅记录
vector<int> loansOfStudent(const string& studentId) const;//获取某个学生的借阅记录,返回下标的vector
vector<int> overdueLoans(const string& today) const;//获取截至今天的所有overdue的借阅记录
bool isLoanOverdue(int index,const string& today) const;//判断第index条借阅记录是否逾期

public://数据文件读写相关成员函数
bool loadData(string& reason);//读取文件
bool saveData(string& reason);//写入文件
void showMenu();

private:
vector<Book> books;            //图书仓库
vector<LoanRecord> loans;      //借条记录
int nextLoanSeq;               //记录下一个借条编号
string dataFile;               //记录data文件路径


};