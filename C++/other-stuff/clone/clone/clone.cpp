// clone.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <fileapi.h>
#include <vector>
#include <string>
using namespace std;

string destination;

char* clone(const char* str) {
    // Allocate memory for the new string
    char* newStr = new char[strlen(str) + 1];
    // Copy the contents of the original string to the new string
    strcpy_s(newStr, strlen(str) + 1, str);
    return newStr;
}

DWORD WINAPI thread(_In_ LPVOID lpParameter) {
	char* cPath = (char*)lpParameter;
    // Directory list
    WIN32_FIND_DATAA findDataA;
    HANDLE hFile = FindFirstFileA((string(cPath) + "*").c_str(), &findDataA);
    vector<HANDLE>threads;
    do {
        if (string(findDataA.cFileName) == "." || string(findDataA.cFileName) == "..") {
            continue;
        }
        if (findDataA.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			//cout << "Directory: " << findDataA.cFileName << endl;
            CreateDirectoryA(
                (string(destination) + findDataA.cFileName).c_str(),
                NULL);
            HANDLE cthread = CreateThread(NULL, 0, thread, (LPVOID)(clone((string(cPath) + string(findDataA.cFileName) + "\\").c_str())), 0, NULL);
			threads.push_back(cthread);
        }
        else {
            //cout << "File: " << findDataA.cFileName << endl;
            CopyFileA(
                (string(cPath) + findDataA.cFileName).c_str(),
                (destination + findDataA.cFileName).c_str(),
                FALSE
			);
        }
    }
	while (FindNextFileA(hFile, &findDataA));
    FindClose(hFile);
    for (HANDLE thread : threads) {
        WaitForSingleObject(thread, INFINITE);
        CloseHandle(thread);
	}
    delete[]cPath;
    return 0;
}


int main()
{
    cout << "Welcome to the ultimate directory cloner! (all directories end in \\)" << endl << "Source: ";
    string source;
    getline(cin, source);
    cout << "Destination: ";
    getline(cin, destination);
    cout << "Your files are being cloned...\n";
    thread((LPVOID)(clone(source.c_str())));
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
