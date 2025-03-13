#include <QTest>
#include <QCoreApplication>

// 删除所有的头文件引用，因为我们不再需要它们
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    return 0; // 返回0因为每个测试类现在都有自己的main函数
} 