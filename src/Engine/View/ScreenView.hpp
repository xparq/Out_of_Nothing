#ifndef _GHF0286SXT7BV5CNZ2MK3X7TYB45087BYVRT87_
#define _GHF0286SXT7BV5CNZ2MK3X7TYB45087BYVRT87_

#include "Camera.hpp"

//#include "Model/Math/Vector2.hpp"

#include <cassert>


namespace Szim { class SimApp; }

namespace Szim::View {

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
	// -------------------------------------------------------------------
	// Setup...
	// -------------------------------------------------------------------

	struct Config
	{
		// Why float dimensions? -> #221: Camera concept/definitions...
		unsigned width  = 1024;
		unsigned height = 768;
//!! Do this here, not in the camera:
//!!		bool  gridlines = true; //!! -> false, as soon as it involves complexity beyond a simple direct query!
	};

//	ScreenView(Config cfg);
	ScreenView(Config cfg, Camera& cam);

	virtual SimApp& app() = 0; //!! Just a lame placeholder for now!
	                           //!! EITHER SHOULDN'T EXIST OR SHOULDN'T BE PURE!
	                           //!! If kept: also needs a ctor with SimApp&!

	virtual void reset(const Config* recfg = nullptr); // Resets things to the last cfg if null.
	virtual void reset(Config&& recfg);
	virtual void resize(unsigned width, unsigned height);

	// -------------------------------------------------------------------
	// Interfacing the model (via a camera)...
	// -------------------------------------------------------------------

	void attach(Camera& camera);
	      Camera& camera()       { assert(_camera); return *_camera; }
	const Camera& camera() const { assert(_camera); return *_camera; }


	// -------------------------------------------------------------------
	// Rendering...
	// -------------------------------------------------------------------

	virtual void draw() = 0;

	// -------------------------------------------------------------------
	// Data...
	// -------------------------------------------------------------------
protected:
	Config  _cfg;
	Camera* _camera = nullptr; // The source/input of the view

}; // class ScreenView

} // namespace Szim::View

#endif // _GHF0286SXT7BV5CNZ2MK3X7TYB45087BYVRT87_
