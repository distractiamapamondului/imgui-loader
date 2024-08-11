#include <iostream>
#include <string>
#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <algorithm>
#include <Psapi.h>
#include <locale>
#include <map>
#include <codecvt>
#pragma warning(disable: 4996)
#include "../xor.hpp"

bool cleaned = false;

std::vector<LPVOID> FindStringAddressesByOrder(DWORD processId, const std::string& sequence)
{
    std::vector<LPVOID> addresses;
    HANDLE processHandle = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (processHandle == NULL)
    {
        return addresses;
    }
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    LPVOID baseAddress = systemInfo.lpMinimumApplicationAddress;
    LPVOID maxAddress = systemInfo.lpMaximumApplicationAddress;
    std::string lowercaseSequence = sequence;
    std::transform(lowercaseSequence.begin(), lowercaseSequence.end(), lowercaseSequence.begin(), ::tolower);
    while (baseAddress < maxAddress)
    {
        MEMORY_BASIC_INFORMATION memoryInfo;
        if (VirtualQueryEx(processHandle, baseAddress, &memoryInfo, sizeof(memoryInfo)) == 0)
        {
            break;
        }
        if (memoryInfo.State == MEM_COMMIT && memoryInfo.Protect != PAGE_NOACCESS && memoryInfo.Protect != PAGE_GUARD)
        {
            const size_t bufferSize = memoryInfo.RegionSize;
            std::vector<char> buffer(bufferSize);
            SIZE_T bytesRead;
            if (ReadProcessMemory(processHandle, memoryInfo.BaseAddress, &buffer[0], bufferSize, &bytesRead))
            {
                std::string memoryString(buffer.begin(), buffer.end());
                std::transform(memoryString.begin(), memoryString.end(), memoryString.begin(), ::tolower);
                size_t foundIndex = memoryString.find(lowercaseSequence);
                while (foundIndex != std::string::npos)
                {
                    LPVOID stringAddress = reinterpret_cast<LPVOID>(reinterpret_cast<DWORD_PTR>(memoryInfo.BaseAddress) + foundIndex);
                    addresses.push_back(stringAddress);
                    foundIndex = memoryString.find(lowercaseSequence, foundIndex + 1);
                }
            }
        }
        baseAddress = reinterpret_cast<LPVOID>(reinterpret_cast<DWORD_PTR>(baseAddress) + memoryInfo.RegionSize);
    }
    CloseHandle(processHandle);
    return addresses;
}

std::vector<LPVOID> FindWStringAddressesByOrder(DWORD processId, const std::wstring& sequence)
{
    std::vector<LPVOID> addresses;
    HANDLE processHandle = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (processHandle == NULL)
    {
        return addresses;
    }
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    LPVOID baseAddress = systemInfo.lpMinimumApplicationAddress;
    LPVOID maxAddress = systemInfo.lpMaximumApplicationAddress;
    std::wstring lowercaseSequence = sequence;
    std::transform(lowercaseSequence.begin(), lowercaseSequence.end(), lowercaseSequence.begin(), ::towlower);
    while (baseAddress < maxAddress)
    {
        MEMORY_BASIC_INFORMATION memoryInfo;
        if (VirtualQueryEx(processHandle, baseAddress, &memoryInfo, sizeof(memoryInfo)) == 0)
        {
            break;
        }
        if (memoryInfo.State == MEM_COMMIT && memoryInfo.Protect != PAGE_NOACCESS && memoryInfo.Protect != PAGE_GUARD)
        {
            const size_t bufferSize = memoryInfo.RegionSize;
            std::vector<wchar_t> buffer(bufferSize / sizeof(wchar_t));
            SIZE_T bytesRead;
            if (ReadProcessMemory(processHandle, memoryInfo.BaseAddress, &buffer[0], bufferSize, &bytesRead))
            {
                std::wstring memoryString(buffer.begin(), buffer.end());
                std::transform(memoryString.begin(), memoryString.end(), memoryString.begin(), ::towlower);
                size_t foundIndex = memoryString.find(lowercaseSequence);
                while (foundIndex != std::wstring::npos)
                {
                    LPVOID stringAddress = reinterpret_cast<LPVOID>(reinterpret_cast<DWORD_PTR>(memoryInfo.BaseAddress) + (foundIndex * sizeof(wchar_t)));
                    addresses.push_back(stringAddress);
                    foundIndex = memoryString.find(lowercaseSequence, foundIndex + 1);
                }
            }
        }
        baseAddress = reinterpret_cast<LPVOID>(reinterpret_cast<DWORD_PTR>(baseAddress) + memoryInfo.RegionSize);
    }
    CloseHandle(processHandle);
    return addresses;
}

