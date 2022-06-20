#include <cstdlib>
#include <iostream>
#include <string>
#include "header.hpp"
#include <sys/time.h>
// #pragma GCC optimize(3)
using namespace std;

///先写一个最简单的double版本，还没有实现支持多个不同的存储单元类型
//
int main(int argc, char **argv)
{

#pragma omp parallel for schedule(dynamic)

    struct timeval start, end;

    const char *filename1 = argv[1];
    const char *filename2 = argv[2];
    const char *filename3 = argv[3];

    FILE *fileprt = NULL;

    // float *result = (float *)malloc(matrix1.row * matrix2.col * sizeof(float));

    float *a = read_matrix(fileprt, filename1);
    float *b = read_matrix(fileprt, filename2);
    Matrix<float> mat1(2048, 2048, a);
    Matrix<float> mat2(2048, 2048, b);

    gettimeofday(&start, NULL);
    Matrix<float> mat3 = mat1 * mat2;
    Matrix<float> mat4 = mat1 + mat2;
    Matrix<float> mat5 = mat1 - mat2;
    // write_matrix(mat3, fileprt, filename3);
    gettimeofday(&end, NULL);

    long timeuse = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;

    printf("The Required Time = %f Seconds\n", timeuse / 1000000.0);
}
