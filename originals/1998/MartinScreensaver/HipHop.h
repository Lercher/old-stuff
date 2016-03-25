// HipHop.h - Includefile mit der Threaddefinition

DWORD WINAPI ThreadProc(LPVOID lpParameter);
void ChangeBrush(void);
void ChangeParams(void);
void ChangeWidth(void);

extern BOOL bThreadContinue;
