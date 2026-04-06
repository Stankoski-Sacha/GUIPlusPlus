#pragma once 

#include <iostream>
#include <vector>
#include <string>
#include <format>
#include <fstream>

// Other import 
#include "compiler.hpp"

namespace CODEGEN {
class Code_Gen {
private:
	Compiler::ComponentNode componants;
		
	// The boilerplate of including headers
	std::string headerCode() {
		return std::format("#include <SDL2/SDL.h>\n"
				"#include <SDL2/SDL_ttf.h>\n"
				"#include <string>\n"
		);
					
	};

	std::string mainFunctionEntry() {
		return std::format("int main(int argc, char** argv) {{\n");
	}

	std::string initCode() {
		return std::format("	SDL_Init(SDL_INIT_VIDEO);\n"
				   "	TTF_Init();\n"
		);
	}

	std::string makeWindowCode() {
		return std::format("	SDL_Window* win = SDL_CreateWindow(\"{}\", {}, {}, {}, {}, {});\n" 
				"	SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);\n"
				"	bool running = true;\n"        
				"	SDL_Event e;\n",
				componants.win.title.c_str(), componants.win.x, componants.win.y, componants.win.w, 
				componants.win.h, componants.win.flag
		);
	}

	std::string renderLoopCodePart1() {
		return std::format("	int mx, my;\n"
				"	while (running) {{ \n"
				"		while (SDL_PollEvent(&e)) {{ \n"
				"			SDL_GetMouseState(&mx, &my);\n"
				"			if (e.type == SDL_QUIT) {{ \n"
				"				running = false;\n"
				"			}}\n"
				"			if (e.type == SDL_MOUSEBUTTONDOWN) {{\n"	
		);
	}

	std::string renderLoopCodePart2() {			
		return std::format("			}}\n"
				"		}}\n"
				"		SDL_SetRenderDrawColor(ren, 255,255,255,255);\n"
				"		SDL_RenderClear(ren);\n"
		);
	}


	std::string renderLoopCodeEnd() {
		return std::format("		SDL_RenderPresent(ren);\n"
				"		SDL_Delay(16);\n"
				"	}}\n"
		);
	}

	std::string cleanupCode() {
		return std::format("	SDL_DestroyWindow(win);\n"
				"	SDL_DestroyRenderer(ren);\n"
				"	TTF_CloseFont(font);\n"
				"	SDL_Quit();\n"
				"	TTF_Quit();\n"
				"	return 0;\n"
				"}}\n"
		);
	}

	// Add the class button to the code before main 
	std::string addClassCode(std::string filepath) {
		std::ifstream file(filepath);
		

		std::string button_code;
		std::string word;

		while (std::getline(file, word)) {
			button_code += word + '\n';
		}

		return button_code;
	}
	

	std::string makeButtonInitCode(int buttonID) {
		return std::format("	Button button{} = Button({}, {}, {}, {}, \"{}\", font);\n",
				buttonID,
				componants.buttonsCreated[buttonID].x ,
				componants.buttonsCreated[buttonID].y ,
				componants.buttonsCreated[buttonID].w ,
				componants.buttonsCreated[buttonID].h ,
				componants.buttonsCreated[buttonID].buttonText
		);
	}

	std::string makeCheckBoxInitCode(int checkBoxID) {
		return std::format("	CheckBox check{} = CheckBox({}, {}, {}, {});\n",
				checkBoxID,
				componants.CheckBoxCreated[checkBoxID].x,
				componants.CheckBoxCreated[checkBoxID].y,
				componants.CheckBoxCreated[checkBoxID].w,
				componants.CheckBoxCreated[checkBoxID].h
		);
	}

	std::string makeSliderInitCode(int sliderID) {
		return std::format("	Slider slider{} = Slider({}, {}, {}, {}, font, {}, {});\n",
				sliderID,
				componants.sliderCreated[sliderID].x,
				componants.sliderCreated[sliderID].y, 
				componants.sliderCreated[sliderID].w,
				componants.sliderCreated[sliderID].h,
				componants.sliderCreated[sliderID].minVal,
				componants.sliderCreated[sliderID].maxVal
		);
	}

