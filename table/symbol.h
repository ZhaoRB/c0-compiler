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
        int value;
        bool isInitialized;
        int level;
    public:
        Symbol(std::string _name, int _value, bool _isInitialized, int _level);
        void addValue(int _value);
    };

    Symbol::Symbol(std::string _name, int _value, bool _isInitialized, int _level) {
        name = std::move(_name);
        value = _value;
        isInitialized = _isInitialized;
        level = _level;
    }

    void Symbol::addValue(int _value) {
        value = _value;
        isInitialized = true;
    }
}

#endif //CC0_SYMBOL_H
