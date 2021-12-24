//
// Created by Garrison on 2021/11/10.
//

#include "m_type.h"

#include <iostream>
#include <stack>
#include <memory>
#include <cstdarg>
#include <thread>
#include <chrono>

#include <GLFW/glfw3.h>

NumList ConvertList(const std::vector<AstPtr>& from) {
    NumList res;
    for (const auto& ast: from) {
        res.push_back(int(ast->eval()));
    }
    return res;
}

const Element EMPTY = { Element::NUM, 0 };

bool CheckVec(const NumList & from, const NumList & index) {
    if (index.size() > from.size()) return false;

    for (int i = index.size() - 1, j = from.size() - 1; i >= 0; i--, j--)
        if (index[i] >= from[j])
            return false;

    return true;
}

Array::Array(const NumList & size): size_(size), bias_(0) {
    if (size.empty()) return;

    int length = 1;
    for (int num: size) length *= num;
    data_ = std::shared_ptr<int[]>(new int[length]);
    for (int i = 0; i < length; i++) data_[i] = 0;
}

ArrayPtr Array::at(const NumList & index) {
    if (CheckVec(size_, index)) {
        auto res = std::make_shared<Array>();
        res->data_ = data_;
        res->bias_ = bias_;

        int len = 1, i = 0, j = 0;
        while (i < (size_.size() - index.size())) {
            len *= size_[i];
            i++;
        }
        while (i < size_.size()) {
            res->bias_ += len * index[j];
            len *= size_[i];
            i++; j++;
        }

        res->size_ = NumList(size_.begin(), size_.begin() + int(size_.size() - index.size()));
        if (res->size_.empty()) res->size_.push_back(1);
        return res;
    }
    std::cout << "invalid index" << std::endl;
    exit(EXIT_FAILURE);
}

Array& Array::operator=(const Array & a) {
    if (this == &a) return *this;

    if (size_ != a.size_) {
        std::cout << "error: can't assign a different size" << std::endl;
        return *this;
    }

    int len = 1;
    for (int num: size_) len *= num;
    memcpy(data_.get() + bias_, a.data_.get() + a.bias_, len * sizeof(int));
    return *this;
}

Array &Array::operator()(const Array &a) {
    data_ = a.data_;
    bias_ = a.bias_;
    size_ = a.size_;
    return *this;
}

Array Array::operator+(const Array &a) {
    if (size_ != a.size_) {
        std::cout << "error: can't add two array with different size" << std::endl;
        return {};
    }

    Array result = Array(size_);
    int len = 1;
    for (int num: size_) len *= num;
    for (int i = 0; i < len; i++) result.data_[i + result.bias_] = data_[i + bias_] + a.data_[i + a.bias_];
    return result;
}

Array Array::operator-(const Array &a) {
    if (size_ != a.size_) {
        std::cout << "error: can't sub two array with different size" << std::endl;
        return {};
    }

    Array result = Array(size_);
    int len = 1;
    for (int num: size_) len *= num;
    for (int i = 0; i < len; i++) result.data_[i + result.bias_] = data_[i + bias_] - a.data_[i + a.bias_];
    return result;
}

Array Array::operator*(const Array &a) {
    if (size_ != a.size_) {
        std::cout << "error: can't mul two array with different size" << std::endl;
        return {};
    }

    Array result = Array(size_);
    int len = 1;
    for (int num: size_) len *= num;
    for (int i = 0; i < len; i++) result.data_[i + result.bias_] = data_[i + bias_] * a.data_[i + a.bias_];
    return result;
}

Array Array::operator/(const Array &a) {
    if (size_ != a.size_) {
        std::cout << "error: can't div two array with different size" << std::endl;
        return {};
    }

    Array result = Array(size_);
    int len = 1;
    for (int num: size_) len *= num;
    for (int i = 0; i < len; i++) result.data_[i + result.bias_] = data_[i + bias_] / a.data_[i + a.bias_];
    return result;
}

bool Array::operator==(const Array &a) {
    if (size_ != a.size_) return false;

    int len = 1;
    for (int num: size_) len *= num;
    for (int i = 0; i < len; i++)
        if (data_[i + bias_] != a.data_[i + a.bias_])
            return false;

    return true;
}

