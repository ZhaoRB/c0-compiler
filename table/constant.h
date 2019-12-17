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

    private:
        std::string type;
        std::string value;
    public:
        Constant(std::string _type, std::string _value);

    };

    Constant::Constant(std::string _type, std::string _value){
        type = std::move(_type);
        value = std::move(_value);
    }
    // std::move 节省空间 ？
}
#endif //CC0_CONSTANT_H
