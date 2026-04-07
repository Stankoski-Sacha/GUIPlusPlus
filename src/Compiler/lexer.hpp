#pragma once

#include <cctype>
#include <fstream>
#include <vector>
#include <cstddef>

// Types of tokens
namespace Lexer {
enum class TokenType {
  // Literals
  IDENTIFIER,
  INT_LITERAL,
  STRING_LITERAL,
  BOOL_LITERAL,

  // Delimeters
  LEFT_BRACE,
  RIGHT_BRACE,
  COLON,
  SEMICOLON,
  COMMA,

  // Before INDENTIFIER
  DOT,

  // Special
  EndOfFile,
  WTF,
};

struct Token {
  TokenType type;
  std::string lexeme;
  int line;
};

class Lexer {
private:
public:
  std::vector<Token> transform_to_tokens(const std::string& file_loc) {
    std::fstream file(file_loc);
    std::string word;
    std::string parsed{};
    int current_line = 1;

    // Get the text from the file into a std::string
    while (std::getline(file, word)) {
      parsed += word + '\n';
    }

    std::vector<Token> tokens{};
    std::size_t i = 0;

    while (i < parsed.length()) {
      char current = parsed[i];
      	
      if (current == '\n') {
	      current_line++;
	      i++;
	      continue;
      }

      if (std::isspace(current)) {
        i++;
        continue;
      }

      if (std::isalpha(current)) {
        std::size_t start = i;
        while (i < parsed.length() && std::isalnum(parsed[i])) {
          i++;
        }

        std::string identifier = parsed.substr(start, i - start);

	if (identifier == "true" or identifier == "false") {
		tokens.emplace_back(Token{TokenType::BOOL_LITERAL, identifier, current_line});
	} else {
		tokens.emplace_back(Token{TokenType::IDENTIFIER, identifier, current_line});
	}
        continue;
      }

      if (std::isdigit(current)) {
        std::size_t start = i;

        while (i < parsed.length() && std::isdigit(parsed[i])) {
          i++;
        }
        std::string num = parsed.substr(start, i - start);
        tokens.emplace_back(Token{TokenType::INT_LITERAL, num, current_line});
        continue;
      }

      if (current == '"') {
        std::size_t start = i + 1; // To have the text and not the "
        i++;

        while (i < parsed.length() && parsed[i] != '"') {
          i++;
        }

        std::string strLiteral = parsed.substr(start, i - start);
        tokens.emplace_back(Token{TokenType::STRING_LITERAL, strLiteral, current_line});
	i++; // The reason why for this i++ is to take the text and skip the second " otherwise the lexer takes everything after as a string
        continue;
      }

      if (current == '{') {
        tokens.emplace_back(Token{TokenType::LEFT_BRACE, "{", current_line});
        i++;
        continue;
      }

      if (current == '.') {
        tokens.emplace_back(Token{TokenType::DOT, ".", current_line});
        i++;
        continue;
      }

      if (current == '}') {
        tokens.emplace_back(Token{TokenType::RIGHT_BRACE, "}", current_line});
        i++;
        continue;
      }

      if (current == ':') {
        tokens.emplace_back(Token{TokenType::COLON, ":", current_line});
        i++;
        continue;
      }

      if (current == ',') {
        tokens.emplace_back(Token{TokenType::COMMA, ",", current_line});
        i++;
        continue;
      }

      if (current == ';') {
        tokens.emplace_back(Token{TokenType::SEMICOLON, ";", current_line});
        i++;
        continue;
      }


      // End
      else {
        tokens.emplace_back(Token{TokenType::WTF, "?", current_line});
        i++;
        continue;
      }
    }

    return tokens;
  }
};
} // namespace Compiler
