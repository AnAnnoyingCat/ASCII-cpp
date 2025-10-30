#include <iostream>
#include <cxxopts.hpp>
#include <Magick++.h>
#include <fstream>
#include <algorithm>
#include <tuple>

using namespace Magick; 

// ASCII Parameters
std::string in_filepath;
std::string out_filepath;
double target_width = 0;
double target_height = 0;
double squishfactor = 1.0;
bool inverted = false;
double pixelRatio = 2.6666666666;
bool inColor = false;

// Declare functions
std::string convertImage();

constexpr int clamp(int x, int minv=0, int maxv=255) {
    return std::min(std::max(x, minv), maxv);
}

// Slightly desaturate and darken (ChatGPT)
std::tuple<int,int,int> makeBackgroundColor(int r, int g, int b) {
    float gray = (r + g + b) / 3.0f;
    float desaturation = 0.6f; // 0 = fully gray, 1 = full color
    float darken = 0.25f;      // fraction to darken
    int nr = clamp(static_cast<int>((gray * (1 - desaturation) + r * desaturation) * darken));
    int ng = clamp(static_cast<int>((gray * (1 - desaturation) + g * desaturation) * darken));
    int nb = clamp(static_cast<int>((gray * (1 - desaturation) + b * desaturation) * darken));
    return {nr, ng, nb};
}

// Slightly brighten and saturate (ChatGPT)
std::tuple<int,int,int> makeForegroundColor(int r, int g, int b) {
    float gray = (r + g + b) / 3.0f;
    float saturation = 1.2f;  // >1 increases saturation
    float brighten = 1.2f;    // >1 brightens
    int nr = clamp(static_cast<int>((gray + (r - gray) * saturation) * brighten));
    int ng = clamp(static_cast<int>((gray + (g - gray) * saturation) * brighten));
    int nb = clamp(static_cast<int>((gray + (b - gray) * saturation) * brighten));
    return {nr, ng, nb};
}

/**
 * @brief Extracts a vibrant accent color from the input image by quantizing to 5 colors
 * 
 * The image is resized and blurred slightly to remove noise. Then it is quantized to 5 representative colors,
 * and the color with the highest saturation × brightness score is chosen as the "accent color".
 * 
 * @return std::tuple<int,int,int> (r,g,b)
 */
std::tuple<int,int,int> extractAccentColor() {
    try {
        Image img(in_filepath);

        // Downsize for speed and to smooth out noise
        img.resize("100x100!");
        img.gaussianBlur(0, 1.0);

        // Quantize to 5 representative colors
        img.quantizeColors(5);
        img.quantize();

        const size_t n = img.colorMapSize();
        if (n == 0) {
            throw std::runtime_error("Color map is empty after quantization.");
        }

        double bestScore = -1.0;
        int bestR = 128, bestG = 128, bestB = 128;

        for (size_t i = 0; i < n; ++i) {
            Color c = img.colorMap(i);

            // Convert to 0–255 range
            double r = c.redQuantum()   * 255.0 / QuantumRange;
            double g = c.greenQuantum() * 255.0 / QuantumRange;
            double b = c.blueQuantum()  * 255.0 / QuantumRange;

            // Compute brightness (luminance) and saturation
            double maxc = std::max({r, g, b});
            double minc = std::min({r, g, b});
            double saturation = (maxc == 0.0) ? 0.0 : (maxc - minc) / maxc;
            double brightness = (r + g + b) / (3.0 * 255.0);

            // Score vibrant colors higher, avoid dark grays
            double score = saturation * (0.4 + brightness);

            if (score > bestScore) {
                bestScore = score;
                bestR = static_cast<int>(r);
                bestG = static_cast<int>(g);
                bestB = static_cast<int>(b);
            }
        }

        return {bestR, bestG, bestB};

    } catch (const std::exception& e) {
        std::cerr << "Error extracting accent color: " << e.what() << std::endl;
        // Default fallback color (light gray)
        return {180, 180, 180};
    }
}

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
		("h,help", "Show this help page")
		("c,color", "Render the image in terminal color");
		

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
		if (result.count("color")) {
			inColor = true;
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
	if (inColor) {

		auto [r, g, b] = extractAccentColor();  // ChatGPT's magical "🎨 automatically find accent". Quantizes image to 5 colors and extracts the most "vivid"

		// Get foreground and background colors
		auto [br, bg, bb] = makeBackgroundColor(r, g, b);
	    auto [fr, fg, fb] = makeForegroundColor(r, g, b);
		// print using accent colors
		std::cout << "\033[48;2;"<<br<<';'<<bg<<';'<<bb<<"m"<<"\033[38;2;"<<fr<<';'<<fg<<';'<<fb<<"m"<< art << "\033[0m\n";
		
	} else {
		std::cout << art << std::endl;
	}
	
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
		//input.write("step1blurred.png");

		// Calculate target size keeping aspect ratio in mind
		if (target_width != 0) {
			if (target_height != 0) {
				// both width and height given → fit inside the given box, preserving aspect ratio
				double aspect = static_cast<double>(width) / height;
				double box_aspect = static_cast<double>(target_width) / target_height;
				if (aspect > box_aspect) {
					// image is wider than the box -> limit by width
					target_height = (target_width / aspect) / pixelRatio;
				} else {
					// image is taller than the box -> limit by height
					target_width = (target_height * aspect) * pixelRatio;
				}
			} else {
				target_height = height * (target_width / width) / pixelRatio;
			}
		} else {
			if (target_height == 0) {
				target_width = 400;
				target_height = height * (target_width / width) / pixelRatio;
			} else {
				target_width = (width * target_height * pixelRatio) / height;
			}
		}

		input.resize(Geometry(std::to_string(std::lround(target_width)) + "x" + std::to_string(std::lround(target_height)) + "!"));
		//input.write("step2resized.png");
		
		width = input.columns();
		height = input.rows();

		input.type(GrayscaleType);
		//input.write("step3grayscale.png");

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