ERASE /A /F /Q *.ncb *.suo *.user

ERASE /F /Q rsoutput\src\view\impl\*.aps
RMDIR /S /Q rsoutput\obj rsoutput\out
ERASE /F /Q "%SystemRoot%\System32\apxCore.dll"

ERASE /F /Q foobar\src\*.aps
RMDIR /S /Q foobar\obj foobar\out
ERASE /F /Q "%ProgramFiles%\foobar2000\components\foo_apx.dll"

ERASE /F /Q winamp\src\*.aps
RMDIR /S /Q winamp\obj winamp\out
ERASE /F /Q "%ProgramFiles%\Winamp\Plugins\out_apx.dll"
ERASE /F /Q "%ProgramFiles%\MediaMonkey\Plugins\out_apx.dll" "%ProgramFiles%\MusicBee\Plugins\out_apx.dll"

ERASE /F /Q xmplay\src\*.aps
RMDIR /S /Q xmplay\obj xmplay\out
ERASE /F /Q "%ProgramFiles%\XMPlay\xmp-apx.dll"