bool Array::operator!=(const Array &a) {
    return !(*this == a);
}

std::ostream& operator<<(std::ostream& os, const Array& array) {
    std::stack<int> in;
    int j = int(array.size_.size()) - 1, index = 0;
    in.push(0);
    os << "[";
    while (!in.empty()) {
        if (in.top() == array.size_[j]) {
            os << "]";
            in.pop();
            if (!in.empty()) {
                in.top()++; j++;
                if (in.top() != array.size_[j]) os << ",";
            }
            continue;
        }
        if (j == 0) {
            os << array.data_[array.bias_ + index++];
            in.top()++;
            if (in.top() < array.size_[j]) os << ",";
        }
        else {
            os << "[";
            in.push(0);
            j--;
        }
    }
    return os;
}

/*
 * Symbol Table
 */
std::vector<SymbolTable> SymbolManager::symbolTables_ = std::vector<SymbolTable>();
std::vector<std::map<std::string, AstPtr>> SymbolManager::funcTables_ = std::vector<std::map<std::string, AstPtr>>();
std::vector<SymbolManager::Status> SymbolManager::status_ = std::vector<SymbolManager::Status>();

ElePtr SymbolManager::lookup(const std::string &name) {
    ElePtr result = nullptr;
    for (auto it = symbolTables_.rbegin(); it != symbolTables_.rend(); ++it)
        if ((*it).find(name) != (*it).end()) {
            result = (*it)[name];
            break;
        }
    return result;
}

ElePtr SymbolManager::lookupTop(const std::string &name) {
    ElePtr result = nullptr;
    if (symbolTables_.back().find(name) != symbolTables_.back().end())
        result = symbolTables_.back()[name];
    return result;
}

void SymbolManager::addLayer() {
    symbolTables_.emplace_back();
}

void SymbolManager::add(const std::string &name, const ElePtr &symbol) {
    symbolTables_.back()[name] = symbol;
}

void SymbolManager::del(const std::string &name) {
    for (auto it = symbolTables_.rbegin(); it != symbolTables_.rend(); ++it)
        if ((*it).find(name) != (*it).end()) {
            it->erase(name);
            break;
        }
}

void SymbolManager::popLayer() {
    symbolTables_.pop_back();
}

AstPtr SymbolManager::lookupF(const std::string &name) {
    AstPtr result = nullptr;
    for (auto it = funcTables_.rbegin(); it != funcTables_.rend(); ++it)
        if ((*it).find(name) != (*it).end()) {
            result = (*it)[name];
            break;
        }
    return result;
}

void SymbolManager::addLayerF() {
    funcTables_.emplace_back();
}

void SymbolManager::addF(const std::string &name, const AstPtr &func) {
    funcTables_.back()[name] = func;
}

void SymbolManager::popLayerF() {
    funcTables_.pop_back();
}

bool SymbolManager::checkFunc() {
    for (auto it = status_.rbegin(); it != status_.rend(); ++it)
        if (*it == FUNC)
            return true;
    return false;
}

void SymbolManager::popFunc() {
    if (!checkFunc()) {
        std::cerr << "can't return here" << std::endl;
        exit(EXIT_FAILURE);
    }
    while(status_.back() != FUNC)
        status_.pop_back();
    status_.push_back(RETURN);
}

/*
 * Abstract Binary Tree
 */
Element Element::operator+(const Element &a) {
    Element result;
    if (type_ == BOOL || a.type_ == BOOL) {
        std::cerr << "can't add a bool" << std::endl;
        exit(1);
    }
    if (type_ == NUM || a.type_ == NUM) {
        result.type_ = NUM;
        result.n_ = int(*this) + int(a);
    }
    else {
        result.type_ = ARRAY;
        result.a_(a_ + a.a_);
    }
    return result;
}

Element Element::operator-(const Element &a) {
    Element result;
    if (type_ == BOOL && a.type_ == BOOL) {
        std::cerr << "can't sub a bool" << std::endl;
        exit(1);
    }
    if (type_ == NUM || a.type_ == NUM) {
        result.type_ = NUM;
        result.n_ = int(*this) - int(a);
    }
    else {
        result.type_ = ARRAY;
        result.a_(a_ - a.a_);
    }
    return result;
}

