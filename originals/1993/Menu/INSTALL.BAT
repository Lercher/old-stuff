call tex &lplaing menu
tdstrip test
lzexe test
tdstrip menu
lzexe menu
del *.old
del *.bak
del *.log
del *.aux
copy menu.exe %bindir%\xmenu.exe
copy menu.dvi %softdir%\texstuff
copy test.exe %bindir%
pkzip -f ..\arc\menu *.*
