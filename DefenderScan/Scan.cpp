#include <iostream>
#include <string>
#include <filesystem>
#include <windows.h>


//Adapted from https://github.com/yolofy/AvScan/blob/master/src/AvScan.WindowsDefender/WindowsDefenderScanner.cs
static int Scan(std::wstring targetFilePath, bool getSig = false)
{
    bool fileExists = std::filesystem::exists(targetFilePath);
    
    if (!fileExists)
    {
        std::cout << "[-] Couldn't open file...";
        std::cin.ignore();
        return 1;
    }
    std::cout << "[+] Opened the file... \n";
    
    std::wstring mpCmdPath = L"MpCmdRun.exe"; //L"C:\\Program Files\\Windows Defender\\MpCmdRun.exe";
    std::wstring arguments = L"-Scan -ScanType 3 -File C:\\Users\\gozuken\\Desktop\\Pryntr.ex"; //L"-Scan -ScanType 3 -File " + targetFilePath + L" -DisableRemediation -Trace -Level 0x10";

                                                           
    // Concatenate full command
    std::wstring fullCommand = mpCmdPath + L" " + arguments;
    
    
    LPCWSTR command = fullCommand.c_str();

    // Pipe to get the output from defender
    HANDLE hPipeRead, hPipeWrite;
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, false };
    if (!CreatePipe(&hPipeRead, &hPipeWrite, &sa, 0)) 
    {
        std::wcerr << L"Failed to create pipe. Error: " << GetLastError() << std::endl;
        return 1; // Or handle the error appropriately
    }
    SetHandleInformation(hPipeRead, HANDLE_FLAG_INHERIT, false);


    // Start with a pipe to get the scan results
    STARTUPINFO si = { sizeof(si) };
    si.hStdOutput = hPipeWrite;
    si.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION pi;
    if (CreateProcessW(
        NULL,              // Application name
        (LPWSTR)command,   // Command line
        NULL,              // Process security attributes
        NULL,              // Thread security attributes
        FALSE,             // Inherit handles
        NULL,  // Hide window
        NULL,              // Environment variables 
        NULL,              // Current directory
        &si,               // Startup info
        &pi                // Process info
    )) 
    {
        std::cout << "[+] Created the defender process \n" ;
        std::wcout << "[I] The full command was : " << fullCommand;
        CloseHandle(hPipeWrite);                    // No use for write pipe so we close it.


        char buffer[4096];
        DWORD bytesRead;
        std::string output;
        // Read data from pipe
        while (ReadFile(hPipeRead, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) 
        {
            output.append(buffer, bytesRead);
            std::cout << "added one...\n";
        }


        std::cout << output;


        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode;
        if (GetExitCodeProcess(pi.hProcess, &exitCode)) 
        {
            std::wcout << L"MpCmdRun exited with code: " << exitCode << std::endl;
        }
        else 
        {
            std::wcerr << L"Failed to get exit code. Error: " << GetLastError() << std::endl;
        }

        // Cleanup
        std::cout << "[+] Performing cleanup \n";
        CloseHandle(hPipeRead);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        std::cout << "[+] Done! \n";
        std::cin.ignore();
        return 0;
    }
    else 
    {
        std::cerr << "[-] Failed to create process. Error: " << GetLastError();
        std::wcout << "[I] The full command was : " << fullCommand;
        std::cin.ignore();
        return 1;
    }

}//                                   C:\Users\gozuken\Desktop\Pryntr.ex