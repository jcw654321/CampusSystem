#include "library_system.h"
#include "book.h"
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;
// 第一批函数:查询类
bool toLowerAscii(string &str) // 将大写字母统一转为小写
{
    for (char &c : str)
    {
        if (c >= 'A' && c <= 'Z')
        {
            c += 'a' - 'A';
        }
    }
    return true;
}

bool isSubSeq(const string &text, const string &query) // 用于实现子序列匹配的接口(双指针)
{
    string textCopy = text;
    string queryCopy = query;
    toLowerAscii(textCopy);
    toLowerAscii(queryCopy);
    char p, q;
    size_t j = 0;
    for (size_t i = 0; i < textCopy.length() && j < queryCopy.length(); i++)
    {
        p = textCopy[i];
        q = queryCopy[j];
        if (p == q)
        {
            j++;
        }
    }
    return j == queryCopy.length();
}

int matchLevel(const string &text, const string &query) // 实现匹配等级的接口,用于搜索结果的排序
{
    string t = text;
    string q = query;
    toLowerAscii(t);
    toLowerAscii(q);

    if (query.empty())
        return 0;
    if (t == q)
        return 4; // 完全匹配
    if (t.rfind(q, 0) == 0)
        return 3; // 前缀匹配
    if (t.find(q, 0) != string::npos)
        return 2; // 包含匹配
    if (isSubSeq(text, query))
        return 1; // 子序列匹配
    return 0;     // 不匹配
}

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
    return static_cast<int>(books.size());
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

vector<int> LibrarySystem::searchByTitle(const string &query) const
{
    vector<pair<int, int>> vec; // 匹配度,下标
    for (int i = 0; i < books.size(); i++)
    {
        int level = matchLevel(books[i].getTitle(), query);
        if (level > 0)
        {
            vec.push_back({level, i});
        }
    }
    for (int i = 0; i + 1 < vec.size(); i++) // 这里不适用i<vec.size()-1,为了避免减法导致的vec为空时的溢出
    {
        for (int j = 0; j < vec.size() - 1 - i; j++)
        {
            if (vec[j].first < vec[j + 1].first)
            {
                swap(vec[j], vec[j + 1]);
            }
        }
    } // 使用冒泡排序将匹配度高的排前面
    vector<int> result;
    for (int i = 0; i < vec.size(); i++)
    {
        result.push_back(vec[i].second);
    }
    return result;
}

vector<int> LibrarySystem::searchByAuthor(const string &query) const
{
    vector<pair<int, int>> vec; // 匹配度,下标
    for (int i = 0; i < books.size(); i++)
    {
        int level = matchLevel(books[i].getAuthor(), query);
        if (level > 0)
        {
            vec.push_back({level, i});
        }
    }
    for (int i = 0; i + 1 < vec.size(); i++) // 这里不适用i<vec.size()-1,为了避免减法导致的vec为空时的溢出
    {
        for (int j = 0; j < vec.size() - 1 - i; j++)
        {
            if (vec[j].first < vec[j + 1].first)
            {
                swap(vec[j], vec[j + 1]);
            }
        }
    } // 使用冒泡排序将匹配度高的排前面
    vector<int> result;
    for (int i = 0; i < vec.size(); i++)
    {
        result.push_back(vec[i].second);
    }
    return result;
}

vector<int> LibrarySystem::searchByCategory(const string &query) const
{
    vector<pair<int, int>> vec; // 匹配度,下标
    for (int i = 0; i < books.size(); i++)
    {
        int level = matchLevel(books[i].getCategory(), query);
        if (level > 0)
        {
            vec.push_back({level, i});
        }
    }
    for (int i = 0; i + 1 < vec.size(); i++) // 这里不适用i<vec.size()-1,为了避免减法导致的vec为空时的溢出
    {
        for (int j = 0; j < vec.size() - 1 - i; j++)
        {
            if (vec[j].first < vec[j + 1].first)
            {
                swap(vec[j], vec[j + 1]);
            }
        }
    } // 使用冒泡排序将匹配度高的排前面
    vector<int> result;
    for (int i = 0; i < vec.size(); i++)
    {
        result.push_back(vec[i].second);
    }
    return result;
}

