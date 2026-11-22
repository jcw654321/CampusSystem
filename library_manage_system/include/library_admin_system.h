#pragma once
#include "library_system.h"
#include "user.h"

class LibraryAdminSystem : public LibrarySystem,public User
{
public:
    LibraryAdminSystem(const string& dataFile,const string& name,const string& email,const string& userid,const string& password,const string& sex = ""):LibrarySystem(dataFile),User(name, email, userid, password, sex){} // 继承构造函数，不用重写

    bool addBook(const Book& book, string& reason)
    {
        return LibrarySystem::addBook(book, reason);
    }

    bool updateBook(const string& id,
                    const string& title,
                    const string& author,
                    const string& category,
                    string& reason)
    {
        return LibrarySystem::updateBook(id, title, author, category, reason);
    }

    bool adjustCopies(const string& id, int newTotal, string& reason)
    {
        return LibrarySystem::adjustCopies(id, newTotal, reason);
    }

    bool setBookStatus(const string& id, BookStatus status, string& reason)
    {
        return LibrarySystem::setBookStatus(id, status, reason);
    }

    bool removeBook(const string& id, string& reason)
    {
        return LibrarySystem::removeBook(id, reason);
    }

    bool canManageBooks() const override
    {
        return true;
    }

};