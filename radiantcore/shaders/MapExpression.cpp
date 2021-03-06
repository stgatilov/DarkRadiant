#include "MapExpression.h"

#include "itextstream.h"
#include "ifilesystem.h"
#include "imodule.h"

#include <iostream>

#include "os/path.h"
#include "string/convert.h"
#include "math/FloatTools.h" // contains float_to_integer() helper
#include "math/Vector3.h"

#include "RGBAImage.h"
#include "textures/HeightmapCreator.h"
#include "textures/TextureManipulator.h"
#include "string/predicate.h"

/* CONSTANTS */
namespace
{
	// Default image maps for optional material stages
	const std::string IMAGE_BLACK = "_black.bmp";
	const std::string IMAGE_CUBICLIGHT = "_cubiclight.bmp";
	const std::string IMAGE_CURRENTRENDER = "_currentrender.bmp";
	const std::string IMAGE_DEFAULT = "_default.bmp";
	const std::string IMAGE_FLAT = "_flat.bmp";
	const std::string IMAGE_FOG = "_fog.bmp";
	const std::string IMAGE_NOFALLOFF = "noFalloff.bmp";
	const std::string IMAGE_POINTLIGHT1 = "_pointlight1.bmp";
	const std::string IMAGE_POINTLIGHT2 = "_pointlight2.bmp";
	const std::string IMAGE_POINTLIGHT3 = "_pointlight3.bmp";
	const std::string IMAGE_QUADRATIC = "_quadratic.bmp";
	const std::string IMAGE_SCRATCH = "_scratch.bmp";
	const std::string IMAGE_SPOTLIGHT = "_spotlight.bmp";
	const std::string IMAGE_WHITE = "_white.bmp";

	inline std::string getBitmapsPath()
	{
		return module::GlobalModuleRegistry().getApplicationContext().getBitmapsPath();
	}
}

