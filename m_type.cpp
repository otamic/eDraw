//
// Created by Garrison on 2021/11/10.
//

#include "m_type.h"

#include <iostream>
#include <stack>
#include <memory>

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
    return nullptr;
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
std::map<std::string, SymbolPtr> Symbol::symbolTable_ = std::map<std::string, SymbolPtr>();

std::shared_ptr<Symbol> Symbol::lookup(const std::string & name) {
    if (symbolTable_.find(name) == symbolTable_.end()) return nullptr;
    return symbolTable_[name];
}

void Symbol::add(const std::string & name, const std::shared_ptr<Symbol> & symbol) {
    symbolTable_[name] = symbol;
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
        case '1': return left_->eval() > right_->eval();
        case '2': return left_->eval() < right_->eval();
        case '3': return left_->eval() != right_->eval();
        case '4': return left_->eval() == right_->eval();
        case '5': return left_->eval() >= right_->eval();
        case '6': return left_->eval() <= right_->eval();

        case 'e': return EMPTY;
        case 'l': if (left_) left_->eval(); if (right_) right_->eval(); return EMPTY;
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
    result.type_ = Element::ARRAY;
    result.a_(value_);
    return result;
}

Element SymRef::eval() {
    Element result;
    symbol_ = Symbol::lookup(name_);
    if (symbol_ == nullptr) {
        std::cerr << "can't refer to a uninitialized variable" << std::endl;
        exit(1);
    }
    switch(symbol_->type_) {
        case 'n': {
            auto numSymbol = std::dynamic_pointer_cast<NumSymbol>(symbol_);
            result.type_ = Element::NUM;
            result.n_ = numSymbol->data_;
            break;
        }
        case 'a': {
            auto arraySymbol = std::dynamic_pointer_cast<ArraySymbol>(symbol_);
            result.type_ = Element::ARRAY;
            result.a_(*arraySymbol->data_);
            break;
        }
        case 'b': {
            auto boolSymbol = std::dynamic_pointer_cast<BoolSymbol>(symbol_);
            result.type_ = Element::BOOL;
            result.n_ = boolSymbol->data_;
        }
    }
    return result;
}

Element ArrayRef::eval() {
    Element result;
    if (left_->type_ != 'N') {
        std::cerr << "only a identifier can be referred" << std::endl;
        exit(1);
    }
    auto symRef = std::dynamic_pointer_cast<SymRef>(left_);
    symRef->eval();
    if (symRef->symbol_->type_ == 'n' || symRef->symbol_->type_ == 'b') {
        std::cout << "only array can be referred" << std::endl;
        exit(1);
    }
    else {
        auto arraySymbol = std::dynamic_pointer_cast<ArraySymbol>(symRef->symbol_);
        result.type_ = Element::ARRAY;
        result.a_(*arraySymbol->data_->at(index_));
    }
    return result;
}

Element SymAsgn::eval() {
    Element result, value = right_->eval();
    if (left_->type_ == 'N') {
        auto symRef = std::dynamic_pointer_cast<SymRef>(left_);
        symRef->eval();
        if (symRef->symbol_->type_ == 'n') {
            auto numSymbol = std::dynamic_pointer_cast<NumSymbol>(symRef->symbol_);
            if (value.type_ == Element::NUM) numSymbol->data_ = value.n_;
            else numSymbol->data_ = value.a_;
            result.type_ = Element::NUM;
            result.n_ = numSymbol->data_;
        }
        else if (symRef->symbol_->type_ == 'a'){
            auto arraySymbol = std::dynamic_pointer_cast<ArraySymbol>(symRef->symbol_);
            if (value.type_ == Element::ARRAY) *arraySymbol->data_ = value.a_;
            else *arraySymbol->data_ = value.n_;
            result.type_ = Element::ARRAY;
            result.a_(*arraySymbol->data_);
        }
        else if (symRef->symbol_->type_ == 'b'){
            auto boolSymbol = std::dynamic_pointer_cast<BoolSymbol>(symRef->symbol_);
            if (value.type_ == Element::BOOL) boolSymbol->data_ = value.n_;
            else {
                std::cerr << "can't assign a bool with other type" << std::endl;
                exit(1);
            }
            result.type_ = Element::BOOL;
            result.n_ = boolSymbol->data_;
        }
    }
    else if (left_->type_ == 'M') {
        auto arrayRef = std::dynamic_pointer_cast<ArrayRef>(left_);
        auto temp = arrayRef->eval();
        // temp.a_ = value.type_ == Element::ARRAY ? value.a_ : value.n_;
        if (value.type_ == Element::ARRAY) temp.a_ = value.a_;
        else if (value.type_ == Element::NUM) temp.a_ = value.n_;
        else {
            std::cerr << "can't assign a bool with other type" << std::endl;
            exit(1);
        }
        result.type_ = Element::ARRAY;
        result.a_(temp.a_);
    }

    return result;
}

Element SymDecl::eval() {
    auto symbol = Symbol::lookup(name_);
    if (symbol != nullptr) {
        std::cerr << "this variable has been used: " << name_ << std::endl;
        exit(1);
    }
    Symbol::add(name_, symbol_);

    Element result;
    switch(symbol_->type_) {
        case 'n': {
            auto numSymbol = std::static_pointer_cast<NumSymbol>(symbol_);
            result.type_ = Element::NUM;
            result.n_ = numSymbol->data_;
            break;
        }
        case 'a': {
            auto arraySymbol = std::static_pointer_cast<ArraySymbol>(symbol_);
            result.type_ = Element::ARRAY;
            result.a_(*arraySymbol->data_);
            break;
        }
        case 'b': {
            auto boolSymbol = std::static_pointer_cast<BoolSymbol>(symbol_);
            result.type_ = Element::BOOL;
            result.n_ = boolSymbol->data_;
            break;
        }
    }
    return result;
}

Element PrintCal::eval() {
    Element res = left_->eval();
    std::cout << res << std::endl;
    return res;
}