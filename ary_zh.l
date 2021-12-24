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

U       [\x80-\xbf]
U2      [\xc2-\xdf]
U3      [\xe0-\xef]
U4      [\xf0-\xf4]
SP      [_a-zA-Z0-9]
SA      [_a-zA-Z]
N1      {SA}|{U2}{U}|{U3}{U}{U}|{U4}{U}{U}{U}
N2      {SP}|{U2}{U}|{U3}{U}{U}|{U4}{U}{U}{U}

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
"%" |
";" { return yytext[0]; }

">"  { yylval->num = '1'; return token::CMP; }
"<"  { yylval->num = '2'; return token::CMP; }
"!=" { yylval->num = '3'; return token::CMP; }
"==" { yylval->num = '4'; return token::CMP; }
">=" { yylval->num = '5'; return token::CMP; }
"<=" { yylval->num = '6'; return token::CMP; }

":=" { return token::ASSIGN; }

"整" |
"int" { return token::INT; }

"输出" |
"print" { return token::PRINT; }
"用时" |
"time" { return token::TIME; }

"若" |
"if" { return token::IF; }
"当" |
"for" { return token::FOR; }

"并" |
"and" { return token::AND; }
"或" |
"or"  { return token::OR; }

"真" |
"true" { return token::TRUE; }
"假" |
"false" { return token::FALSE; }

"函数" |
"func" { return token::FUNC; }

"停" |
"break" { return token::BREAK; }
"过" |
"continue" { return token::CONTINUE; }
"返回" |
"return" { return token::RETURN; }

"窗口" |
"WINSIZE" { return token::WINSIZE; }

[0-9]+ { yylval->num = atoi(yytext); return token::NUMBER; }
{N1}{N2}* { yylval->symbol = new std::string(yytext); return token::IDENTIFIER; }
"\\\n" { loc->lines(1); }
\n { loc->lines(1); return token::EOL; }
"//".* |
[ \t] { loc->step(); }
. { printf("Mystery character %c\n", *yytext); }

%%