#pragma once

#include <cstdint>
#include <utility>
#include <vector>
#include <string>

namespace miniplc0 {

	enum Operation {
        NOP = 0X00,
        BIPUSH = 0X01,
        IPUSH = 0X02,
        POP = 0X04,
        POP2 = 0X05,
        POPN = 0X06,
        // 复制栈顶的元素并入栈
        DUP = 0X07,
        DUP2 = 0X08,
        // 加载常量池的value 格式：loadc index
        LOADC = 0X09,
        // 格式 loada level_diff offset 加载相应位置的栈地址address
        LOADA = 0X0A,
        // 在 堆 / 栈 上分配空间
        NEW = 0X0B,
        SNEW = 0X0C,
        // 从栈顶的地址取值 经常与loada共同使用
        ILOAD = 0X10,
        DLOAD = 0X11,
        ALOAD = 0X12,
        // 从栈顶的地址为首地址 加载数组下标为index的值 相当于value = address[index]
        IALOAD = 0X18,
        DALOAD = 0X19,
        AALOAD = 0X1A,
        // 将栈顶的value存到sp-1位置的地址处
        ISTORE = 0X20,
        DSTORE = 0X21,
        ASTORE = 0X22,
        // 将栈顶的value存到sp-1地址为首地址的index处 address[index] = value
        IASTORE = 0X28,
        DASTORE = 0X29,
        AASTROE = 0X2A,
        // 算数运算
        // + - * /
        IADD = 0X30,
        DADD = 0X31,
        ISUB = 0X34,
        DSUB = 0X35,
        IMUL = 0X38,
        DMUL = 0X39,
        IDIV = 0X3C,
        DDIV = 0X3D,
        // 取负
        INEG = 0X40,
        DNEG = 0X41,
        // 比较 栈顶rhs 次栈顶lhs
        // lhs=rhs为0；lhs>rhs为1;rhs>lhs为-1
        ICMP = 0X44,
        DCMP = 0X45,
        // 类型转换指令
        I2D = 0X60,
        D2I = 0X61,
        I2C = 0X62,
        // 控制转移指令
        JMP = 0X70,
        JE = 0X71,
        JNE = 0X72,
        JL = 0X73,
        JGE = 0X74,
        JG = 0X75,
        JLE = 0X76,
        CALL = 0X80,
        RET = 0X88,
        IRET = 0X89,
        DRET = 0X8A,
        ARET = 0X8B,
        // print scan
        IPRINT = 0XA0,
        DPRINT = 0XA1,
        CPRINT = 0XA2,
        SPRINT = 0XA3,
        PRINTL = 0XAF,
        ISCAN = 0XB0,
        DSCAN = 0XB1,
        CSCAN = 0XB2

	};

	// 1.汇编指令 2.二进制指令 3.操作数 4.操作数的二进制形式
	class Instruction final {

    private:
	    Operation opr;
	    std::string binary_opr;
	    std::vector<int> operand;
	    std::vector<std::string> binary_operand;
    public:
	    Instruction(Operation _opr, std::string  _binary_opr, std::vector<int> _operand, std::vector<std::string> _binary_operand)
	        :opr(_opr), binary_opr(std::move(_binary_opr)), operand(std::move(_operand)), binary_operand(std::move(_binary_operand)) {}

    public:
	    Operation getOpr();
	    std::string getBinaryOpr();
	    std::vector<int> getOperand();
	    std::vector<std::string> getBinaryOperand();

	};

}