Element Element::operator*(const Element &a) {
    Element result;
    if (type_ == BOOL || a.type_ == BOOL) {
        std::cerr << "can't mul a bool" << std::endl;
        exit(1);
    }
    if (type_ == NUM || a.type_ == NUM) {
        result.type_ = NUM;
        result.n_ = int(*this) * int(a);
    }
    else {
        result.type_ = ARRAY;
        result.a_(a_ * a.a_);
    }
    return result;
}

Element Element::operator/(const Element &a) {
    Element result;
    if (type_ == BOOL || a.type_ == BOOL) {
        std::cerr << "can't div a bool" << std::endl;
        exit(1);
    }
    if (type_ == NUM || a.type_ == NUM) {
        result.type_ = NUM;
        result.n_ = int(*this) / int(a);
    }
    else {
        result.type_ = ARRAY;
        result.a_(a_ / a.a_);
    }
    return result;
}

Element Element::operator>(const Element & a) {
    Element result;
    result.type_ = BOOL;
    if (type_ == BOOL || a.type_ == BOOL) {
        std::cerr << "can't compare a bool" << std::endl;
        exit(1);
    }
    if (int(*this) > int(a)) result.n_ = 1;
    else result.n_ = 0;
    return result;
}

Element Element::operator<(const Element & a) {
    Element result;
    result.type_ = BOOL;
    if (type_ == BOOL || a.type_ == BOOL) {
        std::cerr << "can't compare a bool" << std::endl;
        exit(1);
    }
    if (int(*this) < int(a)) result.n_ = 1;
    else result.n_ = 0;
    return result;
}

Element Element::operator!=(const Element & a) {
    Element result = (*this == a);
    result.n_ = result.n_ == 0 ? 1 : 0;
    return result;
}

Element Element::operator==(const Element & a) {
    Element result;
    result.type_ = BOOL;
    if (type_ == BOOL || a.type_ == BOOL) {
        std::cerr << "can't compare a bool" << std::endl;
        exit(1);
    }
    if (type_ == NUM || a.type_ == NUM) {
        if (int(*this) == int(a)) result.n_ = 1;
        else result.n_ = 0;
    }
    else {
        if (a_ == a.a_) result.n_ = 1;
        else result.n_ = 0;
    }
    return result;
}

Element Element::operator>=(const Element & a) {
    Element result;
    result.type_ = BOOL;
    if (type_ == BOOL || a.type_ == BOOL) {
        std::cerr << "can't compare a bool" << std::endl;
        exit(1);
    }
    if (int(*this) >= int(a)) result.n_ = 1;
    else result.n_ = 0;
    return result;
}

Element Element::operator<=(const Element & a) {
    Element result;
    result.type_ = BOOL;
    if (type_ == BOOL || a.type_ == BOOL) {
        std::cerr << "can't compare a bool" << std::endl;
        exit(1);
    }
    if (int(*this) <= int(a)) result.n_ = 1;
    else result.n_ = 0;
    return result;
}

Element Element::operator&&(const Element &a) {
    Element result;
    result.type_ = BOOL;
    if (type_ != BOOL || a.type_ != BOOL) {
        std::cerr << "can only and two booleans" << std::endl;
        exit(1);
    }
    result.n_ = n_ & a.n_;
    return result;
}

Element Element::operator||(const Element &a) {
    Element result;
    result.type_ = BOOL;
    if (type_ != BOOL || a.type_ != BOOL) {
        std::cerr << "can only and two booleans" << std::endl;
        exit(1);
    }
    result.n_ = n_ | a.n_;
    return result;
}

Element Element::operator%(const Element &a) {
    Element result;
    result.type_ = NUM;
    if (!(type_ == NUM || type_ == ARRAY) && !(a.type_ == NUM || type_ == ARRAY)) {
        std::cerr << "bad operate mod" << std::endl;
        exit(EXIT_FAILURE);
    }
    result.n_ = int(*this) % int(a);
    return result;
}

