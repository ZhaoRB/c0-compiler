#pragma once

#include <cstdint>
#include <utility>

namespace miniplc0 {

	enum Operation {
		ILL = 0,
		LIT,        //stack[sp]=x;sp++;                     // 变量和常量声明的时候，压栈
		LOD,        //stack[sp]=stack[x];sp++;              // 计算的时候，将变量或者常量的值放到栈顶
		STO,        //stack[x]=stack[sp-1];sp--;            // 变量赋值
		ADD,        //stack[sp-2]+=stack[sp-1];sp--;        // 加
		SUB,        //stack[sp-2]-=stack[sp-1];sp--;        // 减
		MUL,        //stack[sp-2]*=stack[sp-1];sp--;        // 乘
		DIV,        //stack[sp-2]/=stack[sp-1];sp--;        // 除
		WRT         //printf("%d\n", stack[sp-1]);sp--;     // print
	};
	
	class Instruction final {
	private:
		using int32_t = std::int32_t;
	public:
		friend void swap(Instruction& lhs, Instruction& rhs);
	public:
		Instruction(Operation opr, int32_t x) : _opr(opr), _x(x) {}
		
		Instruction() : Instruction(Operation::ILL, 0){}
		Instruction(const Instruction& i) { _opr = i._opr; _x = i._x; }
		Instruction(Instruction&& i) :Instruction() { swap(*this, i); }
		Instruction& operator=(Instruction i) { swap(*this, i); return *this; }
		bool operator==(const Instruction& i) const { return _opr == i._opr && _x == i._x; }

		Operation GetOperation() const { return _opr; }
		int32_t GetX() const { return _x; }
	private:
		Operation _opr;
		int32_t _x;
	};

	inline void swap(Instruction& lhs, Instruction& rhs) {
		using std::swap;
		swap(lhs._opr, rhs._opr);
		swap(lhs._x, rhs._x);
	}
}