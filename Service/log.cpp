
#include "log.h"

void addLogMessageW(LPTSTR un)
{
	char ch[40] = "";
	WideCharToMultiByte(CP_ACP, 0, un, -1, ch, 40, 0, 0);
	fstream  fout("C:\\Users\\Danil\\Desktop\\ServiceCredProvLogFile.txt", ios::out | ios::app);
	time_t tt;
	tm* tp;
	tt = time(0);
	tp = localtime(&tt);
	fout << asctime(tp) << '\t' << ch << '\n';
	fout.close();
	return;
}
void addLogMessage(char *ch)
{
	fstream  fout("C:\\Users\\Danil\\Desktop\\ServiceCredProvLogFile.txt", ios::out | ios::app);
	time_t tt;
	tm* tp;
	tt = time(0);
	tp = localtime(&tt);
	fout << asctime(tp) << '\t' << ch << '\n';
	fout.close();
	return;
}