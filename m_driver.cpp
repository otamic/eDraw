//
// Created by Garrison on 2021/11/14.
//
#include <fstream>

#include "m_driver.h"

MC::Driver::~Driver() {
    delete(scanner);
    scanner = nullptr;
    delete(parser);
    parser = nullptr;
}

void MC::Driver::parse(const char *const filename) {
    assert(filename != nullptr);
    std::ifstream in_file(filename);
    if (!in_file.good()) {
        exit(EXIT_FAILURE);
    }
    parse_helper(in_file);
}

void MC::Driver::parse(std::istream &stream) {
    if (!stream.good() && stream.eof())
        return;
    parse_helper(stream);
}

void MC::Driver::parse_helper(std::istream &stream) {
    delete(scanner);
    scanner = new MC::Scanner(&stream);

    delete(parser);
    parser = new MC::Parser((*scanner), (*this));

    if (parser->parse() != 0)
        std::cerr << "Parse failed!\n";
}