#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <thread>
#include <cmath>
#include <bits/stdc++.h> ///万能头文件

using namespace std;

vector<vector<double>> file_Reading(string filename);
void float_file_Writing(float **float_matrix, int size, string filename);
void double_file_Writing(double **double_matrix, int size, string filename);
double **FastMatMul(vector<vector<double>> A, vector<vector<double>> B, int row);
float **FastMatMul(int row, vector<vector<double>> A, vector<vector<double>> B);
double **SlowMatMul(vector<vector<double>> A, vector<vector<double>> B, int row);
float **SlowMatMul(int row, vector<vector<double>> A, vector<vector<double>> B);
void print_float_Storage_Format(float f);
void print_double_Storage_Format(double f);
int transfer_to_binaryNumber(int n);
void DoubleToString(double *p_dl);
void thread1(int size, vector<vector<double>> doubletempA, vector<vector<double>> doubletempB, string filename);
void thread2(int size, vector<vector<double>> doubletempA, vector<vector<double>> doubletempB, string filename);
double **double_block_MatMul(vector<vector<double>> A, vector<vector<double>> B, int row);
float **float_block_MatMul(vector<vector<double>> A, vector<vector<double>> B, int row);

int main(int argc, char **argv)
{
    clock_t AllStart = clock();

    vector<vector<double>> doubletempA, doubletempB; // 设置可变长度宽度的二维Vector容器
    string filename = argv[3];
    int size;
    doubletempA = file_Reading(argv[1]);
    doubletempB = file_Reading(argv[2]);
    size = doubletempA.size(); //用来创建矩阵的关键量

    thread task01 = thread(thread1, size, doubletempA, doubletempB, filename);
    thread task02 = thread(thread2, size, doubletempA, doubletempB, filename);
    task01.join();
    task02.join();

    clock_t AllEnd = clock();

    cout << "总时长： " << (double)(AllEnd - AllStart) / CLOCKS_PER_SEC << " s" << endl;

    return 0;

    //验证浮点数在内存中的存储形式
    // cout << "float计算结果： " << float_Answer[0][0] << endl;
    // cout << "float存储形式结果： " << endl;
    // print_float_Storage_Format(float_Answer[0][0]);
    // cout << "double计算结果： " << double_Answer[0][0] << endl;
    // cout << "double存储形式结果： " << endl;
    // DoubleToString(&double_Answer[0][0]);
}

void thread1(int size, vector<vector<double>> doubletempA, vector<vector<double>> doubletempB, string filename)
{
    clock_t floatStart = clock();
    float **float_Answer = FastMatMul(size, doubletempA, doubletempB);
    float_file_Writing(float_Answer, size, filename);
    clock_t floatEnd = clock();
    delete float_Answer;
    cout << "float类型矩阵的输出时长： " << (double)(floatEnd - floatStart) / CLOCKS_PER_SEC << " s" << endl;
}

void thread2(int size, vector<vector<double>> doubletempA, vector<vector<double>> doubletempB, string filename)
{
    clock_t doubleStart = clock();
    double **double_Answer = FastMatMul(doubletempA, doubletempB, size);
    double_file_Writing(double_Answer, size, filename);
    clock_t doubleEnd = clock();
    delete double_Answer;
    cout << "double类型矩阵的输出时长： " << (double)(doubleEnd - doubleStart) / CLOCKS_PER_SEC << " s" << endl;
}

void float_file_Writing(float **float_matrix, int size, string filename)
{
    ofstream outfile("float_" + filename);
    outfile << "Float version :" << endl;
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            outfile << float_matrix[i][j] << " "; ///设置为最大精度
        }
        outfile << endl;
    }
    outfile << endl
            << endl
            << endl
            << endl
            << endl
            << endl
            << endl;
    outfile << flush;
    outfile.close();
}

void double_file_Writing(double **double_matrix, int size, string filename)
{
    ofstream outfile("double_" + filename);
    outfile << "Double version :" << endl;
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            outfile << double_matrix[i][j] << " "; ///设置为最大精度
        }
        outfile << endl;
    }
    outfile << endl
            << endl
            << endl
            << endl
            << endl
            << endl
            << endl;
    outfile << flush;
    outfile.close();
}

