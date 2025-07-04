//!!
//!! Only for the transition; stuff is still needed from there!
//!!
#include "Engine/SimApp.hpp"

#include "Engine/RuntimeContext.hpp"

#include "Engine/diag/Error.hpp"
#include "Engine/diag/Log.hpp"


namespace Szim {

RuntimeContext::RuntimeContext(int argc, char** argv)

	: args(argc, argv, {
		// Long options with 1 param (only those with 1?) don't need to be defined.
		// Short ones do, unfortunately (they're predicates by default, and don't have the '=' syntax to disambiguate when aren't):
		{"C", 1}, // Synonym for "cfg"
	})
	// Load & fixup the SimApp config...
	, cfg(
	/*
		args("cfg").empty()
		? args("C").empty() ? DEFAULT_CFG_FILE // `... ? ""` would use .defaults instead
				: args("C")
		: args("cfg")
	*/
	///* For a pedantic warning:
		args("cfg").empty()
		? args("C").empty()
			? DEFAULT_CFG_FILE
			: args("C")
		: args("C").empty()
			? args("cfg")
			: (Warning("Both -C and --cfg have been specified; ignoring \"-C "s + args("C")),
			args("C"))
	//*/
	, args
	, //! Note: this default config here is pretty redundant, basically only useful for debugging,
		//! as the cfg ctor takes care of the defaults anyway...:
		R"(
		app_name = "Don't put the app name in the config, FFS! ;) "
		[appearance]
		window_title = "Szim-Engine <Initialized with hardcoded defaults!>"
		)"
	) // cfg

	// Bootstrap the backend...
	, backend(SFML_Backend::use(cfg))
	// Init the GUI...
	, gui(((SFML_Backend&)backend).SFML_window(),
		{
			.basePath = cfg.asset_dir.c_str(), // Trailing / ensured by the cfg. fixup!
			.textureFile = "gui/texture.png",
			.bgColor = sfw::Color(cfg.default_bg_hexcolor),
			.fontFile = cfg.default_font_file.c_str(),
		},
		false // Don't manage the window
	)
	//!!	, renderer{View/*!!Not really?...*/::Renderer_SFML::create(main_window())}

	//!! Adapt SessionManager to take a RuntimeContext:
	//!!	, session(*this/*!!, args("session")!!*/)

{
}

} // namespace Szim