int LibrarySystem::unreturnedBooksCount(const string &bookId) const
{
    bool found = false;
    int unreturnedNum = 0;
    for (size_t i = 0; i < loans.size(); i++)
    {
        if (loans[i].getBookId() == bookId)
        {
            if (!loans[i].isreturned())
            {
                found = true;
                unreturnedNum++;
            }
        }
    }
    if (!found)
    {
        return 0;
    }
    return unreturnedNum;
}

int LibrarySystem::availableCopies(const string &bookId) const
{
    for (size_t i = 0; i < books.size(); i++)
    {
        if (books[i].getBookId() == bookId)
        {
            return books[i].getTotalCopies() - unreturnedBooksCount(bookId);
        }
    }
    return -1;
}

// 第二批函数:修改类
bool LibrarySystem::updateBook(const string &bookId, const string &title, const string &author, const string &category, string &reason)
{
    for (size_t i = 0; i < books.size(); i++)
    {
        if (books[i].getBookId() == bookId)
        {
            if (!title.empty())
            {
                books[i].setTitle(title);
            }
            if (!author.empty())
            {
                books[i].setAuthor(author);
            }
            if (!category.empty())
            {
                books[i].setCategory(category);
            }
            return true;
        }
    }
    reason = "未找到Id为" + bookId + "的图书";
    return false;
}

bool LibrarySystem::adjustCopies(const string &bookId, int newTotal, string &reason)
{
    if (newTotal < unreturnedBooksCount(bookId))
    {
        reason = "书籍总数不能小于当前书本未归还的数目!"; // 为了避免该书籍可借数量为负数的情景
        return false;
    }
    for (size_t i = 0; i < books.size(); i++)
    {
        if (books[i].getBookId() == bookId)
        {
            books[i].setTotalCopies(newTotal);
            return true;
        }
    }
    reason = "未找到Id为" + bookId + "的图书";
    return false;
}

bool LibrarySystem::setBookStatus(const string &bookId, BookStatus status, string &reason)
{
    if (status == BookStatus::OUT_SHELF && unreturnedBooksCount(bookId) > 0)
    {
        reason = "该书籍还存在被借出的记录,无法下架!";
        return false;
    }

    for (size_t i = 0; i < books.size(); i++)
    {
        if (books[i].getBookId() == bookId)
        {
            books[i].setStatus(status);
            return true;
        }
    }
    reason = "未找到Id为" + bookId + "的图书";
    return false;
}

