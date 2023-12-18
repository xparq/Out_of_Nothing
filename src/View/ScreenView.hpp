#ifndef _GHF0286SXT7BV5CNZ2MK3X7TYB45087BYVRT87_
#define _GHF0286SXT7BV5CNZ2MK3X7TYB45087BYVRT87_

#include "Camera.hpp"

//#include "Model/Math/Vector2.hpp"

namespace View {

// Screen View(Port) coordinate system:
//
//  0 --> +x
//  |      |
//  v      |
// +y --- (width, height)
//
// Origin: top-left

class ScreenView
{
public:
	struct Config
	{
		// Why float dimensions? -> #221: Camera concept/definitions...
		unsigned width  = 1024;
		unsigned height = 768;
//!!		bool  gridlines = true; //!! -> false, as soon as it involves complexity beyond a simple direct query!
	};

	// -------------------------------------------------------------------
	// Setup...
	// -------------------------------------------------------------------

	ScreenView() {}
	ScreenView(Config cfg);
	ScreenView(Config cfg, Camera& cam);

	void reset(const Config* recfg = nullptr); // Resets things to the last cfg if null.
	void reset(Config&& recfg);
	void resize(unsigned width, unsigned height);

	void attach(Camera& camera);

	// -------------------------------------------------------------------
	// Data...
	// -------------------------------------------------------------------
protected:
	Config  _cfg;
	Camera* _camera = nullptr; // The source/input of the view

}; // class ScreenView

} // namespace View

#endif // _GHF0286SXT7BV5CNZ2MK3X7TYB45087BYVRT87_
