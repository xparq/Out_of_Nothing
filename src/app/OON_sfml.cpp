// Must do this first for Tracy's winsock2.h has to precede any windows.h! :-/
#include "extern/Tracy/public/tracy/Tracy.hpp"

#include "OON_sfml.hpp"
#include "OON_UI.hpp"

//!! This "backend tunneling" should be "allowed" (even properly facilitated,
//!! in a more civilized way) later, after the backend selection becomes more
//!! transparent and/or automatic etc. (#294; finishing what I've started in
//!! MycoGUI).
//!!
//!! A side-note/reminder: the goal here (on the app level) is only separating
//!! (most of?) the *sources* from direct backend dependencies ("write once"),
//!! not the entire compilation process.
//!!
//!! So sad, still...:
#include "Szim/Core/Device/_adapter_switcher.hpp"
#include SWITCHED(BACKEND, _Backend.hpp)
#define SFML_WINDOW() (((Szim::SFML_Backend&)backend).SFML_window())
#define SFML_KEY(KeyName) unsigned(sf::Keyboard::Key::KeyName) //!!XLAT
//!! :-(((
#include "Szim/Core/Device/HCI/Keyboard/adapter/SFML/keycodes.hpp" // SFML -> SimApp keycode translation

#include <SFML/Graphics/RenderWindow.hpp>

#include <memory>
	using std::make_shared;
#include <cstdlib>
	using std::rand; // + RAND_MAX (macro!)
#include <charconv>
	using std::to_chars;
#include <cassert>

#include "Szim/diag/Error.hpp"
#include "Szim/diag/Log.hpp"

using namespace Szim;
using namespace Model;
using namespace View;
using namespace sz;
using namespace std;


