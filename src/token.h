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
			Semicolon,
			Comma,
			Dot,
			Plus,
			Minus,
			Asterisk,
			ForwardSlash,
			LeftArrow,
			RightArrow,
			EqualSign,
			ExclamationMark,
			LeftArrowEqualSign,
			RightArrowEqualSign,
			DoubleEqualSign,
			ExclamationMarkEqualSign,
			DoubleAmpersand,
			DoubleVerticalBar,
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