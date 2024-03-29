#ifndef _SFW_WALLPAPER_HPP_
#define _SFW_WALLPAPER_HPP_

#include "sfw/Gfx/Render.hpp"
#include "sfw/ActivationState.hpp"

#include <string>

namespace sfw
{

class GenericWallpaper
{
public:

	enum Placement : unsigned //!!Generalize this to gfx::Image::Placement!!
	{
		Default, // no alignment/scaling, just toss the image at (0,0) and run...

		HStretch = 1,
		VStretch = HStretch <1,
		KeepAspectRatio = VStretch <1,
		// public: // i.e. use these 3 instead:
		FitWidth = HStretch | KeepAspectRatio,
		FitHeight = VStretch | KeepAspectRatio,
		Stretch = HStretch | VStretch,

		AlignLeft = 8,
		AlignRight = AlignLeft <1,
		AlignTop = AlignRight <1,
		AlignBottom = AlignTop <1,

		HCenter = AlignLeft | AlignRight,
		VCenter = AlignTop | AlignBottom,
		Center = HCenter | VCenter,

		HTile = 128,
		VTile = HTile <1,
		Tile = HTile | VTile,
	};
//	using GenericWallpaper::Placement;

	void disable() { state = ActivationState::Disabled; }
	void enable()  { state = ActivationState::Default; }
	operator bool() const { return state == ActivationState::Default; }

private:
	ActivationState state = ActivationState::Default;
};

} // namespace

#include "sfw/Gfx/backend.hpp"
#include GFX_BACKEND_SPECIFIC(Wallpaper.hpp)

#endif // _SFW_WALLPAPER_HPP_