vector<vector<double>> file_Reading(string filename)
{
    vector<vector<double>> doubletempA;
    ifstream inflie;
    string str1;
    inflie.open(filename);
    while (getline(inflie, str1)) //读到空白处停止
    {
        istringstream input(str1);
        vector<double> tmp;
        double a;
        while (input >> a)
        {
            tmp.push_back(a);
        }
        doubletempA.push_back(tmp);
    }
    inflie.close();
    return doubletempA;
}

//参考博客： https://blog.csdn.net/qq_34149581/article/details/89575227

double **double_block_MatMul(vector<vector<double>> A, vector<vector<double>> B, int row)
{
    int size = row;
    double **answer;
    answer = new double *[size];   //更换循环顺序：https://zhuanlan.zhihu.com/p/146250334
    for (int i = 0; i < size; i++) //动态内存申请
    {
        answer[i] = new double[size];
    }

    int block_size = 7;
    for (int bi = 0; bi < row; bi += block_size)
    {
        for (int bj = 0; bj < row; bj += block_size)
        {
            for (int bk = 0; bk < row; bk += block_size)
            {
                for (int i = bi; i < min(bi + block_size, row); ++i)
                {
                    for (int j = bj; j < min(bj + block_size, row); ++j)
                    {
                        for (int k = bk; k < min(bk + block_size, row); ++k)
                        {
                            answer[i][j] += A[i][k] * B[k][j];
                        }
                    }
                }
            }
        }
    }
    return answer;
}
float **float_block_MatMul(vector<vector<double>> A, vector<vector<double>> B, int row)
{
    int size = row;
    float **answer;
    answer = new float *[size];
    for (int i = 0; i < size; i++)
    {
        answer[i] = new float[size];
    }

    int block_size = 7;
    for (int bi = 0; bi < row; bi += block_size)
    {
        for (int bj = 0; bj < row; bj += block_size)
        {
            for (int bk = 0; bk < row; bk += block_size)
            {
                for (int i = bi; i < min(bi + block_size, row); ++i)
                {
                    for (int j = bj; j < min(bj + block_size, row); ++j)
                    {
                        for (int k = bk; k < min(bk + block_size, row); ++k)
                        {
                            answer[i][j] += A[i][k] * B[k][j];
                        }
                    }
                }
            }
        }
    }
    return answer;
}

double **FastMatMul(vector<vector<double>> A, vector<vector<double>> B, int row) //乘法函数重载
{

    int size = row;
    double **answer;
    answer = new double *[size];
    for (int i = 0; i < size; i++) //动态内存申请
    {
        answer[i] = new double[size];
    }

    //ijk精度会高很多
    for (int i = 0; i < size; i++) //更换循环顺序：https://zhuanlan.zhihu.com/p/146250334
    {
        for (int k = 0; k < size; k++) //就是计算过程中出现了错误
        {
            double middle1 = A[i][k];
            for (int j = 0; j < size; j++)
            {
                answer[i][j] += middle1 * B[k][j];
            }
        }
    }
    return answer;
}

float **FastMatMul(int row, vector<vector<double>> A, vector<vector<double>> B) //乘法函数重载
{
    int size = row;
    float **answer;
    answer = new float *[size];    //更换循环顺序：https://zhuanlan.zhihu.com/p/146250334
    for (int i = 0; i < size; i++) //动态内存申请
    {
        answer[i] = new float[size];
    }

    for (int i = 0; i < size; i++)
    {
        for (int k = 0; k < size; k++)
        {
            float middle2 = A[i][k];
            for (int j = 0; j < size; j++)
            {
                answer[i][j] += middle2 * B[k][j];
            }
        }
    }

    return answer;
}

