#include <iostream>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <memory>
#include <algorithm>

#include <ShlObj_core.h>
#pragma comment(lib, "shell32")
#include <comutil.h> //for _bstr_t (used in the string conversion)
#pragma comment(lib, "comsuppw")

const std::string LOG_TIME_FOLDER_NAME = "/Logtime";
const std::string LOG_TIME_FILE_NAME = "/log.txt";
const std::string BACKUP_FILE_NAME = "/bak.txt";
const int MAX_HOUR = 4;

enum Mode : char
{
    RecordLogIn = '1',
    RecordLogOut = '2',
    UpdateDuration = '3',
    Error = '4'
};
Mode getMode(char opt)
{
    switch (opt)
    {
    case '1':
        return RecordLogIn;
    case '2':
        return RecordLogOut;
    case '3':
        return UpdateDuration;
    default:
        return Error;
    }
}

class HourMin
{
private:
    int hour;
    int minute;

public:
    HourMin(int hour, int minute) : hour(hour), minute(minute) {}

    HourMin(const time_t &t)
    {
        tm *timeinfo = localtime(&t);
        hour = timeinfo->tm_hour;
        minute = timeinfo->tm_min;
    }

    HourMin(int seconds)
    {
        minute = seconds / 60;
        hour = minute / 60;
        minute = minute % 60;
    }

    HourMin &operator+=(const HourMin &rhs)
    {
        this->hour += rhs.hour;
        this->minute += rhs.minute;
        if (this->minute >= 60)
        {
            ++this->hour;
            this->minute -= 60;
        }
        return *this;
    }

    friend HourMin operator+(HourMin, const HourMin &);

    std::string toString()
    {
        std::string ret = std::to_string(hour); //Q: better name?
        ret.append("h");
        ret.append(std::to_string(minute));
        ret.append("m");
        return ret;
    }

    int getHour() const
    {
        return hour;
    }
};

HourMin operator+(HourMin lhs, const HourMin &rhs)
{
    lhs += rhs;
    return lhs;
}

class Session
{

private:
    time_t in, out;

public:
    Session() : in(0), out(0) {}

    Session(const time_t &in, const time_t &out) : in(in), out(out) {}

    time_t getOut() const
    {
        return out;
    }

    time_t getIn() const
    {
        return out;
    }

    void setIn(const time_t &in)
    {
        this->in = in;
    }

    void setOut(const time_t &out)
    {
        this->out = out;
    }

    HourMin getDuration() const
    {
        int seconds = difftime(out, in);
        return HourMin(seconds);
    }
};

class Day
{

private:
    static inline time_t todayMidnight = 0; //Q: How to set here?

    std::vector<Session> sessions;
    int day;

public:
    static bool IsNotInCurrentDay(const Session &s)
    {
        return s.getOut() < todayMidnight;
    }

    static void setTodaysDay()
    {
        auto curr = time(nullptr);
        auto local = localtime(&curr);
        local->tm_sec = 0;
        local->tm_min = 0;
        local->tm_hour = 0;
        todayMidnight = mktime(local);
    }

    Day(std::vector<Session> &&incoming)
    {
        sessions = std::move(incoming);

        for (auto &s : sessions)
        {
            if (s.getIn() < todayMidnight)
            {
                s.setIn(todayMidnight);
            }
        }

        sessions.erase(std::remove_if(sessions.begin(), sessions.end(), IsNotInCurrentDay), sessions.end());
    }

    HourMin getAttendance()
    {
        HourMin hm(0);
        for (auto &s : this->sessions)
        {
            hm += s.getDuration();
        }
        return hm;
    }
};

int getPath(KNOWNFOLDERID folderId, KNOWN_FOLDER_FLAG flags, std::string &path)
{
    LPWSTR wszPath = NULL;
    HRESULT hr = SHGetKnownFolderPath(folderId, KF_FLAG_CREATE, NULL, &wszPath);
    if (SUCCEEDED(hr))
    {
        _bstr_t bstrPath(wszPath);
        path = (char *)bstrPath;
    }
    CoTaskMemFree(wszPath);
    if (!SUCCEEDED(hr))
        return -1;
    return 0;
}

