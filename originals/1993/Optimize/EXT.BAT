if %1x==x goto ende
scan -nf e.dat @%1 $1 $2 $3 $4 $5 >x
scan -nf e.dat #%1 $0$(20)$1 >y
sort <y >>x
del y
ty x
:ende