namespace shaders {

MapExpressionPtr MapExpression::createForToken(DefTokeniser& token) {
	// Switch on the first keyword, to determine what kind of expression this
	// is.
	// Tr3B: don't convert image names to lower because Unix filesystems are case sensitive
	std::string type = token.nextToken();

	if (string::iequals(type, "heightmap")) {
		return MapExpressionPtr(new HeightMapExpression (token));
	}
	else if (string::iequals(type, "addnormals")) {
		return MapExpressionPtr(new AddNormalsExpression (token));
	}
	else if (string::iequals(type, "smoothnormals")) {
		return MapExpressionPtr(new SmoothNormalsExpression (token));
	}
	else if (string::iequals(type, "add")) {
		return MapExpressionPtr(new AddExpression (token));
	}
	else if (string::iequals(type, "scale")) {
		return MapExpressionPtr(new ScaleExpression (token));
	}
	else if (string::iequals(type, "invertalpha")) {
		return MapExpressionPtr(new InvertAlphaExpression (token));
	}
	else if (string::iequals(type, "invertcolor")) {
		return MapExpressionPtr(new InvertColorExpression (token));
	}
	else if (string::iequals(type, "makeintensity")) {
		return MapExpressionPtr(new MakeIntensityExpression (token));
	}
	else if (string::iequals(type, "makealpha")) {
		return MapExpressionPtr(new MakeAlphaExpression (token));
	}
	else {
		// since we already took away the expression into the variable type, we need to pass type instead of token
		return MapExpressionPtr(new ImageExpression(type));
	}
}

MapExpressionPtr MapExpression::createForString(std::string str) {
	parser::BasicDefTokeniser<std::string> token(str);
	return createForToken(token);
}

ImagePtr MapExpression::getResampled(const ImagePtr& input, std::size_t width, std::size_t height)
{
	// Don't process precompressed images
	if (input->isPrecompressed()) {
		rWarning() << "Cannot resample precompressed texture." << std::endl;
		return input;
	}

	// Check if the dimensions differ from the desired ones
	if (width != input->getWidth(0) || height != input->getHeight(0)) {
		// Allocate a new image buffer
		ImagePtr resampled (new RGBAImage(width, height));

		// Resample the texture to match the dimensions of the first image
		TextureManipulator::instance().resampleTexture(
			input->getMipMapPixels(0),
			input->getWidth(0), input->getHeight(0),
			resampled->getMipMapPixels(0),
			width, height, 4
		);
		return resampled;
	}
	else {
		// Nothing to do here, dimensions match
		return input;
	}
}

HeightMapExpression::HeightMapExpression (DefTokeniser& token) {
	token.assertNextToken("(");
	heightMapExp = createForToken(token);
	token.assertNextToken(",");
	scale = string::convert<float>(token.nextToken());
	token.assertNextToken(")");
}

ImagePtr HeightMapExpression::getImage() const {
	// Get the heightmap from the contained expression
	ImagePtr heightMap = heightMapExp->getImage();

	if (heightMap == NULL) return ImagePtr();

	// Don't process precompressed images
	if (heightMap->isPrecompressed()) {
		rWarning() << "Cannot evaluate map expression with precompressed texture." << std::endl;
		return heightMap;
	}

	// Convert the heightmap into a normalmap
	ImagePtr normalMap = createNormalmapFromHeightmap(heightMap, scale);
	return normalMap;
}

std::string HeightMapExpression::getIdentifier() const {
	std::string identifier = "_heightmap_";
	identifier.append(heightMapExp->getIdentifier() + string::to_string(scale));
	return identifier;
}

AddNormalsExpression::AddNormalsExpression (DefTokeniser& token) {
	token.assertNextToken("(");
	mapExpOne = createForToken(token);
	token.assertNextToken(",");
	mapExpTwo = createForToken(token);
	token.assertNextToken(")");
}

ImagePtr AddNormalsExpression::getImage() const {
    ImagePtr imgOne = mapExpOne->getImage();

    if (imgOne == NULL) return ImagePtr();

    std::size_t width = imgOne->getWidth(0);
    std::size_t height = imgOne->getHeight(0);

    ImagePtr imgTwo = mapExpTwo->getImage();

    if (imgTwo == NULL) return ImagePtr();

	// Don't process precompressed images
	if (imgOne->isPrecompressed() || imgTwo->isPrecompressed()) {
		rWarning() << "Cannot evaluate map expression with precompressed texture." << std::endl;
		return imgOne;
	}

	// The image must match the dimensions of the first
	imgTwo = getResampled(imgTwo, width, height);

    ImagePtr result (new RGBAImage(width, height));

    byte* pixOne = imgOne->getMipMapPixels(0);
    byte* pixTwo = imgTwo->getMipMapPixels(0);
    byte* pixOut = result->getMipMapPixels(0);

    // iterate through the pixels
    for( std::size_t y = 0; y < height; y++ )
	{
		for( std::size_t x = 0; x < width; x++ )
		{
			// create the two vectors
			Vector3 vectorOne(
    			static_cast<double>(pixOne[0]),
    			static_cast<double>(pixOne[1]),
    			static_cast<double>(pixOne[2])
			);
			Vector3 vectorTwo(
    			static_cast<double>(pixTwo[0]),
    			static_cast<double>(pixTwo[1]),
    			static_cast<double>(pixTwo[2])
			);
			// Take the mean value of the two vectors
			Vector3 vectorOut = (vectorOne + vectorTwo) * 0.5;

			pixOut[0] = static_cast<byte>(float_to_integer(vectorOut.x()));
			pixOut[1] = static_cast<byte>(float_to_integer(vectorOut.y()));
			pixOut[2] = static_cast<byte>(float_to_integer(vectorOut.z()));
			pixOut[3] = 255;

			// advance the pixel pointer
			pixOne += 4;
			pixTwo += 4;
			pixOut += 4;
		}
    }
    return result;
}

std::string AddNormalsExpression::getIdentifier() const {
	std::string identifier = "_addnormals_";
	identifier.append(mapExpOne->getIdentifier() + mapExpTwo->getIdentifier());
	return identifier;
}

SmoothNormalsExpression::SmoothNormalsExpression (DefTokeniser& token) {
	token.assertNextToken("(");
	mapExp = createForToken(token);
	token.assertNextToken(")");
}

ImagePtr SmoothNormalsExpression::getImage() const {

	ImagePtr normalMap = mapExp->getImage();

	if (normalMap == NULL) return ImagePtr();

	// Don't process precompressed images
	if (normalMap->isPrecompressed()) {
		rWarning() << "Cannot evaluate map expression with precompressed texture." << std::endl;
		return normalMap;
	}

	std::size_t width = normalMap->getWidth(0);
	std::size_t height = normalMap->getHeight(0);

	ImagePtr result (new RGBAImage(width, height));

	byte* in = normalMap->getMipMapPixels(0);
	byte* out = result->getMipMapPixels(0);

	struct KernelElement {
		// offset to the current pixel
		int dx, dy;
	};

	// a 3x3 kernel with the surrounding pixels including the pixel itself
	const int kernelSize = 9;
	KernelElement kernel[kernelSize] = {
		{-1, -1 },
		{ 0, -1 },
		{ 1, -1 },
		{ 1,  0 },
		{ 1,  1 },
		{ 0,  1 },
		{-1,  1 },
		{-1,  0 },
		{ 0,  0 }
	};
	const float perKernelSize = 1.0f/kernelSize;

	// iterate through the pixels
	for( std::size_t y = 0; y < height; y++) {
		for( std::size_t x = 0; x < width; x++) {
			//the new normal vector for this pixel
			Vector3 smoothVector(0,0,0);

			// calculate the average direction of the surrounding vectors
			for (KernelElement* i = kernel; i != kernel + kernelSize; ++i) {
				// temporary vector to represent one of the surrounding pixels
				byte* pixel = getPixel(in, width, height, x + i->dx, y + i->dy);
				Vector3 temp(pixel[0], pixel[1], pixel[2]);

				smoothVector += temp;
			}

			// Take the average normal vector as result
			smoothVector *= perKernelSize;

			out[0] = static_cast<byte>(float_to_integer(smoothVector.x()));
			out[1] = static_cast<byte>(float_to_integer(smoothVector.y()));
			out[2] = static_cast<byte>(float_to_integer(smoothVector.z()));
			out[3] = 255;

			// advance the pixel pointer
			out += 4;
	    }
	}
    return result;
}

std::string SmoothNormalsExpression::getIdentifier() const {
	std::string identifier = "_smoothnormals_";
	identifier.append(mapExp->getIdentifier());
	return identifier;
}

AddExpression::AddExpression (DefTokeniser& token) {
	token.assertNextToken("(");
	mapExpOne = createForToken(token);
	token.assertNextToken(",");
	mapExpTwo = createForToken(token);
	token.assertNextToken(")");
}

ImagePtr AddExpression::getImage() const {
    ImagePtr imgOne = mapExpOne->getImage();

    if (imgOne == NULL) return ImagePtr();

    std::size_t width = imgOne->getWidth(0);
    std::size_t height = imgOne->getHeight(0);

	ImagePtr imgTwo = mapExpTwo->getImage();

	if (imgTwo == NULL) return ImagePtr();

	// Don't process precompressed images
	if (imgOne->isPrecompressed() || imgTwo->isPrecompressed()) {
		rWarning() << "Cannot evaluate map expression with precompressed texture." << std::endl;
		return imgOne;
	}

	// Resize the image to match the dimensions of the first
    imgTwo = getResampled(imgTwo, width, height);

    ImagePtr result (new RGBAImage(width, height));

    byte* pixOne = imgOne->getMipMapPixels(0);
    byte* pixTwo = imgTwo->getMipMapPixels(0);
    byte* pixOut = result->getMipMapPixels(0);

    // iterate through the pixels
    for( std::size_t y = 0; y < height; y++)
	{
		for( std::size_t x = 0; x < width; x++)
		{
			// add the colors
			pixOut[0] = static_cast<byte>(float_to_integer((static_cast<float>(pixOne[0]) + pixTwo[0]) * 0.5f));
			pixOut[1] = static_cast<byte>(float_to_integer((static_cast<float>(pixOne[1]) + pixTwo[1]) * 0.5f));
			pixOut[2] = static_cast<byte>(float_to_integer((static_cast<float>(pixOne[2]) + pixTwo[2]) * 0.5f));
			pixOut[3] = static_cast<byte>(float_to_integer((static_cast<float>(pixOne[3]) + pixTwo[3]) * 0.5f));

			//advance the pixel pointer
			pixOne += 4;
			pixTwo += 4;
			pixOut += 4;
		}
    }
	return result;
}

std::string AddExpression::getIdentifier() const {
	std::string identifier = "_add_";
	identifier.append(mapExpOne->getIdentifier() + mapExpTwo->getIdentifier());
	return identifier;
}

ScaleExpression::ScaleExpression (DefTokeniser& token) : scaleGreen(0),scaleBlue(0),scaleAlpha(0) {
	token.assertNextToken("(");
	mapExp = createForToken(token);
	token.assertNextToken(",");
	scaleRed = string::convert<float>(token.nextToken());
	if (token.nextToken() == ")") {
		return;
	}
	scaleGreen = string::convert<float>(token.nextToken());
	if (token.nextToken() == ")") {
		return;
	}
	scaleBlue = string::convert<float>(token.nextToken());
	if (token.nextToken() == ")") {
		return;
	}
	scaleAlpha = string::convert<float>(token.nextToken());
	token.assertNextToken(")");
}

ImagePtr ScaleExpression::getImage() const {
    ImagePtr img = mapExp->getImage();

    if (img == NULL) return ImagePtr();

	// Don't process precompressed images
	if (img->isPrecompressed()) {
		rWarning() << "Cannot evaluate map expression with precompressed texture." << std::endl;
		return img;
	}

    std::size_t width = img->getWidth(0);
    std::size_t height = img->getHeight(0);

    if (scaleRed < 0 || scaleGreen < 0 || scaleBlue < 0 || scaleAlpha < 0) {
        rConsole() << "[shaders] ScaleExpression: Invalid scale values found." << std::endl;
		return img;
	}

    ImagePtr result (new RGBAImage(width, height));

    byte* in = img->getMipMapPixels(0);
    byte* out = result->getMipMapPixels(0);

    // iterate through the pixels
    for( std::size_t y = 0; y < height; ++y)
	{
		for( std::size_t x = 0; x < width; ++x)
		{
			// prevent negative values and check for values >255
			int red = float_to_integer(static_cast<float>(in[0]) * scaleRed);
			out[0] = (red>255) ? 255 : static_cast<byte>(red);

			int green = float_to_integer(static_cast<float>(in[1]) * scaleGreen);
			out[1] = (green>255) ? 255 : static_cast<byte>(green);

			int blue = float_to_integer(static_cast<float>(in[2]) * scaleBlue);
			out[2] = (blue>255) ? 255 : static_cast<byte>(blue);

			int alpha = float_to_integer(static_cast<float>(in[3]) * scaleAlpha);
			out[3] = (alpha>255) ? 255 : static_cast<byte>(alpha);

			// advance the pixel pointer
			in += 4;
			out += 4;
		}
    }
	return result;
}

std::string ScaleExpression::getIdentifier() const {
	std::string identifier = "_scale_";
	identifier.append(mapExp->getIdentifier() + string::to_string(scaleRed) + string::to_string(scaleGreen) + string::to_string(scaleBlue) + string::to_string(scaleAlpha));
	return identifier;
}

InvertAlphaExpression::InvertAlphaExpression (DefTokeniser& token) {
	token.assertNextToken("(");
	mapExp = createForToken(token);
	token.assertNextToken(")");
}

ImagePtr InvertAlphaExpression::getImage() const {
	ImagePtr img = mapExp->getImage();

	if (img == NULL) return ImagePtr();

	// Don't process precompressed images
	if (img->isPrecompressed()) {
		rWarning() << "Cannot evaluate map expression with precompressed texture." << std::endl;
		return img;
	}

	std::size_t width = img->getWidth(0);
	std::size_t height = img->getHeight(0);

	ImagePtr result (new RGBAImage(width, height));

	byte* in = img->getMipMapPixels(0);
	byte* out = result->getMipMapPixels(0);

	// iterate through the pixels
	for( std::size_t y = 0; y < height; ++y)
	{
		for( std::size_t x = 0; x < width; ++x)
		{
			out[0] = in[0];
			out[1] = in[1];
			out[2] = in[2];
			out[3] = 255 - in[3];

			// advance the pixel pointer
			in += 4;
			out += 4;
		}
	}

	return result;
}

std::string InvertAlphaExpression::getIdentifier() const {
	std::string identifier = "_invertalpha_";
	identifier.append(mapExp->getIdentifier());
	return identifier;
}

InvertColorExpression::InvertColorExpression (DefTokeniser& token) {
	token.assertNextToken("(");
	mapExp = createForToken(token);
	token.assertNextToken(")");
}

ImagePtr InvertColorExpression::getImage() const {
	ImagePtr img = mapExp->getImage();

	if (img == NULL) return ImagePtr();

	// Don't process precompressed images
	if (img->isPrecompressed()) {
		rWarning() << "Cannot evaluate map expression with precompressed texture." << std::endl;
		return img;
	}

	std::size_t width = img->getWidth(0);
	std::size_t height = img->getHeight(0);

	ImagePtr result (new RGBAImage(width, height));

	byte* in = img->getMipMapPixels(0);
	byte* out = result->getMipMapPixels(0);

	// iterate through the pixels
	for( std::size_t y = 0; y < height; y++) {
		for( std::size_t x = 0; x < width; x++) {
			out[0] = 255 - in[0];
			out[1] = 255 - in[1];
			out[2] = 255 - in[2];
			out[3] = in[3];

			// advance the pixel pointer
			in += 4;
			out += 4;
		}
	}

	return result;
}

std::string InvertColorExpression::getIdentifier() const {
	std::string identifier = "_invertcolor_";
	identifier.append(mapExp->getIdentifier());
	return identifier;
}

MakeIntensityExpression::MakeIntensityExpression (DefTokeniser& token) {
	token.assertNextToken("(");
	mapExp = createForToken(token);
	token.assertNextToken(")");
}

ImagePtr MakeIntensityExpression::getImage() const {
	ImagePtr img = mapExp->getImage();

	if (img == NULL) return ImagePtr();

	// Don't process precompressed images
	if (img->isPrecompressed()) {
		rWarning() << "Cannot evaluate map expression with precompressed texture." << std::endl;
		return img;
	}

	std::size_t width = img->getWidth(0);
	std::size_t height = img->getHeight(0);

	ImagePtr result (new RGBAImage(width, height));

	byte* in = img->getMipMapPixels(0);
	byte* out = result->getMipMapPixels(0);

	// iterate through the pixels
	for( std::size_t y = 0; y < height; ++y)
	{
		for( std::size_t x = 0; x < width; ++x)
		{
			out[0] = in[0];
			out[1] = in[0];
			out[2] = in[0];
			out[3] = in[0];

			// advance the pixel pointer
			in += 4;
			out += 4;
		}
	}

	return result;
}

std::string MakeIntensityExpression::getIdentifier() const {
	std::string identifier = "_makeintensity_";
	identifier.append(mapExp->getIdentifier());
	return identifier;
}

MakeAlphaExpression::MakeAlphaExpression (DefTokeniser& token) {
	token.assertNextToken("(");
	mapExp = createForToken(token);
	token.assertNextToken(")");
}

ImagePtr MakeAlphaExpression::getImage() const {
	ImagePtr img = mapExp->getImage();

	if (img == NULL) return ImagePtr();

	// Don't process precompressed images
	if (img->isPrecompressed()) {
		rWarning() << "Cannot evaluate map expression with precompressed texture." << std::endl;
		return img;
	}

	std::size_t width = img->getWidth(0);
	std::size_t height = img->getHeight(0);

	ImagePtr result (new RGBAImage(width, height));

	byte* in = img->getMipMapPixels(0);
	byte* out = result->getMipMapPixels(0);

	// iterate through the pixels
	for( std::size_t y = 0; y < height; y++)
	{
		for( std::size_t x = 0; x < width; x++)
		{
			out[0] = 255;
			out[1] = 255;
			out[2] = 255;
			out[3] = (in[0] + in[1] + in[2])/3;

			// advance the pixel pointer
			in += 4;
			out += 4;
		}
	}

	return result;
}

std::string MakeAlphaExpression::getIdentifier() const {
	std::string identifier = "_makealpha_";
	identifier.append(mapExp->getIdentifier());
	return identifier;
}

/* ImageExpression */

ImageExpression::ImageExpression(const std::string& imgName)
{
	// Replace backslashes with forward slashes and strip of
	// the file extension of the provided token, and store
	// the result in the provided string.
	_imgName = os::standardPath(imgName).substr(0, imgName.rfind("."));
}

ImagePtr ImageExpression::getImage() const
{
	// Check for some image keywords and load the correct file
	if (_imgName == "_black") {
		return GlobalImageLoader().imageFromFile(
            getBitmapsPath() + IMAGE_BLACK
        );
	}
	else if (_imgName == "_cubiclight") {
		return GlobalImageLoader().imageFromFile(
            getBitmapsPath() + IMAGE_CUBICLIGHT
        );
	}
	else if (_imgName == "_currentRender") {
		return GlobalImageLoader().imageFromFile(
            getBitmapsPath() + IMAGE_CURRENTRENDER
        );
	}
	else if (_imgName == "_default") {
		return GlobalImageLoader().imageFromFile(
            getBitmapsPath() + IMAGE_DEFAULT
        );
	}
	else if (_imgName == "_flat") {
		return GlobalImageLoader().imageFromFile(
            getBitmapsPath() + IMAGE_FLAT
        );
	}
	else if (_imgName == "_fog") {
		return GlobalImageLoader().imageFromFile(
            getBitmapsPath() + IMAGE_FOG
        );
	}
	else if (_imgName == "_nofalloff") {
		return GlobalImageLoader().imageFromFile(
            getBitmapsPath() + IMAGE_NOFALLOFF
        );
	}
	else if (_imgName == "_pointlight1") {
		return GlobalImageLoader().imageFromFile(
            getBitmapsPath() + IMAGE_POINTLIGHT1
        );
	}
	else if (_imgName == "_pointlight2") {
		return GlobalImageLoader().imageFromFile(
            getBitmapsPath() + IMAGE_POINTLIGHT2
        );
	}
	else if (_imgName == "_pointlight3") {
		return GlobalImageLoader().imageFromFile(
            getBitmapsPath() + IMAGE_POINTLIGHT3
        );
	}
	else if (_imgName == "_quadratic") {
		return GlobalImageLoader().imageFromFile(
            getBitmapsPath() + IMAGE_QUADRATIC
        );
	}
	else if (_imgName == "_scratch") {
		return GlobalImageLoader().imageFromFile(
            getBitmapsPath() + IMAGE_SCRATCH
        );
	}
	else if (_imgName == "_spotlight") {
		return GlobalImageLoader().imageFromFile(
            getBitmapsPath() + IMAGE_SPOTLIGHT
        );
	}
	else if (_imgName == "_white") {
		return GlobalImageLoader().imageFromFile(
            getBitmapsPath() + IMAGE_WHITE
        );
	}
	else
    {
        // this is a normal material image, so we load the image from VFS
		return GlobalImageLoader().imageFromVFS(_imgName);
	}
}

std::string ImageExpression::getIdentifier() const
{
	return _imgName;
}

} // namespace shaders
