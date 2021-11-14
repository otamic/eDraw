//
// Created by Garrison on 2021/11/14.
//

#ifndef ARRAY_M_DRIVER_H
#define ARRAY_M_DRIVER_H

#include <string>
#include <istream>

#include "m_scanner.h"
#include "ary.tab.hpp"

namespace MC {

class Driver {
public:
    Driver() = default;

    virtual ~Driver();

    void parse(const char * const filename);
    void parse(std::istream &iss);

private:
    void parse_helper(std::istream &stream);

    MC::Parser * parser = nullptr;
    MC::Scanner * scanner = nullptr;
};
}

#endif //ARRAY_M_DRIVER_H
