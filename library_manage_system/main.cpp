#include<windows.h>
#include"book.h"
#include"loan_record.h"
#include"dateutils.h"
#include<library_system.h>

using namespace std;
//用于测试的cpp文件,不必在意
int main()
{
    SetConsoleOutputCP(CP_UTF8); // 设置控制台输出为UTF-8编码
    SetConsoleCP(CP_UTF8);
    cout<<today()<<endl;
    LibrarySystem lib("data/library.txt");
    lib.showMenu();
}