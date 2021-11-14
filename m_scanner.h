//
// Created by Garrison on 2021/11/14.
//

#ifndef ARRAY_M_SCANNER_H
#define ARRAY_M_SCANNER_H

#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "m_type.h"
#include "ary.tab.hpp"
#include "location.hh"

namespace MC {

class Scanner : public yyFlexLexer {
public:
    Scanner(std::istream * in) : yyFlexLexer(in) {}
    ~Scanner() override = default;

    using FlexLexer::yylex;

    virtual int yylex(MC::Parser::semantic_type * const lval, MC::Parser::location_type * location);

private:
    MC::Parser::semantic_type * yylval = nullptr;
};

}

#endif //ARRAY_M_SCANNER_H
