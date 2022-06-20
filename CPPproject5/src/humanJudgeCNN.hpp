//  ####  CNN的头文件

#pragma once
///用于比较大小的宏
#ifndef MIN
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#endif

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include "humanJudgeCNN-parameter.hpp"
#include <algorithm>
#include <ctime>
#include <omp.h>

#pragma GCC optimize(3, "Ofast", "inline")

using namespace std;
using namespace cv;

float *standardization(const char *path);

// Mat BGRToRGB(Mat &img);

inline int Convert(int old, int pad, int stride, int kernelSize);

inline float DotProduct_p1s2(int key, const float *curCentre, float *curKernelHead, int row, int col);

inline int typeSwitch_p1s2(const float *curPtr, float *start, int row, int col);

//用于进行最小单元计算的结构体,本次proj的行数列数都不会超过,channel step可能很大
class myMatrix //最小单元是一个多核矩阵
{
public:
    int row;
    int col;
    int channel;
    float *data;

    myMatrix()
    {
        row = 0;
        col = 0;
        channel = 0;
        data = nullptr;
    }

    myMatrix(int r, int c, int ch, float *d)
    {
        row = r;
        col = c;
        channel = ch;
        data = d;
    }

    //直接当做二维数组访问,因为循环体从0开始//访问通道C的二维数组[X][Y]的元素
    float operator()(int x, int y, int c) const
    {
        if ((x < 0) || (x > this->row - 1) || (y < 0) || (y > this->col - 1))
        {
            cerr << __FUNCTION__ << ":failed. Because you input row or col is incorrect" << endl;
            return -1;
        }

        if (c < 0 || c > this->channel)
        {
            cerr << __FUNCTION__ << ":failed. Because you input channel is incorrect" << endl;
            return -1;
        }

        else
        {
            return this->data[(c - 1) * row * col + y + this->col * x];
        }
    }

    float &operator()(int x, int y) const //重载，默认只有一个通道,减少计算操作
    {

        if (this->channel != 0)
        {
            cerr << __FUNCTION__ << ":failed. Because you input channel is incorrect" << endl;
            return *(float *)nullptr;  //返回空引用的做法
        }
        if ((x < 0) || (x > this->row - 1) || (y < 0) || (y > this->col - 1))
        {
            cerr << __FUNCTION__ << ":failed. Because you input row or col is incorrect" << endl;
            return *(float *)nullptr;
        }
        else
        {
            return this->data[y + this->col * x];
        }
    }

    float ptr(int x, int y) const //默认只有一个通道,减少计算操作
    {
        if (this->channel != 0)
        {
            cerr << __FUNCTION__ << ":failed. Because you input channel is incorrect" << endl;
            return 0;
        }
        if ((x < 0) || (x > this->row - 1) || (y < 0) || (y > this->col - 1))
        {

            return 0.0f; //等效越界了，直接返回0就可以了
        }
        else
        {
            return this->data[y + this->col * x];
        }
    }

    myMatrix &operator=(const myMatrix &) = delete;

    // myMatrix(const myMatrix &) = delete; //矩阵乘法会用到

    ~myMatrix()
    {                                              //内存不需要在这里释放，只用作计算的容器
        if (this->channel == 1 && data != nullptr) //用于计算的矩阵
        {
            delete[] data;
            data = nullptr;
        }
    }
};

class DataBlob
{ //这里模仿了face的写法，实际类似于proj4的矩阵
public:
    int rows;
    int cols;
    int channels;
    float *data; //实际运算使用的是float类型,但是这里的float*不是连续的，考虑修改构造函数和析构函数,BRG直接放在一起了

public:
    DataBlob() // DataBlob数值的改变只在全连接层和卷积层发生，不需要其他的初始化操作
    {
        rows = 0;
        cols = 0;
        channels = 0;
        data = nullptr;
    }

    DataBlob(const DataBlob &) = delete; //此类不存在拷贝操作

    DataBlob &operator=(const DataBlob &) = delete; //此类不存在赋值拷贝操作

    explicit DataBlob(const char *path) //直接把图像读取的操作内置在初始化blob中了，默认为第一次初始化
    {
        this->data = standardization(path);
        rows = 128;
        cols = 128;
        channels = 3;
    }

    float operator()(int x, int y, int c) const
    {

        if (c < 0 || c > this->channels)
        {
            cerr << __FUNCTION__ << ":failed. Because you input channel is incorrect" << endl;
            return -1;
        }

        if ((x < 0) || (x > this->rows - 1) || (y < 0) || (y > this->cols - 1))
        {
            return 0.0f; //越界直接返回0
        }

        else
        {
            return this->data[(c - 1) * rows * cols + y + cols * x];
        }
    }

    //在卷积操作过程中产生新的DataBlob

    bool isEmpty() const
    {
        if (this->rows == 0 || this->cols == 0 || this->channels == 0)
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
        this->~DataBlob();
    }

    ~DataBlob()
    {
        //依次释放掉
        if (this->data != nullptr)
        {
            this->rows = 0;
            this->cols = 0;
            this->channels = 0;
            delete[] this->data; //第一个元素是指向整块内存的头的指针
            this->data = nullptr;
        }
    }
};

//缺点：为了便利后续的计算操作，没有直接使用已有的一维数组，导致访问的内存空间不是连续的
// filter就在参数头文件中，定义好了，但是我们要进行转化
class Filter
{
public:
    int channels;   //当前层过滤器的通道数
    int num_kernel; //当前层过滤器的卷积核数
    int kernelSize; //当前层卷积核的大小
    int padding;    //当前层操作的填充数
    int stride;     //当前层操作的步长
    float *data;    //应该是四维： 卷积核个数 * BRG三通道 * 矩阵
    float *bias;    //最多32个元素，不考虑使用指针了,bias的个数和当前层的核数相同

    explicit Filter(int layer) //将获得的参数转化为过滤器，即多个矩阵 , 打算新开辟内存空间，虽然慢但不容易错
    {
        this->channels = conv_params012[layer].in_channels;
        this->num_kernel = conv_params012[layer].out_channels;
        this->kernelSize = conv_params012[layer].kernel_size;
        this->padding = conv_params012[layer].pad;
        this->stride = conv_params012[layer].stride;
        this->data = conv_params012[layer].p_weight; //记录权重矩阵
        this->bias = conv_params012[layer].p_bias;
    }

    Filter(const Filter &) = delete; //此类不存在拷贝操作

    Filter &operator=(const Filter &) = delete; //此类不存在赋值拷贝操作

    // filter中不存在于堆中开辟的内存
};

bool MaxPooling(DataBlob &old, DataBlob &result);

void OneByOneMaxPooling(const myMatrix &oldMat, myMatrix &newMat);

bool convolution_Relu_withoutMatMul(DataBlob &old, DataBlob &result, int layer);

void OneByOneConvolution(const myMatrix &curMat, const myMatrix &curKernel, const Filter &ftr, float *ptr, size_t blobSize);

void humanJudgeCNN(const char *path);

float *mat_mul(myMatrix &mat1, myMatrix &mat2);

float *flatten_FC(DataBlob &old);

void softMax(float *vector);

void oldMatTransform(myMatrix &oldMat, Filter &ftr, DataBlob &old);

bool mat_mul_conv(const myMatrix &left,const myMatrix &right, float *data);

bool convolution_Relu_withMatMul(DataBlob &old, DataBlob &result, int layer);