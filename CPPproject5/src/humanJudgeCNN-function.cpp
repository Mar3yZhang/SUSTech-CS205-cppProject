//  ##### 用于存放处理数据的函数
//只引入头文件即可，防止重复引入头文件

#include "humanJudgeCNN.hpp"
#include <cmath>

//要写单元测试

//对输入进行初始化步骤之一，压缩图片为目标输入值
//注： 还没有做输入的检错操作,待功能实现后再写

//应该把所有的数据都存在
float *standardization(const char *path)
{
    Mat pre = imread(path);
    Mat temp;
    resize(pre, temp, Size(128, 128), (0.0), (0.0), INTER_LINEAR);

    if (temp.channels() != 3)
    {
        cerr << __FUNCTION__ << ":failed. Because image is not a RGB image." << endl;
        return nullptr;
    }
    else
    {
        //访问mat中的元素，将他们存到三个一维数组矩阵里面,然后这个一维数组可以用于proj3数组的初始化
        float *data = new float[3 * 128 * 128]{}; //申请整块内存空间//存储的是子矩阵的矩阵头的指针,方便处理
#pragma omp for schedule(dynamic)
        for (size_t k = 0; k < 3; ++k) //依次访问 B R G 在mat[i，j]中的元素
        {
            for (size_t i = 0; i < 128; ++i)
            {
                for (size_t j = 0; j < 128; ++j)
                {
                    data[k * 128 * 128 + i * 128 + j] = temp.at<Vec3b>(i, j)[k] / 255.0;
                }
            }
        }
        return data;
    }
}

inline int Convert(int old, int pad, int stride, int kernelSize)
{
    int temp = (old + 2 * pad - kernelSize) / stride;
    return temp + 1;
}

//第0层卷积运算 ，p = 1 , S = 2, K = 3

//一一对应的卷积运算,传入的是当前的矩阵头，当前权重矩阵头，结果矩阵的位置,传入当前的计算的矩阵序号，一共有 ker_num * channel个结果矩阵

//这一步都是代码实现是相同的，为了少debug，应该写成类似模板的形式

bool MaxPooling(DataBlob &old, DataBlob &result)
{

    if (old.rows % 2 != 0 || old.cols % 2 != 0)
    {
        cerr << __FUNCTION__ << ":failed. The old blob's row# or col# is not even" << endl;
        return false;
    }

    //类似一个2 X 2的卷积操作

    result.rows = old.rows / 2;
    result.cols = old.cols / 2;
    result.channels = old.channels;
    float *data = new float[(old.channels * result.rows * result.cols)]{};

#pragma omp for schedule(dynamic)
    for (size_t i = 0; i < old.channels; i++)
    {
        float *ptr1 = old.data + i * old.rows * old.cols;   //当前通道的原始矩阵头
        float *ptr2 = data + i * result.rows * result.cols; //当前通道的结果矩阵头
        myMatrix oldMat(old.rows, old.cols, 0, ptr1);       //防止声明中的释放内存，0是特殊用法
        myMatrix newMat(result.rows, result.cols, 0, ptr2);

        OneByOneMaxPooling(oldMat, newMat);
    }

    result.data = data;

    old.clear();
    return true;
}

void OneByOneMaxPooling(const myMatrix &oldMat, myMatrix &newMat)
{
    float *ptr = oldMat.data;
    int oldCol = oldMat.col;

    for (size_t i = 0; i < newMat.row * newMat.col; i++)
    {
        if (i != 0)
        {
            if ((ptr - oldMat.data + 2) % oldCol == 0) //抵达边界,不再执行 += 2 操作, 列数一定是偶数
            {
                ptr += oldCol + 2;
            }
            else
            {
                ptr += 2;
            }
        }
        float max = MAX(MAX(*ptr, *(ptr + 1)), MAX(*(ptr + oldCol), *(ptr + 1 + oldCol)));

        newMat.data[i] = max;
    }
}

