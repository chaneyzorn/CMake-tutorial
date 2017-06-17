# CMake-tutorial([原文](https://cmake.org/cmake-tutorial/))

这份渐进式的教程涵盖了 CMake 帮助处理的一些常见的构建问题。许多议题已经在[《Mastering CMake》](http://www.kitware.com/products/books/CMakeBook.html)中作为独立的话题介绍过，但是了解它们是如何在示例项目中结合在一起的将非常有帮助。你可以在 CMake 源码中的 [Tests/Tutorial](https://gitlab.kitware.com/cmake/cmake/tree/master/Tests/Tutorial) 文件夹找到这份教程，每一步的内容都放置在各自的子文件夹中。

## 一个基本的出发点 (Step1)

最简单的项目是从源代码文件中构建一个可执行文件，CMakeLists.txt 文件仅需要两行，这将作为我们教程的起点，内容如下：

```cmake
cmake_minimum_required (VERSION 2.6)
project (Tutorial)
add_executable(Tutorial tutorial.cxx)
```

文件中的命令支持大写、小写或者混合使用，这个例子中的命令使用小写。tutorial.cxx 用于计算一个数的平方根，源码的第一版非常简单：

```c++
// 计算一个数的平方根
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
int main (int argc, char *argv[])
{
  if (argc < 2)
    {
    fprintf(stdout,"Usage: %s number\n",argv[0]);
    return 1;
    }
  double inputValue = atof(argv[1]);
  double outputValue = sqrt(inputValue);
  fprintf(stdout,"The square root of %g is %g\n",
          inputValue, outputValue);
  return 0;
}
```

### 添加一个版本号并配置头文件

你可以直接在源代码中添加版本号，但在 CMakeLists.txt 文件中提供版本号将会更加灵活，我们将文件修改如下：

```cmake
cmake_minimum_required (VERSION 2.6)
project (Tutorial)
# 版本号 1.0
set (Tutorial_VERSION_MAJOR 1)
set (Tutorial_VERSION_MINOR 0)

# 配置一个头文件将一些 CMake 设置传入到源代码中
# 以 TutorialConfig.h.in 为模版，替换相关变量
# 以生成 TutorialConfig.h
configure_file (
  "${PROJECT_SOURCE_DIR}/TutorialConfig.h.in"
  "${PROJECT_BINARY_DIR}/TutorialConfig.h"
  )

# 将构建目录添加到 include 的搜索路径中以便找到
# TutorialConfig.h 文件
include_directories("${PROJECT_BINARY_DIR}")

# 添加可执行文件
add_executable(Tutorial tutorial.cxx)
```

因为配置文件将会写入到构建目录中，所以我们将这个目录添加到包含文件的搜索路径中。在源代码中添加 TutorialConfig.h.in 文件：

```c++
// the configured options and settings for Tutorial
#define Tutorial_VERSION_MAJOR @Tutorial_VERSION_MAJOR@
#define Tutorial_VERSION_MINOR @Tutorial_VERSION_MINOR@
```

当 CMake 生成这个头文件时，@Tutorial_VERSION_MAJOR@ 和 @Tutorial_VERSION_MINOR@ 的值将会由 CMakeLists.txt 中对应的值替换。接下来我们将头文件包含到 tutorial.cxx 中并且使用这个版本号，代码如下：

```c++
// A simple program that computes the square root of a number
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "TutorialConfig.h"

int main (int argc, char *argv[])
{
  if (argc < 2)
    {
    fprintf(stdout,"%s Version %d.%d\n",
            argv[0],
            Tutorial_VERSION_MAJOR,
            Tutorial_VERSION_MINOR);
    fprintf(stdout,"Usage: %s number\n",argv[0]);
    return 1;
    }
  double inputValue = atof(argv[1]);
  double outputValue = sqrt(inputValue);
  fprintf(stdout,"The square root of %g is %g\n",
          inputValue, outputValue);
  return 0;
}
```

主要的改变是包含了头文件并且在使用方法信息中打印了版本号。

## 添加一个库 (Step 2)

现在我们要在项目中添加一个库，这个库将会包含我们自己的计算平方根的实现。可执行文件将可以使用这个库，而不是使用编译器提供的平方根标准方法。本教程中将这个库放到名为 MathFunctions 的子文件夹中，这个子文件夹需要包含一个 CMakeLists.txt 文件，文件中有如下一行：

```cmake
add_library(MathFunctions mysqrt.cxx)
```

mysqrt.cxx 文件中有一个叫做 mysqrt 的函数，它提供与编译器的 sqrt 函数相同的功能。我们在顶层的 CMakeLists.txt 中添加一个 add_subdirectory 调用以构建这个库。为了找到 MathFunctions/MathFunctions.h 头文件中的函数原型，我们添加另一条包含路径。最后一个改动是将这个库添加到可执行文件中。顶层 CMakeLists.txt 文件中添加的最新几行如下：

```cmake
include_directories ("${PROJECT_SOURCE_DIR}/MathFunctions")
add_subdirectory (MathFunctions)

# add the executable
add_executable (Tutorial tutorial.cxx)
target_link_libraries (Tutorial MathFunctions)
```

考虑一下将这个库设计为可选的，本教程中这样做也许是不必要的，但是当使用更大的库或者第三方的库时你也许会用到。第一步是在顶层的 CMakeLists.txt 中添加一个选择：

```cmake
# 是否使用我们自己的函数？
option (USE_MYMATH
        "Use tutorial provided math implementation" ON)
```

CMake GUI 中将会显示一个 ON 的默认值，用户可以按需更改。这个设置将会被缓存，这样在每次对这个项目运行 CMake 时用户不需要再次设置。接下来的更改是将 MathFunctions 库的构建和连接设置为可选的，我们在顶层 CMakeLists.txt 的最后修改如下：

```cmake
# add the MathFunctions library?
if (USE_MYMATH)
  include_directories ("${PROJECT_SOURCE_DIR}/MathFunctions")
  add_subdirectory (MathFunctions)
  set (EXTRA_LIBS ${EXTRA_LIBS} MathFunctions)
endif (USE_MYMATH)

# add the executable
add_executable (Tutorial tutorial.cxx)
target_link_libraries (Tutorial  ${EXTRA_LIBS})
```

这将根据 USE_MYMATH 的设置来决定是否编译并使用 MathFunctions 库。注意这里使用了一个 EXTRA_LIBS 变量来收集任何可选的库，以在之后链接到可执行文件中。对有许多可选组件的项目，这是一种保持其整洁的常用方法。相应的源代码更改如下：

```c++
/ A simple program that computes the square root of a number
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "TutorialConfig.h"
#ifdef USE_MYMATH
#include "MathFunctions.h"
#endif

int main (int argc, char *argv[])
{
  if (argc < 2)
    {
    fprintf(stdout,"%s Version %d.%d\n", argv[0],
            Tutorial_VERSION_MAJOR,
            Tutorial_VERSION_MINOR);
    fprintf(stdout,"Usage: %s number\n",argv[0]);
    return 1;
    }

  double inputValue = atof(argv[1]);

#ifdef USE_MYMATH
  double outputValue = mysqrt(inputValue);
#else
  double outputValue = sqrt(inputValue);
#endif

  fprintf(stdout,"The square root of %g is %g\n",
          inputValue, outputValue);
  return 0;
}
```

在源代码中我们同样使用了 USE_MYMATH 变量。通过在 TutorialConfig.h.in 中添加如下配置，Cmake 将这个变量引入到源代码中：

```c++
#cmakedefine USE_MYMATH
```

## 安装与测试 (Step 3)

接下来我们在项目中添加安装规则和测试支持。安装规则非常直接，对于 MathFunctions 库的安装，我们在  MathFunctions 的 CMakeLists.txt 中添加如下几行：

```cmake
install (TARGETS MathFunctions DESTINATION bin)
install (FILES MathFunctions.h DESTINATION include)
```

对于应用程序可执行文件和头文件的安装，我们在顶层的 CMakeLists.txt 中添加如下几行：

```cmake
# add the install targets
install (TARGETS Tutorial DESTINATION bin)
install (FILES "${PROJECT_BINARY_DIR}/TutorialConfig.h"
         DESTINATION include)
```

万事俱备，接下来你应该可以构建这个项目，然后键入 `make install` （或者在 IDE 中构建 INSTALL 目标），它将会安装合适的头文件，库和执行文件。CMake 的 CMAKE_INSTALL_PREFIX 变量用于决定文件安装位置的根。添加测试也是一个同样直接的过程。在顶层 CMakeLists.txt 的结尾，我们可以添加几个基础测试以判别程序是否工作正常：

```camke
include(CTest)

# does the application run
add_test (TutorialRuns Tutorial 25)

# does it sqrt of 25
add_test (TutorialComp25 Tutorial 25)
set_tests_properties (TutorialComp25 PROPERTIES PASS_REGULAR_EXPRESSION "25 is 5")

# does it handle negative numbers
add_test (TutorialNegative Tutorial -25)
set_tests_properties (TutorialNegative PROPERTIES PASS_REGULAR_EXPRESSION "-25 is 0")

# does it handle small numbers
add_test (TutorialSmall Tutorial 0.0001)
set_tests_properties (TutorialSmall PROPERTIES PASS_REGULAR_EXPRESSION "0.0001 is 0.01")

# does the usage message work?
add_test (TutorialUsage Tutorial)
set_tests_properties (TutorialUsage PROPERTIES PASS_REGULAR_EXPRESSION "Usage:.*number")
```

构建完成后，可以使用命令行工具 `ctest` 运行测试。第一个测试只是验证程序是否运行，没有段错误或其他的崩溃，并返回零值。这是一个 CTest 测试的基本形式。接下来的几个测试都使用 PASS_REGULAR_EXPRESSION 测试属性来验证测试的输出是否包含某些字符串。这样用来验证预期的计算结果，并且当参数数目不正确时打印使用信息。如果你想添加大量测试来测试不同的输入值，你可能会考虑创建如下所示的宏：

```cmake
#define a macro to simplify adding tests, then use it
macro (do_test arg result)
  add_test (TutorialComp${arg} Tutorial ${arg})
  set_tests_properties (TutorialComp${arg}
    PROPERTIES PASS_REGULAR_EXPRESSION ${result})
endmacro (do_test)

# do a bunch of result based tests
do_test (25 "25 is 5")
do_test (-25 "-25 is 0")
```

每调用一次 do_test，根据传递的参数，都会添加一个拥有名字、输入和输出的测试。

## 添加系统自检 (Step 4)

接下来让我们向项目中添加一些代码，这些代码依赖的功能目标平台可能没有提供。这个例子中，我们添加的代码依赖于目标平台是否提供了对数 log 和指数 exp 函数。当然几乎所有的平台都提供了这样的函数，本教程假定它们是不常见的功能。如果平台提供了 log，那么我们可以在 mysqrt 函数中使用它计算平方根。我们首先使用顶层 CMakeLists.txt 文件中的 CheckFunctionExists.cmake 宏来测试这些功能的可用性，如下所示：

```cmake
# does this system provide the log and exp functions?
include (CheckFunctionExists)
check_function_exists (log HAVE_LOG)
check_function_exists (exp HAVE_EXP)
```

当 CMake 在平台上发现它们时，我们在 TutorialConfig.h.in 中定义这些值：

```cmake
// does the platform provide exp and log functions?
#cmakedefine HAVE_LOG
#cmakedefine HAVE_EXP
```

log 和 exp 的测试需要放在 configure_file 命令之前，configure_file 命令会立即使用 CMake 中的当前设置生成文件。当系统提供了这两个函数时，我们可以使用以下代码在 mysqrt 函数中提供一个基于 log 和 exp 的替代实现：

```c++
// if we have both log and exp then use them
#if defined (HAVE_LOG) && defined (HAVE_EXP)
  result = exp(log(x)*0.5);
#else // otherwise use an iterative approach
  . . .
```

## 添加生成文件和生成器 (Step 5)

在本节中，我们将展示如何将生成的源文件添加到应用程序的构建过程中。本例中，我们将会在构建过程中创建一个预先计算的平方根表，然后将这张表编译进我们的程序中。我们首先需要一个生成这张表的程序，为此我们在 MathFunctions 的子文件夹中添加一个新的名为 MakeTable.cxx 的源文件：

```c++
// A simple program that builds a sqrt table
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main (int argc, char *argv[])
{
  int i;
  double result;

  // make sure we have enough arguments
  if (argc < 2)
    {
    return 1;
    }

  // open the output file
  FILE *fout = fopen(argv[1],"w");
  if (!fout)
    {
    return 1;
    }

  // create a source file with a table of square roots
  fprintf(fout,"double sqrtTable[] = {\n");
  for (i = 0; i < 10; ++i)
    {
    result = sqrt(static_cast<double>(i));
    fprintf(fout,"%g,\n",result);
    }

  // close the table with a zero
  fprintf(fout,"0};\n");
  fclose(fout);
  return 0;
}
```

注意这张表会以有效的 C++ 代码的形式生成，输出文件的名字以参数的形式提供。接下来向 MathFunctions 的 CMakeLists.txt 文件中添加合适的命令以构建 MakeTable 的可执行文件，并运行它作为构建过程的一部分。需要几个命令来完成此操作，如下所示：

```cmake
# first we add the executable that generates the table
add_executable(MakeTable MakeTable.cxx)

# add the command to generate the source code
add_custom_command (
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Table.h
  COMMAND MakeTable ${CMAKE_CURRENT_BINARY_DIR}/Table.h
  DEPENDS MakeTable
  )

# add the binary tree directory to the search path for
# include files
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

# add the main library
add_library(MathFunctions mysqrt.cxx ${CMAKE_CURRENT_BINARY_DIR}/Table.h  )
```

首先像添加其他执行文件那样添加 MakeTable 的执行文件。然后我们添加一个自定义命令以使用 MakeTable 生成 Table.h 文件。接下来我们需要将生成的文件添加到 MathFunctions 库的源文件列表中，以让 CMake 知道 mysqrt.cxx 依赖于 Table.h 文件。我们也需要将当前的构建文件夹添加到包含文件列表中，以让 Table.h 文件可以被发现并包含到 mysqrt.cxx 中。当构建这个项目时，它会先构建 MakeTable 的执行文件，然后运行 MakeTable 生成 Table.h 文件，最后它会编译包含有 Table.h 的 mysqrt.cxx 以生成 MathFunctions 库。此时，顶层 CMakeLists.txt 文件如下所示：

```cmake
cmake_minimum_required (VERSION 2.6)
project (Tutorial)
include(CTest)

# The version number.
set (Tutorial_VERSION_MAJOR 1)
set (Tutorial_VERSION_MINOR 0)

# does this system provide the log and exp functions?
include (${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)

check_function_exists (log HAVE_LOG)
check_function_exists (exp HAVE_EXP)

# should we use our own math functions
option(USE_MYMATH
  "Use tutorial provided math implementation" ON)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/TutorialConfig.h.in"
  "${PROJECT_BINARY_DIR}/TutorialConfig.h"
  )

# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
include_directories ("${PROJECT_BINARY_DIR}")

# add the MathFunctions library?
if (USE_MYMATH)
  include_directories ("${PROJECT_SOURCE_DIR}/MathFunctions")
  add_subdirectory (MathFunctions)
  set (EXTRA_LIBS ${EXTRA_LIBS} MathFunctions)
endif (USE_MYMATH)

# add the executable
add_executable (Tutorial tutorial.cxx)
target_link_libraries (Tutorial  ${EXTRA_LIBS})

# add the install targets
install (TARGETS Tutorial DESTINATION bin)
install (FILES "${PROJECT_BINARY_DIR}/TutorialConfig.h"
         DESTINATION include)

# does the application run
add_test (TutorialRuns Tutorial 25)

# does the usage message work?
add_test (TutorialUsage Tutorial)
set_tests_properties (TutorialUsage
  PROPERTIES
  PASS_REGULAR_EXPRESSION "Usage:.*number"
  )


#define a macro to simplify adding tests
macro (do_test arg result)
  add_test (TutorialComp${arg} Tutorial ${arg})
  set_tests_properties (TutorialComp${arg}
    PROPERTIES PASS_REGULAR_EXPRESSION ${result}
    )
endmacro (do_test)

# do a bunch of result based tests
do_test (4 "4 is 2")
do_test (9 "9 is 3")
do_test (5 "5 is 2.236")
do_test (7 "7 is 2.645")
do_test (25 "25 is 5")
do_test (-25 "-25 is 0")
do_test (0.0001 "0.0001 is 0.01")
```

TutorialConfig.h.in 如下：

```c++
// the configured options and settings for Tutorial
#define Tutorial_VERSION_MAJOR @Tutorial_VERSION_MAJOR@
#define Tutorial_VERSION_MINOR @Tutorial_VERSION_MINOR@
#cmakedefine USE_MYMATH

// does the platform provide exp and log functions?
#cmakedefine HAVE_LOG
#cmakedefine HAVE_EXP
```

MathFunctions 的 CMakeLists.txt 文件如下：

```cmake
# first we add the executable that generates the table
add_executable(MakeTable MakeTable.cxx)
# add the command to generate the source code
add_custom_command (
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Table.h
  DEPENDS MakeTable
  COMMAND MakeTable ${CMAKE_CURRENT_BINARY_DIR}/Table.h
  )
# add the binary tree directory to the search path
# for include files
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

# add the main library
add_library(MathFunctions mysqrt.cxx ${CMAKE_CURRENT_BINARY_DIR}/Table.h)

install (TARGETS MathFunctions DESTINATION bin)
install (FILES MathFunctions.h DESTINATION include)
```

## 构建安装程序 (Step 6)

接下来假设我们想将我们的项目分发给其他人，以便他们可以使用它。我们希望在各种平台上提供二进制和源代码分发。这与之前的第三步有些不同，在这个例子中，我们将构建安装包以支持二进制安装和包管理功能，比如 cygwin，debian，RPMs 等。我们将会使用 CPack 来创建平台相关的安装程序。具体来说，我们需要在我们的顶层 CMakeLists.txt 文件的底部添加几行：

```cmake
# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE
     "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set (CPACK_PACKAGE_VERSION_MAJOR "${Tutorial_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${Tutorial_VERSION_MINOR}")
include (CPack)
```

我们从包含 InstallRequiredSystemLibraries 开始。该模块包含有这个项目在当前平台所需的任何运行时库。然后我们设置一些 CPack 变量指明此项目许可证和版本信息的位置。版本信息使用了本教程前面设置的变量。最后我们包含了 CPack 模块，它将使用你设置的这些变量和其他系统属性来配置安装程序。

接下来就是按照通常的方法构建项目，然后运行 CPack 命令。要构建一个二进制分发，你可以运行：

```bash
cpack --config CPackConfig.cmake
```

要创建一个源码分发，你可以键入：

```bash
cpack --config CPackSourceConfig.cmake
```

## 添加对仪表板的支持 (Step 7)

添加将测试结果提交给仪表板的支持非常简单。在教程之前的步骤中已经定义了一些测试，我们只需运行这些测试并且将它们提交给一个仪表板。要包括对仪表板的支持，我们将 CTest 模块包含在我们的顶层 CMakeLists.txt 文件中：

```cmake
# enable dashboard scripting
include (CTest)
```

我们还创建一个CTestConfig.cmake文件，可以在该文件中为仪表板指定此项目的名称。

```cmake
set (CTEST_PROJECT_NAME "Tutorial")
```

当运行 CTest 时它会读取这个文件。要创建简单的仪表板，你可以在项目中运行 CMake，然后切换目录到构建目录中运行 `ctest –D Experimental`. 仪表板的结构将会上传到 Kitware 的公共仪表板中（[这里](http://www.cdash.org/CDash/index.php?project=PublicDashboard)）。