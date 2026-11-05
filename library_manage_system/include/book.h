#pragma once
#include<iostream>
#include<string>
using namespace std;

enum class BookStatus{
    ON_SHELF,
    OUT_SHELF
};

class Book{

    public:
    Book(string bookId, string title, string author, string category, BookStatus status, int totalCopies)
        : bookId(bookId), title(title), author(author), category(category), status(status), totalCopies(totalCopies) {}

        Book():bookId(""),title(""),author(""),category(""),status(BookStatus::ON_SHELF),totalCopies(0){}

        string getBookId() const;          // 获取图书编号
        string getTitle() const;           // 获取图书标题
        string getAuthor() const;          // 获取图书作者  
        string getCategory() const;        // 获取图书分类
        BookStatus getStatus() const;      // 获取图书状态
        int getTotalCopies() const;        // 获取图书总数量

        void setTitle(const string& title);            // 设置图书标题
        void setAuthor(const string& author);          // 设置图书作者
        void setCategory(const string& category);      // 设置图书分类
        void setStatus(const BookStatus& status);      // 设置图书状态
        void setTotalCopies(const int& totalCopies);   // 设置图书总数量

        bool isonshelf() const;              // 判断图书是否在架
        void displayBookInfo() const;        // 显示图书信息
        string bookstatustostring() const;   // 将图书状态转换为字符串
 

    private:
    string bookId;          // 图书编号
    string title;           // 图书标题 
    string author;          // 图书作者
    string category;        // 图书分类
    BookStatus status;      // 图书状态
    int totalCopies;        // 图书总数量


};

