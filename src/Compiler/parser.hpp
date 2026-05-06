#pragma once

/*
 * The hard part :( 
 */

#include <string>
#include <vector>
#include <cstddef>
#include <format>
#include <stdexcept>

// Lexer part
#include "lexer.hpp"

namespace Parser {

enum class Context {
	WINDOW,
	BUTTON,
	CHECKBOX,
	TEXTBOX,
	SLIDER,
	NONE
};

constexpr std::string_view TokenToString(const Lexer::TokenType& t) {
	switch (t) {
		case Lexer::TokenType::IDENTIFIER:    return "Identifier";
		case Lexer::TokenType::INT_LITERAL:   return "Integer";
		case Lexer::TokenType::STRING_LITERAL:return "String";
		case Lexer::TokenType::BOOL_LITERAL:  return "Bool";
		case Lexer::TokenType::LEFT_BRACE:    return "Left Curly Brace '{'";
		case Lexer::TokenType::RIGHT_BRACE:   return "Right Curly Brace '}'";
		case Lexer::TokenType::COLON:         return "Colon ':'";
		case Lexer::TokenType::SEMICOLON:     return "Semicolon ';'";
		case Lexer::TokenType::COMMA:         return "Comma ','";
		case Lexer::TokenType::DOT:           return "Dot '.'";
		case Lexer::TokenType::EndOfFile:     return "EOF";
		case Lexer::TokenType::WTF:           return "Unknown character";
		default:                              return "Error somewhere dawg";
	}
}

class Parser {
private:
	std::vector<Lexer::Token> lexer_tokens;
	std::size_t pos = 0;
	Context currentContext = Context::NONE;

	// ── Helpers ──────────────────────────────────────────────────────────

	// Are we at or past the end of the token stream?
	bool atEnd() const {
		return pos >= lexer_tokens.size()
			|| lexer_tokens[pos].type == Lexer::TokenType::EndOfFile;
	}

	// Current token (does not advance)
	Lexer::Token curr() const {
		return lexer_tokens[pos];
	}

	// Look one token ahead without advancing
	Lexer::Token peek() const {
		if (pos + 1 < lexer_tokens.size())
			return lexer_tokens[pos + 1];
		return lexer_tokens.back(); // EOF token
	}

	// Advance and return the token we just left
	Lexer::Token advance() {
		Lexer::Token t = lexer_tokens[pos];
		pos++;
		return t;
	}

	// Assert the current token is of the expected type, throw if not.
	// Does NOT advance — call advance() separately when you want to consume.
	Lexer::Token expect(Lexer::TokenType expected) {
		if (curr().type != expected) {
			throw std::runtime_error(
				std::format("Parse error on line {}: expected {} but got {} ('{}').",
					curr().line,
					TokenToString(expected),
					TokenToString(curr().type),
					curr().lexeme)
			);
		}
		return curr();
	}

	// Shorthand: assert + consume in one call
	Lexer::Token consume(Lexer::TokenType expected) {
		expect(expected);
		return advance();
	}

	// ── Context helpers ──────────────────────────────────────────────────

	Context identifierToContext(const std::string& name) {
		if (name == "Window")   return Context::WINDOW;
		if (name == "Button")   return Context::BUTTON;
		if (name == "TextBox")  return Context::TEXTBOX;
		if (name == "CheckBox") return Context::CHECKBOX;
		if (name == "Slider")   return Context::SLIDER;
		return Context::NONE;
	}

	// ── Property parsers ─────────────────────────────────────────────────

	// dimensions: INT, INT;
	// position:   INT, INT;
	void parsePairProperty() {
		consume(Lexer::TokenType::INT_LITERAL);
		consume(Lexer::TokenType::COMMA);
		consume(Lexer::TokenType::INT_LITERAL);
		consume(Lexer::TokenType::SEMICOLON);
	}

	// title: "string";
	// text:  "string";
	void parseStringProperty() {
		consume(Lexer::TokenType::STRING_LITERAL);
		consume(Lexer::TokenType::SEMICOLON);
	}

	// editable: true; / editable: false;
	void parseBoolProperty() {
		consume(Lexer::TokenType::BOOL_LITERAL);
		consume(Lexer::TokenType::SEMICOLON);
	}

	// range: INT, INT;
	void parseRangeProperty() {
		consume(Lexer::TokenType::INT_LITERAL);
		consume(Lexer::TokenType::COMMA);
		consume(Lexer::TokenType::INT_LITERAL);
		consume(Lexer::TokenType::SEMICOLON);
	}

	// ── Block parsers ─────────────────────────────────────────────────────