double **SlowMatMul(vector<vector<double>> A, vector<vector<double>> B, int row) //乘法函数重载
{
    int size = row;
    double **answer;
    answer = new double *[size];   //更换循环顺序：https://zhuanlan.zhihu.com/p/146250334
    for (int i = 0; i < size; i++) //动态内存申请
    {
        answer[i] = new double[size];
    }

    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            for (int k = 0; k < size; k++)
            {
                answer[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    return answer;
}

float **SlowMatMul(int row, vector<vector<double>> A, vector<vector<double>> B) //乘法函数重载
{
    int size = row;
    float **answer;
    answer = new float *[size];    //更换循环顺序：https://zhuanlan.zhihu.com/p/146250334
    for (int i = 0; i < size; i++) //动态内存申请
    {
        answer[i] = new float[size];
    }
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            for (int k = 0; k < size; k++)
            {
                answer[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return answer;
}

void print_float_Storage_Format(float f) ///float类型的长度是4bytes,32bits
{
    char *p = (char *)&f; /////将存储结果转换成16进制数
    cout << "[";
    cout << transfer_to_binaryNumber((int)(*p));
    cout << transfer_to_binaryNumber((int)(*(p + 1)));
    cout << transfer_to_binaryNumber((int)(*(p + 2)));
    cout << transfer_to_binaryNumber((int)(*(p + 3))) << "]" << endl;
}

void print_double_Storage_Format(double f) ///double类型的长度是8bytes,64bits
{
    char *p = (char *)&f; /////将存储结果转换成16进制数
    cout << hex << ((int)(*p)) << " ";
    cout << hex << ((int)(*(p + 1))) << " ";
    cout << hex << ((int)(*(p + 2))) << " ";
    cout << hex << ((int)(*(p + 3))) << " ";
    cout << hex << ((int)(*(p + 4))) << " ";
    cout << hex << ((int)(*(p + 5))) << " ";
    cout << hex << ((int)(*(p + 6))) << " ";
    cout << hex << ((int)(*(p + 7))) << endl;
}

//https://blog.csdn.net/mydriverc2/article/details/84628950 ；
void DoubleToString(double *p_dl)
{
    char c_dl_16[16 + 1];
    char c_dl_64[64 + 1];
    memset(c_dl_64, '\0', sizeof(c_dl_64));
    unsigned char *t = (unsigned char *)p_dl;
    int charCount = sizeof(double);
    memset(c_dl_16, '\0', sizeof(c_dl_16));

    if (t == NULL)
        return;
    int i = 0;
    for (i = 0; i < charCount; i++)
    {
        sprintf(c_dl_16 + i * 2, "%02x", t[i]);
    }
    /*printf("\n");  */
    printf("[%s]\n", c_dl_16);
    /*对每一位，将十六进制转换为二进制的字符串*/
    char tmpc[4 + 1];
    for (int j = 0; j < 16; j++)
    {
        memset(tmpc, '\0', sizeof(tmpc));
        switch (c_dl_16[j])
        {
        case '0':
            memcpy(tmpc, "0000", 4);
            break;
        case '1':
            memcpy(tmpc, "0001", 4);
            break;
        case '2':
            memcpy(tmpc, "0010", 4);
            break;
        case '3':
            memcpy(tmpc, "0011", 4);
            break;
        case '4':
            memcpy(tmpc, "0100", 4);
            break;
        case '5':
            memcpy(tmpc, "0101", 4);
            break;
        case '6':
            memcpy(tmpc, "0110", 4);
            break;
        case '7':
            memcpy(tmpc, "0111", 4);
            break;
        case '8':
            memcpy(tmpc, "1000", 4);
            break;
        case '9':
            memcpy(tmpc, "1001", 4);
            break;
        case 'a':
            memcpy(tmpc, "1010", 4);
            break;
        case 'b':
            memcpy(tmpc, "1011", 4);
            break;
        case 'c':
            memcpy(tmpc, "1100", 4);
            break;
        case 'd':
            memcpy(tmpc, "1101", 4);
            break;
        case 'e':
            memcpy(tmpc, "1110", 4);
            break;
        case 'f':
            memcpy(tmpc, "1111", 4);
            break;
        }
        sprintf(c_dl_64 + j * 4, "%s", tmpc);
    }
    printf("[%s]\n", c_dl_64);
}

//利用数字逻辑十进制转二进制数的方法设计的转换函数
int transfer_to_binaryNumber(int n)
{
    int flag = 1, y = 0, remainder;
    while (1)
    {
        remainder = n % 2;
        n /= 2;
        y += remainder * flag;
        flag *= 10;
        if (n == 0)
        {
            break;
        }
    }
    return y;
}
