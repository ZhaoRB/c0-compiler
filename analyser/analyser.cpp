#include "analyser.h"

#include <climits>
#include <sstream>

namespace miniplc0 {

    std::pair<std::vector<Instruction>, std::optional<CompilationError>> Analyser::Analyse() {
		auto err = analyseProgram();
		if (err.has_value())
			return std::make_pair(std::vector<Instruction>(), err);
		else
			return std::make_pair(_instructions, std::optional<CompilationError>());
	}

	// 所有的递归子程序
	std::optional<CompilationError> analyseProgram() {

    }

	std::optional<Token> Analyser::nextToken() {
		if (_offset == _tokens.size())
			return {};
		// 考虑到 _tokens[0..._offset-1] 已经被分析过了
		// 所以我们选择 _tokens[0..._offset-1] 的 EndPos 作为当前位置
		_current_pos = _tokens[_offset].GetEndPos();
		return _tokens[_offset++];
	}

	void Analyser::unreadToken() {
		if (_offset == 0)
			DieAndPrint("analyser unreads token from the begining.");
		_current_pos = _tokens[_offset - 1].GetEndPos();
		_offset--;
	}

//	void Analyser::_add(const Token& tk, std::map<std::string, int32_t>& mp) {
//		if (tk.GetType() != TokenType::IDENTIFIER)
//			DieAndPrint("only identifier can be added to the table.");
//		mp[tk.GetValueString()] = _nextTokenIndex;
//		_nextTokenIndex++;
//	}


}