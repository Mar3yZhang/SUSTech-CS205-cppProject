#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <omp.h>
#include <math.h>
#include "header.h"
#include <cblas.h>

#pragma GCC optimize(3)

const enum CBLAS_ORDER Order = CblasRowMajor;
const enum CBLAS_TRANSPOSE TransA = CblasNoTrans;
const enum CBLAS_TRANSPOSE TransB = CblasNoTrans;
const float alpha = 1;
const float beta = 0;
int main(int argc, char **argv)
{

    #pragma omp parallel for schedule(dynamic)

        // i7 10750H 线程数为15
        for (int i = 0; i < 15; i++)
        {
            printf("Thread Check number %d from thread = %d\n", i, omp_get_thread_num());
        }

    //直接使用time.h计时存在问题，故使用过新的计时方法，
    //参考博客： https://blog.csdn.net/fz_ywj/article/details/8109368

    struct timeval start, end;

    const char *filename1 = argv[1];
    const char *filename2 = argv[2];
    const char *filename3 = argv[3];

    FILE *fileprt = NULL;
    Matrix matrix1 = creat2(fileprt, filename1);

    Matrix matrix2 = creat2(fileprt, filename2);

    float *result = (float *)malloc(matrix1.row * matrix2.col * sizeof(float));

    gettimeofday(&start, NULL);

    cblas_sgemm(Order, TransA, TransB, matrix1.row, matrix2.col,
                matrix1.col, alpha, matrix1.elements, matrix1.col,
                matrix2.elements, matrix2.col, beta, result, matrix2.col);


    Matrix matrix3 = mul_fast(matrix1, matrix2);

    gettimeofday(&end, NULL);

    long timeuse = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;

    printf("The Required Time = %f Seconds\n", timeuse / 1000000.0);

    Matrix matrix3 = mul_fast(matrix1, matrix2);

    Matrix correct = creat5(matrix1.row, matrix2.col, result);

    Error error = accuracyCheck(matrix3, correct);

    printf("The max error: %f%%\n", error.max);
    printf("The average error: %f%%", error.average);

    write_matrix(correct,fileprt,filename3 );  //使用openBlas计算

    write_matrix(matrix3, fileprt, filename3);
}
