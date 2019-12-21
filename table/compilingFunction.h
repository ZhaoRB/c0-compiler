//
// Created by Decade on 2019/12/21 0021.
//

#ifndef CC0_COMPILINGFUNCTION_H
#define CC0_COMPILINGFUNCTION_H

#include <string>
#include <utility>
#include <vector>

namespace miniplc0 {

    class CompilingFunction {

    private:
        std::string functionName;
        // 存储参数的类型
        // 但是基础c0 只有int 所以其实也没什么必要 只需要存一个参数的数量就行了
        int parameterNum;
        // std::vector<std::string> paras;
    public:
        CompilingFunction(std::string _functionName, int _parameterNum);
        // void addParas(const std::string& parameterType);
        std::string getName();
        int getNum();
        void addNum();
    };
    CompilingFunction::CompilingFunction(std::string _functionName, int _parameterNum) {
        functionName = std::move(_functionName);
        parameterNum = _parameterNum;
    }
    std::string CompilingFunction::getName() {
        return functionName;
    }
    int CompilingFunction::getNum() {
        return parameterNum;
    }
    void CompilingFunction::addNum() {
        parameterNum++;
    }

}

#endif //CC0_COMPILINGFUNCTION_H
