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
        std::string returnType;
    public:
        CompilingFunction(std::string _functionName, int _parameterNum, std::string _returnType);
        std::string getName();
        int getNum();
        void addNum();
        std::string getType();
    };
    CompilingFunction::CompilingFunction(std::string _functionName, int _parameterNum, std::string _returnType) {
        functionName = std::move(_functionName);
        parameterNum = _parameterNum;
        returnType = std::move(_returnType);
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

    std::string CompilingFunction::getType() {
        return returnType;
    }

}

#endif //CC0_COMPILINGFUNCTION_H
