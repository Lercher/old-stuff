enum mousestatus { INIT=0, SHOW=1, HIDE=2 };

int ms_getpos(int* x,int*y);
void ms_do(enum mousestatus s);
int ms_poll(int* x,int* y,int(*abbruch)(void));
