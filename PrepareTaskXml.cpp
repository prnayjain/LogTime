#include <fstream>
#include <lmcons.h> //max username length
#include <string>
#include <vector>
#include <windows.h>
#pragma comment(lib, "advapi32") //getUserName()

#include "Constants.h"
#include <iostream>
using namespace std;
typedef basic_ofstream<TCHAR, char_traits<TCHAR>> Ofstream;

String currentDirectory, username, computername;

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

  req = MAX_COMPUTERNAME_LENGTH + 1;
  vector<TCHAR>(req).swap(buf);
  GetComputerName(&buf[0], &req);
  computername = &buf[0];
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
  taskTemplate = replace(taskTemplate, Constants::COMPUTERNAME_PLACEHOLDER, computername);
  taskTemplate = replace(taskTemplate, Constants::USERNAME_PLACEHOLDER, username);
  taskTemplate = replace(taskTemplate, Constants::INSTALLPATH_PLACEHOLDER, installPath);
  logInXml << taskTemplate;
}

int main(int argc, char *argv[])
{
  loadEnvVars();
  generateTaskXml(Constants::LOG_OUT_TASK, currentDirectory + Constants::LOG_OUT_XML);
  generateTaskXml(Constants::LOG_IN_TASK, currentDirectory + Constants::LOG_IN_XML);
  generateTaskXml(Constants::UPDATE_TASK, currentDirectory + Constants::UPDATE_XML);
  return 0;
}
