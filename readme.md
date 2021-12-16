## 运行说明
本实验在 MacOS 环境下实现，需要额外三个依赖库，分别为 OpenGL Flex Bison，详情可以在 CMakeLists 里看到

其中 OpenGL 使用的 glfw3 和 glad 放在了 libraries 中

使用 cmake-build-debug 下可执行文件 eDraw 运行，带一个参数，为代码的路径

例如：

$ ./eDraw ../code/tetris.txt

下面是文件组织的简单描述

### code

该文件夹下包含了一些该语言下的实例代码，下面是一些重要的例子：

tetris.txt 俄罗斯方块

window.txt 飞机射击

window_zh.txt 飞机射击的汉化版本

### libraries

一些依赖库

### cmake-build-debug

本实验使用 CLion 编写，一些中间文件，和最后生成的可执行文件都在这里

### ary.l 和 ary_zh.l

词法描述文件（flex），其中 zh 版本是支持汉化的版本

### ary.yy

语法描述文件（bison）

### 其他

CMakeLists.txt 该项目使用 CMake 来管理和生成

剩下的 C++ 代码，为程序的实现，主要部分在 m_type.h 和 m_type.cpp 中，包含了数据和AST的定义和实现
