
#include "log.h"
bool isfstream()
{
	fstream file;
	file.open(fstremname);
	if (!file)
		fstremname = "Service_android_provider.txt";
	return true;
}

	
void addLogMessage(LPTSTR un)
{
	fstream  fout(fstremname, ios::out | ios::app);
	char ch[255] = "";
	WideCharToMultiByte(CP_ACP, 0, un, -1, ch, 255, 0, 0);
	fout << '\t' << ch << '\n';
	fout.close();

}

void addLogMessage(char *ch)
{
	fstream  fout(fstremname, ios::out | ios::app);
	fout << '\t' << ch << '\n';
	fout.close();

}
void addLogMessage(unsigned __int64  ch)
{
	fstream  fout(fstremname, ios::out | ios::app);
	fout << '\t' << ch << '\n';
	fout.close();
}

void WriteTime()
{
	fstream  out(fstremname, ios::out | ios::app);
	SYSTEMTIME time;
	GetSystemTime(&time);
	out.width(2);
	out.unsetf(std::ios::left);
	out.setf(std::ios::right);
	out << time.wHour << ":";
	out << time.wMinute << ":";
	out.width(2);
	out << time.wSecond;
	out.width(3);
	out << ":" << time.wMilliseconds<<'\n';
	out.close();
}