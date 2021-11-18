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
    std::vector<AstPtr> * array;
};

%token <num> NUMBER
%token <symbol> IDENTIFIER
%token EOL INT ASSIGN PRINT IF WHILE

%nonassoc <num> CMP
%right '='
%left '+' '-' OR
%left '*' '/' AND
%type <ast> expression postfix_expression primary_expression declaration statement statement_list initializer bool_expression cmp_expression compound_statement
%type <array> expression_list

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
    | IF expression compound_statement { $$ = new IfSta(std::shared_ptr<Ast>($2), std::shared_ptr<Ast>($3)); }
    | WHILE expression compound_statement { $$ = new WhileSta(std::shared_ptr<Ast>($2), std::shared_ptr<Ast>($3)); }
    ;

declaration
    : IDENTIFIER ASSIGN initializer {
        $$ = new SymDecl(*$1, std::shared_ptr<Ast>($3));
        delete $1;
    }
    ;

initializer
    : expression { $$ = $1; }
    | INT '[' expression_list ']' { $$ = new NumArray(Array(ConvertList(*$3))); delete $3; }
    ;

compound_statement
    : '{' statement_list '}' { $$ = new CpdSta(std::shared_ptr<Ast>($2)); }
    ;

expression_list
    : expression {
        $$ = new std::vector<AstPtr>();
        $$->push_back(std::shared_ptr<Ast>($1));
    }
    | expression ',' expression_list { 
        $3->push_back(std::shared_ptr<Ast>($1));
        $$ = $3;
    }
    ;

expression 
    : bool_expression { $$ = $1; }
    | expression '+' expression { $$ = new Ast('+', std::shared_ptr<Ast>($1), std::shared_ptr<Ast>($3)); }
    | expression '-' expression { $$ = new Ast('-', std::shared_ptr<Ast>($1), std::shared_ptr<Ast>($3)); }
    | expression '*' expression { $$ = new Ast('*', std::shared_ptr<Ast>($1), std::shared_ptr<Ast>($3)); }
    | expression '/' expression { $$ = new Ast('/', std::shared_ptr<Ast>($1), std::shared_ptr<Ast>($3)); }
    | '(' expression ')' { $$ = $2; }
    | postfix_expression { $$ = $1; }
    | postfix_expression '=' expression { $$ = new SymAsgn(std::shared_ptr<Ast>($1), std::shared_ptr<Ast>($3)); }
    ;

bool_expression
    : cmp_expression { $$ = $1; }
    | '(' bool_expression ')' { $$ = $2; }
    | bool_expression AND bool_expression { $$ = new Ast('a', std::shared_ptr<Ast>($1), std::shared_ptr<Ast>($3)); }
    | bool_expression OR  bool_expression { $$ = new Ast('o', std::shared_ptr<Ast>($1), std::shared_ptr<Ast>($3)); }
    ;

cmp_expression
    : expression CMP expression { $$ = new Ast($2,  std::shared_ptr<Ast>($1), std::shared_ptr<Ast>($3)); }
    ;

postfix_expression
    : primary_expression { $$ = $1; }
    | postfix_expression '[' expression_list ']' { $$ = new ArrayRef(std::shared_ptr<Ast>($1), *$3); delete $3; }
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