template <typename T>
int readAndCheckError(std::ifstream &ifs, T &var)
{
    ifs >> var;
    if (ifs.bad() || ifs.fail())
        return 1;
    return 0;
}

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        std::cerr << "Usage: LogTime <1|2|3>\n";
        return -1;
    }

    Mode mode = getMode(argv[1][0]);
    if (mode == Error || strlen(argv[1]) != 1)
    {
        std::cerr << "Usage: LogTime <1|2|3>\n";
        return -1;
    }

    std::string appDataPath;
    if (getPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, appDataPath) == -1)
    {
        std::cerr << "Could not load AppData folder path\n";
        return -2;
    }

    std::filesystem::create_directory(appDataPath + LOG_TIME_FOLDER_NAME);
    std::string fullPath = appDataPath + LOG_TIME_FOLDER_NAME + LOG_TIME_FILE_NAME;

    if (mode == RecordLogIn || mode == RecordLogOut)
    {
        time_t unix_time = time(nullptr);
        std::ofstream log(fullPath, std::ofstream::app);
        log << (char)mode << ' ' << unix_time << std::endl;
        log.close();
        return log.rdstate();
    }
    else
    {
        // Update Attendance
        std::vector<Session> sessions;

        std::ifstream log(fullPath, std::ofstream::in);
        log.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        char mode;
        time_t timestamp;
        try
        {
            log >> mode;
            log >> timestamp;

            // Ignore if a LogOut is the first entry
            if (mode == RecordLogOut && !log.eof())
            {
                log >> mode;
                log >> timestamp;
            }

            while (!log.eof())
            {
                Session s;
                s.setIn(timestamp);

                log >> mode;
                log >> timestamp;
                s.setOut(timestamp);

                sessions.push_back(s);

                log >> mode;
                log >> timestamp;
            }
        }
        catch (std::ifstream::failure e)
        {
            if (!log.eof())
            {
                std::cerr << "Exception opening/reading/closing file " << fullPath << std::endl;
                log.close();
                return -4;
            }
        }

        // Count time from last LogIn till now
        if (mode == RecordLogIn)
        {
            sessions.push_back({timestamp, time(nullptr)});
        }
        log.close();

        std::string desktopPath;
        if (getPath(FOLDERID_Desktop, KF_FLAG_DEFAULT, desktopPath) == -1)
        {
            std::cerr << "Could not load Desktop folder path\n";
            return -2;
        }

        Day::setTodaysDay();
        Day day(std::move(sessions));
        HourMin attendance = day.getAttendance();

        // Save file name so that it can be deleted afterwards
        std::string backupPath = appDataPath + LOG_TIME_FOLDER_NAME + BACKUP_FILE_NAME;
        if (std::filesystem::exists(backupPath))
        {
            std::ifstream backup(backupPath, std::ifstream::in);
            std::string lastFilePath;
            std::getline(backup, lastFilePath);
            if (backup.good() && !lastFilePath.empty())
            {
                // this returns false if file doesnt exist, so no problem
                std::filesystem::remove(lastFilePath);
            }
        } // backup closed

        std::string attendanceStr = "/" + attendance.toString();
        std::string outputPath = desktopPath + attendanceStr + ".txt";
        std::ofstream output(outputPath, std::ofstream::out); //closes on destroy
        std::ofstream backup(backupPath, std::ofstream::out);
        backup << outputPath << '\n';

        if (attendance.getHour() >= MAX_HOUR)
            output << "Take some rest!\n";
        else
            output << "Keep working!\n";
        return output.rdstate();
    }
}

// string homedrive(getenv("HOMEDRIVE"));
// string homepath(getenv("HOMEPATH"));