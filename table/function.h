//
// Created by Decade on 2019/12/16 0016.
//

#ifndef CC0_FUNCTION_H
#define CC0_FUNCTION_H

// 这个是函数表
namespace miniplc0 {

    class Function {

    private:
        int name_index;
        int paras_size;
        int level;
    public:
        Function(int _name_size, int _paras_size, int _level)
            : name_index(_name_size), paras_size(_paras_size), level(_level) {}
    };

}



#endif //CC0_FUNCTION_H
