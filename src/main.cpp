// Standart includes
#include <iostream>
#include <fstream>

// External includes
#include <external/cxxopts.hpp>

// My includes
#include <asciirenderer.h>
#include <colorhelper.h>

using namespace Magick; 

Parameters params;

int main(int argc, char const *argv[]) {
	// Set up Magick
	InitializeMagick(*argv);

	// Read and set up Options
	cxxopts::Options options("Ascii renderer", "Renders given image in ASCII");
	options.add_options()
		("i,input", "Path to the input file (required)", cxxopts::value<std::string>())
		("o,output", "Saves the ASCII art to a path", cxxopts::value<std::string>())
		("p,print", "Prints the ASCII art to console after rendering it")
		("w,width", "Target ASCII art character width", cxxopts::value<int>())
		("H,height", "Target ASCII art character height. If both height and width are given, width is prioritized", cxxopts::value<int>())
		("s,squishfactor", "Adjust this if your image is squished or stretched vertically. Larger value -> more squished.", cxxopts::value<double>())
		("n,invert", "Inverts the colors of the ascii art, from white on black to black on white")
		("h,help", "Show this help page")
		("c,color", "Render the image in terminal color");

	try {
		auto result{options.parse(argc, argv)};

		// If user wants help, return it and end the program
		if (result.count("help")) {
			std::cout << options.help() << std::endl;
			return 0;
		}

		// Get all parameters
		if (result.count("input")) {
			params.in_filepath = result["input"].as<std::string>();
		} else {
			throw std::invalid_argument("No input path specified");
		}
		if (result.count("output")) params.out_filepath = result["output"].as<std::string>();
		if (result.count("width")) params.target_width = result["width"].as<int>();
		if (result.count("height")) params.target_height = result["height"].as<int>();
		if (result.count("squishfactor")) params.squishfactor = result["squishfactor"].as<double>();
		if (result.count("invert")) params.inverted = true;
		if (result.count("color")) params.inColor = true;
		if (result.count("print")) params.print = true;
		params.pixelRatio *= params.squishfactor;

	} catch (const std::exception& e) {
		std::cerr << "Error parsing arguments: " << e.what() << std::endl;
		return 1;
	}
	
	// get the image
	std::string art = renderImage(params);

	// store it
	if (params.out_filepath) {
		std::ofstream out(params.out_filepath.value());
		if (!out) {
			throw std::runtime_error("Could not open output file: " + params.out_filepath.value());
		}
		out << art;
	}
	if (params.print) {
		if (params.inColor) {
			auto [r, g, b] = extractAccentColor(params.in_filepath);

			auto [br, bg, bb] = makeBackgroundColor(r, g, b);
			auto [fr, fg, fb] = makeForegroundColor(r, g, b);
			// print using accent colors
			std::cout << "\033[48;2;"<<br<<';'<<bg<<';'<<bb<<"m"<<"\033[38;2;"<<fr<<';'<<fg<<';'<<fb<<"m"<< art << "\033[0m\n";
		} else {
			std::cout << art << std::endl;
		}
	}
	return 0;
}
