#pragma once

typedef struct Matrix
{
    long long row;
    long long col;
    float *elements;

} Matrix;

typedef struct Error
{
    float max;
    float average;
} Error;

Error accuracyCheck(Matrix A, Matrix B);

void delete (Matrix matrix);

Matrix creat1(long long row, long long col);

Matrix creat2(FILE *fileprt, const char *filename);

Matrix creat3(long long row, long long col);

Matrix creat4(long long row, long long col, float bound);

Matrix creat5(long long row, long long col, float *arr);

void copy(Matrix mat1, Matrix mat2);

Matrix mul_slow(Matrix mat1, Matrix mat2);

Matrix mul_fast(Matrix mat1, Matrix mat2);

void print_matrix(Matrix Matrix);

void write_matrix(Matrix matrix, FILE *fileprt, const char *filename);
