tdstrip refresh
lzexe refresh
del *.sym
del *.old
del *.bak
arj u ..\arc\refresh *.*
arj u c:\math\martin\c\arc\refresh
copy refresh.exe %bindir%
