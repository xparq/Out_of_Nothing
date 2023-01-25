busybox:
	32-bit .exe (for max. Windows coverage), compressed (with upx -9, down from 600K).
	For heavy-duty scripted use, you 
	Starting with v1.36, one that alreaady has httpd -- but could not get CGI running, it's probably not built with CGI support!
	Does not have embedded zip, hence the separate zip.exe.
	When `tar x` fails to create symlinks ("normal" on Windows), it will at least not terminate.
	(Other (or just "most"?) upstream busybox tar errors are fatal.)


zip:
	32-bit .exe, compressed (with upx -9, down from 300K).
	Upgraded to 3.0, the latest "officially distributed" version.
	(The previously used. version 2.31 still has a buffer overflow error fixed in a later pre-3 version I couldn't find.
	(Note .)

gnumake:
	32-bit .exe built from sources (--without-guile), not compressed, as my Windows7 SE
	constantly kept deleting it, no matter which version or options of upx I tried. :-/
