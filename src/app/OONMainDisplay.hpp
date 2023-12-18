#ifndef _S89U4589YU7845037845DN68945Y68756VM87HY56TRIJ_
#define _S89U4589YU7845037845DN68945Y68756VM87HY56TRIJ_

#include "Engine/View/ScreenView.hpp"
//#include "Engine/View/Camera.hpp"
namespace Szim::View { class Camera; }

//#include "Model/Math/Vector2.hpp"

namespace OON {

// Screen View(Port) coordinate system:
//
//  0 --> +x
//  |      |
//  v      |
// +y --- (width, height)
//
// Origin: top-left

class OONMainDisplay : public Szim::View::ScreenView
{
public:
	// -------------------------------------------------------------------
	// Setup...
	// -------------------------------------------------------------------
/*
	struct OONViewConfig : Szim::View::ScreenView::Config
	{
	};
*/
//	OONMainDisplay() {}
//	OONMainDisplay(Config cfg);
	OONMainDisplay(Config cfg, Szim::View::Camera& cam);

/*
	void reset(const Config* recfg = nullptr) override; // Resets things to the last cfg if null.
	void reset(Config&& recfg) override;
	void resize(unsigned width, unsigned height) override;
*/

	// -------------------------------------------------------------------
	// Data...
	// -------------------------------------------------------------------
protected:
	Config  _cfg;

}; // class OONMainDisplay

} // namespace OON

#endif // _S89U4589YU7845037845DN68945Y68756VM87HY56TRIJ_
