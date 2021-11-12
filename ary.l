%option noyywrap
%{
    #include <string>
    #include <cstdlib>
    #include "m_type.h"
    #include "ary.tab.hpp"

    #define YY_DECL int yylex(yy::Ary::semantic_type * yylval, yy::Ary::location_type * yylloc)
    #define YY_USER_ACTION yylloc->columns(yyleng);

    using token = yy::Ary::token;    
%}

%%
%{
    yylloc->step();
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

[a-zA-Z][a-zA-Z0-9]* { yylval->symbol = new std::string(yytext); return token::IDENTIFIER; }
[0-9]+ { yylval->num = atoi(yytext); return token::NUMBER; }
\n { yylloc->lines(1); return token::EOL; }
"//".* |
[ \t] { yylloc->step(); }
. { printf("Mystery character %c\n", *yytext); }

%%