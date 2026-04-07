#include <optional>
#include <iostream>
#include <fstream>
#include <random>
#include <cstdlib>

// Compiler part
#include "Compiler/lexer.hpp"
#include "Compiler/parser.hpp"
#include "Compiler/compiler.hpp"
#include "Compiler/code_gen.hpp"

void showHelp() {
	std::cout << "GUI++ Help:\n"
		<< "Usage : GUI++ [Source File].gui\n" 
		<< "Optional flags :\n"
		<< "-o [NAME] : set output executable name\n"
		<< "-keep-temp : keep temporary C++ generated code\n";
	std::exit(1);
}


int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "GUI++ : Error too few argument" << '\n';
    return 1;
  }

  using namespace std; // <- don't worry about it, it's not a bad thing

  optional<string> filename = nullopt;
  optional<string> outputname = nullopt;
  bool useClang = false;
  bool output_token_exists = false;
  bool keepTempFile = false;

  // GUI++ main.gui -o app
  // GUI++ main.gui -> default name to a.out or the file name

  for (auto i{1}; i <= argc - 1; i++) {
    auto searchfile = string(argv[i]).find(".gui");
    auto output_token = string(argv[i]).find("-o");
    auto keepTempFileIt = string(argv[i]).find("-keep-temp");
    auto showHelpIt = string(argv[i]).find("--help");
    auto useClangIt = string(argv[i]).find("-clang++");

    if (searchfile != string::npos) {
      filename = argv[i];
    }
    if (output_token != string::npos) {
	    output_token_exists = true;
    }
    if (keepTempFileIt != string::npos) {
	    keepTempFile = true;
    }
    if (showHelpIt !=  string::npos) {
	    showHelp();
    }
    if (useClangIt != string::npos) {
	    useClang = true;
    }
    if (string(argv[i]).find(".gui") == string::npos && string(argv[i]) != "-o" 
	&& outputname == nullopt && output_token_exists) {
	    outputname = argv[i];
    }
  }

  if (not filename) {
    cerr << "GUI++ : Error no source file found" << endl;
    cerr << "Compilation Stopped" << endl;
    return 1;
  }

  Lexer::Lexer lex{};
  auto lexer_tok = lex.transform_to_tokens(*filename);
  //for (const auto& i : lexer_tok) {
  //	cout << i << endl;
  //}

  // std::exit(0); // for debug the lexer 

  Parser::Parser parser = Parser::Parser(lexer_tok);
  // parser.mainParserLoop();

  Compiler::compiler comp{lexer_tok};
  
  auto comp_tok = comp.makeNode();

  CODEGEN::Code_Gen gen{comp_tok};

  std::string final_SDL2_code = gen.make_final_code();

  // make a random generated name 
  mt19937 random_gen(random_device{}());
  string file_name{};
  uniform_int_distribution<int> dist(10,30);
  int size = dist(random_gen);

  for (auto i{0}; i <= size; i++) {
	  file_name += static_cast<char>(dist(random_gen) % 26 + 'a');
  }

  file_name += ".cpp";

  // Make the file
  ofstream file(file_name); 
  file << final_SDL2_code;
  file.close();

  // Make the command
  string command;

  if (useClang) { 
	  command = format("clang++ {} -o {} -lSDL2 -lSDL2_ttf", file_name, outputname.value_or("a.out"));
  }

  command = format("g++ {} -o {} -lSDL2 -lSDL2_ttf", file_name, outputname.value_or("a.out"));

  // Execute the command to compile 
  system(command.c_str());

  // Remove the temporary C++ File 
  if (not keepTempFile) {
  	filesystem::remove(file_name);
  }

  return 0;
}
