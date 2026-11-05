#pragma once
#include<string>
using namespace std;
struct Date{
    int year,month,day;
};
int getMaxDay(int year, int month);                        //用于获取当前月份的最大天数
void normalizeDate(Date& date);                            //用于标准化日期
bool isDateValid(string date);                             //用于判断日期是否合法,例如not-a-date和2026-13-45都会返回false(文档中指定日期格式为*-*-*)
bool parseDate(const string& date, Date& parsedDate);      // 解析日期字符串为Date结构体
bool compareDate(const string& date1,const string& date2); // 比较两个日期字符串的大小
bool addDays(string& date,int daynum);
