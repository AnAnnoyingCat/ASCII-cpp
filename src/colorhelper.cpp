#include<colorhelper.h>

using namespace Magick;

constexpr int clamp(int x, int min, int max) {
    return std::min(std::max(x, min), max);
}

std::tuple<int,int,int> makeBackgroundColor(int r, int g, int b) {
    float gray = (r + g + b) / 3.0f;
    float desaturation = 0.6f; 
    float darken = 0.25f;     
    int nr = clamp(static_cast<int>((gray * (1 - desaturation) + r * desaturation) * darken), 0, 255);
    int ng = clamp(static_cast<int>((gray * (1 - desaturation) + g * desaturation) * darken), 0, 255);
    int nb = clamp(static_cast<int>((gray * (1 - desaturation) + b * desaturation) * darken), 0, 255);
    return {nr, ng, nb};
}

std::tuple<int,int,int> makeForegroundColor(int r, int g, int b) {
    float gray = (r + g + b) / 3.0f;
    float saturation = 1.2f;  
    float brighten = 1.2f;
    int nr = clamp(static_cast<int>((gray + (r - gray) * saturation) * brighten), 0, 255);
    int ng = clamp(static_cast<int>((gray + (g - gray) * saturation) * brighten), 0, 255);
    int nb = clamp(static_cast<int>((gray + (b - gray) * saturation) * brighten), 0, 255);
    return {nr, ng, nb};
}

std::tuple<int,int,int> extractAccentColor(std::string filepath) {
	try {
		Image img(filepath);
		// Downsize for speed and to smooth out noise
		img.resize("100x100!");
		img.gaussianBlur(0, 1.0);

		// Quantize to 5 representative colors
		img.quantizeColors(5);
		img.quantize();

		img.write("test-quantized.png");

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

			// Compute brightness and saturation
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
	} catch (Exception e) {
		std::cerr << "Encountered an error trying to extract accent colors: " << e.what() << std::endl;
		return {180, 180, 180};
	}
}