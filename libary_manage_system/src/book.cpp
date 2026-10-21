#include<book.h>
#include<iostream>
#include<string>
using namespace std;
string Book::getBookId() const {
    return bookId;
}
string Book::getTitle() const {
    return title;
}
string Book::getAuthor() const {
    return author;
}
string Book::getCategory() const {
    return category;
}
BookStatus Book::getStatus() const{
    return status;
}
int Book::getTotalCopies() const {
    return totalCopies;
}
void Book::setTitle(const string& title) {
    this->title = title;
}
void Book::setAuthor(const string& author) {
    this->author = author;
}
void Book::setCategory(const string& category) {
    this->category = category;
}
void Book::setStatus(const BookStatus& status) {
    this->status = status;
}
void Book::setTotalCopies(const int& totalCopies) {
    if(totalCopies>=0)
    {
        this->totalCopies = totalCopies;
    }
    else
    {
        cout<<"Total copies cannot be negative!"<<endl;
    }
}
bool Book::isonshelf() const {
    return status == BookStatus::ON_SHELF;
}
string Book::bookstatustostring() const {
    switch(status){
        case BookStatus::ON_SHELF:
        return "On Shelf";
        case BookStatus::OUT_SHELF:
        return "Out Shelf";
    }
    return "Unknown Status";
}
void Book::displayBookInfo()const{
    cout<<"Book Id: "<<bookId<<" "<<"Title: "<<title<<" "<<"Author: "<<author<<" "<<"Category: "<<category<<" "<<"Status: "<<bookstatustostring()<<" "<<"Total Copies: "<<totalCopies<<endl;
}