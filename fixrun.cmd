@echo off

if not _%1_ == __ (
	set loop=%1
) else (
	set loop=500
)

wtime %~dp0test.cmd test --fixed-dt --fps-limit=0 --exit-on-finish --loop-cap=%loop% %*
