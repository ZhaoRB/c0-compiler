//
// Created by Decade on 2019/12/22 0022.
//
#include "compilingFunction.h"
#include <string>

namespace miniplc0 {
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

    int CompilingFunction::getIndex() {
        return index;
    }
}
