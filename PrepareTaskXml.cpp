#define SECURITY_WIN32
#include <fstream>
#include <lmcons.h> //max username length
#include <string>
#include <vector>
#include <tchar.h>
#include <windows.h>
#include <security.h>
#include <iostream>
#pragma comment(lib, "advapi32") //getUserName()

#pragma comment(lib, "secur32") //getUserNameEx()

#pragma comment(lib, "kernel32") //getLastError()

#include "Constants.h"
#include <iostream>
using namespace std;
typedef basic_ofstream<TCHAR, char_traits<TCHAR>> Ofstream;

String currentDirectory, username, sid;

void loadEnvVars()
{
  DWORD req = GetCurrentDirectory(0, NULL);
  vector<TCHAR> buf(req + 1);
  GetCurrentDirectory(req + 1, &buf[0]);
  currentDirectory = &buf[0];

  req = UNLEN + 1;
  vector<TCHAR>(req).swap(buf);
  GetUserName(&buf[0], &req);
  username = &buf[0];

  // ULONG req2 = 1;
  // vector<TCHAR>(1).swap(buf);
  // GetUserNameEx(NameFullyQualifiedDN, &buf[0], &req2);
  // DWORD err = GetLastError();
  // cout << err << endl;
  // vector<TCHAR> fqdn(req2);
  // GetUserNameEx(NameFullyQualifiedDN, &fqdn[0], &req2);
  // vector<TCHAR> refDomain(500);
  // DWORD domainReq;
  // PSID_NAME_USE useless;
  // LookupAccountName(NULL, &fqdn[0], NULL, &req, NULL, &domainReq, useless);
  // vector<TCHAR> domainName(domainReq);
  // vector<TCHAR>(req).swap(buf);
  // LookupAccountName(NULL, &fqdn[0], &buf[0], &req, &domainName[0], &domainReq, useless);
  // sid = &buf[0];
}

std::ostream &operator<<(std::ostream &os, const std::basic_string<TCHAR> &str)
{
  for (auto ch : str)
    os << static_cast<char>(ch);
  return os;
}

String replace(String &task, const String &placeholder, const String &replacement)
{
  auto pos = task.find(placeholder);
  if (pos == string::npos)
    return task;
  String replaced = task.substr(0, pos);
  replaced += replacement;
  replaced += task.substr(pos + placeholder.size());
  return replaced;
}

void generateTaskXml(String taskTemplate, String fileName)
{
  String installPath = currentDirectory + Constants::BINARY_NAME;
  Ofstream logInXml(fileName, Ofstream::out);
  taskTemplate = replace(taskTemplate, Constants::SID_PLACEHOLDER, sid);
  taskTemplate = replace(taskTemplate, Constants::USERNAME_PLACEHOLDER, username);
  taskTemplate = replace(taskTemplate, Constants::INSTALLPATH_PLACEHOLDER, installPath);
  logInXml << taskTemplate;
}

int _tmain(int argc, TCHAR* argv[])
{
  if (argc != 2) return 1;
  sid = argv[1];
  loadEnvVars();
  generateTaskXml(Constants::LOG_OUT_TASK, currentDirectory + Constants::LOG_OUT_XML);
  generateTaskXml(Constants::LOG_IN_TASK, currentDirectory + Constants::LOG_IN_XML);
  generateTaskXml(Constants::UPDATE_TASK, currentDirectory + Constants::UPDATE_XML);
  return 0;
}
