#include <iostream>
#include <cxxopts.hpp>
#include <Magick++.h>
#include <fstream>

using namespace Magick; 

// ASCII Parameters
std::string in_filepath;
std::string out_filepath;
double target_width = 0;
double target_height = 0;
double squishfactor = 1.0;
bool inverted = false;
double pixelRatio = 2.6666666666;

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
	// Set up Magick
	InitializeMagick(*argv);

	// =============== Read Options and set up render =============== //
	cxxopts::Options options("Ascii renderer", "Renders given image in ASCII");
	options.add_options()
		("i,input", "Path to the input file (required)", cxxopts::value<std::string>())
		("o,output", "Path to the output .txt file", cxxopts::value<std::string>()->default_value("out.txt"))
		("w,width", "Target ASCII art character width", cxxopts::value<int>())
		("t,height", "Target ASCII art character height. If both height and width are given, width is prioritized", cxxopts::value<int>())
		("s,squishfactor", "Adjust this if your image is squished or stretched vertically. Larger value -> more squished.", cxxopts::value<double>()->default_value("1.0"))
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
		if (result.count("width")) target_width = result["width"].as<int>();
		if (result.count("height")) target_height = result["height"].as<int>();
		squishfactor = result["squishfactor"].as<double>();
		pixelRatio *= squishfactor;
		if (result.count("invert")) {
			inverted = true;
		}
	} catch (const std::exception& e) {
		std::cerr << "Error parsing arguments: " << e.what() << std::endl;
		return 1;
	}
	
	// get the image
	std::string art = convertImage();

	// store it
	std::ofstream out(out_filepath);
	if (!out) {
		throw std::runtime_error("Could not open output file: " + out_filepath);
	}
	out << art;
	out.close();
	return 0;
}

/**
 * @brief Reads the input image. Then applies a light blur to denoise it and converts it to ascii characters
 * 
 *
 */
std::string convertImage() {
	std::array<std::string_view, 256> lut;
	std::string art = "";
	
	if (!inverted) {
		lut = LUT_WOB;
	} else {
		lut = LUT_BOW;
	}
	try {
		// Get the image
		Image input(in_filepath);

		int width = input.columns();
		int height = input.rows();
		art.reserve(width * (height + 1));

		// Blur it to denoise it
		input.gaussianBlur(0,1.0);
		input.write("step1blurred.png");

		// Calculate target size keeping aspect ratio in mind
		if (target_width != 0) {
			target_height = height * (target_width / width) / pixelRatio;
		} else {
			if (target_height == 0) {
				target_width = 400;
				target_height = height * (target_width / width) / pixelRatio;
			} else {
				target_width = (width * target_height * pixelRatio) / height;
			}
		}

		input.resize(Geometry(std::to_string(std::lround(target_width)) + "x" + std::to_string(std::lround(target_height)) + "!"));
		input.write("step2resized.png");
		
		width = input.columns();
		height = input.rows();

		input.type(GrayscaleType);
		input.write("step3grayscale.png");

		// Create view of the pixel cache
		Pixels view(input);

		// Get a pointer to the pixel data
		const PixelPacket* pixels = view.getConst(0, 0, input.columns(), input.rows());

		const double scale = 255.0 / QuantumRange;
		for (int j = 0; j < height; j++) {
			for (int i = 0; i < width; i++) {
				const int brightness = static_cast<int>(pixels[j * width + i].red * scale);
				art += lut[brightness];
			}
			art += '\n';
		}

	} catch (const std::exception& e) {
		std::cerr << "Error reading / editing image: " << e.what() << std::endl;
	}
	
	return art;
}