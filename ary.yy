%language "C++"
%defines 
%define api.namespace {MC}
%define api.parser.class {Parser}

%code requires{
    namespace MC {
        class Driver;
        class Scanner;
    }
}

%parse-param { Scanner &scanner }
%parse-param { Driver &driver }

%{
    #include <iostream>    
    #include <fstream>
    #include "m_type.h"
    #include "m_driver.h"

    #undef yylex
    #define yylex scanner.yylex
%}

%define parse.assert

%union {
    Ast * ast;
    int num;
    std::string * symbol;
    Symbol * s;
    NumList * array;
};

%token <num> NUMBER
%token <symbol> IDENTIFIER
%token EOL INT ASSIGN PRINT

%right '='
%left '+' '-'
%left '*' '/'
%type <ast> expression postfix_expression primary_expression
%type <array> expression_list
%type <s> initializer

%locations

%%
statement_list
    : statement 
    | statement_list statement 
    ;

statement
    : EOL 
    | declaration EOL
    | expression EOL { $1->eval(); delete $1; }
    | PRINT expression EOL {
        Element res = $2->eval();
        if (res.type_ == Element::NUM) std::cout << "= " << res.n_ << std::endl;
        else res.a_.print();

        delete $2;
    }
    ;

declaration
    : IDENTIFIER ASSIGN initializer {
        auto symbol = Symbol::lookup(*$1);
        if (symbol != nullptr) {
            std::cerr << "this variable has been used" << std::endl;
            exit(1);
        }
        Symbol::add(*$1, std::shared_ptr<Symbol>($3));

        delete $1;
    }
    ;

initializer
    : expression { 
        Element res = $1->eval();
        if (res.type_ == Element::NUM) $$ = new NumSymbol(res.n_);
        else {
            auto array = std::make_shared<Array>();
            (*array)(res.a_);
            $$ = new ArraySymbol(array);
        }

        delete $1;
    }
    | INT '[' expression_list ']' { $$ = new ArraySymbol(std::make_shared<Array>(*$3)); delete $3; }
    ;

expression_list
    : expression {
        Element res = $1->eval();
        auto list = new NumList();
        if (res.type_ == Element::NUM) list->push_back(res.n_);
        else list->push_back(int(res.a_));
        $$ = list;
        
        delete $1;
    }
    | expression ',' expression_list { 
        Element res = $1->eval();
        if (res.type_ == Element::NUM) $3->push_back(res.n_);
        else $3->push_back(int(res.a_));
        $$ = $3;

        delete $1;
    }
    ;

expression 
    : expression '+' expression { $$ = new Ast('+', std::shared_ptr<Ast>($1), std::shared_ptr<Ast>($3)); }
    | expression '-' expression { $$ = new Ast('-', std::shared_ptr<Ast>($1), std::shared_ptr<Ast>($3)); }
    | expression '*' expression { $$ = new Ast('*', std::shared_ptr<Ast>($1), std::shared_ptr<Ast>($3)); }
    | expression '/' expression { $$ = new Ast('/', std::shared_ptr<Ast>($1), std::shared_ptr<Ast>($3)); }
    | '(' expression ')' { $$ = $2; }
    | postfix_expression { $$ = $1; }
    | postfix_expression '=' expression { $$ = new SymAsgn(std::shared_ptr<Ast>($1), std::shared_ptr<Ast>($3)); }
    ;

postfix_expression
    : primary_expression { $$ = $1; }
    | postfix_expression '[' expression_list ']' { $$ = new ArrayRef(std::shared_ptr<Ast>($1), *$3); }
    ;

primary_expression
    : NUMBER { $$ = new Num($1); }
    | IDENTIFIER { 
        auto symbol = Symbol::lookup(*$1);
        if (symbol == nullptr) {
            std::cerr << "can't refer to a uninitialized variable" << std::endl;
            exit(1);
        }
        $$ = new SymRef(symbol); 

        delete $1;
    }
    ;
%%

namespace MC {
    void Parser::error(location const &loc, const std::string & s) {
        std::cerr << "error at " << loc << ": " << s << std::endl;
    }
}