std::wstring StringToWideString(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

bool RemoveStringFromProcess(DWORD processId, const std::string& sequence, HANDLE processHandle)
{
    std::vector<LPVOID> addresses = FindStringAddressesByOrder(processId, sequence);
    for (const auto& address : addresses)
    {
        SIZE_T sequenceSize = sequence.size();
        std::string emptySequence(sequenceSize, '\0');
        if (WriteProcessMemory(processHandle, address, emptySequence.c_str(), sequenceSize, NULL) == 0)
        {
            // Tratamento de erro, se necessário
        }
        else
        {
            cleaned = true;
        }
    }

    std::wstring wsequence = StringToWideString(sequence);
    std::vector<LPVOID> waddresses = FindWStringAddressesByOrder(processId, wsequence);
    for (const auto& waddress : waddresses)
    {
        SIZE_T sequenceSize = wsequence.size() * sizeof(wchar_t);
        std::wstring emptyWSequence(sequenceSize / sizeof(wchar_t), L'\0');
        if (WriteProcessMemory(processHandle, waddress, emptyWSequence.c_str(), sequenceSize, NULL) == 0)
        {
            // Tratamento de erro, se necessário
        }
        else
        {
            cleaned = true;
        }
    }

    return cleaned;
}

BOOL EnablePrivilege()
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tokenPrivileges;
    LUID luid;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return FALSE;

    if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid))
    {
        CloseHandle(hToken);
        return FALSE;
    }

    tokenPrivileges.PrivilegeCount = 1;
    tokenPrivileges.Privileges[0].Luid = luid;
    tokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
    {
        CloseHandle(hToken);
        return FALSE;
    }

    CloseHandle(hToken);
    return TRUE;
}

DWORD GetProcessIdFromServiceName(const std::wstring& serviceName)
{
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (!scm)
    {
        return 0;
    }

    int bufferSize = WideCharToMultiByte(CP_ACP, 0, serviceName.c_str(), -1, NULL, 0, NULL, NULL);
    char* serviceNameA = new char[bufferSize];
    WideCharToMultiByte(CP_ACP, 0, serviceName.c_str(), -1, serviceNameA, bufferSize, NULL, NULL);

    SC_HANDLE service = OpenServiceA(scm, serviceNameA, SERVICE_QUERY_STATUS); // Use OpenServiceA
    delete[] serviceNameA;
    if (!service)
    {
        CloseServiceHandle(scm);
        return 0;
    }

    SERVICE_STATUS_PROCESS status;
    DWORD bytesNeeded;
    if (!QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, reinterpret_cast<LPBYTE>(&status), sizeof(SERVICE_STATUS_PROCESS), &bytesNeeded))
    {
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        return 0;
    }

    CloseServiceHandle(service);
    CloseServiceHandle(scm);
    return status.dwProcessId;
}

DWORD GetProcessIdFromExecutableName(const std::wstring& executableName)
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    PROCESSENTRY32W processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32W);

    if (!Process32FirstW(snapshot, &processEntry))
    {
        CloseHandle(snapshot);
        return 0;
    }

    DWORD pid = 0;
    do
    {
        std::wstring processName(processEntry.szExeFile);
        if (processName == executableName)
        {
            pid = processEntry.th32ProcessID;
            break;
        }
    } while (Process32NextW(snapshot, &processEntry));

    CloseHandle(snapshot);
    return pid;
}

