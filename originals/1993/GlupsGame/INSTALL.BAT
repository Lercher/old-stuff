tdstrip glups
lzexe glups
del *.old
del *.sym
del *.bak
pkzip -u ..\arc\glups
if exist c:\math\martin\c\arc\glups.zip pkzip -u c:\math\martin\c\arc\glups
copy glups.exe %bindir%\games\glups
copy glups     %bindir%\games\glups
copy numbers   %bindir%\games\glups
copy glups.bau %bindir%\games\glups
copy glups.hlp %bindir%\games\glups
rem del %commondir%\glups.sco
