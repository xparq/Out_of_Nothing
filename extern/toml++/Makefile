# Must use what the stock SFML lib also uses, i.e. -MD[d]:
MSVC_CRT_LINKMODE = MD

toml-$(MSVC_CRT_LINKMODE).lib: toml.obj
	lib /nologo $** /out:$@

toml.obj: src/*cpp *.h* Makefile
	cl /nologo /c /EHsc /std:c++latest /O2 /$(MSVC_CRT_LINKMODE) \
	/Iinclude\
	/DTOML_EXCEPTIONS=0 /DTOML_ENABLE_FORMATTERS=0 \
	src/*cpp