Element &Element::operator=(const Element &a) {
    type_ = a.type_;
    switch (type_) {
        case NUM:
        case BOOL: n_ = a.n_; break;
        case ARRAY: a_(a.a_);
    }
    return *this;
}

std::ostream& operator<<(std::ostream& os, const Element & element) {
    switch (element.type_) {
        case Element::NUM:
            os << "num: " << element.n_; break;
        case Element::ARRAY:
            os << "array: " << element.a_; break;
        case Element::BOOL:
            os << "bool: ";
            if (element.n_ == 0) os << "false";
            else os << "true"; break;
    }
    return os;
}

Element::operator int() const {
    switch (type_) {
        case NUM:
        case BOOL: return n_;
        case ARRAY: return a_;
    }
}

Element Ast::eval() {
    switch (type_) {
        case '+': return left_->eval() + right_->eval();
        case '-': return left_->eval() - right_->eval();
        case '*': return left_->eval() * right_->eval();
        case '/': return left_->eval() / right_->eval();
        case '%': return left_->eval() % right_->eval();
        case '1': return left_->eval() > right_->eval();
        case '2': return left_->eval() < right_->eval();
        case '3': return left_->eval() != right_->eval();
        case '4': return left_->eval() == right_->eval();
        case '5': return left_->eval() >= right_->eval();
        case '6': return left_->eval() <= right_->eval();

        case 'e': return EMPTY;
        case 'l':
            if (left_) {
                Element result = left_->eval();
                if (SymbolManager::topStatus() == SymbolManager::CONTINUE ||
                    SymbolManager::topStatus() == SymbolManager::BREAK ||
                    SymbolManager::topStatus() == SymbolManager::RETURN)
                    return result;
            }
            if (right_) {
                Element result = right_->eval();
                if (SymbolManager::topStatus() == SymbolManager::CONTINUE ||
                    SymbolManager::topStatus() == SymbolManager::BREAK ||
                    SymbolManager::topStatus() == SymbolManager::RETURN)
                    return result;
            }
            return EMPTY;

        case 'a': return left_->eval() && right_->eval();
        case 'o': return left_->eval() || right_->eval();

        case 'b':
            if (SymbolManager::topStatus() != SymbolManager::WHILE) {
                std::cerr << "invalid break" << std::endl;
                exit(EXIT_FAILURE);
            }
            SymbolManager::addStatus(SymbolManager::BREAK);
            return EMPTY;
        case 'c':
            if (SymbolManager::topStatus() != SymbolManager::WHILE) {
                std::cerr << "invalid continue" << std::endl;
                exit(EXIT_FAILURE);
            }
            SymbolManager::addStatus(SymbolManager::CONTINUE);
            return EMPTY;
        case 'r': {
            if (!SymbolManager::checkFunc()) {
                std::cerr << "can't return here" << std::endl;
                exit(EXIT_FAILURE);
            }
            SymbolManager::addStatus(SymbolManager::RETURN);
            if (left_ != nullptr)
                return left_->eval();
            return EMPTY;
        }

        case 'w':
            WINDOW_HEIGHT = (int)left_->eval();
            WINDOW_WIDTH = (int)right_->eval();
            std::this_thread::sleep_for(std::chrono::milliseconds (200));
            return EMPTY;
        default : return {};
    }
}

Element Num::eval() {
    Element result;
    result.type_ = Element::NUM;
    result.n_ = value_;
    return result;
}

Element NumArray::eval() {
    Element result;
    if (!size_.empty())
        value_(Array(ConvertList(size_)));
    result.type_ = Element::ARRAY;
    result.a_(value_);
    return result;
}

Element SymRef::eval() {
    symbol_ = SymbolManager::lookup(name_);
    if (symbol_ == nullptr) {
        std::cerr << "can't refer to a uninitialized variable: " << name_ << std::endl;
        exit(1);
    }
    return *symbol_;
}

Element ArrayRef::eval() {
    Element result;
    if (left_->type_ != 'N') {
        std::cerr << "only a identifier can be referred" << std::endl;
        exit(1);
    }
    auto symRef = std::dynamic_pointer_cast<SymRef>(left_);
    result = symRef->eval();
    if (result.type_ != Element::ARRAY) {
        std::cout << "only array can be referred" << std::endl;
        exit(1);
    }
    else {
        result.a_(*result.a_.at(ConvertList(index_)));
    }
    return result;
}

