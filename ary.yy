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

%nonassoc <num> CMP
%right '='
%left '+' '-'
%left '*' '/'
%type <ast> expression postfix_expression primary_expression declaration statement statement_list
%type <array> expression_list
%type <s> initializer

%locations

%%
start
    : statement_list { $1->eval(); delete $1; }
    ;

statement_list
    : statement { $$ = $1; }
    | statement_list statement { $$ = new Ast('l', std::shared_ptr<Ast>($1), std::shared_ptr<Ast>($2)); }
    ;

statement
    : EOL { $$ = new Ast('e', nullptr, nullptr); }
    | declaration EOL { $$ = $1; }
    | expression EOL { $$ = $1; }
    | PRINT expression EOL { $$ = new PrintCal(std::shared_ptr<Ast>($2)); }
    ;

declaration
    : IDENTIFIER ASSIGN initializer {
        $$ = new SymDecl(*$1, std::shared_ptr<Symbol>($3));
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
    : expression CMP expression { $$ = new Ast($2,  std::shared_ptr<Ast>($1), std::shared_ptr<Ast>($3)); }
    | expression '+' expression { $$ = new Ast('+', std::shared_ptr<Ast>($1), std::shared_ptr<Ast>($3)); }
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
        $$ = new SymRef(*$1); 
        delete $1;
    }
    ;
%%

namespace MC {
    void Parser::error(location const &loc, const std::string & s) {
        std::cerr << "error at " << loc << ": " << s << std::endl;
    }
}