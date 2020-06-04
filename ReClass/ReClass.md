
## 解析结构

c++ 测试代码
```cpp
#include <iostream>
using namespace std;

struct MyStruct
{
  int a = 1;
  const char* cs = "str";
  const wchar_t* wcs = L"str2";
} s;

int main()
{
  printf("pointer: %lp\n", &s);
  cin.get();
  return 0;
}
```

reclass 附加进程 `file -> attrch`

![](./images/../../images/2021-07-17-23-58-04.png)

创建一个class

![](./images/../../images/2021-07-17-23-59-21.png)
![](./images/../../images/2021-07-18-00-00-47.png)

双击地址设置为c++程序打印的地址，双击类名改为struct

![](./images/../../images/2021-07-18-00-02-31.png)

解析类，我们知道a是int

右键第一行选择`change type -> int32`

![](./images/../../images/2021-07-18-00-05-32.png)

设置类型后

![](./images/../../images/2021-07-18-00-07-12.png)

下面有4字节对齐的pading，将类型设置为text，大小4字节

![](./images/../../images/2021-07-18-00-09-34.png)


剩下两个我们知道是`char*`和`wchar_t*`，定义类型后

![](./images/../../images/2021-07-18-00-11-40.png)

多余的字节可以使用`delete`删除掉，字节不够可以右键`add bytes`


如果你想在c++中使用解析的class `右键 类名 -> show c++ code`

![](./images/../../images/2021-07-18-00-21-36.png)

复制到你的c++程序使用

![](./images/../../images/2021-07-18-00-22-17.png)

## data**

c++
```cpp
#include <iostream>
using namespace std;

class MyStruct
{
  int a = 1;
  const char* cs = "str";
  const wchar_t* wcs = L"str2";
};

int main()
{
  auto* pX = new MyStruct();
  auto** ppX = &pX;
  printf("pointer: %lp\n", ppX);
  cin.get();
  return 0;
}
```

![](./images/../../images/2021-07-18-08-37-02.png)

使用CE解析结构

![](./images/../../images/2021-07-18-08-47-09.png)

## 数组 1 `B* arr[3]`

c++
```cpp
#include <iostream>
using namespace std;

class B { public: size_t i; };
class A { public: B* arr[3]; int a; };

int main()
{
  auto* pA = new A;
  pA->a = 100;

  for (size_t i = 0; i < 3; i++)
  {
    pA->arr[i] = new B{ i };
  }

  printf("pointer: %lp\n", pA);
  cin.get();
  return 0;
}
```

![](./images/../../images/2021-07-18-09-05-31.png)

## 数组 2 `B** arr`

c++
```cpp
#include <iostream>
using namespace std;

class B { public: size_t i; };
class A { public: B** arr; int a; };

int main()
{
  auto* pA = new A;
  pA->a = 100;

  B* arr[3];
  for (size_t i = 0; i < 3; i++)
    arr[i] = new B{ i };
  pA->arr = arr;

  printf("pointer: %lp\n", pA);
  cin.get();
  return 0;
}
```

![](./images/../../images/2021-07-18-09-16-03.png)

## 数组 3 `B** arr[3]`
```cpp
#include <iostream>
using namespace std;

class B { public: size_t i; };
class A { public: B** arr[3]; int a; };

int main()
{
  auto* pA = new A;
  pA->a = 100;

  B* pB;
  B** ppB;
  for (size_t i = 0; i < 3; i++)
  {
    pB =  new B{ i };
    ppB = (B**)malloc(sizeof(uintptr_t));
    memcpy_s(ppB, 8, &pB, 8);
    pA->arr[i] = ppB;
  }

  printf("pointer: %lp\n", pA);
  cin.get();
  return 0;
}
``` 

![](./images/../../images/2021-07-18-09-34-10.png)