#include <iostream>
#include <omp.h>
#include <time.h>
#include <cstdlib>
#include <memory>
#include <stdio.h> //输入输出
#include <stdlib.h>
#include <string.h>
#include <math.h>

using namespace std;
void Error(int key);
typedef struct error

{

    float max;

    float average;

} error;

template <class T>
class Matrix
{
private:
    long long col;
    long long row;
    long long step;
    shared_ptr<T> data; /// shared_ptr用于记录始祖矩阵的矩阵头
    T *ptr;
    bool isAncestor = true; //不做任何操作，默认是祖先矩阵
    int channel;            //稍后实现

public:
    Matrix()
    {
        this->row = 0;
        this->col = 0;
        this->step = 0;
        T *temp = new T[0 * 0]();
        this->ptr = temp;
        this->data = shared_ptr<T>(temp);
    }

    Matrix(const Matrix &a) ///拷贝构造函数,区分于赋值操作符重载只能采用硬拷贝
    {

        this->row = a.row;
        this->col = a.col;
        this->step = a.step;
        this->data = shared_ptr<T>(new T[a.row * a.col]()); //默认初始化为0
        this->ptr = this->data.get();
        this->isAncestor = a.isAncestor;
        T *MatA = this->data.get();
        T *MatB = a.data.get();
        for (long long i = 0; i < a.row * a.col; i++)
        {
            MatA[i] = MatB[i];
        }
    }

    Matrix(long long row, long long column, T *elements)
    {
        this->row = row;
        this->col = column;
        this->step = row;
        this->ptr = elements;
        this->data = shared_ptr<T>(elements);
    }

    Matrix(long long row, long long column)
    {
        T *temp = new T[column * row](); //默认初始化为0
        *temp = 0;
        this->row = row;
        this->col = column;
        this->step = row;
        this->ptr = temp;
        this->data = shared_ptr<T>(temp);
    }

    long long getStep()
    {
        return step;
    }

    long long getRow()
    {
        return row;
    }

    long long getColumn()
    {
        return col;
    }

    shared_ptr<T> getData()
    {
        return data;
    }
    T *getptr()
    {
        return ptr;
    }
    bool getIsAncestor()
    {
        return this->isAncestor;
    }

    Matrix &operator+(Matrix &a)
    {
        if (this->col != a.col || this->row != a.row)
        {
            Error(1); ///通过报错函数来实现报错，更美观
            return *new Matrix();
        }

        if (this->isAncestor && a.isAncestor)
        {
            //待修改为满足ROI的矩阵计算
            Matrix *c = new Matrix(a.row, a.col);
            T *MatA = this->data.get(); //创建三个临时指针，提升代码的可读性
            T *MatB = a.data.get();
            T *MatC = c->data.get();
#pragma omp parallel for schedule(dynamic)

            for (long long i = 0; i < a.row * a.col; i++)
            {

                MatC[i] = MatA[i] + MatB[i];
            }
            return *c;
        }
        else
        {
            //新矩阵是始祖矩阵
            Matrix *c = new Matrix(a.row, a.col);
            cout << "A的step：" << this->getStep() << "B的step：" << a.getStep() << " C的step：" << c->getStep();
#pragma omp parallel for schedule(dynamic)
            for (long long i = 0; i < a.row; i++)
            {
                for (long long j = 0; j < a.col; j++)
                {
                    *(c->Mat(i, j)) = *(this->Mat(i, j)) + *(a.Mat(i, j));
                }
            }
            return *c;
        }
    }

    Matrix &operator-(Matrix &a)
    {
        if (this->col != a.col || this->row != a.row)
        {
            Error(2);
            return *new Matrix();
        }
        if (this->isAncestor && a.isAncestor)
        {
            Matrix *c = new Matrix(a.row, a.col);
            T *MatA = this->data.get();
            T *MatB = a.data.get();
            T *MatC = c->data.get();

#pragma omp parallel for schedule(dynamic)
            for (long long i = 0; i < a.row * a.col; i++)
            {
                MatC[i] = MatA[i] - MatB[i];
            }
            return *c;
        }
        else
        {
            Matrix *c = new Matrix(a.row, a.col);
#pragma omp parallel for schedule(dynamic)
            for (long long i = 0; i < a.row; i++)
            {
                for (long long j = 0; j < a.col; j++)
                {
                    *(c->Mat(i, j)) = *(this->Mat(i, j)) - *(a.Mat(i, j));
                }
            }
            return *c;
        }
    }

