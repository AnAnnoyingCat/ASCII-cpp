#include "asciirenderer.h"

using namespace Magick; 

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

std::string renderImage(Parameters params) {
	std::array<std::string_view, 256> lut;
	std::string art = "";
	
	if (!params.inverted) {
		lut = LUT_WOB;
	} else {
		lut = LUT_BOW;
	}
	try {
		// Get the image
		Image input(params.in_filepath);

		int width = input.columns();
		int height = input.rows();
		art.reserve(width * (height + 1));

		// Blur it to denoise it
		input.gaussianBlur(0,1.0);
		//input.write("step1blurred.png");

		// Calculate target size keeping aspect ratio in mind
		if (params.target_width != 0) {
			if (params.target_height != 0) {
				// both width and height given → fit inside the given box, preserving aspect ratio
				double aspect = static_cast<double>(width) / height;
				double box_aspect = static_cast<double>(params.target_width) / (params.target_height * params.pixelRatio);
				if (aspect > box_aspect) {
					// image is wider than the box -> limit by width
					params.target_height = height * (params.target_width / width) / params.pixelRatio;
				} else {
					// image is taller than the box -> limit by height
					params.target_width = (width * params.target_height * params.pixelRatio) / height;
				}
			} else {
				params.target_height = height * (params.target_width / width) / params.pixelRatio;
			}
		} else {
			if (params.target_height == 0) {
				params.target_width = 400;
				params.target_height = height * (params.target_width / width) / params.pixelRatio;
			} else {
				params.target_width = (width * params.target_height * params.pixelRatio) / height;
			}
		}

		input.resize(Geometry(std::to_string(std::lround(params.target_width)) + "x" + std::to_string(std::lround(params.target_height)) + "!"));
		
		width = input.columns();
		height = input.rows();

		input.type(GrayscaleType);

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