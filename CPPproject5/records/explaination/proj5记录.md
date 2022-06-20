bonus：You are welcome to implement more CNN layers, and to make the implemented CNN to be more general (such as the convolutional layer can be for any size of kernels, not just 3x3).

欢迎实现更多的 CNN 层，并使实现的 CNN 更通用（例如卷积层可以用于任何大小的内核，而不仅仅是 3x3）。



kernel（权重矩阵）是一个**矩阵**，可在图像上滑动并与输入相乘，从而以某种我们期望的方式增强输出。

通过深层 CNN，我们无需再用手工设计的kernel来提取特征，而是可以直接学习这些可提取潜在特征的kernel值。

简单区分一下kernel和filter，kernel就是权重矩阵即数据量为h * w，filter就是多层的权重矩阵即数据量为

c * h * w。



关于本次proj的分析：

1. 先使用C++实现具体的功能，再考虑对代码的可读性，鲁棒性和运算效率做提升（可以参考YSQ老师的faceDetect的报错做法）

2. 我们要做什么，即CNN的哪些部分需要我们实现？

   我们要完成的先是3通道下的卷积操作实现，完成了本操作后4个层的处理非常相似。

3. 图像和过滤器对应模块的卷积运算和矩阵乘法非常相似，考虑使用之前的代码

4. 我们已经得到了训练过的**卷积参数**，所以训练部分的代码是不需要的，完成卷积运算部分的设计后直接导入参数即可。

   qustion：我们应该把参数导入到什么地方？导入到一个层中的某个指针做运算。要设计的过滤层应该有几层？

5.  

   1. 需要使用OpenCV来读取传入的图像，OpenCV中的cv::mat会将图像信息转化一个unsigned char类型的矩阵.

   2. 需要将这个矩阵中的所有数据转化为一个大小在[0.0, 1.0]之间的float类型数（转换过程估计是通过某个的散列函数来完成，最简单的想法是直接对unsigned char类型数除以 **通过宏定义的unsigned char类型最大值**，这样可以保证结果一定在[0.0, 1.0]）。

      只要求float类型的话，可以使用project3中c语言完成的矩阵乘法逻辑，速度快而且不容易错

   3. cv::mat 读取图像后返回的矩阵的通道顺序是 ： **BRG 而不是 RGB **，而且返回的内存是一段连续的内存，应该编写一个转换函数把mat中的数据取出来， 到时候实现卷积操作时要注意处理层的顺序。

   4. CNN模型的输入要求是 channel * height * width （通道数 x 矩阵行数 x 矩阵列数）

6.  CNN模型的输出将是一个**由两个浮点数组成的向量[c0, c1]，c0 + c1 = 1.0 **， 其中 c1是输入照片的是人的百分率（照片只有人的上半身）。

   输出结果肯定是要通过提供的参数计算出来的，但是怎么用还没有搞懂

7. 还是和上次一样需要在分别在 x86 和 arm 平台下对软件做测试。





注：最后要使用Softmax来计算结果的置信度，也就是C1的值



分析老师提供的训练参数集：



kernel_size ： 权重矩阵的size的长度

pad：填充层数

stride： 平移格数

in_channel： 输入通道数

out_channel ： 输出通道数



FaceDetection 源码启示

头文件用来定义将要使用到的数据类型



//技巧：函数返回值是bool类型 const记录来自指针的输入 常量记录来自指针的输出 便于内存管理

用于二元大小判断的宏：

```
#ifndef MIN
#  define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#  define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif
```




阅读了一下于老师人脸识别的源码：用于层与层传输数据，保存运算结果CDataBlob的成员变量

```
public:
    T * data;
   int rows;
   int cols;
   int channels; //in element
    int channelStep; //in byte
```

facedetectcnn.cpp的作用：定义CDataBlob类和其存储数据的格式。

输入的图片必须是：