    Matrix &operator*(Matrix &a)
    {
        if (this->col != a.row)
        {
            Error(3);
            return *new Matrix();
        }
        if (this->isAncestor && a.isAncestor) //两个都是祖先矩阵的时候才适用
        {
            Matrix *c = new Matrix(this->row, a.col);
            T *MatA = this->data.get();
            T *MatB = a.data.get();
            T *MatC = c->data.get();
#pragma omp parallel for schedule(dynamic)
            for (long long i = 0; i < this->row; i++)
            {
                for (long long k = 0; k < a.col; k++)
                {
                    T temp = MatA[this->col * i + k];
                    for (long long j = 0; j < this->col; j++) // mat1.col == mat2.
                    {
                        MatC[a.row * i + j] += temp * MatB[a.col * k + j];
                    }
                }
            }
            return *c;
        }
        else
        {
            //新矩阵新开辟一块内存空间,是内存访问上的问题,两种乘法的结果是一致的
            Matrix *c = new Matrix(this->row, a.col);
#pragma omp parallel for schedule(dynamic)
            for (long long i = 0; i < this->row; i++)
            {
                for (long long k = 0; k < a.col; k++)
                {
                    T *temp = this->Mat(i, k);
                    for (long long j = 0; j < this->col; j++)
                    {
                        *(c->Mat(i, j)) += *temp * *(a.Mat(k, j));
                    }
                }
            }
            return *c;
        }
    }

    Matrix &operator=(const Matrix &a)
    {
        this->row = a.row;
        this->col = a.col;
        this->ptr = a.ptr;
        this->step = a.step;
        this->isAncestor = a.isAncestor;

        cout << "赋值运算符重载" << endl;
        if (this->data == a.data) //本来就指向同一块内存指向同一块
        {
            Error(4); //提示无效赋值
            return *this;
        }
        else
        {
            this->data = a.data;
            return *this;
        }
    }

