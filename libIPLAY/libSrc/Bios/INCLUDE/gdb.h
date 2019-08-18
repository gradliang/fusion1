
//#define BREAK_POINT()		__asm("break 100")
#if 0
void BreakPoint(void);
//#define CHK_MALLOC(s, msg)	 if (!(s)) { DpString(msg);BREAK_POINT();}
void GdbAgentInit(void);
void BreakHandler(void);
void DpString(BYTE *);
void DpWord(DWORD);
#endif

