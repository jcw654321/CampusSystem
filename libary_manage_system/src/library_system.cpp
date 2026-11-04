#include <library_system.h>
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
    if(findBookIndex(book.getBookId())!=-1)
    {
        reason = "图书编号重复!";
        return false;
    }
    books.push_back(book);
    return true;
}

int LibrarySystem::findBookIndex(const string& bookId) const
{
    for(int i = 0; i<books.size();i++) 
    {
        if(books[i].getBookId() ==bookId)
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

bool LibrarySystem::getBookAtIndex(int index,Book& outbook) const
{
    if(index<0 || index>=books.size())
    {
        return false;
    }
    outbook = books[index];
    return true;
}

void LibrarySystem::listBooks() const
{
    for(int i = 0;i<books.size();i++)
    {
        books[i].displayBookInfo();
    }
}

void LibrarySystem::showMenu() 
{
    
}