std::map<std::wstring, std::vector<std::string>> serviceSequences = {
    { XorStr(L"Dnscache"), { XorStr("skript.gg"), XorStr("skript"), XorStr("keyauth"), XorStr("keyauth.win") }},
    { XorStr(L"Dps"), { XorStr("winhlp64.exe"), XorStr("!!winhlp64.exe!2023 / 12 / 13:20 : 14 : 01!0!"), XorStr("!!winhlp64.exe!2023 / 12 / 13:20 : 14 : 01!0!") }},
    { XorStr(L"Pcasvc"), { XorStr("winhlp64.exe"), XorStr("skript"), XorStr("keyauth"), XorStr("0x2e1f000"), XorStr("keyauth.win") }}
    //so adicionar mais se quiser( aqui e so servico )
};

std::map<std::wstring, std::vector<std::string>> processSequences = {
    { XorStr(L"lsass.exe"), { XorStr("skript.gg"), XorStr("skript"), XorStr("keyauth"), XorStr("keyauth.win"), XorStr("50301"), XorStr("Google Trust Services LLC1"), XorStr("1.3.6.1.4.1.11129.2.5.3"), XorStr("http://crls.pki.goog/gts1p5/bJcOhcmiYRM.crl0"), XorStr("http://pki.goog/repo/certs/gts1p5.der0!"), XorStr("GTS Root R10"), XorStr("20231226154332Z0"), XorStr("http://ocsp.pki.goog/s/gts1p5/ghf_lTR8_n801"), XorStr("GTS CA 1P5"), XorStr("??\MAILSLOT\NET\GETDC7D32A39B"), XorStr("http://ocsp.pki.goog/s/gts1p5/ghf_lTR8_n8"), XorStr("20231219164333Z"), XorStr("http://ocsp.pki.goog/gtsr100"), XorStr("!http://crl.pki.goog/gsr1/gsr1.crl0;"), XorStr("$http://pki.goog/repo/certs/gtsr1.der04"), XorStr("http://ocsp.pki.goog/gsr10)"), XorStr("+http://crls.pki.goog/gts1p5/_W4pCBAcO48.crl0"), XorStr("https://pki.goog/repository/0"), XorStr("http://pki.goog/gsr1/gsr1.crt02"), XorStr("%http://pki.goog/repo/certs/gts1p5.der0!"), XorStr("#http://crl.pki.goog/gtsr1/gtsr1.crl0M"), XorStr("http://crls.pki.goog/gts1p5/_7T6Mrwy7MI.crl0"), XorStr("http://ocsp.pki.goog/s/gts1p5/Z6slLsZWaiw01") }},
    { XorStr(L"memorycompression.exe"), { XorStr("skript.gg"), XorStr("skript"), XorStr("keyauth"), XorStr("keyauth.win") }},
    { XorStr(L"explorer.exe"), { XorStr("winhlp64.exe"), XorStr("winhlp64.dll"), XorStr(":Zone.Identifier"), XorStr("Visited:") }}
};


int stringclean()
{
    EnablePrivilege();
    for (auto& entry : processSequences)
    {
        const std::wstring& processName = entry.first;
        const std::vector<std::string>& stringsToRemove = entry.second;

        DWORD pid = GetProcessIdFromExecutableName(processName.c_str());
        if (pid != 0)
        {

            const auto processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
            if (processHandle != nullptr)
            {
                for (const std::string& sequence : stringsToRemove)
                {
                    if (RemoveStringFromProcess(pid, sequence, processHandle));
                }

                CloseHandle(processHandle);
            }
        }
    }

    for (auto& entry : serviceSequences)
    {
        const std::wstring& serviceName = entry.first;
        const std::vector<std::string>& stringsToRemove = entry.second;

        DWORD pid = GetProcessIdFromServiceName(serviceName);
        if (pid != 0)
        {

            const auto processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
            if (processHandle != nullptr)
            {
                for (const std::string& sequence : stringsToRemove)
                {
                    if (RemoveStringFromProcess(pid, sequence, processHandle));
                }

                CloseHandle(processHandle);
            }
        }
    }

    return 0;
}