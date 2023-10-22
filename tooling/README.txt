busybox:
	32-bit .exe (for max. Windows coverage), compressed (with upx -9, down from 600K).
	Starting with v1.37, one with pdpmake! (So, gnumake is no longer needed in this toolkit.)
	(It also has httpd -- but alas, it's not built with CGI support! :-/ )
	Does not have embedded zip (only unzip), hence the separate zip.exe.
	When `tar x` fails to create symlinks ("normal" on Windows), it will at least not terminate.
	(Other (or just "most"?) upstream busybox tar errors are fatal.)

zip:
	32-bit .exe, compressed (with upx -9, down from 300K).
	Upgraded to 3.0, the latest "officially distributed" version.
	(The previously used version 2.31 still has a buffer overflow error fixed
	in a later pre-3 version I couldn't find.)
