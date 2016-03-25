call emxgpp
refresh -qcr -fgcc.exe c:\emx f:\pd\emx\bin
refresh -qr -fgdb.exe c:\emx f:\pd\emx\bin
refresh -qr -fmake.exe c:\emx f:\pd\emx\bin
refresh -qr -femxbind.exe c:\emx f:\pd\emx\bin
refresh -qr -fue.exe c:\emx h:\math\bin
map ins s1:=c:\emx
vvesa.com
set VESAMODE=259
set XCONQLIB=lib
rem 800x600x256
