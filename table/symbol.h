//
// Created by Decade on 2019/12/17 0017.
//

#ifndef CC0_SYMBOL_H
#define CC0_SYMBOL_H

#include <string>
#include <utility>

// 符号表 在编译程序运行的时候使用
// 常量表一个 变量表一个
namespace miniplc0 {

    class Symbol {

    private:
        std::string name;
        int level;
    public:
        Symbol(std::string _name, int _level)
            : name(std::move(_name)), level(_level) {}

    public:
        std::string getName();
        int getLevel();
    };

//    Symbol::Symbol(std::string _name, int _level) {
//        name = std::move(_name);
//        level = _level;
//    }



}

#endif //CC0_SYMBOL_H
