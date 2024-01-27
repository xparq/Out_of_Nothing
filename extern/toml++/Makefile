# Optional, if you want to precompile TOML++, instead of using it in header-only mode

# Must use the same as what the SFML lib (and the others thereof)
# also also use (i.e. MD[d]/MT[d] etc.):
#MSVC_CRT_LINKMODE = MD
MSVC_CRT_LINKMODE = MT

toml-$(MSVC_CRT_LINKMODE).lib: toml.obj
	lib /nologo $** /out:$@

toml.obj: src/*cpp *.h* Makefile
	cl /nologo /c /EHsc /std:c++latest /O2 /$(MSVC_CRT_LINKMODE) \
	/Iinclude\
	/DTOML_EXCEPTIONS=0 /DTOML_ENABLE_FORMATTERS=0 \
	src/*cpp
