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
bool addBook(const Book& book,string& reason );       //reason用于记录增加书本失败的原因
int findBookIndex(const string& bookname)  const;     //寻找书本下标
void listBooks() const;                               //用于遍历打印(测试时方便使用)
void searchByTitle(const string& bookname) const;     //通过书名查找书籍
void searchByBookId(const string& bookId) const;      //通过id查找书籍
void searchByAuthor(const string& author) const;      //通过作者查找书籍
void searchByCategory(const string& category) const;  //通过种类查找书籍
bool updateBook(const string& bookId,string& reason); //更新书籍信息,reason用于记录失败原因
bool adjustCopies(const string& bookId, int newTotal,string& reason); //调整书籍数量,reason用于记录失败原因
int unreturnedBooksCount(const string& ) const;                       //统计未归还书籍数量


private:
vector<Book> books;            //图书仓库
vector<LoanRecord> loans;      //借条记录
int nextLoanSeq;               //记录下一个借条编号
string dataFile;               //记录data文件路径

};