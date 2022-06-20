#include <iostream>
#include <string>
#include <cstdlib>
#include <sstream>
#include <algorithm>   //调用字符串反转的函数

using namespace std;

bool isInteger(string a);    //函数声明
long long transform(string a);
string IntToString(int &i);
bool isNegative(string a);
string Multiplication(string a, string b);

bool isInteger(string a) //用于判断传入的字符串是否为整数
{
    bool judge = false;
    for (int i = 0; i < a.length(); ++i)
    {
        if (a.at(0) == '0' && a.length() != 1) //排除不正当输入的情况
        {
            return false;
        }
        if (a.at(i) == '-' && a.length() > 1) //a可能会出现负数
        {
            continue;
        }
        else
        {
            if (!(a.at(i) <= '0' || a.at(i) >= '9')) //字符0-9在ASCII中连续出现，包含不在0-9之间的字符就一定不是整数
            {
                judge = true;
                break;
            }
        }
    }
    return judge;
}

long long transform(string a) //将字符串转化为long long类型
{
    long long answer = atoll(a.c_str()); //atoll接收char数组的地址，返回对应的longlong类型整数
    return answer;                       //c_str()函数返回一个指向正规C字符串的指针, 内容与本string串相同
}

string IntToString(int &i) //此方法将传入整数类型地址返回相应字符串，摘自博客：http://www.voidcn.com/article/p-onwlyush-ut.html
{
    string s;
    stringstream ss(s);
    ss << i;
    return ss.str();
}

bool isNegative(string a) //判断一个整数字符串是否是负数
{
    return a.at(0) == '-';
}

string Multiplication(string a, string b) //此时的string a和b一定是正确输入
{
    string longStr = a.length() >= b.length() ? a : b;
    string shortStr = a.length() < b.length() ? a : b;
    bool judge1 = false; //利用逻辑运算用于判断最后的结果是否包含符号
    bool judge2 = false;
    if (isNegative(longStr))
    {
        judge1 = true;
        longStr.erase(remove(longStr.begin(), longStr.end(), '-'), longStr.end());
    }
    if (isNegative(shortStr))
    {
        judge2 = true;
        shortStr.erase(remove(shortStr.begin(), shortStr.end(), '-'), shortStr.end());
    }

    reverse(longStr.begin(), longStr.end());
    reverse(shortStr.begin(), shortStr.end());
    int length = a.length() + b.length() + 1;
    int array[length];

    for (int i = 0; i < length; i++) //先把数组完全置0
    {
        array[i] = 0;
    }

    int temp = 0;

    for (int i = 0; i < shortStr.length(); i++) //得到位数还没有进位的数组
    {
        for (int j = 0; j < longStr.length(); j++)
        {
            array[j + temp] = array[j + temp] + (shortStr.at(i) - 48) * (longStr.at(j) - 48);
        }
        temp++;
    }

    for (int i = 0; i < length; i++) //进位操作
    {
        if (array[i] >= 10)
        {
            int carry = array[i] / 10;
            array[i] = array[i] % 10;
            array[i + 1] = array[i + 1] + carry;
        }
    }

    temp = length - 1; //找到第一个非零的数，即为乘积的最大位，temp变量重复使用，节约空间
    while (array[temp] == 0)
    {
        temp--;
    }
    string answer;
    if (judge1 || judge2) //存在负数
    {
        if ((judge1 && !judge2) || (!judge1 && judge2)) //一负一正的情况
        {
            answer = answer + "-";
        }
    }

    for (temp; temp >= 0; temp--)
    {
        answer = answer + IntToString(array[temp]);
    }
    return answer;
}

int main(int argc, char **argv) //
{
    string a, b;
    if (argc > 1)
    {
        char *var1 = argv[1];
        char *var2 = argv[2];
        a = var1;
        b = var2;
    }
    else
    {
        cout << "Please input two integers" << endl;
        cin >> a;
        cin >> b;
    }

    bool check = true; //尝试直到输入正确为止
    while (check)
    {
        if (isInteger(a) && isInteger(b))
        {
            check = false;
            long long a_Number = transform(a);
            long long b_Number = transform(b);
            cout << "低精度计算器：" << endl;
            cout << a_Number << " * " << b_Number << "= " << a_Number * b_Number << endl;
            cout << "高精度计算器：" << endl;
            cout << a << " * " << b << "= " << Multiplication(a, b) << endl;
        }
        else
        {
            cout << "Illegal input , please try again" << endl;
            cin >> a;
            cin >> b;
        }
    }
    return 0;
}
