@echo OFF
FOR /F "delims=" %%i IN ('whoami /user /fo csv /nh') DO set info=%%i
REM echo %info%
PrepareTaskXml.exe %info%
schtasks /Create /XML RecordLogIn.xml /tn RecordLogIn
del RecordLogIn.xml
schtasks /Create /XML RecordLogOut.xml /tn RecordLogOut
del RecordLogOut.xml
schtasks /Create /XML Update.xml /tn Update
del Update.xml
