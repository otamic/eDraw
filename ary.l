%{
    #include <string>
    #include "m_scanner.h"
    #undef YY_DECL
    #define YY_DECL int MC::Scanner::yylex(MC::Parser::semantic_type * const lval, MC::Parser::location_type * loc)

    using token = MC::Parser::token;

    #define yytermiante() return(token::END)

    #define YY_NO_UNISTD_H

    #define YY_USER_ACTION loc->step(); loc->columns(yyleng);
%}

%option debug
%option nodefault
%option yyclass="MC::Scanner"
%option noyywrap
%option c++

%%
%{
    yylval = lval;
%}

"+" |
"-" |
"*" |
"/" |
"(" |
")" |
"[" |
"]" |
"," |
"=" { return yytext[0]; }

":=" { return token::ASSIGN; }
"int" { return token::INT; }

"print" { return token::PRINT; }

[a-zA-Z][a-zA-Z0-9]* { yylval->symbol = new std::string(yytext); return token::IDENTIFIER; }
[0-9]+ { yylval->num = atoi(yytext); return token::NUMBER; }
\n { loc->lines(1); return token::EOL; }
"//".* |
[ \t] { loc->step(); }
. { printf("Mystery character %c\n", *yytext); }

%%