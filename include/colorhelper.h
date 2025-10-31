#pragma once
#include <tuple>
#include <Magick++.h>
#include <iostream>

/// Simply clamps the given int to a given range, default 0-255
constexpr int clamp(int x, int min, int max);

// Given a color, slightly desturate and darken it to get a cool background color for the terminal.
std::tuple<int,int,int> makeBackgroundColor(int r, int g, int b);

// Given a color, slightly saturate and brighten it to get a cool font color for the terminal.
std::tuple<int,int,int> makeForegroundColor(int r, int g, int b);

/// Given an image, downsizes, blurs and quantizes it, then extracts the most vibrant color to serve as an automatic image accent color
std::tuple<int,int,int> extractAccentColor(std::string filepath);