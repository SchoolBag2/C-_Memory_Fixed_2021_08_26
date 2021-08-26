/****
* 
* Copyright © 2017-2021 KLS(KeLinStudio)
* auther    KLS(KeLinStudio)
*
* Contact with me:
* E-mail: 2098573804@qq.com
* QQ: 2098573804
* WeChat: ykl5689
* 
****/
#include <iostream>
#include <iomanip> 
#include <Windows.h>
#include <TlHelp32.h>
# include<unordered_set>

using namespace std;

HANDLE g_hProcess;
unordered_set<DWORD> g_HashMap;

BOOL CompareAPage(DWORD dwBaseAddr, DWORD dwValue);
BOOL FindFirst(DWORD dwValue);
BOOL FindNext(DWORD dwValue);
BOOL WriteMemory(DWORD dwAddr, DWORD dwValue);
int ShowProcess();
HANDLE FindProcess(DWORD ProcessID);

int main()
{
	ShowProcess();
	DWORD ProcessID;
	cout << "输入 ProcessID 来查询: ";
	cin >> ProcessID;
	g_hProcess = FindProcess(ProcessID);
	cout << endl << "-----------------------------" << endl;
	int val;
	cout << "输入 val = ";
	cin >> val;
	FindFirst(val);
	//ShowHash();
	cout << endl << "-----------------------------" << endl;
	size_t countAddr = 0;
	while (g_HashMap.size() > 1 && countAddr != g_HashMap.size())
	{
		countAddr = g_HashMap.size();
		cout << "输入 val = ";
		cin >> val;

		FindNext(val);
		if (FindNext == FALSE)
		{
			cout << "找不到此数据!" << endl;
			return -1;
		}
		//ShowHash();
		cout << "-----------------------------" << endl;
	}
	cout << "输入新的 val = ";
	cin >> val;
	auto it = g_HashMap.begin();
	while (it!=g_HashMap.begin())
	{
		if (WriteMemory(*it, val))
		{
			cout << "写入数据成功!" << endl;
		}
		else
		{
			cout << "写入数据失败!" << endl;
		}
		it++;
	}
	return 0;
}

BOOL CompareAPage(DWORD dwBaseAddr, DWORD dwValue)
{
	BYTE arBytes[4096];
	if (!ReadProcessMemory(g_hProcess, (LPVOID)dwBaseAddr, arBytes, 4096, NULL))
	{
		return FALSE;
	}
	DWORD* pdw;
	for (int i = 0;i < 4 * 1024 - 3;i++)
	{
		pdw = (DWORD*)&arBytes[i];
		if (pdw[0] == dwValue)
		{
			if (g_HashMap.size() >= 1024)
			{
				return FALSE;
			}
			else
			{
				g_HashMap.insert(dwBaseAddr + i);
			}
		}
	}
	return TRUE;
}

BOOL FindFirst(DWORD dwValue)
{
	const DWORD dwOneGB = 1024 * 1024 * 1024;
	const DWORD dwOnePage = 4 * 1024;
	if (g_hProcess == NULL)
	{
		return FALSE;
	}
	DWORD dwBase;
	OSVERSIONINFO vi = { sizeof(vi) };
	if (vi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	{
		dwBase = 4 * 1024 * 1024;
	}
	else
	{
		dwBase = 64 * 1024;
	}
	for (;dwBase < 2 * dwOneGB;dwBase += dwOnePage)
	{
		CompareAPage(dwBase, dwValue);
	}
	return TRUE;
}

BOOL FindNext(DWORD dwValue)
{
	BOOL bRet = FALSE;
	DWORD dwReadValue;
	unordered_set<DWORD> newSet;
	for (auto& e : g_HashMap)
	{
		if (ReadProcessMemory(g_hProcess, (LPVOID)e, &dwReadValue, sizeof(DWORD), NULL))
		{
			if (dwReadValue == dwValue)
			{
				newSet.insert(e);
				bRet = TRUE;
			}
		}
	}
	g_HashMap.swap(newSet);
	return bRet;
}

BOOL WriteMemory(DWORD dwAddr, DWORD dwValue)
{
	return WriteProcessMemory(g_hProcess, (LPVOID)dwAddr, &dwValue, sizeof(DWORD),NULL);
}

int ShowProcess()
{
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(pe32);
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		cout << "调用失败" << endl;
		return -1;
	}

	BOOL bMore = Process32First(hProcessSnap, &pe32);

	while (bMore)
	{
		wcout << "进程名称: " << setiosflags(ios::left) << setw(50) << pe32.szExeFile;
		cout << "进程ID: " << pe32.th32ParentProcessID << endl;
		bMore = Process32Next(hProcessSnap, &pe32);
	}
	CloseHandle(hProcessSnap);
	system("pause");
	return 0;
}

HANDLE FindProcess(DWORD ProcessID)
{
	PROCESSENTRY32 _pe32;
	_pe32.dwSize = sizeof(_pe32);
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	BOOL bMore = Process32First(hProcessSnap, &_pe32);
	
	while (bMore)
	{
		if (_pe32.th32ProcessID == ProcessID)
		{
			wcout << "找到 ID: " << ProcessID << "  进程名称: " << _pe32.szExeFile << endl;
			break;
		}
		bMore = Process32Next(hProcessSnap, &_pe32);
	}
	return hProcessSnap;
	//CloseHandle(hProcessSnap);
}
