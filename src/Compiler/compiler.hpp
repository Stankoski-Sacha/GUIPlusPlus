#pragma once

#include <SDL2/SDL.h>
#include <utility>
#include <string>
#include <vector>

// Includes 
#include "lexer.hpp"

namespace Compiler {

enum class Context {
	WINDOW,
	TEXTBOX,
	BUTTON,
	CHECKBOX,
	SLIDER
};

struct WindowNode {
	std::string title = "Window"; // Default title to be changed by the user 
	int x,y,w,h; // Must indicate all of these during window creation 
	Uint32 flag = SDL_WINDOW_SHOWN; // Default flags maybe a TODO to make so that the user can choose custom flags 

	WindowNode() = default;
};

struct Button {
	std::string buttonText = " "; // Default text to 'button'
	int x, y, w, h; // Mandatory to put
	
	Button() = default;
};

struct TextBox {
	std::string defaultText = " "; // Default text to a blank space ' '
	int x, y, w, h; // Mandatory to put 
	bool isEditable; // Also mandatory 
	
	TextBox() = default;
};

struct CheckBox {
	int x, y, w, h;
	CheckBox() = default;
};

struct Slider {
	int x, y, w, h, minVal, maxVal;
	Slider() = default;
};


// All componants inside of a struct to make it easier to move around
struct ComponentNode {
	WindowNode win;
	std::vector<Button> buttonsCreated;
	std::vector<TextBox> TextBoxCreated;
	std::vector<CheckBox> CheckBoxCreated;
	std::vector<Slider> sliderCreated;
};


class compiler {
private:
	std::vector<Lexer::Token> toks;

	std::pair<int, int> returnVals(const std::vector<Lexer::Token>& toks) {
		std::pair<int, int> pair;

		int t1 = -1 ,t2 = -1;

		for (const auto& tokens: toks) {
			if (tokens.type == Lexer::TokenType::INT_LITERAL && t1 == -1) {
				t1 = std::stoi(tokens.lexeme);
			} else if (tokens.type == Lexer::TokenType::INT_LITERAL && t1 != -1) {
				t2 = std::stoi(tokens.lexeme);
			}
		}

		pair = std::make_pair(t1,t2);

		return pair;

	}

	WindowNode makeWindow(const std::vector<Lexer::Token>& toks) {
		using namespace Lexer;
		WindowNode node;

		std::pair<int, int> dimensionsPair;
		std::pair<int, int> positionPair;
		std::string title;
		for (auto i{0}; i < toks.size(); i++) {
			if (toks[i].type == TokenType::IDENTIFIER && toks[i].lexeme == "dimensions") {
				dimensionsPair = returnVals(std::vector<Token>{
					toks.begin() + i, std::find_if(
					toks.begin() + i, toks.end(), 
					[](const Token& tok) { 
							return tok.type == TokenType::SEMICOLON; 
						}
				)});	

				do {
				       i++;
				} while (toks[i].type != TokenType::SEMICOLON);	       
			}
			else if (toks[i].type == TokenType::IDENTIFIER && toks[i].lexeme == "position") {
				positionPair = returnVals(std::vector<Token>{
					toks.begin() + i, std::find_if(
					toks.begin() + i, toks.end(), 
					[](const Token& tok) { 
							return tok.type == TokenType::SEMICOLON; 
							}
				)});	

				do {
				       i++;
				} while (toks[i].type != TokenType::SEMICOLON);	       
			}
			else if (toks[i].type == TokenType::IDENTIFIER && toks[i].lexeme == "title") {
				do {
					i++;
				} while (toks[i].type != TokenType::STRING_LITERAL);

				title = toks[i].lexeme;

				do {
					i++;
				} while (toks[i].type != TokenType::SEMICOLON);	
			}
		}

		node.w = dimensionsPair.first;
		node.h = dimensionsPair.second;
		node.x = positionPair.first;
		node.y = positionPair.second;
		node.title = title;

		return node;
	}

