#ifndef _GLYPHSET_H_
#define _GLYPHSET_H_

#include "GlyphInfo.h"
#include <boost/shared_ptr.hpp>

namespace fonts
{

class GlyphSet;
typedef boost::shared_ptr<GlyphSet> GlyphSetPtr;

// Each D3 font has three resolutions
enum Resolution
{
	Resolution12,
	Resolution24,
	Resolution48,
	NumResolutions
};

// Each font resolution has its own set of glyphs
class GlyphSet
{
private:
	// Private constructor, initialises this class using 
	// the Q3-style info structure (as read in from the .DAT file)
	GlyphSet(const q3font::Q3FontInfo& q3info, const std::string& fontname, 
		const std::string& language, Resolution resolution_);

public:
	// 12, 24, 48
	Resolution resolution;

	// each set has 256 glyphs
	GlyphInfoPtr glyphs[q3font::GLYPH_COUNT_PER_FONT];

	// VFS paths to the textures (file extension and dds/ prefix are omitted)
	std::vector<std::string> textures;

	// Public named constructor
	static GlyphSetPtr createFromDatFile(const std::string& vfsPath, 
										 const std::string& fontname,
										 const std::string& language,
										 Resolution resolution);
};
typedef boost::shared_ptr<GlyphSet> GlyphSetPtr;

} // namespace fonts

#endif /* _GLYPHSET_H_ */