Element SymAsgn::eval() {
    Element result, value = right_->eval();
    if (left_->type_ == 'N') {
        auto symRef = std::dynamic_pointer_cast<SymRef>(left_);
        result = symRef->eval();
        switch (symRef->symbol_->type_) {
            case Element::NUM: {
                if (value.type_ == Element::NUM) symRef->symbol_->n_ = value.n_;
                else if (value.type_ == Element::ARRAY) symRef->symbol_->n_ = value.a_;
                else if (value.type_ == Element::BOOL) {
                    std::cerr << "can't assign a boolean to a num" << std::endl;
                    exit(EXIT_FAILURE);
                }
                break;
            }
            case Element::ARRAY: {
                if (value.type_ == Element::NUM) symRef->symbol_->a_ = value.n_;
                else if (value.type_ == Element::ARRAY) symRef->symbol_->a_ = value.a_;
                else if (value.type_ == Element::BOOL) {
                    std::cerr << "can't assign a boolean to an array" << std::endl;
                    exit(EXIT_FAILURE);
                }
                break;
            }
            case Element::BOOL: {
                if (value.type_ == Element::BOOL) symRef->symbol_->n_ = value.n_;
                else {
                    std::cerr << "can't assign a bool with other type" << std::endl;
                    exit(1);
                }
                break;
            }
        }
    }
    else if (left_->type_ == 'M') {
        auto arrayRef = std::dynamic_pointer_cast<ArrayRef>(left_);
        result = arrayRef->eval();
        if (value.type_ == Element::ARRAY) result.a_ = value.a_;
        else if (value.type_ == Element::NUM) result.a_ = value.n_;
        else {
            std::cerr << "can't assign a bool with other type" << std::endl;
            exit(1);
        }
    }

    return result;
}

Element SymDecl::eval() {
    auto symbol = SymbolManager::lookupTop(name_);
    if (symbol != nullptr) {
        std::cerr << "this variable has been used: " << name_ << std::endl;
        exit(1);
    }
    ElePtr result = std::make_shared<Element>();
    *result = value_->eval();
    SymbolManager::add(name_, result);
    return *result;
}

Element PrintCal::eval() {
    Element res = left_->eval();
    std::cout << res << std::endl;
    return res;
}

Element IfSta::eval() {
    Element exp = left_->eval();
    if (exp.type_ != Element::BOOL) {
        std::cerr << "invalid if statement" << std::endl;
        exit(1);
    }
    if (exp.n_ == 1) {
        Element result = right_->eval();
        if (SymbolManager::topStatus() == SymbolManager::RETURN) {
            SymbolManager::popFunc();
            return result;
        }
    }
    return EMPTY;
}

Element WhileSta::eval() {
    Element exp = left_->eval();
    if (exp.type_ != Element::BOOL) {
        std::cerr << "invalid while statement" << std::endl;
        exit(1);
    }
    SymbolManager::addStatus(SymbolManager::WHILE);
    while(exp.n_ == 1) {
        Element result = right_->eval();
        if (SymbolManager::topStatus() == SymbolManager::CONTINUE)
            SymbolManager::popStatus();
        else if (SymbolManager::topStatus() == SymbolManager::BREAK) {
            SymbolManager::popStatus();
            break;
        }
        else if (SymbolManager::topStatus() == SymbolManager::RETURN) {
            SymbolManager::popFunc();
            return result;
        }
        exp = left_->eval();
    }
    SymbolManager::popStatus();
    return EMPTY;
}

