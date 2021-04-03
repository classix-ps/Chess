#include "connector.hpp"

Engine::Engine(LPCWSTR path) {
    enginePath = path;
}

void Engine::ConnectToEngine() {
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

    WriteFile(pipin_w, "uci\nsetoption name Skill Level value 2\n", 39, &writ, NULL);

    if (!CreateProcess(enginePath, NULL, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &sti, &pi))
        std::cout << "Failed to open engine executable." << std::endl;
}

std::string Engine::getNextMove(std::string position) {
    std::string str;
    position = "position fen " + position + "\ngo depth 10\n";

    WriteFile(pipin_w, position.c_str(), position.length(), &writ, NULL);
    Sleep(500);

    PeekNamedPipe(pipout_r, buffer, sizeof(buffer), &read, &available, NULL);
    do
    {
        ZeroMemory(buffer, sizeof(buffer));
        if (!ReadFile(pipout_r, buffer, sizeof(buffer), &read, NULL) || !read) break;
        buffer[read] = { 0 };
        str += (char*)buffer;
    }     while (read >= sizeof(buffer));

    int n = str.find("bestmove");
    if (n != -1)
        //std::cout << str.substr(n + 9) << std::endl; // UCI castling notation contains only king movement
        return str.substr(n + 9, 5); // length 5 in case of promotion

    return "error";
}

void Engine::CloseConnection() {
    WriteFile(pipin_w, "quit\n", 5, &writ, NULL);
    if (pipin_w != NULL) CloseHandle(pipin_w);
    if (pipin_r != NULL) CloseHandle(pipin_r);
    if (pipout_w != NULL) CloseHandle(pipout_w);
    if (pipout_r != NULL) CloseHandle(pipout_r);
    if (pi.hProcess != NULL) CloseHandle(pi.hProcess);
    if (pi.hThread != NULL) CloseHandle(pi.hThread);
}