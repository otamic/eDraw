#include <iostream>

#include "m_type.h"

NumList evalNumList(int n, ...) {
    NumList res;
    int size;
    va_list vl;
    va_start(vl, n);
    for (int i = 0; i < n; i++) {
        size = va_arg(vl, int);
        res.push_back(size);
    }
    return res;
}

int main() {
    auto graph = std::make_shared<Array>(evalNumList(3, 3, 2, 2));
    graph->print();

    auto pixel = std::make_unique<Array>(evalNumList(1, 3));
    *pixel->at(evalNumList(1, 0)) =
    *pixel->at(evalNumList(1, 1)) =
    *pixel->at(evalNumList(1, 2)) = 255;
    pixel->print();

    // *graph->at(evalNumList(2, 1, 0)) = *pixel;
    // graph->print();

    auto pixel2 = *pixel + *pixel;
    pixel2.print();

    pixel2 = *pixel - *pixel;
    pixel2.print();

    pixel2 = *pixel * *pixel;
    pixel2.print();

    pixel2 = *pixel / *pixel;
    pixel2.print();

    int num = *pixel / 2;
    std::cout << num << std::endl;

    Ast ast('+', static_cast<AstPtr>(new Num(10)), static_cast<AstPtr>(new NumArray(*pixel)));
    auto result = ast.eval();
    std::cout << result.n_ << std::endl;

    SymAsgn asgn(static_cast<AstPtr>(new ArrayRef(std::make_shared<ArraySymbol>(graph), evalNumList(2, 1, 0))), static_cast<AstPtr>(new NumArray(*pixel)));
    auto res = asgn.eval();
    res.a_.print();

    graph->print();

    return 0;
}
