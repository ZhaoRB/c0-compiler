//
// Created by Decade on 2019/12/24 0024.
//
#include "instruction.h"

#include <string>

namespace miniplc0 {

    Operation Instruction::getOpr() {
        return opr;
    }

    std::vector<std::string> Instruction::getBinaryOpr() {
        return binary_opr;
    }

    std::vector<int> Instruction::getOperand() {
        return operand;
    }

    std::vector<std::string> Instruction::getBinaryOperand() {
        return binary_operand;
    }

}
