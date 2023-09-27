#pragma once
#include <string>

namespace Tolo
{
	struct Token
	{
		enum class Type
		{
			StartPar,
			EndPar,
			StartCurly,
			EndCurly,
			Comma,
			Dot,
			Plus,
			Minus,
			Asterisk,
			ForwardSlash,
			LeftArrow,
			RightArrow,
			EqualSign,
			Name,
			ConstChar,
			ConstInt,
			ConstFloat
		};

		Type type;
		std::string text;
		int line;
	};
}