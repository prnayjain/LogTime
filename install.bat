@echo OFF
FOR /F "delims=" %%i IN ('whoami /user /fo csv /nh') DO set info=%%i
cd %~dp0
PrepareTaskXml.exe %info%
schtasks /delete /tn RecordLogIn /f
schtasks /create /XML RecordLogIn.xml /tn RecordLogIn
del RecordLogIn.xml
schtasks /delete /tn RecordLogOut /f
schtasks /create /XML RecordLogOut.xml /tn RecordLogOut
del RecordLogOut.xml
schtasks /delete /tn Update /f
schtasks /create /XML Update.xml /tn Update
del Update.xml
