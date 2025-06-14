/*
Particle Emitter for Skytyping
(Used e.g. as the "rasterizer" for the "Lorem Ipsum Drive")

NOTES:

   -	The output after each "print" (emit) action is a new vertical scanline
	consisting of n < NOZZLE_COUNT active pixels, with normalized ("abstract")
	coordinates of { x = NOZZLE_X, y ∈ [-V_SCALE/2, V_SCALE/2] }.

   -	A valid output is only available if emit() has returned true. (I.e. also
	implying that there's no initial valid output before first calling emit()!)

TODO:

! UTF-8... (E.g. https://stackoverflow.com/a/3029872/1479945 and https://stackoverflow.com/a/2954379/1479945, on the same page.)
! Script direction (RTL, vertical etc.)
! Rotation
! Actually do something with Lorem Ipsum...
*/
#ifndef _CN87BG78G3GB7878FBCY3N7B47Y358TYDJK78478B45_
#define _CN87BG78G3GB7878FBCY3N7B47Y358TYDJK78478B45_

#include "Model/Emitter.hpp"

#include "sz/lang/.hh" // AUTO_CONST

#include <string>
#include <cassert>
#include <cstdint>

//#include <iostream> //!!DEBUG

namespace Model {

#include "_testfont_7x10.h"

class SkyPrint //!!?? : public Emitter //! The base is NOT the `emitter` object, which is actually the parent of this!
{
public://!!for now...
	AUTO_CONST V_DUP = true;
	AUTO_CONST NOZZLE_COUNT = (uint8_t) font_height * (V_DUP ? 2u:1u);
	//!! These also used to be const, but...:
	mutable float V_SCALE = 4.0f;   // Vert. text magnification
	mutable float H_SCALE = 1.0f;   //!! UNUSED YET! (Mostly useless anyway; depends on call frequency etc.)
	mutable float NOZZLE_X = -1.0f; // Abstract X position of the printed scanline.

	using Vect = Math::V2<Phys::NumType>;
	Vect nozzles[NOZZLE_COUNT];
		//!! Nozzle positions should be precalculated and (generally) fixed though, and
		//!! only the on/off state + "ink color" should be calculated each time!
		//!! (Not counting various distortion effects, like italic etc. -> Rich text support!)
		//!!
		//!! But that would require a) another data structure to hold the (output) indexes of
		//!! active pixels, b) force the caller decode that, and c) would make it difficult to
		//!! add transformations that modify the pixel count (like e.g. V_DUP already does!).

	std::string banner_str;
	const char* banner;
	bool repeat = true; //!! If no `repeat`, and "nothing left to say", the drive should stop producing thrust! :)
	                    //!! But, then again, the Lorem Ipsum Drive should never run out of "ideas"!...
	                    //!! Only other flavors -- actual "chat engines" -- should, with "real" text that actually isn't infinite.)

	unsigned short char_index;
	unsigned short glyph_index;
	uint8_t        vline_index; // Vertical line of current glyph (from left, 0-based)
	uint8_t        active_pixels; // # of pixels (so far) in the current "print job" (normally a single scan-line)

	char current_codepoint; //!! Well, yeah, it's just ASCII for now...

public:
	explicit SkyPrint(std::string text, bool loop = true)
	{
		set_text(text, loop); // Also resets the "volatile" work state!
	}

	void set_text(std::string text, bool loop = true)
	{
		banner_str = text;
		banner = banner_str.c_str();
		repeat = loop;

		_reset_print_state();
	}

	void _reset_print_state()
	{
		char_index = 0;

		if (banner && *banner)
			_fetch_next_glyph();
	}

	char _fetch_next_glyph()
	{
		assert(banner);
		assert(*banner);
 		current_codepoint = banner[char_index];
		if (current_codepoint) { //!! Without this check, the font stuff would be required to handle 0 gracefully.
			++char_index;    // Only inc. if not already at the end!
			glyph_index = font_glyph_index(current_codepoint); //!! Jesus FUCK! Like in the good old days, huh?... :) Ugh...
			_glyph_start(/*glyph_index*/); //! Note how this is not called for the EOS case,
		}
		return current_codepoint;
	}

	void _glyph_start() { vline_index = 0; }

	bool _glyph_finished() const { return vline_index == font_width; }

	[[nodiscard]] // Nozzle/pixel (output) data is likely invalid, when false!
	bool emit(/*!!...!!*/) // False means nothing came out of the engine... (So e.g. ' ' would be false, too.)
	{
		if (!banner || !*banner)
			return false;

		if (_glyph_finished()) {
			if (!_fetch_next_glyph()) { // Ended? Wrap around, if allowed, or stop...
				if (repeat) _reset_print_state();
				else return false;
			}
		}

		assert(vline_index >= 0 && vline_index < font_width);
		unsigned h_bitmask = 1u << (font_width-1 - vline_index);

		// Collect the pixels of the current vertical scan line...
		active_pixels = 0;
		for (unsigned hline_index = 0; hline_index < font_height; ++hline_index) {
			if (current_codepoint != ' ' &&
			    font[glyph_index * font_height + hline_index] & h_bitmask) {
				assert(active_pixels < NOZZLE_COUNT);
				nozzles[active_pixels++] =
					{ NOZZLE_X, -(hline_index * V_SCALE/font_height - V_SCALE/2) };

			  if (V_DUP) { // Add a set of interleaving pixels:
				assert(active_pixels < NOZZLE_COUNT);
				nozzles[active_pixels++] =
					{ NOZZLE_X * 1.2f, -(hline_index * V_SCALE/font_height - V_SCALE/2 + V_SCALE/2 / font_height) };
			  }
			}
		}

		++vline_index;

		return true;
	}
};

} // namespace Model

#endif // _CN87BG78G3GB7878FBCY3N7B47Y358TYDJK78478B45_