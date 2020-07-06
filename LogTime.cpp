#include <iostream>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>
#include <filesystem>

#include <ShlObj_core.h>
#pragma comment(lib, "shell32")
#include <comutil.h> //for _bstr_t (used in the string conversion)
#pragma comment(lib, "comsuppw")

static const std::string LogTimeFolderPath = "/Logtime";
static const std::string LogTimeFilePath = "/log.txt";

int main(int argc, char *argv[]) {

    if (argc != 2)
            return -1;

    std::string opt(argv[1]);
    if (opt != "1" && opt != "2")
        return -2;

    std::string appDataPath;
    LPWSTR wszPath = NULL;
    HRESULT hr;
    hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &wszPath);
    if (SUCCEEDED(hr)) {
        _bstr_t bstrPath(wszPath);
        appDataPath = (char*)bstrPath;
    }
    CoTaskMemFree(wszPath);
    if (!SUCCEEDED(hr)) return -3;

    std::filesystem::create_directory(appDataPath + LogTimeFolderPath);

    std::string fullPath = appDataPath + LogTimeFolderPath + LogTimeFilePath;
    time_t unix_time = time(nullptr);
    std::ofstream log(fullPath, std::ofstream::app); //closes on destroy
    if (opt == "1")
        log << "Log In: ";
    else if (opt == "2")
        log << "Log Out: ";
    else return -4;
    log << unix_time << std::endl;

    return log.rdstate();
}

// string homedrive(getenv("HOMEDRIVE"));
// string homepath(getenv("HOMEPATH"));