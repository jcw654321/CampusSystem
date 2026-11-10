#include <dateutils.h>
#include <cstdio>
int getMaxDay(int year, int month)
{
    int dayofmonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && (year % 4 == 0 && year % 100 != 0 || year % 400 == 0))
    {
        return 29;
    }
    return dayofmonth[month - 1];
}

void normalizeDate(Date &date)
{
    while(date.day<1)
    {
        if(date.month==1)
        {
            date.month=12;
            date.year--;
        }
        else
        {
            date.month--;
        }
        date.day+=getMaxDay(date.year,date.month);
    }
    while (date.day > getMaxDay(date.year, date.month))
    {
        date.day -= getMaxDay(date.year, date.month);
        date.month++;
        if (date.month > 12)
        {
            date.month -= 12;
            date.year++;
        }
    }
}

bool isDateValid(string date)
{
    string y, m, d;
    size_t pos1 = date.find('-');
    y = date.substr(0, pos1);
    size_t pos2 = date.find('-', pos1 + 1);
    m = date.substr(pos1 + 1, pos2 - pos1 - 1);
    d = date.substr(pos2 + 1, date.length() - pos2 - 1);
    if (y.empty() || m.empty() || d.empty() || y.length() > 4 || m.length() > 2 || d.length() > 2)
        return false;

    for (int i = 0; i < y.length(); i++)
    {
        char c = y[i];
        if (c > 57 || c < 48)
        {
            return false;
        }
    }
    for (int i = 0; i < m.length(); i++)
    {
        char c = m[i];
        if (c > 57 || c < 48)
        {
            return false;
        }
    }
    for (int i = 0; i < d.length(); i++)
    {
        char c = d[i];
        if (c > 57 || c < 48)
        {
            return false;
        }
    }
    int year = stoi(y);
    int month = stoi(m);
    int day = stoi(d);
    if (year < 0 || month < 1 || month > 12 || day < 1)
    {
        return false;
    }
    if (day > getMaxDay(year, month))
    {
        return false;
    }
    return true;
}

bool parseDate(const string &date, Date &parsedDate)
{
    int y, m, d, usedstr;
    if (sscanf_s(date.c_str(), "%d-%d-%d%n", &y, &m, &d, &usedstr) != 3 || usedstr != date.length())
        return false;
    if (y < 0 || m < 1 || m > 12 || d < 1)
        return false;
    int maxday = getMaxDay(y, m);
    if (d > maxday)
        return false;
    parsedDate.year = y;
    parsedDate.month = m;
    parsedDate.day = d;
    return true;
}

bool compareDate(const string &date1, const string &date2)
{ // 如果date1>date2返回true，否则返回false
    Date d1, d2;
    if (!parseDate(date1, d1) || !parseDate(date2, d2)) // 解析日期失败则返回false
    {
        return false;
    }
    if (d1.year > d2.year)
        return true;
    if (d1.year < d2.year)
        return false;
    if (d1.month > d2.month)
        return true;
    if (d1.month < d2.month)
        return false;
    return d1.day > d2.day;
}

bool addDays(string &date, int daynum)
{
    Date date_s;
    if (!parseDate(date, date_s))
    {
        return false;
    }
    date_s.day += daynum;
    normalizeDate(date_s);
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d", date_s.year, date_s.month, date_s.day);
    string dateadded(buffer);
    date = dateadded;
    return true;
}