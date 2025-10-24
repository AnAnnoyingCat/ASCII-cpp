#include <iostream>
#include <cxxopts.hpp>
#include <Magick++.h>

using namespace Magick; 

// ASCII Parameters
std::string in_filepath;
std::string out_filepath;
int target_width;
int target_height;
int line_space;
bool inverted = false;

// Declare functions
std::string convertImage();

// =============== Set up the LUT =============== //
constexpr std::array<std::string_view, 256> makeLUT(bool inverted) {
    std::array<std::string_view, 256> LUT{};

    if (!inverted) {
        for (int i = 0; i < 256; i++) {
            if (i < 30)        LUT[i] = " ";
            else if (i < 90)   LUT[i] = "░";
            else if (i < 152)  LUT[i] = "▒";
            else if (i < 219)  LUT[i] = "▓";
            else               LUT[i] = "█";
        }
    } else {
        for (int i = 0; i < 256; i++) {
            if (i > 218)       LUT[i] = " ";
            else if (i > 151)  LUT[i] = "░";
            else if (i > 89)   LUT[i] = "▒";
            else if (i > 29)   LUT[i] = "▓";
            else               LUT[i] = "█";
        }
    }

    return LUT;
}

constexpr auto LUT_BOW = makeLUT(true);   // black on white
constexpr auto LUT_WOB = makeLUT(false);  // white on black

int main(int argc, char const *argv[]) {
	// =============== Read Options and set up render =============== //
	cxxopts::Options options("Ascii renderer", "Renders given image in ASCII");
	options.add_options()
		("i,input", "Path to the input file (required)", cxxopts::value<std::string>())
		("o,output", "Path to the output .txt file", cxxopts::value<std::string>()->default_value("out.txt"))
		("w,width", "Target ASCII art character width", cxxopts::value<int>()->default_value("400"))
		("t,height", "Target ASCII art character height. If both height and width are given, width is prioritized", cxxopts::value<int>()->default_value("200"))
		("s,space", "Space between the lines of the render. Adjust this if your image is squished or stretched vertically. Larger value -> more squished.", cxxopts::value<int>()->default_value("7"))
		("n,invert", "Inverts the colors of the ascii art, from white on black to black on white")
		("h,help", "Show this help page");

	try {
		auto result{options.parse(argc, argv)};
		if (result.count("help")) {
			std::cout << options.help() << std::endl;
			return 0;
		}
		if (result.count("input")) {
			in_filepath = result["input"].as<std::string>();
		} else {
			throw std::invalid_argument("Did not recieve input path.");
		}
		out_filepath = result["output"].as<std::string>();
		target_width = result["width"].as<int>();
		target_height = result["height"].as<int>();
		line_space = result["space"].as<int>();
		if (result.count("invert")) {
			inverted = true;
		}
	} catch (const std::exception& e) {
		std::cerr << "Error parsing arguments: " << e.what() << std::endl;
		return 1;
	}
	
	std::string art = convertImage();

	return 0;
}


std::string convertImage() {
	std::array<std::string_view, 256> lut;
	if (!inverted) {
		lut = LUT_WOB;
	} else {
		lut = LUT_BOW;
	}
	try {
		Image input(in_filepath);
		input.gaussianBlur(0,1.0);
		input.write("output.png");
	} catch (const std::exception& e) {
		std::cerr << "Error reading / editing image: " << e.what() << std::endl;
	}
	


	return "";
}