//============================================================================
namespace OON {

//----------------------------------------------------------------------------
OONApp_sfml::OONApp_sfml(const RuntimeContext& runtime)
	: OONApp(runtime, oon_main_view_) //!! The OONApp ctor MUST only set a ref to the view, not use it!!!
	, oon_main_view_(*this) //!! Initialized later than the app itself! :-/
{
//std::cerr << "--- OONApp_sfml ctor" << std::endl;
}


//----------------------------------------------------------------------------
void OONApp_sfml::process(const SAL::event::Input& event) //override
{
	using namespace Szim::HCI;

			switch (event.type) //!! See above: morph into using abstracted events!
			{
			case myco::event::KeyDown:
			{
				auto keycode = event.get_if<myco::event::KeyDown>()->code;
#ifdef DEBUG
	if (cfg.DEBUG_show_keycode) Note("key code: " + keycode); //!! SFML3 has started making things harder every day... :-/
#endif
				switch (keycode)
				{
				case SFML_KEY(Enter):     timestep_start(keystate(SHIFT) ?  10 :  1); break;
				case SFML_KEY(Backspace): timestep_start(keystate(SHIFT) ? -10 : -1); break;

				case SFML_KEY(Tab): toggle_interact_n2n(); break;

				case SFML_KEY(Insert): spawn(player_entity_ndx(),
						keystate(SHIFT) ? 100 : keystate(CTRL) ? 10 : 1); break;
				case SFML_KEY(Delete): remove_random_bodies(
						keystate(SHIFT) ? 100 : keystate(CTRL) ? 10 : 1); break;

//#586:				case SFML_KEY(F1):  keystate(SHIFT) ? quick_load_snapshot(1) : quick_save_snapshot(1); break;
				case SFML_KEY(F1): toggle_help(); break; // See also '?'!

				case SFML_KEY(F2):  keystate(SHIFT) ? quick_load_snapshot(2) : quick_save_snapshot(2); break;
				case SFML_KEY(F3):  keystate(SHIFT) ? quick_load_snapshot(3) : quick_save_snapshot(3); break;
				case SFML_KEY(F4):  keystate(SHIFT) ? quick_load_snapshot(4) : quick_save_snapshot(4); break;
				case SFML_KEY(F5):  keystate(SHIFT) ? quick_load_snapshot(5) : quick_save_snapshot(5); break;
				case SFML_KEY(F6):  keystate(SHIFT) ? quick_load_snapshot(6) : quick_save_snapshot(6); break;
				case SFML_KEY(F7):  keystate(SHIFT) ? quick_load_snapshot(7) : quick_save_snapshot(7); break;
				case SFML_KEY(F8):  keystate(SHIFT) ? quick_load_snapshot(8) : quick_save_snapshot(8); break;

				case SFML_KEY(Home): // See also Numpad5!
					if (keystate(CTRL)) {
						//!! These should be "upgraded" to "Camera/view reset"!
						//!! oon_main_camera().reset() already exists, but that's
						//!! not enough; see notes in zoom_reset() why!

						//!! Alas, pan_reset below also clears the focused entity.
						//!! It would be better to preserve it...
						//!!auto save_focused = focused_entity_ndx;

						pan_view_reset();
						//zoom_reset();

						//!!focused_entity_ndx = save_focused;
						//!! ...but a bad side-effect of that currently is implicit
						//!! automatic view-confinement -- immediately messing up
						//!! the view position if the focus object is bolting away! :)
						//!! Would be better to keep the focus obj. and just turn
						//!! off view confinement, but it can't be done yet. :-/
					} else {
 						// Select the player obj. by default (or with a dedicated modifier); same as with MouseButton!
						if (/*keystate(ALT) || */focused_entity_ndx == Entity::None)
							focused_entity_ndx = player_entity_ndx();

						assert(focused_entity_ndx != Entity::None);
						center(focused_entity_ndx);
					}
					break;

				case SFML_KEY(Numpad5): // See also Ctrl+Home!
					pan_view_reset();
					zoom_reset();
					break;

				case SFML_KEY(F12): toggle_huds();
					myco::set<myco::CheckBox>("Show HUDs", huds_active());
					break;
				case SFML_KEY(F11):
					toggle_fullscreen();
					//!! Refresh all our own (app-level) dimensions, too!
					//!! E.g. #288, and wrong .view size etc.!...
					break;

				default: // Keep GCC happy about unhandled enum values...
//LOGD << "UNHANDLED KEYPRESS: " << event.key.code;
					;
				}
				break;
			}
			case myco::event::TextInput:
			{
				const auto* textinput = event.get_if<myco::event::TextInput>();
				if (textinput->codepoint > 127) break; // non-ASCII!
				switch (static_cast<char>(textinput->codepoint)) {
				case 'g':
					myco::call<GravityModeSelector>("Gravity mode",
						[](auto* gs) { gs->selectNext(); });
					break;
//				case 'f': world().props.friction -= 0.01f; break;
//				case 'F': world().props.friction += 0.01f; break;
				case 'r': time.reversed = !time.reversed; break;
				case 't': time.scale *= 2.0f; break;
				case 'T': time.scale /= 2.0f; break;
				case 'h': toggle_pause(); break;
				case 'm': toggle_muting();
					myco::set<myco::CheckBox>("Audio: ", backend.audio.enabled);
					break;
				case 'M': toggle_music();
					myco::set<myco::CheckBox>(" - Music: ", backend.audio.music_enabled);
					break;
				case 'N': toggle_sound_fx();
					myco::set<myco::CheckBox>(" - FX: ", backend.audio.fx_enabled);
					break;
//!! #543			case 'P': fps_throttling(!fps_throttling()); break;
				case 'x': toggle_fixed_model_dt();
					myco::set<myco::CheckBox>("Fixed model Δt", cfg.fixed_model_dt_enabled);
					break;
				case '?': toggle_help(); break; // See also F1!
				}
				break;
/*!!NOT YET, AND NOT FOR SPAWN (#83):
			case myco::event::MouseButtonPressed:
				if (event.mouseButton.button == sf::Mouse::Button::Left) {
					spawn(player_entity_ndx(), 100);
				}
				break;
!!*/
			}
			case myco::event::MouseWheel:
			{
				//!! As a quick workaround for #334, we just check the GUI rect here
				//!! directly and pass the event if it belongs there...
//sf::Vector2f mouse = gui.mouse_position() + gui.getPosition();
//LOGD << "-- mouse: " << mouse.x <<", "<< mouse.y;
				if (gui.focused() || gui.hovered())
					goto process_ui_event; //!! Let the GUI also have some fun with the mouse! :) (-> #334)

				auto mousewheel = event.get_if<myco::event::MouseWheel>();
				view_control(mousewheel->delta); //! Apparently always 1 or -1...
//oon_main_view().p_alpha += (uint8_t)event.mouseWheelScroll.delta * 4;
				break;
			}

			case myco::event::MouseButtonDown:
			{
				const auto* mousepress = event.get_if<myco::event::MouseButtonDown>();
//sf::Vector2f mouse = gui.mouse_position() + gui.getPosition();
//LOGD << "-- mouse: " << event.mouseButton.x <<", "<< event.mouseButton.y;

//!!??auto vpos = oon_main_camera().screen_to_view_coord(x, y); //!!?? How the FUCK did this compile?!?!? :-o
//!!?? Where did this x,y=={-520,-391} come from?! :-ooo
//!!??cerr << "???? x = " << x << ", y = " << y << " <-- WHAT THE HELL ARE THESE??? :-ooo\n";

				//!! As a quick workaround for #334, we just check the GUI rect here
				//!! directly and pass the event if it belongs there...
				if (gui.hovered())
					goto process_ui_event; //!! Let the GUI also have some fun with the mouse! :) (-> #334)

				auto vpos = oon_main_camera().screen_to_view_coord(mousepress->position.x, mousepress->position.y);
				oon_main_camera().focus_offset = vpos;
				EntityID clicked_entity_id = Entity::None;
				if (entity_at_viewpos(oon_main_view(), vpos.x, vpos.y, &clicked_entity_id)) {
LOGI << "Click: following object #"<<clicked_entity_id<<" now...";
				} else {
LOGD << "Click: no obj.";
					assert(clicked_entity_id == Entity::None);
				}

			//!! PROCESSING SHIFT MAKES NO SENSE WHILE ALSO HAVING SHIFT+MOVE, AS THAT WOULD ALWAYS JUST KEEP
			//!! THE CURRENT OBJECT AT THE M. POINTER, MAKING IT IMPOSSIBLE TO CLICK ON ANYTHING ELSE! :)
			//!! -- EVEN IF NOTHING IS SELECTED, AS SHIFT+MOVE IS FREE PANNING!...

				// Select the clicked object, if any (unless holding CTRL!)
				/*if (!keystate(CTRL))*/ //!! Really should be ALT, but... that's the stupid shield. :)
					focused_entity_ndx = clicked_entity_id == Entity::None
					                     ? (/*keystate(ALT) ? player_entity_ndx() // Select the player with a dedicated modifier; same as with Home!
				                                                : */(keystate(SHIFT) ? focused_entity_ndx : Entity::None))
				                             : clicked_entity_id; // Entity::None if none... //!!... Whoa! :-o See updates_for_next_frame()!
/*!!
				// Pan the selected object to focus, if holding SHIFT
				//!!?? -- WHAT? There should be no panning whatsoever on a simple click!
				if (keystate(SHIFT)) {
 					// Select the player by default; same as with Home!
 					// (Unless, as above, holding CTRL!)
					if (//!keystate(CTRL) &&
					    focused_entity_ndx == Entity::None)
						focused_entity_ndx = player_entity_ndx();
//!!?? -- SHIFT should just have the usual effect of locking the scroll!
					pan_to_focus(focused_entity_ndx); //! Tolerates Entity::None!
				}
!!*/
				if (focused_entity_ndx == Entity::None)
					Note("- Nothing there. Focusing on the deep void..."); //!! Do something better than this... :)
				break;
			}

			case myco::event::MouseMoved:
			{
				const auto* mousemove = event.get_if<myco::event::MouseMoved>();

				if (gui.focused() || gui.hovered()) goto process_ui_event; //!! Let the GUI also have some fun with the mouse! :) (-> #334)

				auto vpos = oon_main_camera().screen_to_view_coord(mousemove->position.x, mousemove->position.y);

				if (keystate(SHIFT) || sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) { //!! Direct SFML use!
					// pan_to_focus(anything), essentially:
					oon_main_camera().pan_view(oon_main_camera().focus_offset - vpos);
					oon_main_camera().focus_offset = vpos;
				}

				auto entity = Entity::None;
				if (entity_at_viewpos(oon_main_view(), vpos.x, vpos.y, &entity)) {
					hovered_entity_ndx = entity;
//LOGD << "Hover: pointing to #"<<hovered_entity_ndx;
				} else {
//LOGD << "Hover: no obj.";
					hovered_entity_ndx = Entity::None;
				}
				break;
			}

			case myco::event::WindowUnfocused:
				oon_main_view().dim();
				break;

			case myco::event::WindowFocused:
				oon_main_view().undim();
				break;

			default:
process_ui_event:		// The GUI should be given a chance *before* this entire `switch`, but... -> #334: it can't swallow events!
				gui.dispatch(event);
				//!! Also, it's kinda inconsistent with this `IDLE` state assumption below!...
				//!! (Hopefully it's not even used nowadays at all though...)
				ui_event_state = UIEventState::IDLE;

				break;
			} // switch
} // process()

//----------------------------------------------------------------------------
void OONApp_sfml::draw() const // override
//!!?? Is there a nice, exact criteria by which UI rendering can be distinguished from model rendering?
{
	// Draw the model first...

//#ifdef DEBUG
	if (!controls.ShowOrbits) // -> #225
//#endif
		SFML_WINDOW().clear();

	oon_main_view().draw(); //!! Change it to draw(surface)!

/*LOGD	<< std::boolalpha
	<< "wallpapr? "<<gui.hasWallpaper() << ", "
	<< "clear bg? "<<myco::Theme::clearBackground << ", "
	<< hex << myco::Theme::bgColor.toInteger();
*/

	// Draw the UI last, as an overlay...

	// Embarrassing way to turn all (but the Help) HUDs on/off, after #644:
	ui_gebi(TimingStats).active(_ui_show_huds);
	ui_gebi(WorldData).active(_ui_show_huds);
	ui_gebi(ViewData).active(_ui_show_huds);
	ui_gebi(ObjMonitor).active(_ui_show_huds);
#ifdef DEBUG
	ui_gebi(Debug).active(_ui_show_huds);
#else
	ui_gebi(Debug).active(false);
#endif
/*!! OLD:
	if (ui_gebi(HelpPanel).active())  //!!?? This active()-chk is redundant: HUD::draw() does the same. TBD: who's boss?
	    ui_gebi(HelpPanel).draw(ctx); //!!?? "Activity" means more than just drawing, so... (Or actually both should control it?)
!!*/
	//!! A "meh, good enough" approximation of the old "help on/off" logic (which
	//!! remembered its `active` state regardless of the global _ui_show_huds flag):
	ui_gebi(HelpPanel).active(_ui_show_huds && ui_gebi(HelpPanel).active());

	gui.render();

	// Commit...
	SFML_WINDOW().display();
} // draw()


} // namespace OON
