#pragma once

#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <string>

class Engine {
public:
    Engine(LPCWSTR);

    void ConnectToEngine();
    std::string getNextMove(std::string);
    void CloseConnection();
private:
    LPCWSTR enginePath;
    STARTUPINFO sti = { 0 };
    SECURITY_ATTRIBUTES sats = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    HANDLE pipin_w, pipin_r, pipout_w, pipout_r;
    BYTE buffer[8192];
    DWORD writ, excode, read, available;
};