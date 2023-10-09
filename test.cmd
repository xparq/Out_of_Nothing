@setlocal
@if "%1" == "" ( set cycles=-1 ) else ( set cycles=%1 )

run --snd=off --interact --bodies=300 --friction=0.01 --zoom=0.1 --loopcap=%cycles%
