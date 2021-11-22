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
"=" |
"{" |
"}" |
"%" { return yytext[0]; }

">"  { yylval->num = '1'; return token::CMP; }
"<"  { yylval->num = '2'; return token::CMP; }
"!=" { yylval->num = '3'; return token::CMP; }
"==" { yylval->num = '4'; return token::CMP; }
">=" { yylval->num = '5'; return token::CMP; }
"<=" { yylval->num = '6'; return token::CMP; }

":=" { return token::ASSIGN; }
"int" { return token::INT; }

"print" { return token::PRINT; }

"if" { return token::IF; }
"while" { return token::WHILE; }

"and" { return token::AND; }
"or"  { return token::OR; }

"true" { return token::TRUE; }
"false" { return token::FALSE; }

"func" { return token::FUNC; }

"break" { return token::BREAK; }
"continue" { return token::CONTINUE; }

[a-zA-Z][a-zA-Z0-9]* { yylval->symbol = new std::string(yytext); return token::IDENTIFIER; }
[0-9]+ { yylval->num = atoi(yytext); return token::NUMBER; }
\n { loc->lines(1); return token::EOL; }
"//".* |
[ \t] { loc->step(); }
. { printf("Mystery character %c\n", *yytext); }

%%