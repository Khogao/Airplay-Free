SET BonjourSdkDir=..\..\bonjour-3.0.0.10\
SET LibAlacDir=..\..\libalac-1.0.4p1\
SET LibSrcDir=..\..\libsamplerate-0.1.8\
SET OpenSslDir=..\..\openssl-1.0.1j\
SET PocoCppDir=..\..\poco-1.6.0\
SET WtlDir=..\..\wtl-8.1.12085\

SET FoobarDir=%Public%\Foobar\
SET FoobarExe=%FoobarDir%foobar2000.exe

SET WinampDir=%Public%\Winamp\
SET WinampExe=%WinampDir%winamp.exe
REM SET WinampDir=%Public%\MediaMonkey\
REM SET WinampExe=%WinampDir%MediaMonkey.exe
REM SET WinampDir=%Public%\MusicBee\
REM SET WinampExe=%WinampDir%MusicBee.exe

SET XMPlayDir=%Public%\XMPlay\
SET XMPlayExe=%XMPlayDir%xmplay.exe

CALL "_Remote Speakers output.vs2010.sln"