// 第三批函数:借阅类
int LibrarySystem::findLoanIndex(const string &loanId) const
{
    for (size_t i = 0; i < loans.size(); i++)
    {
        if (loans[i].getLoanId() == loanId)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int LibrarySystem::getLoanCount() const
{
    return static_cast<int>(loans.size());
}

bool LibrarySystem::getLoanAtIndex(int index, LoanRecord &outloan) const
{
    if (index < 0 || index  >=static_cast<int>(loans.size()) )
    {
        return false;
    }

    outloan = loans[index];
    return true;
}

bool LibrarySystem::borrowBook(const string &bookId, const string &studentId, const string &today, string &reason)
{
    if (findBookIndex(bookId) == -1)
    {
        reason = "图书不存在!";
        return false;
    }
    Book outbook;
    getBookAtIndex(findBookIndex(bookId), outbook);
    if (outbook.getStatus() == BookStatus::OUT_SHELF)
    {
        reason = "该图书已下架";
        return false;
    }

    if (availableCopies(bookId) < 1)
    {
        reason = "库存不足";
        return false;
    }

    char buf[16];
    snprintf(buf, sizeof(buf), "L%04d", nextLoanSeq);
    string loanId(buf);
    nextLoanSeq++;
    string dueDate = today;
    addDays(dueDate, kLoanDays);
    loans.push_back(LoanRecord(loanId, studentId, bookId, today, "", dueDate));
    return true;
}

bool LibrarySystem::returnBook(const string &loanId, const string &today, string &reason)
{
    if (findLoanIndex(loanId) == -1)
    {
        reason = "未找到该借阅记录";
        return false;
    }
    if (loans[findLoanIndex(loanId)].isreturned())
    {
        reason = "该借阅记录已归还";
        return false;
    }
    if (!loans[findLoanIndex(loanId)].markasreturned(today))
    {
        reason = "归还日期不合法";
        return false;
    }
    return true;
}

vector<int> LibrarySystem::loansOfStudent(const string &studentId) const
{
    vector<int> result;
    for (int i = 0; i < loans.size(); i++)
    {
        if (loans[i].getStudentId() == studentId)
        {
            result.push_back(i);
        }
    }
    return result;
}

vector<int> LibrarySystem::overdueLoans(const string &today) const
{
    vector<int> result;
    for (size_t i = 0; i < loans.size(); i++)
    {
        if (loans[i].isoverdue(today) && !loans[i].isreturned())
        {
            result.push_back(static_cast<int>(i));
        }
    }
    return result;
}

void LibrarySystem::showMenu()
{
    while (true)
    {
        cout << "--------图书管理--------" << endl;
        cout << "1. 新增图书" << endl
             << "2. 图书列表" << endl
             << "3. 按编号查询" << endl
             << "4. 按书名查询" << endl
             << "5. 按作者查询" << endl
             << "6. 按分类查询" << endl
             << "7. 修改图书" << endl
             << "8. 调整库存" << endl
             << "9. 调整上架/下架" << endl
             << "10. 借书" << endl
             << "11. 还书" << endl
             << "12. 查看个人借阅记录" << endl
             << "13. 查看所有借阅记录" << endl
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
            getline(cin, line);
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
        case 2:
            listBooks();
            break;
        case 3:
        {
            Book outBook;
            string bookId;
            cout << "请输入书籍编号:" << endl;
            getline(cin, bookId);
            int index = findBookIndex(bookId);
            if (index == -1)
            {
                cout << "未找到该书籍!" << endl;
                break;
            }
            getBookAtIndex(index, outBook);
            cout << "已成功找到编号为" << bookId << "的书籍,信息如下:" << endl;
            outBook.displayBookInfo();
            break;
        }

        case 4:
        {
            string bookname;
            vector<int> result;
            cout << "请输入书名" << endl;
            getline(cin, bookname);
            result = searchByTitle(bookname);
            if (result.empty())
            {
                cout << "未找到该书籍!" << endl;
                break;
            }
            cout << "找到的书籍有:" << endl;
            Book outbook;
            for (int i = 0; i < result.size(); i++)
            {
                getBookAtIndex(result[i], outbook);
                outbook.displayBookInfo();
            }
            break;
        }

        case 5:
        {
            string author;
            vector<int> result;
            cout << "请输入作者" << endl;
            getline(cin, author);
            result = searchByAuthor(author);
            if (result.empty())
            {
                cout << "未找到该书籍!" << endl;
                break;
            }
            cout << "找到的书籍有:" << endl;
            Book outbook;
            for (int i = 0; i < result.size(); i++)
            {
                getBookAtIndex(result[i], outbook);
                outbook.displayBookInfo();
            }
            break;
        }

        case 6:
        {
            string category;
            vector<int> result;
            cout << "请输入分类" << endl;
            getline(cin, category);
            result = searchByCategory(category);
            if (result.empty())
            {
                cout << "未找到该书籍!" << endl;
                break;
            }
            cout << "找到的书籍有:" << endl;
            Book outbook;
            for (int i = 0; i < result.size(); i++)
            {
                getBookAtIndex(result[i], outbook);
                outbook.displayBookInfo();
            }
            break;
        }

        case 7:
        {
            string bookid, title, author, category;
            string reason = "";
            cout << "请按照此格式书本编号|书名|作者|种类输入信息：" << endl;
            string line1;
            getline(cin, line1);
            stringstream ss(line1);
            bool isValid = getline(ss, bookid, '|') && getline(ss, title, '|') && getline(ss, author, '|') && getline(ss, category, '|');
            if (!isValid)
            {
                cout << "无效的输入,请按照格式输入:书本编号|书名|作者|分类" << endl;
                break;
            }
            if (updateBook(bookid, title, author, category, reason))
            {
                cout << "更新成功" << endl;
                break;
            }
            else
            {
                cout << reason << endl;
                break;
            }
        }

        case 8:
        {
            string bookid, newcount;
            int newtotal;
            string line1;
            cout << "请按照输入书籍id" << endl;
            getline(cin, bookid);
            int index = findBookIndex(bookid);
            if (index == -1)
            {
                cout << "未找到该书籍" << endl;
                break;
            }
            cout << "请输入新的数量" << endl;
            getline(cin, newcount);
            stringstream ss(newcount);
            if (!(ss >> newtotal))
            {
                cout << "请输入整数!" << endl;
                break;
            }

            string extra;
            if (ss >> extra)
            {
                cout << "多余的字符" << extra << endl;
                break;
            }
            string reason = "";
            if (!adjustCopies(bookid, newtotal, reason))
            {
                cout << reason << endl;
                break;
            }
            cout << "调整成功" << endl;
            break;
        }

        case 9: // 调整书本上架下架
        {
            string bookId;
            string reason = "";
            cout << "请输入书籍编号:" << endl;
            getline(cin, bookId);
            int index = findBookIndex(bookId);
            if (index == -1)
            {
                cout << "未找到该书籍!" << endl;
                break;
            }
            cout << "请输入数字以调整:0表示上架,1表示下架" << endl;
            string num;
            getline(cin, num);
            stringstream ss(num);
            int n;
            if (!(ss >> n))
            {
                cout << "输入错误!" << endl;
                break;
            }
            else if (n == 0)
            {
                if (setBookStatus(bookId, BookStatus::ON_SHELF, reason))
                {
                    cout << "调整成功!" << endl;
                    break;
                }
                else
                {
                    cout << reason << endl;
                    break;
                }
            }
            else if (n == 1)
            {
                if (setBookStatus(bookId, BookStatus::OUT_SHELF, reason))
                {
                    cout << "调整成功!" << endl;
                    break;
                }
                else
                {
                    cout << reason << endl;
                    break;
                }
            }
            else
            {
                cout << "输入错误!" << endl;
                break;
            }
        }

        case 10: // 借书
        {
            string bookId, studentId;
            string date = today();
            string reason = "";
            cout << "请输入书籍编号" << endl;
            getline(cin, bookId);
            cout << "请输入学生Id" << endl;
            getline(cin, studentId);
            if ((borrowBook(bookId, studentId, date, reason)))
            {
                cout << "借书成功!" << endl;
                break;
            }
            else
            {
                cout << reason << endl;
                break;
            }
            break;
        }

        case 11: // 还书
        {
            string loanId;
            string date = today();
            string reason = "";
            cout << "请输入借书编号" << endl;
            getline(cin, loanId);
            if ((returnBook(loanId, date, reason)))
            {
                cout << "还书成功!" << endl;
                break;
            }
            else
            {
                cout << reason << endl;
                break;
            }
        }

        case 12: // 查询个人借阅记录
        {
            string studentId;
            cout << "请输入学生Id" << endl;
            getline(cin, studentId);
            vector<int> personalLoans = loansOfStudent(studentId);
            if (personalLoans.empty())
            {
                cout << "没有找到该学生借阅记录" << endl;
            }
            else
            {
                cout << "该学生借阅记录如下:" << endl;
                for (size_t i = 0; i < personalLoans.size(); i++)
                {
                    loans[personalLoans[i]].displayLoanRecordInfo();
                }
            }
            break;
        }

        case 13:
        {
            for (size_t i = 0; i < loans.size(); i++)
            {
                loans[i].displayLoanRecordInfo();
            }
            break;
        }

        default:
            break;
        }
    }
}