	// Parses everything between the '{' and '}' of a .data block.
	// The opening '{' must already be consumed before calling this.
	void parseDataBlock() {
		while (!atEnd() && curr().type != Lexer::TokenType::RIGHT_BRACE) {

			// Unexpected token inside .data
			if (curr().type == Lexer::TokenType::WTF) {
				throw std::runtime_error(
					std::format("Parse error on line {}: unknown character '{}' inside .data block.",
						curr().line, curr().lexeme)
				);
			}

			auto key = consume(Lexer::TokenType::IDENTIFIER);
			consume(Lexer::TokenType::COLON);

			// ── pair properties (valid for all components) ──
			if (key.lexeme == "dimensions" || key.lexeme == "position") {
				parsePairProperty();
			}

			// ── string properties ──
			else if (key.lexeme == "title") {
				if (currentContext != Context::WINDOW) {
					throw std::runtime_error(
						std::format("Parse error on line {}: 'title' is only valid inside a Window.", key.line)
					);
				}
				parseStringProperty();
			}
			else if (key.lexeme == "text") {
				if (currentContext != Context::BUTTON && currentContext != Context::TEXTBOX) {
					throw std::runtime_error(
						std::format("Parse error on line {}: 'text' is only valid inside a Button or TextBox.", key.line)
					);
				}
				parseStringProperty();
			}

			// ── bool properties ──
			else if (key.lexeme == "editable") {
				if (currentContext != Context::TEXTBOX) {
					throw std::runtime_error(
						std::format("Parse error on line {}: 'editable' is only valid inside a TextBox.", key.line)
					);
				}
				parseBoolProperty();
			}

			// ── range property (Slider only) ──
			else if (key.lexeme == "range") {
				if (currentContext != Context::SLIDER) {
					throw std::runtime_error(
						std::format("Parse error on line {}: 'range' is only valid inside a Slider.", key.line)
					);
				}
				parseRangeProperty();
			}

			// ── unknown property key ──
			else {
				throw std::runtime_error(
					std::format("Parse error on line {}: unknown property '{}' in {} block.",
						key.line, key.lexeme,
						key.lexeme /* context name would be nice here too */)
				);
			}
		}

		// Consume the closing '}'
		consume(Lexer::TokenType::RIGHT_BRACE);
	}

	// Parses a .section { ... } block.
	// Expects: DOT IDENTIFIER(sectionName) LEFT_BRACE ...
	void parseSectionBlock() {
		consume(Lexer::TokenType::DOT);
		auto sectionName = consume(Lexer::TokenType::IDENTIFIER);
		consume(Lexer::TokenType::LEFT_BRACE);

		if (sectionName.lexeme == "data") {
			parseDataBlock();                 // consumed its own '}'
		}
		else if (sectionName.lexeme == "content") {
			parseContentBlock();              // consumed its own '}'
		}
		else {
			throw std::runtime_error(
				std::format("Parse error on line {}: unknown section '.{}'. Expected '.data' or '.content'.",
					sectionName.line, sectionName.lexeme)
			);
		}
	}

	// Parses everything inside a .content { ... } block.
	// The opening '{' is already consumed before we get here.
	void parseContentBlock() {
		while (!atEnd() && curr().type != Lexer::TokenType::RIGHT_BRACE) {

			if (curr().type == Lexer::TokenType::WTF) {
				throw std::runtime_error(
					std::format("Parse error on line {}: unknown character '{}' inside .content block.",
						curr().line, curr().lexeme)
				);
			}

			// Each child inside .content is a full component declaration
			parseComponent();
		}

		// Consume the closing '}'
		consume(Lexer::TokenType::RIGHT_BRACE);
	}

	// Parses a single component:
	//   ComponentName { .section { ... } ... }
	void parseComponent() {
		// Save the caller's context so nested components don't corrupt it
		Context savedContext = currentContext;

		// ComponentName
		auto nameToken = consume(Lexer::TokenType::IDENTIFIER);
		Context newCtx = identifierToContext(nameToken.lexeme);

		if (newCtx == Context::NONE) {
			throw std::runtime_error(
				std::format("Parse error on line {}: '{}' is not a known component type. "
					"Expected Window, Button, TextBox, CheckBox, or Slider.",
					nameToken.line, nameToken.lexeme)
			);
		}

		currentContext = newCtx;

		// Opening '{'
		consume(Lexer::TokenType::LEFT_BRACE);

		// One or more sections (.data, .content)
		bool hasData = false;
		while (!atEnd() && curr().type != Lexer::TokenType::RIGHT_BRACE) {

			if (curr().type != Lexer::TokenType::DOT) {
				throw std::runtime_error(
					std::format("Parse error on line {}: expected a section (e.g. '.data') inside '{}', "
						"but got {} ('{}').",
						curr().line, nameToken.lexeme,
						TokenToString(curr().type), curr().lexeme)
				);
			}

			// Peek at the section name to give a better error if .content
			// appears on a non-Window component
			if (pos + 1 < lexer_tokens.size()
				&& lexer_tokens[pos + 1].lexeme == "content"
				&& currentContext != Context::WINDOW)
			{
				throw std::runtime_error(
					std::format("Parse error on line {}: '.content' is only valid inside a Window.",
						lexer_tokens[pos + 1].line)
				);
			}

			if (lexer_tokens[pos + 1].lexeme == "data") {
				hasData = true;
			}

			parseSectionBlock();
		}

		if (!hasData) {
			throw std::runtime_error(
				std::format("Parse error: component '{}' on line {} has no '.data' section.",
					nameToken.lexeme, nameToken.line)
			);
		}

		// Closing '}'
		consume(Lexer::TokenType::RIGHT_BRACE);

		// Restore parent context
		currentContext = savedContext;
	}

public:
	explicit Parser(std::vector<Lexer::Token> tok) : lexer_tokens(std::move(tok)) {}

	// Entry point. Returns true on success, throws std::runtime_error on any
	// parse error with a message that includes the line number.
	bool mainParserLoop() {
		while (!atEnd()) {
			// Skip stray RIGHT_BRACEs at top level (shouldn't happen in valid
			// code, but gives a cleaner error than an infinite loop)
			if (curr().type == Lexer::TokenType::RIGHT_BRACE) {
				throw std::runtime_error(
					std::format("Parse error on line {}: unexpected '}}' at top level.",
						curr().line)
				);
			}

			parseComponent();
		}

		return true;
	}
};

} // namespace Parser
