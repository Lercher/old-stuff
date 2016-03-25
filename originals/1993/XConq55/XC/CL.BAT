@if exist vga.o del vga.o
gcc -g vga.c -o vconq *.o vesa/libgraph.a vesa/libvesa.a -lm -DMSDOS -DUNIX -DSELECT2 -DSTACK
@if errorlevel 1 goto error
emxbind vconq -acim
@if errorlevel 1 goto error
del vconq
@if errorlevel 1 goto error
gdb conq.exe
wait4key 10
mode co80
:error
