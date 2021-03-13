#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <string>

STARTUPINFO sti = {0};
SECURITY_ATTRIBUTES sats = {0};
PROCESS_INFORMATION pi = {0};
HANDLE pipin_w, pipin_r, pipout_w, pipout_r;
BYTE buffer[8192];
DWORD writ, excode, read, available;

void ConnectToEngine(LPCWSTR path)
{
   pipin_w = pipin_r = pipout_w = pipout_r = NULL;
   sats.nLength = sizeof(sats);
   sats.bInheritHandle = TRUE;
   sats.lpSecurityDescriptor = NULL;

   CreatePipe(&pipout_r, &pipout_w, &sats, 0);
   CreatePipe(&pipin_r, &pipin_w, &sats, 0);
         
   sti.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
   sti.wShowWindow = SW_HIDE;
   sti.hStdInput = pipin_r;
   sti.hStdOutput = pipout_w;
   sti.hStdError = pipout_w;

   //if (!CreateProcess(NULL, (LPWSTR)path, NULL, NULL, TRUE, 0, NULL, NULL, &sti, &pi))
   //    std::cout << "Failed to open engine executable." << std::endl;
   if (!CreateProcess(path, NULL, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &sti, &pi))
       std::cout << "Failed to open engine executable." << std::endl;
}

std::string toUCI(std::string position) {
    std::vector<std::string> castle = { "e1g1 h1f1 ", "e8g8 h8f8 ", "e1c1 a1d1 ", "e8c8 a8d8 " };
    for (std::string c : castle) {
        size_t pos = position.find(c);
        if (pos != std::string::npos) {
            position.erase(pos + 5, 4);
        }
    }
    return position;
}

std::string getNextMove(std::string position)
{   
    std::string str;
    position = "position startpos moves " + position + "\ngo\n";

    WriteFile(pipin_w, position.c_str(), position.length(), &writ, NULL);
    Sleep(500);

    PeekNamedPipe(pipout_r, buffer, sizeof(buffer), &read, &available, NULL);
    do
    {   
        ZeroMemory(buffer, sizeof(buffer));
        if(!ReadFile(pipout_r, buffer, sizeof(buffer), &read, NULL) || !read) break;
        //buffer[read] = 0;
        buffer[read] = { 0 };
        str+=(char*)buffer;
    }
    while(read >= sizeof(buffer));

    int n = str.find("bestmove");
    if (n!=-1)
        //std::cout << str.substr(n + 9) << std::endl; // UCI castling notation contains only king movement
        return str.substr(n + 9, 5); // length 5 in case of promotion

    return "error";
}

void CloseConnection()
{
    WriteFile(pipin_w, "quit\n", 5,&writ, NULL);
    if(pipin_w != NULL) CloseHandle(pipin_w);
    if(pipin_r != NULL) CloseHandle(pipin_r);
    if(pipout_w != NULL) CloseHandle(pipout_w);
    if(pipout_r != NULL) CloseHandle(pipout_r);
    if(pi.hProcess != NULL) CloseHandle(pi.hProcess);
    if(pi.hThread != NULL) CloseHandle(pi.hThread);
}


#endif CONNECTOR_H