    bool operator==(const Matrix &a) const
    {
        if (this->row == a.row && this->col == a.col && this->step == a.step && this->data == a.data)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    void clear()
    {
        this->~Matrix();
    }

    void Reset(long long row, long long column, const T *element)
    {
        this->row = row;
        this->col = column;
        this->data.reset(element);
    }
    // T *ROI(long long rowStart, long long rowEnd, long long colStart, long long col End);

    void show()
    {
        if (this->isAncestor)
        {
            printf("Rows: %lld  Columns:%lld \n", this->row, this->col);
            cout << "[";
            T *Mat = this->data.get();
            for (long long i = 0; i < this->row * this->col; i++)
            {
                if ((i + 1) % this->col == 0) //最后一列
                {
                    if (i != this->row * this->col - 1)
                    {
                        cout << Mat[i] << ";" << endl;
                    }
                    else
                    {
                        cout << Mat[i] << "]" << endl;
                    }
                }
                else
                {
                    cout << Mat[i] << ", ";
                }
            }
        }
        else
        {
            printf("Rows: %lld  Columns:%lld \n", this->row, this->col);
            cout << "[";
            for (long long i = 0; i < this->row; i++)
            {
                for (long long j = 0; j < this->col; j++)
                {
                    if (j != this->col - 1) //不是最后一列
                    {
                        cout << *(this->Mat(i, j)) << ", ";
                    }
                    else //是最后一列
                    {
                        if (i != this->row - 1) //不是最后一行最后一列
                        {
                            cout << *(this->Mat(i, j)) << ";" << endl;
                        }
                        else
                        {
                            cout << *(this->Mat(i, j)) << "]" << endl;
                        }
                    }
                }
            }
        }
    }

    //挖去第r1~r2行、c1~c2列的矩阵，
    Matrix &ROI(long long startRow, long long endRow, long long startCol, long long endCol) //真正需要修改的是读取和计算过程,这里只需要简单的进行复制操作
    {
        if (startRow < 1 || endRow > this->row || startCol < 1 || endCol > this->col)
        {
            Error(5);
            return *new Matrix();
        }
        if (endRow - startRow + 1 > this->row || endCol - startCol + 1 > this->col)
        {
            Error(5);
            return *new Matrix();
        }
        ///错误提示
        long long row = endRow - startRow + 1;
        long long col = endCol - startCol + 1;

        Matrix<T> *c = new Matrix(row, col); //这里申请了新的内存，但是马上就被释放掉了，不存在内存泄露
        (c->data).reset(this->data.get());   //共享矩阵头,这里会使计数器发生变化
        c->step = this->step;                //继承始祖矩阵的step
        c->ptr = this->ptr + (startRow - 1) * c->step + (startCol - 1);
        c->isAncestor = false; //新矩阵不再是祖先矩阵

        return *c;
    }

    T *Mat(long long x, long long y) ///成员访问函数,用于访问矩阵的元素,等效二维数组
    {
        if ((x < 0) || (x > this->row - 1) || (y < 0) || (y > this->col - 1))
        {
            Error(6);
            return 0;
        }
        else
        {
            return getptr() + x * step + y; //返回一个指向这个数据的指针
        }
    }

    ~Matrix()
    {
        cout << "内存地址 " << this->data.get() << "   删除后的引用次数 : " << this->data.use_count() - 1 << endl;
    }
};

void Error(int key)
{

    cout << "Error Type ： " << key << endl;
    switch (key)
    {
    case 1:
        cout << "操作失败, 行数与列数不匹配，不能进行加法，请重试" << endl;
        break;
    case 2:
        cout << "操作失败, 行数与列数不匹配，不能进行减法，请重试" << endl;
        break;

    case 3:
        cout << "操作失败, 行数与列数不匹配，不能进行乘法，请重试" << endl;
        break;

    case 4:
        cout << "操作失败，等号左右的矩阵是完全相同的矩阵，请重试" << endl;
        break;

    case 5:
        cout << "输入的起始行、终止行、起始列或终止列存在错误，结果矩阵已置0，请重试" << endl;
        break;

    case 6:
        cout << "矩阵不存在目标行或目标列，返回0，请重试" << endl;
        break;
    default:
        break;
    }
}

template <class T>
error accuracyCheck(Matrix<T> A, Matrix<T> B)
{
    if (A.row != B.row || A.col != B.col)
    {
        printf("A and B have different size");
        error err;
        err.max = -1;
        err.average = -1;
        return err;
    }
    else
    {
        long long size = A.col * A.row;
        error err;
        err.max = 0;     //峰值
        err.average = 0; //平均值
        for (long long i = 0; i < size; i++)
        {

            // fabs求浮点数x的绝对值
            T delta = fabs(A.elements[i] - B.elements[i]) / B.elements[i];
            if (err.max < delta)
            {
                err.max = delta;
            }
            err.average += delta;
        }
        err.average = err.average / (size);
        return err;
    }
}

//测试时要使用float类型

float *read_matrix(FILE *fileprt, const char *filename)
{
    long long row, column = 0;

    fileprt = fopen(filename, "r");

    fscanf(fileprt, "%lld %lld\n", &row, &column);

    // T *elements = (T *)malloc(sizeof(float) * row * column);

    float *elements = new float[row * column]();

    for (long long i = 0; i < row * column; i++)
    {
        fscanf(fileprt, "%f", &elements[i]);
    }

    // Matrix<T> matrix = new Matrix<T>(row, column, elements);

    fclose(fileprt);

    return elements;
}

// template <class T>
// void write_matrix(Matrix<T> matrix, FILE *fileprt, const char *filename)
// {

//     fileprt = fopen(filename, "w");

//     //写入行数 和 列数
//     fprintf(fileprt, "%lld  %lld\n", matrix.getColumn(), matrix.getRow());

//     for (long long i = 0; i < matrix.getColumn() * matrix.getRow(); i++)
//     {
//         if ((i + 1) % matrix..getColumn()== 0) //列的最后一位
//         {

//             fprintf(fileprt, "%lf\n", matrix.elements[i]);
//         }
//         else
//         {
//             fprintf(fileprt, "%lf ", matrix.elements[i]);
//         }
//     }
//     fclose(fileprt);
// }