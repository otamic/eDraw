//
// Created by Garrison on 2021/11/10.
//

#ifndef ARRAY_M_TYPE_H
#define ARRAY_M_TYPE_H

#include <memory>
#include <utility>
#include <vector>
#include <string>
#include <map>

class Array;

typedef std::vector<int> NumList;
typedef std::shared_ptr<Array> ArrayPtr;

class Array {
public:
    Array() = default;
    ~Array() = default;
    explicit Array(const NumList & size);
    ArrayPtr at(const NumList & index);
    Array& operator=(int num) { data_[bias_] = num; return *this; }
    Array& operator=(const Array & a);
    Array& operator()(const Array & a);

    Array operator+(const Array & a);
    Array operator-(const Array & a);
    Array operator*(const Array & a);
    Array operator/(const Array & a);

    bool operator==(const Array & a);
    bool operator!=(const Array & a);

    friend std::ostream& operator<<(std::ostream& os, const Array& array);

    operator int() const { return data_[bias_]; }

private:
    std::shared_ptr<int[]> data_;
    int bias_;
    NumList size_;
};

/*
 * Symbol Table
 */
class Symbol;
typedef std::shared_ptr<Symbol> SymbolPtr;

class Symbol {
public:
    Symbol(int type): type_(type) {}
    virtual ~Symbol() = default;
    int type_;

    static SymbolPtr lookup(const std::string & name);
    static void add(const std::string & name, const SymbolPtr & symbol);
private:
    static std::map<std::string, SymbolPtr> symbolTable_;
};

class NumSymbol: public Symbol {
public:
    NumSymbol(int num): Symbol('n'), data_(num) {}
    int data_;
};

class ArraySymbol: public Symbol {
public:
    ArraySymbol(ArrayPtr array): Symbol('a'), data_(std::move(array)){}
    ArrayPtr data_;
};

class BoolSymbol: public Symbol {
public:
    BoolSymbol(int num): Symbol('b'), data_(num) {}
    int data_;
};

/*
 * Abstract Binary Tree
 */
class Ast;
typedef std::shared_ptr<Ast> AstPtr;

class Element {
public:
    Element() = default;

    Element operator+(const Element & a);
    Element operator-(const Element & a);
    Element operator*(const Element & a);
    Element operator/(const Element & a);

    Element operator>(const Element & a);
    Element operator<(const Element & a);
    Element operator!=(const Element & a);
    Element operator==(const Element & a);
    Element operator>=(const Element & a);
    Element operator<=(const Element & a);

    friend std::ostream& operator<<(std::ostream& os, const Element & element);

    enum Type {NUM, ARRAY, BOOL} type_;
    struct {
        int n_;
        Array a_;
    };

    operator int() const;
};

class Ast {
public:
    int type_;
    AstPtr left_, right_;

    Ast(int type, AstPtr left, AstPtr right): type_(type), left_(std::move(left)), right_(std::move(right)) {}
    virtual Element eval();
};

class Num : public Ast {
public:
    int value_;

    explicit Num(int value):Ast('K', nullptr, nullptr), value_(value) {}
    Element eval();
};

class NumArray : public Ast {
public:
    Array value_;

    explicit NumArray(const Array& value):Ast('A', nullptr, nullptr), value_(value) {}
    Element eval();
};

class SymRef : public Ast {
public:
    std::string name_;
    SymbolPtr symbol_;

    explicit SymRef(std::string name):Ast('N', nullptr, nullptr), name_(std::move(name)) {}
    Element eval();
};

class ArrayRef : public Ast {
public:
    NumList index_;

    ArrayRef(AstPtr symbol, NumList index):Ast('M', std::move(symbol), nullptr), index_(std::move(index)) {}
    Element eval();
};

class SymAsgn : public Ast {
public:
    SymAsgn(AstPtr ref, AstPtr value):Ast('=', std::move(ref), std::move(value)) {}
    Element eval();
};

class SymDecl : public Ast {
public:
    SymDecl(std::string name, SymbolPtr symbol):Ast('D', nullptr, nullptr), name_(std::move(name)), symbol_(std::move(symbol)) {}
    Element eval();
    std::string name_;
    SymbolPtr symbol_;
};

class PrintCal : public Ast {
public:
    PrintCal(AstPtr exp):Ast('C', std::move(exp), nullptr) {}
    Element eval();
};

#endif //ARRAY_M_TYPE_H