	TextBox makeTextBox(const std::vector<Lexer::Token>& toks) {
		using namespace Lexer;
		TextBox node;

		std::pair<int, int> dimensionsPair;
		std::pair<int, int> positionPair;
		std::string title;
		bool isEdit;
		for (auto i{0}; i < toks.size(); i++) {
			if (toks[i].type == TokenType::IDENTIFIER && toks[i].lexeme == "dimensions") {
				dimensionsPair = returnVals(std::vector<Token>{
					toks.begin() + i, std::find_if(
					toks.begin() + i, toks.end(), 
					[](const Token& tok) { 
							return tok.type == TokenType::SEMICOLON; 
						}
				)});

				do {
				       i++;
				} while (toks[i].type != TokenType::SEMICOLON);	       
			}
			else if (toks[i].type == TokenType::IDENTIFIER && toks[i].lexeme == "position") {
				positionPair = returnVals(std::vector<Token>{
					toks.begin() + i, std::find_if(
					toks.begin() + i, toks.end(), 
					[](const Token& tok) { 
							return tok.type == TokenType::SEMICOLON; 
						}
				)});	

				do {
				       i++;
				} while (toks[i].type != TokenType::SEMICOLON);	       
			}
			else if (toks[i].type == TokenType::IDENTIFIER && toks[i].lexeme == "text") {
				do {
					i++;
				} while (toks[i].type != TokenType::STRING_LITERAL);

				title = toks[i].lexeme;

				do {
					i++;
				} while (toks[i].type != TokenType::SEMICOLON);	
			}
			else if (toks[i].type == TokenType::BOOL_LITERAL) {
				isEdit = (toks[i].lexeme == "true") ? 1 : 0;
			}
		}

		node.w = dimensionsPair.first;
		node.h = dimensionsPair.second;
		node.x = positionPair.first;
		node.y = positionPair.second;
		node.defaultText = title;
		node.isEditable = isEdit;

		return node;
	}

	Button makeButton(const std::vector<Lexer::Token>& toks) {
		using namespace Lexer;
		Button node;

		std::pair<int, int> dimensionsPair;
		std::pair<int, int> positionPair;
		std::string title;
		for (auto i{0}; i < toks.size(); i++) {
			if (toks[i].type == TokenType::IDENTIFIER && toks[i].lexeme == "dimensions") {
				dimensionsPair = returnVals(std::vector<Token>{
					toks.begin() + i, std::find_if(
					toks.begin() + i, toks.end(), 
					[](const Token& tok) { 
							return tok.type == TokenType::SEMICOLON; 
						}
				)});	
					
				do {
				       i++;
				} while (toks[i].type != TokenType::SEMICOLON);	       
			}
			else if (toks[i].type == TokenType::IDENTIFIER && toks[i].lexeme == "position") {
				positionPair = returnVals(std::vector<Token>{
					toks.begin() + i, std::find_if(
					toks.begin() + i, toks.end(), 
					[](const Token& tok) {
							return tok.type == TokenType::SEMICOLON; 
						}
				)});
					
				do {
				       i++;
				} while (toks[i].type != TokenType::SEMICOLON);	       
			}
			else if (toks[i].type == TokenType::IDENTIFIER && toks[i].lexeme == "text") {
				do {
					i++;
				} while (toks[i].type != TokenType::STRING_LITERAL);

				title = toks[i].lexeme;

				do {
					i++;
				} while (toks[i].type != TokenType::SEMICOLON);	
			}
		}

		node.w = dimensionsPair.first;
		node.h = dimensionsPair.second;
		node.x = positionPair.first;
		node.y = positionPair.second;
		node.buttonText = title;

		return node;

	}

	CheckBox makeCheckBox(const std::vector<Lexer::Token>& toks) {
		using namespace Lexer;
		CheckBox node;

		std::pair<int, int> dimensionsPair;
		std::pair<int, int> positionPair;
		for (auto i{0}; i < toks.size(); i++) {
			if (toks[i].type == TokenType::IDENTIFIER && toks[i].lexeme == "dimensions") {
				dimensionsPair = returnVals(std::vector<Token>{
					toks.begin() + i, std::find_if(
					toks.begin() + i, toks.end(), 
					[](const Token& tok) { 
							return tok.type == TokenType::SEMICOLON; 
						}
				)});
		
				do {
				       i++;
				} while (toks[i].type != TokenType::SEMICOLON);	       
			}
			else if (toks[i].type == TokenType::IDENTIFIER && toks[i].lexeme == "position") {
				positionPair = returnVals(std::vector<Token>{
					toks.begin() + i, std::find_if(
						toks.begin() + i, toks.end(), 
						[](const Token& tok) { 
							return tok.type == TokenType::SEMICOLON; 
							}
				)});

				do {
				       i++;
				} while (toks[i].type != TokenType::SEMICOLON);	       
			}
		}

		node.w = dimensionsPair.first;
		node.h = dimensionsPair.second;
		node.x = positionPair.first;
		node.y = positionPair.second;

		return node;
	}

