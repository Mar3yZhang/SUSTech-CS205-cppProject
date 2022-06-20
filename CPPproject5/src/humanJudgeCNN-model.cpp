//  ##### 用于存放CNN的3个卷积层函数 和 一个全连接层函数，是CNN的骨架
#include "humanJudgeCNN.hpp"

using namespace std;

void humanJudgeCNN(const char *path)
{

    clock_t start = clock(); //程序开始计时

    DataBlob dataBlobs[10]{}; //因为要使用特殊的初始化 ，浪费第一个空间了
    DataBlob startBlob(path);
    //############  卷积层1  ############
     convolution_Relu_withoutMatMul(startBlob, dataBlobs[1], 0);
//    convolution_Relu_withMatMul(startBlob, dataBlobs[1], 0);
    //############  池化层1  ############
    MaxPooling(dataBlobs[1], dataBlobs[2]);
    //############  卷积层2  ############
     convolution_Relu_withoutMatMul(dataBlobs[2], dataBlobs[3], 1);
//    convolution_Relu_withMatMul(dataBlobs[2], dataBlobs[3], 1);
    //############  池化层2  ############
    MaxPooling(dataBlobs[3], dataBlobs[4]);
    //############  卷积层3  ############
     convolution_Relu_withoutMatMul(dataBlobs[4], dataBlobs[5], 2);
//    convolution_Relu_withMatMul(dataBlobs[4], dataBlobs[5], 2);
    //############  全连接层  ############
    float *vector = flatten_FC(dataBlobs[5]);
    softMax(vector);
    //############  结果打印  ############
    cout << "bg score: " << vector[0] * 100 << " % "
         << ", "
         << "face score: " << vector[1] * 100 << "% " << endl;

    clock_t end = clock(); //程序结束用时
    double endtime = (double)(end - start) / CLOCKS_PER_SEC ;
    cout << "Total time:" << endtime * 1000 << "ms" << endl; // ms为单位
}
