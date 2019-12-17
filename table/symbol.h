//
// Created by Decade on 2019/12/17 0017.
//

#ifndef CC0_SYMBOL_H
#define CC0_SYMBOL_H

#include <string>
#include <utility>

// 符号表 在编译程序运行的时候使用
namespace miniplc0 {

    class Symbol {

    private:
        std::string name;
        char type;
        std::string value;
        bool isInitialized;
    public:
        Symbol(std::string _name, char _type, std::string _value, bool _isInitialized);
        void addValue(std::string _value);
    };

    Symbol::Symbol(std::string _name, char _type, std::string _value, bool _isInitialized) {
        name = std::move(_name);
        type = _type;
        value = std::move(_value);
        isInitialized = _isInitialized;
    }

    void Symbol::addValue(std::string _value) {
        value = std::move(_value);
        isInitialized = true;
    }
}

#endif //CC0_SYMBOL_H