	Slider makeSlider(const std::vector<Lexer::Token>& toks) {
		using namespace Lexer;
		Slider node;

		std::pair<int, int> dimensionsPair;
		std::pair<int, int> positionPair;
		std::pair<int, int> minMaxPair;

		for (auto i{0}; i < toks.size(); i++) {
			if (toks[i].type == TokenType::IDENTIFIER && toks[i].lexeme == "dimensions") {
				dimensionsPair = returnVals(std::vector<Token>{
					toks.begin() + i, std::find_if(
						toks.begin() + i, toks.end(), 
						[](const Token& tok) { 
								return tok.type == TokenType::SEMICOLON; 	
							}
				)});	
				do {
				       i++;
				} while (toks[i].type != TokenType::SEMICOLON);	       
			}
			else if (toks[i].type == TokenType::IDENTIFIER && toks[i].lexeme == "position") {
				positionPair = returnVals(std::vector<Token>{
					toks.begin() + i, std::find_if(
						toks.begin() + i, toks.end(), 
						[](const Token& tok) { 
							return tok.type == TokenType::SEMICOLON; 
						}
				)});

				do {
				       i++;
				} while (toks[i].type != TokenType::SEMICOLON);	       
			}
			else if (toks[i].type == TokenType::IDENTIFIER && toks[i].lexeme == "range") {
				minMaxPair = returnVals(std::vector<Token> {
					toks.begin() + i, std::find_if(
						toks.begin() + i, toks.end(), 
						[] (const Token& tok) {
							return tok.type == TokenType::SEMICOLON;
						}
				)});
						
	
			}
			
		}

		node.w = dimensionsPair.first;
		node.h = dimensionsPair.second;
		node.x = positionPair.first;
		node.y = positionPair.second;
		node.minVal = minMaxPair.first;
		node.maxVal = minMaxPair.second;

		return node;
	}




public:
	explicit compiler(std::vector<Lexer::Token> tok) : toks(tok) {};

	ComponentNode makeNode() {
		ComponentNode node{};
		Context context;
		
		for (auto i{0}; i < toks.size(); i++) {
			switch (toks[i].type) {
			case Lexer::TokenType::IDENTIFIER:		
				if (toks[i].lexeme == "Window") {
					context = Context::WINDOW; 
				} else if (toks[i].lexeme == "Button") {
					context = Context::BUTTON;
				} else if (toks[i].lexeme == "TextBox") {
					context = Context::TEXTBOX;
				} else if (toks[i].lexeme == "CheckBox") {
					context = Context::CHECKBOX;
				} else if (toks[i].lexeme == "Slider") {
					context = Context::SLIDER;
				}

				if (toks[i].lexeme == "data") {
					auto brace_begin = std::find_if(
						toks.begin() + i, toks.end(),
						[](const Lexer::Token& token) {
						return token.type == Lexer::TokenType::LEFT_BRACE;
					});

					auto data_end = std::find_if(
						brace_begin, toks.end(),
						[](const Lexer::Token& token) {
							return token.type == Lexer::TokenType::RIGHT_BRACE;
					});
					// The data between the "{" and "}" of the .data 
					std::vector<Lexer::Token> subvec(toks.begin() + i + 1, data_end);	

					switch (context) {
					case Context::WINDOW:
						node.win = makeWindow(subvec);
						break;
					case Context::BUTTON:
						node.buttonsCreated.emplace_back(makeButton(subvec));
						break;
					case Context::TEXTBOX:
						node.TextBoxCreated.emplace_back(makeTextBox(subvec));
						break;
					case Context::CHECKBOX:
						node.CheckBoxCreated.emplace_back(makeCheckBox(subvec));
						break;
					case Context::SLIDER:
						node.sliderCreated.emplace_back(makeSlider(subvec));
						break;

					}
					i = std::distance(toks.begin() + 1, data_end);
				}
			default: continue;
			
 			}				 
		}
									
	

		return node;
	}


};
}
