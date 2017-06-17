#include "MathFunctions.h"
#include "TutorialConfig.h"
#include <stdio.h>

// 包含生成的文件
#include "Table.h"

#include <math.h>

// 求平方根
double mysqrt(double x) {
    if (x <= 0) {
        return 0;
    }

    double result;
#if defined(HAVE_LOG) && defined(HAVE_EXP)
    result = exp(log(x) * 0.5);
    fprintf(stdout, "Computing sqrt of %g to be %g using log\n", x, result);
#else
    result = x;
    // use the table to help find an initial value
    if (x >= 1 && x < 10) {
        result = sqrtTable[static_cast<int>(x)];
    }

    // 10次迭代
    for (int i = 0; i < 10; ++i) {
        result = 0.5 * (result + x / result);
        fprintf(stdout, "Computing sqrt of %g to be %g\n", x, result);
    }
#endif

    return result;
}


