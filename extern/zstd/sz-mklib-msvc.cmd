@echo off

:: Run this in zstd-repo-root/lib!

set crt=MT
set out=msvc

mkdir %out%
cl /c /EHsc /%crt% /O2 common/*.c compress/*.c decompress/*.c -I.;decompress;compress;common /Fo%out%/ 
pushd %out%
lib /out:zstd-%crt%.lib *.obj 
popd