bool convolution_Relu_withMatMul(DataBlob &old, DataBlob &result, int layer)
{
    if (old.isEmpty())
    {
        cerr << __FUNCTION__ << ":failed. The old blob is Empty" << endl;
        return false;
    }
    else
    {
        Filter ftr(layer);
        result.channels = ftr.num_kernel;
        result.rows = Convert(old.rows, ftr.padding, ftr.stride, ftr.kernelSize);
        result.cols = Convert(old.cols, ftr.padding, ftr.stride, ftr.kernelSize);

        float *data = new float[ftr.num_kernel * result.rows * result.cols]{};

        myMatrix Kernel(ftr.num_kernel, ftr.channels * ftr.kernelSize * ftr.kernelSize, 0, ftr.data);
        float *MatrixData = new float[result.rows * result.cols * ftr.channels * ftr.kernelSize * ftr.kernelSize]{};

        cout << result.rows * result.cols * ftr.channels * ftr.kernelSize * ftr.kernelSize << endl;

        myMatrix oldMat(ftr.channels * ftr.kernelSize * ftr.kernelSize, result.rows * result.cols, 0, MatrixData);
        oldMatTransform(oldMat, ftr, old); //加入矩阵乘法再转化一下就行了
        mat_mul_conv(Kernel, oldMat, data);

        result.data = data;

        for (size_t j = 0; j < ftr.num_kernel * ftr.kernelSize * ftr.kernelSize; j++)
        {
            size_t cur_num_kernel = j / ftr.kernelSize * ftr.kernelSize;

            data[j] += ftr.bias[cur_num_kernel]; // >0 为1 小于等于0 为0 不满足条件
            //先加上bias，再对结果做激活

            data[j] = (data[j] > 0) * data[j]; //这里由于类型转化会出现 -0
        }
        ///没有想到怎么用广播函数同时做偏移和激活操作，放弃使用

        delete[] MatrixData;

        old.clear();

        return true;
    }
}

bool mat_mul_conv(const myMatrix &left, const myMatrix &right, float *data)
{
    if (left.col != right.row)
    {
        cerr << __FUNCTION__ << ":failed. Left mat's # cols is not equal to right mat's rows." << endl;
        return false;
    }

#pragma omp for schedule(dynamic)

    for (long long i = 0; i < left.row; i++)
    {
        for (long long k = 0; k < right.row; k++)
        {
            float s = left.data[left.col * i + k];
            for (long long j = 0; j < left.col; j++) // mat1.col == mat2.
            {
                data[right.row * i + j] += s * right.data[right.col * k + j];
            }
        }
    }

    return true;
}

void oldMatTransform(myMatrix &oldMat, Filter &ftr, DataBlob &old)
{
    int result_rows = Convert(old.rows, ftr.padding, ftr.stride, ftr.kernelSize); // 64
    int result_cols = Convert(old.cols, ftr.padding, ftr.stride, ftr.kernelSize); // 64

    int stride = ftr.stride;
    int padding = ftr.padding;

    size_t rows = oldMat.row;
    size_t cols = oldMat.col;
    size_t index = 0;

    int x = -padding; //当前点
    int y = -padding;

    for (size_t i = 0; i < result_rows; i++)
    {
        x += stride;
        for (size_t j = 0; j < result_cols; j++)
        {
            y += stride;
            for (size_t k = 0; k < ftr.channels; k++)
            {
                oldMat.data[index++] = old(x + 0, y + 0, k);
                oldMat.data[index++] = old(x + 0, y + 1, k);
                oldMat.data[index++] = old(x + 0, y + 2, k);
                oldMat.data[index++] = old(x + 1, y + 0, k);
                oldMat.data[index++] = old(x + 1, y + 1, k);
                oldMat.data[index++] = old(x + 1, y + 2, k);
                oldMat.data[index++] = old(x + 2, y + 0, k);
                oldMat.data[index++] = old(x + 2, y + 1, k);
                oldMat.data[index++] = old(x + 2, y + 2, k);
            }
        }
    }
}