Element ForSta::eval() {
    if(left_->type_ != 'D') {
        std::cerr << "the first scope of the for statement must be a declaration" << std::endl;
        exit(1);
    }
    SymbolManager::addLayer();
    left_->eval();
    Element exp = right_->eval();
    if (exp.type_ != Element::BOOL) {
        std::cerr << "invalid for statement" << std::endl;
        exit(1);
    }
    SymbolManager::addStatus(SymbolManager::WHILE);
    while (exp.n_ == 1) {
        Element result = _statements->eval();
        if (SymbolManager::topStatus() == SymbolManager::CONTINUE)
            SymbolManager::popStatus();
        else if (SymbolManager::topStatus() == SymbolManager::BREAK) {
            SymbolManager::popStatus();
            break;
        }
        else if (SymbolManager::topStatus() == SymbolManager::RETURN) {
            SymbolManager::popFunc();
            return result;
        }
        _expression->eval();
        exp = right_->eval();
    }
    SymbolManager::popStatus();
    SymbolManager::popLayer();
    return EMPTY;
}

Element CpdSta::eval() {
    SymbolManager::addLayer();
    Element result = left_->eval();
    SymbolManager::popLayer();
    return result;
}

Element BoolCons::eval() {
    if (type_ == 't')
        return { Element::BOOL, 1 };
    else
        return { Element::BOOL, 0 };
}

Element FuncDecl::eval() {
    auto func = SymbolManager::lookupF(name_);
    if (func != nullptr) {
        std::cerr << "this functions has been defined" << std::endl;
        exit(EXIT_FAILURE);
    }
    for (const auto & ast: parameters_)
        if (ast->type_ != 'N') {
            std::cerr << "invalid parameters in func: " << name_ << std::endl;
            exit(EXIT_FAILURE);
        }

    func = std::make_shared<FuncDecl>(name_, parameters_, left_);
    SymbolManager::addF(name_, func);
    return EMPTY;
}

Element FuncCall::eval() {
    auto def = SymbolManager::lookupF(name_);
    if (def == nullptr) {
        std::cerr << "no such functions called: " << name_ << std::endl;
        exit(EXIT_FAILURE);
    }
    auto func = std::static_pointer_cast<FuncDecl>(def);
    if (func->parameters_.size() != parameters_.size()) {
        std::cerr << "need " << func->parameters_.size() << " parameters, but providing " << parameters_.size() << std::endl;
        exit(EXIT_FAILURE);
    }
    SymbolManager::addLayer();
    SymbolManager::addStatus(SymbolManager::FUNC);
    for (int i = 0; i < parameters_.size(); i++) {
        auto symRef = std::static_pointer_cast<SymRef>(func->parameters_[i]);
        auto temp = new SymDecl(symRef->name_, parameters_[i]);
        temp->eval();
        delete temp;
    }
    Element result = func->left_->eval();
    if (SymbolManager::topStatus() == SymbolManager::RETURN) {
        SymbolManager::popStatus();
        SymbolManager::popStatus();
        SymbolManager::popLayer();
        return result;
    }
    SymbolManager::popStatus();
    SymbolManager::popLayer();
    return EMPTY;
}

NumList EvalVec(int n, ...) {
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

Element ArrayPack::eval() {
    Element result;
    result.type_ = Element::ARRAY;
    Element first = contain_.back()->eval();
    if (first.type_ == Element::NUM || (first.type_ == Element::ARRAY && first.a_.size_ == EvalVec(1, 1)) ) {
        Array array(EvalVec(1, int(contain_.size())));
        *array.at(EvalVec(1, 0)) = int(first);
        for (int i = 1; i < int(contain_.size()); i++)
            *array.at(EvalVec(1, i)) = int(contain_[int(contain_.size()) - 1 - i]->eval());
        result.a_(array);
    }
    else if (first.type_ == Element::ARRAY) {
        NumList size = first.a_.size_;
        size.push_back(int(contain_.size()));
        Array array(size);
        *array.at(EvalVec(1, 0)) = first.a_;
        for (int i = 1; i < int(contain_.size()); i++) {
            Element temp = contain_[int(contain_.size()) - 1 - i]->eval();
            if (temp.type_ != Element::ARRAY && temp.a_.size_ != first.a_.size_) {
                std::cerr << "bad array pack" << std::endl;
                exit(EXIT_FAILURE);
            }
            *array.at(EvalVec(1, i)) = temp.a_;
        }
        result.a_(array);
    }
    return result;
}

Element InnerCall::eval() {
    Element result;
    switch(type_) {
        case TIME:
            result.type_ = Element::NUM;
            result.n_ = int(glfwGetTime() * 1000);
    }
    return result;
}