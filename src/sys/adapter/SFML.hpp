/*
	Stuff that's still not abstracted in Myco/SAL...
*/

#pragma once

namespace sys::adapter::UI::Input
{

enum MouseButton {
	Primary, Main = Primary, Left = Primary,
	Secondary, Right = Secondary,
	Middle, Wheel = Middle,
	Extra1, Extra2, Extra3, Extra4,
	_last_
};
bool mouse_button_down(int which = MouseButton::Main);

} // namespace sys::adapter::UI::Input