bool convolution_Relu_withoutMatMul(DataBlob &old, DataBlob &result, int layer)
{
    if (old.isEmpty())
    {
        cerr << __FUNCTION__ << ":failed. The old blob is Empty" << endl;
        return false;
    }
    else
    {

        ///卷积部分有一点点错误 下次修改就能够实现功能了
        Filter ftr(layer);
        //进行卷积操作

        result.rows = Convert(old.rows, ftr.padding, ftr.stride, ftr.kernelSize); // 64
        result.cols = Convert(old.cols, ftr.padding, ftr.stride, ftr.kernelSize); // 64
        result.channels = ftr.num_kernel;                                         // 16

        //不需要记录channel的间隔，因为row和col就可以计算出channel_STEP
        size_t blobsize = result.rows * result.cols;
        //直接为目标Blob申请内存

        //通道数貌似是多余的

        float *data = new float[ftr.num_kernel * blobsize]{};
#pragma omp for schedule(dynamic)
        for (size_t i = 0; i < ftr.num_kernel; i++) // i划分的是来自不同卷积核的矩阵，里面包含多通道的每一个单色卷积核，也就是一个矩阵
        {
            //当前结果的第一个卷积核的头矩阵
            float *curData = data + i * blobsize;
            myMatrix curMat(old.rows, old.cols, old.channels, old.data);
            float *ptr = ftr.data + i * ftr.channels * 3 * 3; //指向当前BRG卷积核矩阵的矩阵头，应该只有9个的大小，9是卷积核的size
            myMatrix kernelMat(3, 3, ftr.channels, ptr);
            OneByOneConvolution(curMat, kernelMat, ftr, curData, blobsize);
        }
#pragma omp for schedule(dynamic)

        for (size_t j = 0; j < ftr.num_kernel * blobsize; j++)
        {
            size_t cur_num_kernel = j / blobsize;

            data[j] += ftr.bias[cur_num_kernel]; // >0 为1 小于等于0 为0 不满足条件
            //先加上bias，再对结果做激活

            data[j] = (data[j] > 0) * data[j]; //这里由于类型转化会出现 -0
        }
        ///没有想到怎么用广播函数同时做偏移和激活操作

        result.data = data;

        old.clear(); //释放掉old的内存空间
        return true;
    }
}

// ftr中含有当前卷积层操作的参数,row和col是输入blob的数据 , 这里的data和上面的不是同一个
void OneByOneConvolution(const myMatrix &curMat, const myMatrix &curKernel, const Filter &ftr, float *data,
                         size_t blobSize)
{

    int stride = ftr.stride;
    int padding = ftr.padding;
    if (padding == 1 && stride == 2) // hard version
    {
        if (curMat.row % 2 == 0)
        { // r 、 l是偶数
            for (size_t i = 0; i < ftr.channels; i++)
            {
                //当前通道的两个子矩阵头
                float *curMatHead = curMat.data + i * curMat.row * curMat.col; //如何判断当前中心有没有界限？把八个界限的地址位置找出来
                float *curKernelHead = curKernel.data + i * 3 * 3;
                float *curCentre = curMatHead;
                for (size_t j = 0; j < blobSize; j++) //用于填充data，但是只修改了一部分data
                {
                    if (j != 0)
                    {
                        if ((curCentre + 2 - curMatHead) % curMat.col == 0) // stride 2的处理, 向下也是跳2格
                        {
                            curCentre += curMat.col + 2;
                        }
                        else
                        {
                            curCentre += 2;
                        }
                    }
                    int key = typeSwitch_p1s2(curCentre, curMatHead, curMat.row, curMat.col);
                    data[j] += DotProduct_p1s2(key, curCentre, curKernelHead, curMat.row,
                                               curMat.col); //没有把矩阵的ROI用进去很可惜，知道要用！！！！
                }
            }
        }
        else
        { ////r、l是奇数
            for (size_t i = 0; i < ftr.channels; i++)
            {
                //当前通道的两个子矩阵头
                float *curMatHead = curMat.data + i * curMat.row * curMat.col; //如何判断当前中心有没有界限？把八个界限的地址位置找出来
                float *curKernelHead = curKernel.data + i * 3 * 3;
                float *curCentre = curMatHead;

                for (size_t j = 0; j < blobSize; j++) //用于填充data，但是只修改了一部分data
                {
                    if (j != 0)
                    {
                        if ((curCentre + 1 - curMatHead) % curMat.col == 0) // stride 2的处理, 向下也是跳2格
                        {
                            curCentre += curMat.col + 1;
                        }
                        else
                        {
                            curCentre += 2;
                        }
                    }
                    int key = typeSwitch_p1s2(curCentre, curMatHead, curMat.row, curMat.col);
                    data[j] += DotProduct_p1s2(key, curCentre, curKernelHead, curMat.row,
                                               curMat.col); //没有把矩阵的ROI用进去很可惜，知道要用！！！！
                }
            }
        }
    }
    else if (padding == 0 && stride == 1)
    { // ez version{
        for (size_t i = 0; i < ftr.channels; i++)
        {
            //当前通道的两个子矩阵头
            float *curMatHead = curMat.data + i * curMat.row * curMat.col;
            float *curKernelHead = curKernel.data + i * 3 * 3;
            float *curCentre = curMatHead + curMat.col + 1;
            for (size_t j = 0; j < blobSize; j++) //用于填充data，但是只修改了一部分data
            {
                if (j != 0)
                {
                    if ((curCentre + 2 - curMatHead) % curMat.col == 0) // stride 2的处理, 向下也是跳2格
                    {
                        curCentre += 3;
                    }
                    else
                    {
                        curCentre += 1;
                    }
                }
                data[j] += DotProduct_p1s2(9, curCentre, curKernelHead, curMat.row,
                                           curMat.col); //没有把矩阵的ROI用进去很可惜，知道要用！！！！
            }
        }
    }
    else
    {
        cerr << __FUNCTION__ << ":failed. The padding or stride is incorrect." << endl;
    }
}