	std::string renderCode(std::string toRender) {
		return std::format("		{}.render(ren);\n", toRender);
	}

	std::string embedFontCode() {
	    std::ifstream src(std::string(PROJECT_SOURCE_DIR) + "/font/LiberationSans-Regular.ttf", std::ios::binary);
	    if (!src.is_open()) {
		std::cerr << "Could not find arial.ttf\n";
		return "";
	    }
	    std::ofstream dst("arial.ttf", std::ios::binary);
	    dst << src.rdbuf();
	    return "";
	}

	std::string openFontCode() {
		return "	TTF_Font* font = TTF_OpenFont(\"LiberationSans-Regular.ttf\", 20);\n";
	}

	std::string makeTextBoxisInsideCode(int textBoxID) {
		return ""; // Temporary until I actually add a textbox lol
	}

	std::string makeCheckBoxisInsideCode(int checkBoxID) {
		return std::format("				if (check{}.isInside(mx, my)) {{ check{}.switchBox(ren);\n }}", 
				checkBoxID, checkBoxID
		);

	}

public:
	explicit Code_Gen(Compiler::ComponentNode node) : componants(node) {}

	std::string make_final_code() {
		std::string code = "";
		code += headerCode();
		int buttons = -1, textboxes = -1, checkbox = -1, slider = -1;

		// Check for any componant 
		if (not componants.buttonsCreated.empty()) {
			code += addClassCode(
			std::string(PROJECT_SOURCE_DIR) + "/src/GUI_Componants/Button.hpp"
			);
			buttons = componants.buttonsCreated.size() - 1;
		}

		if (not componants.TextBoxCreated.empty()) {
			code += addClassCode(
			std::string(PROJECT_SOURCE_DIR) + "/src/GUI_Componants/TextBox.hpp"
			);
			textboxes = componants.TextBoxCreated.size() - 1;
		}

		if (not componants.CheckBoxCreated.empty()) {
			code += addClassCode(
			std::string(PROJECT_SOURCE_DIR) + "/src/GUI_Componants/CheckBox.hpp"
			);
			checkbox = componants.CheckBoxCreated.size() - 1;
		}

		if (not componants.sliderCreated.empty()) {
			code += addClassCode(
				std::string(PROJECT_SOURCE_DIR) + "/src/GUI_Componants/Slider.hpp"
			);
			slider = componants.sliderCreated.size() - 1;
		}

		// Start of the code 
		code += mainFunctionEntry();
		code += initCode();
		code += makeWindowCode();
		code += openFontCode();
		
		// GUI Comp init here 
		int bTemp = buttons;
		int checkTemp = checkbox;
		int sliderTemp = slider;
		while (bTemp >= 0) {
			code += makeButtonInitCode(bTemp);
			bTemp--;
		}

		while (checkTemp >= 0) {
			code += makeCheckBoxInitCode(checkTemp);
			checkTemp--;
		}

		while (sliderTemp >= 0) {
			code += makeSliderInitCode(sliderTemp);
			sliderTemp--;
		}

		// End of that
		code += renderLoopCodePart1();

		// Add code for mouse input and isInside triggers
		checkTemp = checkbox;

		while (checkTemp >= 0) {
			code += makeCheckBoxisInsideCode(checkTemp);
			checkTemp--;
		}

		// End of that part 
		code += renderLoopCodePart2();

		// Add code for render button etc not now 
		bTemp = buttons, checkTemp = checkbox, sliderTemp = slider;
		while (bTemp >= 0) {
			code += renderCode(std::format("button{}", bTemp));
			bTemp--;
		}

		while (checkTemp >= 0) {
			code += renderCode(std::format("check{}", checkTemp));

			checkTemp--;
		}

		while (sliderTemp >= 0) {
			code += renderCode(std::format("slider{}", sliderTemp));
			sliderTemp--;
		}

		// End of render
		code += renderLoopCodeEnd();

		// End of the file 
		code += cleanupCode();
		return code;
	}
};
}
