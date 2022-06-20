#include <stdio.h> //输入输出
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <omp.h> //OPENMP加速
#include "header.h"

#pragma GCC optimize(3)

Error accuracyCheck(Matrix A, Matrix B)
{
    if (A.row != B.row || A.col != B.col)
    {
        printf("A and B have different size");
        Error err;
        err.max = -1;
        err.average = -1;
        return err;
    }
    else
    {
        long long size = A.col * A.row;
        Error err;
        err.max = 0;     //峰值
        err.average = 0; //平均值
        for (long long i = 0; i < size; i++)
        {

            // fabs求浮点数x的绝对值
            float delta = fabs(A.elements[i] - B.elements[i]) / B.elements[i];
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

void delete (Matrix matrix)
{
    float *ptr = matrix.elements;
    free(ptr);
    matrix.row = 0;
    matrix.col = 0;
}

Matrix creat1(long long row, long long col)
{

    float *elements = (float *)malloc(sizeof(float) * row * col);
    Matrix matrix = {row, col, elements};
    return matrix;
}

//假设文件的第一行保存了矩阵的 行数 和  列数
//从文件中读入矩阵

///数据读入存在bug

Matrix creat2(FILE *fileprt, const char *filename)
{
    long long row, column = 0;

    fileprt = fopen(filename, "r");

    fscanf(fileprt, "%lld %lld\n", &row, &column);

    float *elements = (float *)malloc(sizeof(float) * row * column);

    for (long long i = 0; i < row * column; i++)
    {
        fscanf(fileprt, "%f", &elements[i]);
    }

    Matrix matrix = {row, column, elements};

    fclose(fileprt);

    return matrix;
}

//默认创建单位矩阵
//尝试设置数量矩阵
Matrix creat3(long long row, long long col)
{
    float *arr = (float *)malloc(row * col * sizeof(float));
    for (long long i = 0; i < row * col; i++)
    {
        arr[i] = 1.0;
    }

    //默认设计为单位矩阵

    Matrix matrix = {row, col, arr};

    return matrix;
}

///生成随机矩阵 , 默认为大于等于0的数
Matrix creat4(long long row, long long col, float bound)
{
    float *arr = (float *)malloc(row * col * sizeof(float));
    for (int i = 0; i < row * col; i++)
    {
        arr[i] = 0 + 1.0 * rand() / RAND_MAX * (bound);
    }
}

Matrix creat5(long long row, long long col, float *arr)
{
    Matrix matrix = {row, col, arr};
    return matrix;
}

void copy(Matrix mat1, Matrix mat2)
{ // mat1是复制完后的原数组，mat2是复制目标
    mat1.col = mat2.col;
    mat1.row = mat2.col;
    mat1.elements = mat2.elements; //存在bug，mat2内存释放后，mat1的矩阵失效
}

//参考博客：https://blog.csdn.net/flying_saker/article/details/83350731

///先尝试使用一维数组的朴素三重循环

Matrix mul_slow(Matrix mat1, Matrix mat2)
{

    if (mat1.col == mat2.row)
    {

        float *arr = (float *)malloc(mat1.row * mat2.col * sizeof(float));

#pragma omp parallel for schedule(dynamic)
        for (long long i = 0; i < mat1.row; i++)
        {
            for (long long j = 0; j < mat2.row; j++)
            {
                for (long long k = 0; k < mat1.col; k++) // mat1.col == mat2.row
                {
                    arr[mat2.row * i + j] += mat1.elements[mat1.col * i + k] * mat2.elements[mat2.col * k + j];
                }
            }
        }
        Matrix result = {mat1.row, mat2.col, arr};

        return result;
    }
    else
    {
        printf("Can not mul because Mat1's row is not equal to Mat2's column");
    }
}

Matrix mul_fast(Matrix mat1, Matrix mat2)
{
    if (mat1.col == mat2.row)
    {

        float *arr = (float *)malloc(mat1.row * mat2.col * sizeof(float));

#pragma omp parallel for schedule(dynamic)
        for (long long i = 0; i < mat1.row; i++)
        {
            for (long long k = 0; k < mat2.row; k++)
            {

                float s = mat1.elements[mat1.col * i + k];
                for (long long j = 0; j < mat1.col; j++) // mat1.col == mat2.
                {
                    arr[mat2.row * i + j] += s * mat2.elements[mat2.col * k + j];
                }
            }
        }
        Matrix result = {mat1.row, mat2.col, arr};

        return result;
    }
    else
    {
        printf("Can not mul because Mat1's row is not equal to Mat2's column");
    }
}

void print_matrix(Matrix Matrix)
{

    for (long long i = 0; i < Matrix.row * Matrix.col; i++)
    {
        if ((i + 1) % Matrix.col == 0) //列的最后一位
        {
            printf("%f\n", Matrix.elements[i]);
        }
        else
        {
            printf("%f ", Matrix.elements[i]);
        }
    }
}

void write_matrix(Matrix matrix, FILE *fileprt, const char *filename)
{

    fileprt = fopen(filename, "w");

    //写入行数 和 列数
    fprintf(fileprt, "%lld  %lld\n", matrix.row, matrix.col);

    for (long long i = 0; i < matrix.row * matrix.col; i++)
    {
        if ((i + 1) % matrix.col == 0) //列的最后一位
        {

            fprintf(fileprt, "%lf\n", matrix.elements[i]);
        }
        else
        {
            fprintf(fileprt, "%lf ", matrix.elements[i]);

        }
    }
    fclose(fileprt);
}