//只需要实现卷积核为3的情况,找到当前kernel的计算结果

//没有使用Project4的矩阵，因为当时功能实现的时候有问题

inline float DotProduct_p1s2(int key, const float *curCentre, float *curKernelHead, int row, int col)
{

    //等效两个channel为1的矩阵在做运算
    //这里很难debug

    float *curKernelcentre = curKernelHead + 4; //定位到当前curKnernel的中心,3x3的矩阵
    float a = 0;

    switch (key)
    {
    case 1:

        a += *curCentre * *curKernelcentre;
        a += *(curCentre + 1) * *(curKernelcentre + 1);
        a += *(curCentre + col) * *(curKernelcentre + 3);
        a += *(curCentre + col + 1) * *(curKernelcentre + 4);
        break;

    case 2:

        a += *(curCentre - 1) * *(curKernelcentre - 1);
        a += *curCentre * *curKernelcentre;
        a += *(curCentre - 1 + col) * *(curKernelcentre + 2);
        a += *(curCentre + col) * *(curKernelcentre + 3);
        break;

    case 3:
        a += *(curCentre - col) * *(curKernelcentre - 3);
        a += *(curCentre + 1 - col) * *(curKernelcentre - 2);
        a += *curCentre * *curKernelcentre;
        a += *(curCentre + 1) * *(curKernelcentre + 1);
        break;

    case 4:
        a += *curCentre * *curKernelcentre;
        a += *(curCentre - 1) * *(curKernelcentre - 1);
        a += *(curCentre - col) * *(curKernelcentre - 3);
        a += *(curCentre - 1 - col) * *(curKernelcentre - 4);
        break;

    case 5:
        a += *(curCentre - 1) * *(curKernelcentre - 1);
        a += *curCentre * *curKernelcentre;
        a += *(curCentre + 1) * *(curKernelcentre + 1);
        a += *(curCentre - 1 + col) * *(curKernelcentre + 2);
        a += *(curCentre + col) * *(curKernelcentre + 3);
        a += *(curCentre + 1 + col) * *(curKernelcentre + 4);
        break;

    case 6:

        a += *(curCentre - col) * *(curKernelcentre - 3);
        a += *(curCentre + 1 - col) * *(curKernelcentre - 2);
        a += *curCentre * *curKernelcentre;
        a += *(curCentre + 1) * *(curKernelcentre + 1);
        a += *(curCentre + col) * *(curKernelcentre + 3);
        a += *(curCentre + 1 + col) * *(curKernelcentre + 4);
        break;

    case 7:

        a += *(curCentre - 1 - col) * *(curKernelcentre - 4);
        a += *(curCentre - col) * *(curKernelcentre - 3);
        a += *(curCentre - 1) * *(curKernelcentre - 1);
        a += *curCentre * *curKernelcentre;
        a += *(curCentre - 1 + col) * *(curKernelcentre + 2);
        a += *(curCentre + col) * *(curKernelcentre + 3);
        break;

    case 8:

        a += *(curCentre - 1 - col) * *(curKernelcentre - 4);
        a += *(curCentre - col) * *(curKernelcentre - 3);
        a += *(curCentre + 1 - col) * *(curKernelcentre - 2);
        a += *(curCentre - 1) * *(curKernelcentre - 1);
        a += *curCentre * *curKernelcentre;
        a += *(curCentre + 1) * *(curKernelcentre + 1);
        break;

    default: // 9,就是p0S1的情况

        a += *(curCentre - 1 - col) * *(curKernelcentre - 4);
        a += *(curCentre - col) * *(curKernelcentre - 3);
        a += *(curCentre + 1 - col) * *(curKernelcentre - 2);
        a += *(curCentre - 1) * *(curKernelcentre - 1);
        a += *curCentre * *curKernelcentre;
        a += *(curCentre + 1) * *(curKernelcentre + 1);
        a += *(curCentre - 1 + col) * *(curKernelcentre + 2);
        a += *(curCentre + col) * *(curKernelcentre + 3);
        a += *(curCentre + 1 + col) * *(curKernelcentre + 4);
        break;
    }
    return a;
}

