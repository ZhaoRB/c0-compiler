//
// Created by Decade on 2019/12/24 0024.
//
#include "instruction.h"

#include <string>
// #include <rpcndr.h>

namespace miniplc0 {

    using byte = unsigned char;

    Operation Instruction::getOpr() {
        return opr;
    }

    std::vector<byte> Instruction::getBinaryOpr() {
        return binary_opr;
    }

    std::vector<int> Instruction::getOperand() {
        return operand;
    }

    std::vector<std::vector<byte>> Instruction::getBinaryOperand() {
        return binary_operand;
    }

    int Instruction::getOffsetNum() {
        return offset_num;
    }

    void Instruction::addOperand(int offset) {
        operand.push_back(offset);
    }

    void Instruction::addBinaryOperand(std::vector<byte> binary_offset) {
        binary_operand.push_back(binary_offset);
    }

    void Instruction::setOffsetNum(int n) {
        offset_num = n;
    }

}
