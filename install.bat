PrepareTaskXml.exe "%CD%"
schtasks /Create /XML RecordLogIn.xml /tn RecordLogIn
del RecordLogIn.xml
schtasks /Create /XML RecordLogOut.xml /tn RecordLogOut
del RecordLogOut.xml
schtasks /Create /XML Update.xml /tn Update
del Update.xml
