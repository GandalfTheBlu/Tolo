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
			Colon,
			DoubleColon,
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
			DoubleLeftArrow,
			DoubleRightArrow,
			Tilde,
			Caret,
			Ampersand,
			VerticalBar,
			DoubleAmpersand,
			DoubleVerticalBar,
			Name,
			ConstChar,
			ConstInt,
			ConstFloat,
			ConstString
		};

		Type type;
		std::string text;
		int line;
	};
}