inline int typeSwitch_p1s2(const float *curPtr, float *start, int row, int col)
{
    float *node1 = start;
    float *node2 = start + col - 1;
    float *node3 = node1 + (row - 1) * col;
    float *node4 = node2 + (row - 1) * col;

    if (curPtr == node1)
    {
        return 1;
    }
    else if (curPtr == node2)
    {
        return 2;
    }
    else if (curPtr == node3)
    {
        return 3;
    }
    else if (curPtr == node4)
    {
        return 4;
    }
    else if (curPtr > node1 && curPtr < node2)
    {
        return 5;
    }
    else if ((curPtr - node1) % col == 0)
    {
        return 6;
    }
    else if ((curPtr - node2) % col == 0)
    {
        return 7;
    }
    else if (curPtr > node3 && curPtr < node4)
    {
        return 8;
    }
    else
    {
        return 9;
    }
}

float *mat_mul(myMatrix &mat1, myMatrix &mat2)
{
    //这里直接用矩阵乘法好像不太适合,因为 mat2.row * i + j的值很大很大，直接数组越界了
    if (mat1.col == mat2.row)
    {
        float *result = new float[mat1.row * mat2.col]{};
#pragma omp for schedule(dynamic)
        for (size_t i = 0; i < mat1.row * mat2.col; i++)
        {
            int temp = mat1.col * i;
            for (size_t j = 0; j < mat1.col; j++)
            {
                result[i] += mat1.data[temp + j] * mat2.data[j];
            }
        }
        return result;
    }
    else
    {
        cerr << __FUNCTION__ << ":Can not mul because Mat1's row is not equal to Mat2's column." << endl;
        return nullptr;
    }
}

///等效矩阵乘法
float *flatten_FC(DataBlob &old)
{ //连接成一个 2 * 1的矩阵

    if (old.rows * old.cols * old.channels != fc_params.in_features)
    {
        cerr << __FUNCTION__ << ":failed. The old blob is not the required blob." << endl;
    }

    float *bias = fc_params.p_bias;

    myMatrix input(2048, 1, 0, old.data); //特殊的用法，保证析构函数不会释放内存

    myMatrix weight(2, 2048, 0, fc_params.p_weight);

    float *result = mat_mul(weight, input); ///这里的行列很重要！

#pragma omp for schedule(dynamic)
    for (size_t i = 0; i < 2; i++)
    {
        result[i] += bias[i];
    }

    old.clear();
    return result;
}

void softMax(float *vector)
{
    float sum = 0;
#pragma omp for schedule(dynamic)
    for (size_t i = 0; i < 2; i++)
    {
        sum += exp(vector[i]);
    }
#pragma omp for schedule(dynamic)
    for (size_t i = 0; i < 2; i++)
    {
        vector[i] = exp(vector[i]) / sum;
    }
}
