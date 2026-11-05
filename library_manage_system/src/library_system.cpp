#include "library_system.h"
#include "book.h"
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;
LibrarySystem::LibrarySystem(string dataFile) : dataFile(dataFile), nextLoanSeq(1)
{
    // 构造函数,只用来保存数据库的位置,具体的文件读写交给专门的成员函数
}

bool LibrarySystem::addBook(const Book &book, string &reason)
{
    if (book.getBookId().empty())
    {
        reason = "图书编号不能为空!";
        return false;
    }
    if (book.getTitle().empty())
    {
        reason = "图书标题不能为空!";
        return false;
    }
    if (book.getTotalCopies() < 0)
    {
        reason = "图书总数量必须大于0!";
        return false;
    }
    // 检查是否有重复的图书编号
    if (findBookIndex(book.getBookId()) != -1)
    {
        reason = "图书编号重复!";
        return false;
    }
    books.push_back(book);
    return true;
}

int LibrarySystem::findBookIndex(const string &bookId) const
{
    for (int i = 0; i < books.size(); i++)
    {
        if (books[i].getBookId() == bookId)
        {
            return i;
        }
    }
    return -1;
}

int LibrarySystem::getBookCount() const
{
    return books.size();
}

bool LibrarySystem::getBookAtIndex(int index, Book &outbook) const
{
    if (index < 0 || index >= books.size())
    {
        return false;
    }
    outbook = books[index];
    return true;
}

void LibrarySystem::listBooks() const
{
    for (int i = 0; i < books.size(); i++)
    {
        books[i].displayBookInfo();
    }
}

void LibrarySystem::showMenu()
{
    while (true)
    {
        cout << "--------图书管理--------" << endl;
        cout << "1. 新增图书" << endl
             << "2. 图书列表" << endl
             << "3. 按编号查询" << endl
             << "0. 返回" << endl;
        cout << "请选择:" << endl;
        int choice;
        string line;
        if (!getline(cin, line))
        {
            return;
        }
        stringstream ss(line);
        if (!(ss >> choice))
        {
            cout << "无效的输入,请输入数字以匹配相应功能" << endl;
            continue;
        }

        switch (choice)
        {
        case 0:
            return;
        case 1:
        {
            cout << "请按照如下格式输入:书本编号|书名|作者|分类|册数" << endl;
            string bookID, bookName, author, categroy, copiesText;
            int count;
            getline(cin,line);
            stringstream ss1(line);
            bool isValid = getline(ss1, bookID, '|') && getline(ss1, bookName, '|') && getline(ss1, author, '|') && getline(ss1, categroy, '|') && getline(ss1, copiesText);
            if (!isValid)
            {
                cout << "无效的输入,请按照格式输入:书本编号|书名|作者|分类|册数" << endl;
                break;
            }
            stringstream cs(copiesText);
            if (!(cs >> count))
            {
                cout << "册数需为整数" << endl;
                break;
            }
            string extra; // 避免用户传入15avc这种
            if (cs >> extra)
            {
                cout << "多余的字符: " << extra << endl;
                break;
            }

            string reason;
            if (addBook(Book(bookID, bookName, author, categroy, BookStatus::ON_SHELF, count), reason))
            {
                cout << "添加成功!" << endl;
            }
            else
            {
                cout << "添加失败!" << reason << endl;
            }

            break;
        }
        default:
            break;
        }
    }
}

