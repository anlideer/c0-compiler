#pragma once

#include <cstdint>
#include <utility>

namespace miniplc0 {

	enum Operation {
		ILL = 0,

		// .???
		CONSTANTS, // .constants
		CONSTANT, // {index} {type} {content}
		START,	// .start
		FUNCTIONS, // .functions
		FUNCN, // .Fn
		FUNCINFO, // function information: {index} {name_index} {params_size} {level}

		// memory operations (now we only have a/i without char&double). For convenience, we only use int.
		LOADA, 
		LOADC,
		IPUSH,
		ILOAD,
		ISTORE,
		//IALOAD,
		//IASTORE,
		NEW,
		SNEW,
		POP,
		DUP,

		// calculation operations
		IADD,
		ISUB,
		IMUL,
		IDIV,
		INEG,
		ICMP,

		// type cast oprations
		// we now don't have this part.

		// condition control operations
		// jump with condition
		JE,
		JNE,
		JL,
		JGE,
		JG,
		JLE,
		JMP, // jump without any condition
		CALL,
		RET,
		IRET,

		// helper
		IPRINT,
		CPRINT,
		ISCAN,
		// SPRINT,
		// PRINTL,

	};
	
	class Instruction final {
	private:
		using int32_t = std::int32_t;
		using string = std::string;
	public:
		friend void swap(Instruction& lhs, Instruction& rhs);
	public:
		Instruction(Operation opr, int32_t index, int32_t x, int32_t name_index, int32_t params_size, int32_t level) : _opr(opr), _index(index), _x(x), _name_index(name_index), _params_size(params_size), _level(level){}
		Instruction(Operation opr, int32_t index, int32_t x): _opr(opr), _index(index), _x(x){}
		Instruction(Operation opr, int32_t index): _opr(opr), _index(index){}
		Instruction(Operation opr, int32_t index, string str): _opr(opr), _index(index), _str(str){}
		Instruction(Operation opr): _opr(opr){}

		
		Instruction() : Instruction(Operation::ILL, 0){}
		Instruction(const Instruction& i) { _opr = i._opr; _x = i._x;  _index = i._index;
											_name_index = i._name_index; _params_size = i._params_size; _level = i._level; _str = i._str;}
		Instruction(Instruction&& i) :Instruction() { swap(*this, i); }
		Instruction& operator=(Instruction i) { swap(*this, i); return *this; }
		bool operator==(const Instruction& i) const { return _opr == i._opr && _x == i._x && _index == i._index && _name_index == i._name_index && 
													_params_size == i._params_size && _level == i._level && _str == i._str; }

		Operation GetOperation() const { return _opr; }
		int32_t GetX() const { return _x; }
		int32_t GetIndex() const {return _index;}
		int32_t GetNameIndex() const {return _name_index;}
		int32_t GetParamsSize() const {return _params_size;}
		int32_t GetLevel() const {return _level;}
		string GetStr() const {return _str;}
	private:
		// set default value
		Operation _opr = Operation::ILL;
		int32_t _x = 0;
		// append some attrs
		int32_t _index = 0;
		// for function difinition only
		int32_t _name_index = 0;
		int32_t _params_size = 0;
		int32_t _level = 0;
		string _str = "";

	};

	inline void swap(Instruction& lhs, Instruction& rhs) {
		using std::swap;
		swap(lhs._opr, rhs._opr);
		swap(lhs._x, rhs._x);
		swap(lhs._index, rhs._index);
		swap(lhs._name_index, rhs._name_index);
		swap(lhs._params_size, rhs._params_size);
		swap(lhs._level, rhs._level);
	}
}