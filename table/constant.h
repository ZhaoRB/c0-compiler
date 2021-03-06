//
// Created by Decade on 2019/12/16 0016.
//

#ifndef CC0_CONSTANT_H
#define CC0_CONSTANT_H

#include <string>
#include <utility>


// 这个是常量表
namespace miniplc0 {

    class Constant {

    public:
        char type;
        std::string value;
    public:
        Constant(char _type, std::string _value)
            : type(_type), value(std::move(_value)) {}

    };

}
#endif //CC0_CONSTANT_H
