Compiler = bcc -Fc -d -c -ms -W \
  -IC:\BC\INCLUDE 

ALL : MNOTIFY.exe MNOTIFY.hlp

MNOTIFY.obj : MNOTIFY.c MNOTIFY.h
   $(Compiler) MNOTIFY.c

MNOTIFY.res: MNOTIFY.rc MNOTIFY.h appl_ico.ico mnotify.mnu
   rc -r -IC:\BC\INCLUDE MNOTIFY.rc

MNOTIFY.exe: MNOTIFY.obj MNOTIFY.res MNOTIFY.def 
    tlink  /Twe /n /c /LC:\BC\LIB @MNOTIFY.lnk
    rc MNOTIFY.res MNOTIFY.exe

mnotify.hlp : mnotify.hpj hilfe.rtf
    hc31 mnotify
