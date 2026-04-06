#pragma once

/*
 * The hard part :( 
 */

#include <string>
#include <vector>
#include <cstddef>

// Lexer part
#include "lexer.hpp"

namespace Parser {

enum class Context {
	WINDOW,
	BUTTON,
	CHECKBOX,
	TEXTBOX,
	SLIDER
};

constexpr std::string_view TokenToString(const Lexer::TokenType& t) {
	switch (t) {
		case Lexer::TokenType::IDENTIFIER: return "Identifier";
		case Lexer::TokenType::INT_LITERAL: return "Integer";
		case Lexer::TokenType::STRING_LITERAL: return "String";
		case Lexer::TokenType::LEFT_BRACE: return "Left Curly Brace";
		case Lexer::TokenType::RIGHT_BRACE: return "Right Curly Brace";
		case Lexer::TokenType::COLON: return "Colon";
		case Lexer::TokenType::SEMICOLON: return "Semicolon";
		case Lexer::TokenType::COMMA: return "Comma";
		case Lexer::TokenType::DOT: return "Dot";
		case Lexer::TokenType::EndOfFile: return "EOF";
		case Lexer::TokenType::WTF: return "Huh?";
		case Lexer::TokenType::BOOL_LITERAL: return "Bool";
		default: return "Error somewhere dawg";
	}
}

class Parser {
private:
	std::vector<Lexer::Token> lexer_tokens;
	std::size_t pos = 0;
	Context currentContext;

	Lexer::Token curr() const { return lexer_tokens[pos]; }
	Lexer::Token next() { return lexer_tokens[pos]; }

	Lexer::Token except(Lexer::TokenType excepted) {
		if (curr().type != excepted) {
			throw std::runtime_error(std::format("Unexcepted token on line {}, excepted {}  got {} {}.",
					curr().line, TokenToString(excepted), TokenToString(curr().type), curr().lexeme
			));	
		}		

		return curr();
		
	}

	void parseDataBlock() {
		while (lexer_tokens[pos].type != Lexer::TokenType::RIGHT_BRACE) {	
			auto key = except(Lexer::TokenType::IDENTIFIER); pos++;
			except(Lexer::TokenType::COLON); pos++;
			if (key.lexeme == "dimensions" or key.lexeme == "position") {
				except(Lexer::TokenType::INT_LITERAL); pos++;
				except(Lexer::TokenType::COMMA); pos++;
				except(Lexer::TokenType::INT_LITERAL); pos++;
				except(Lexer::TokenType::SEMICOLON); pos++;
			} else if (key.lexeme == "title" && currentContext == Context::WINDOW) {
				except(Lexer::TokenType::STRING_LITERAL); pos++;
				except(Lexer::TokenType::SEMICOLON); pos++;

			}
		}		
	}


public:
	explicit Parser(std::vector<Lexer::Token> tok) : lexer_tokens(tok) {};

	bool mainParserLoop() {
		while (lexer_tokens[pos].type != Lexer::TokenType::EndOfFile) {
			if (curr().type == Lexer::TokenType::RIGHT_BRACE) break; // End of file
			except(Lexer::TokenType::IDENTIFIER); pos++;
			except(Lexer::TokenType::LEFT_BRACE); pos++;
			except(Lexer::TokenType::DOT); pos++;

			if (lexer_tokens[pos].lexeme == "data") {
				pos++;
				except(Lexer::TokenType::LEFT_BRACE); pos++;
				parseDataBlock();
				except(Lexer::TokenType::RIGHT_BRACE); pos++;
			}
		}	

		return true;
	}
	
};
} // namespace Compiler
