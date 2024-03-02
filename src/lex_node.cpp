#include "lex_node.h"

namespace Tolo
{
	LexNode::LexNode(Type _type, const Token& _token) :
		type(_type),
		token(_token)
	{}
}