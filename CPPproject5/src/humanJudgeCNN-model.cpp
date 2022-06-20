//  ##### ���ڴ��CNN��3������㺯�� �� һ��ȫ���Ӳ㺯������CNN�ĹǼ�
#include "humanJudgeCNN.hpp"

using namespace std;

void humanJudgeCNN(const char *path)
{

    clock_t start = clock(); //����ʼ��ʱ

    DataBlob dataBlobs[10]{}; //��ΪҪʹ������ĳ�ʼ�� ���˷ѵ�һ���ռ���
    DataBlob startBlob(path);
    //############  �����1  ############
     convolution_Relu_withoutMatMul(startBlob, dataBlobs[1], 0);
//    convolution_Relu_withMatMul(startBlob, dataBlobs[1], 0);
    //############  �ػ���1  ############
    MaxPooling(dataBlobs[1], dataBlobs[2]);
    //############  �����2  ############
     convolution_Relu_withoutMatMul(dataBlobs[2], dataBlobs[3], 1);
//    convolution_Relu_withMatMul(dataBlobs[2], dataBlobs[3], 1);
    //############  �ػ���2  ############
    MaxPooling(dataBlobs[3], dataBlobs[4]);
    //############  �����3  ############
     convolution_Relu_withoutMatMul(dataBlobs[4], dataBlobs[5], 2);
//    convolution_Relu_withMatMul(dataBlobs[4], dataBlobs[5], 2);
    //############  ȫ���Ӳ�  ############
    float *vector = flatten_FC(dataBlobs[5]);
    softMax(vector);
    //############  �����ӡ  ############
    cout << "bg score: " << vector[0] * 100 << " % "
         << ", "
         << "face score: " << vector[1] * 100 << "% " << endl;

    clock_t end = clock(); //���������ʱ
    double endtime = (double)(end - start) / CLOCKS_PER_SEC ;
    cout << "Total time:" << endtime * 1000 << "ms" << endl; // msΪ��λ
}
