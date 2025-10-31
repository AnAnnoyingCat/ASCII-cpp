#pragma once
#include <string>
#include <array>
#include <Magick++.h>
#include <iostream>
#include <optional>

/// Struct to hold parameters for the ascii-fication of the image
struct Parameters {
	std::string in_filepath;
	std::optional<std::string> out_filepath;
	double target_width = 0;
	double target_height = 0;
	double squishfactor = 1.0;
	bool inverted = false;
	double pixelRatio = 2.6666666666;
	bool inColor = false;
	bool print = false;
};

// Given a filled out parameter struct, generates and returns an ascii art according to specifications
std::string renderImage(Parameters params);

// Generates the LUT for instant access to characters
constexpr std::array<std::string_view, 256> makeLUT(bool inverted);