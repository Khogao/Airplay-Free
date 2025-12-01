ERASE /A /F /Q *.ncb *.opensdf *.sdf *.suoX *.userX
RMDIR /S /Q ipch

ERASE /F /Q rsoutput\src\view\impl\*.aps rsoutput.*.user
RMDIR /S /Q rsoutput\obj rsoutput\out

ERASE /F /Q foobar\src\*.aps
RMDIR /S /Q foobar\obj foobar\out
ERASE /F /Q "%Public%\Foobar\components\foo_apx.dll"

ERASE /F /Q winamp\src\*.aps
RMDIR /S /Q winamp\obj winamp\out
ERASE /F /Q "%Public%\Winamp\Plugins\out_apx.dll"
ERASE /F /Q "%Public%\MediaMonkey\Plugins\out_apx.dll" "%Public%\MusicBee\Plugins\out_apx.dll"

ERASE /F /Q xmplay\src\*.aps
RMDIR /S /Q xmplay\obj xmplay\out
ERASE /F /Q "%Public%\XMPlay\xmp-apx.dll"
