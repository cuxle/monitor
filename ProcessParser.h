#ifndef __PROCESSPARSER_H__
#define __PROCESSPARSER_H__

#include <algorithm>
#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>
#include <iterator>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include "constants.h"


using namespace std;

class ProcessParser{
public:
    std::ifstream stream;
    public:
    static string getCmd(string pid);
    static vector<string> getPidList();
    static std::string getVmSize(string pid);
    static std::string getCpuPercent(string pid);
    static long int getSysUpTime();
    static std::string getProcUpTime(string pid);
    static string getProcUser(string pid);
    static vector<string> getSysCpuPercent(string coreNumber = "");
    static float getSysRamPercent();
    static string getSysKernelVersion();
    static int getNumberOfCores();
    static int getTotalThreads();
    static int getTotalNumberOfProcesses();
    static int getNumberOfRunningProcesses();
    static string getOSName();
    static std::string PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2);
    static bool isPidExisting(string pid);
};

// TODO: Define all of the above functions below:
string ProcessParser::getCmd(string pid)
{
    //cout << "getCmd:" << pid << endl;
    if (!isPidExisting(pid)) {
        return string();
    }
    string location = Path::basePath() + pid + Path::cmdPath();
    ifstream stream = Util::getStream(location);
    string line;
    getline(stream, line);

    return line;
}

vector<string> ProcessParser::getPidList()
{
    vector<string> pid_container;
    //1. open a dir /proc
    //2. return a dir if it is a dir?
    //3. is the dir name is a digit ? if sht char * name can convert to a num
    DIR *dir = opendir(Path::basePath().c_str());
    if (!dir) {
        throw std::runtime_error(std::strerror(errno));
    }
    while (dirent *drent = readdir(dir)) {
        if (drent->d_type != DT_DIR) 
            continue;
            //if all the character in the name is a digit, we refer it as a pid
        if (all_of(drent->d_name, drent->d_name + strlen(drent->d_name), [](char c){ return isdigit(c);})) {
            pid_container.push_back(drent->d_name);
        }
    }
    if (closedir(dir)) {
        throw std::runtime_error(std::strerror(errno));
    }
    return pid_container;
}
//你的问题在哪里卡住了，
//1. 不确定自己的方法可以确定找到包含vmSize的那一行，
// a. 可以一行一行读，然后find匹配的字符串，然后把后边的数字值取出，但是不确定用什么有效的方式取出

string ProcessParser::getVmSize(string pid)
{
    //cout << "getVmSize:" << pid << endl;
    //1. construct a string for location of the status file in proc
    //2. read the information in the file
    //3. find the vm info
    if (!isPidExisting(pid)) {
        return string();
    }
    string match_name = "VmSize";
    string vmSizePath = Path::basePath() + pid + Path::statusPath();
    ifstream stream = Util::getStream(vmSizePath);
    string line;
    float result = 0;
    while(getline(stream, line)) {
        if (line.compare(0, match_name.size(), match_name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result = (stof(values[1])/float(1024*1024));
            break;
        }
    }
    return to_string(result);
}

std::string ProcessParser::getCpuPercent(std::string pid)
{
    //cout << "getCpuPercent:" << pid << endl;
    if (!isPidExisting(pid)) {
        return "";
    }
    string line;
    string value;
    float result;
    ifstream stream = Util::getStream(Path::basePath() + pid + "/" + Path::statPath());
    getline(stream, line);
    string str = line;
    istringstream buf(str);
    istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end);
    //cout << "values.size:" << values.size() << endl;

    float utime = stof(ProcessParser::getProcUpTime(pid));
    float stime = stof(values[14]);
    float cutime = stof(values[15]);
    float cstime = stof(values[16]);
    float starttime = stof(values[21]);
    //cout << "alive after startime" << endl;
    float uptime = ProcessParser::getSysUpTime();
    float freq = sysconf(_SC_CLK_TCK);
    //cout << "alive after sysconf" << endl;
    float total_time = utime + stime + cutime + cstime;
    float seconds = uptime - (starttime / freq);

    result = 100.0 * ((total_time/freq)/seconds);
   // cout << "result :" << result << endl;
    return to_string(result);    
}

long int ProcessParser::getSysUpTime()
{
    string line;
    ifstream stream = Util::getStream(Path::basePath() + Path::upTimePath());
    getline(stream, line);
    istringstream buf(line);
    istream_iterator<string> beg(buf), end;
    vector<string> values(beg, end);
    return stoi(values[0]);
}

std::string ProcessParser::getProcUpTime(string pid)
{
    //cout << "getProcUpTime:" << pid << endl; 
    if (!isPidExisting(pid)) {
        //cout << "not exist pid:" << pid << endl;
        return "";
    }
    string line;
    string value;
    float result;
    ifstream stream  = Util::getStream(Path::basePath() + pid +"/" + Path::statPath());
    getline(stream, line);
    //cout << "line:" << line << endl;
    string str = line;
    istringstream buf(str);
    istream_iterator<string> beg(buf),end;
    vector<string> values(beg, end);

    return to_string(float(stof(values[13])/sysconf(_SC_CLK_TCK)));
}

string ProcessParser::getProcUser(string pid)
{
    //cout << "getProcUser:" << pid << endl;
    if (!isPidExisting(pid)) {
        return string();
    }
    DIR *dir = nullptr;
    string strPath = Path::basePath() + pid + Path::statusPath();
    string line;
    string name = "Uid:";
    string result = "";
    ifstream stream = Util::getStream(strPath);

    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result = values[1];
            //cout <<"pid" << pid << "result:" << result << endl;
            break;
        }
    }

    stream = Util::getStream("/etc/passwd");
    name = ("x:" + result);

    while (std::getline(stream, line)) {
        if (line.find(name) != std::string::npos) {
            result = line.substr(0, line.find(":"));
            return result;
        }
    }

    return "";
}

vector<string> ProcessParser::getSysCpuPercent(string coreNumber)
{
    if (coreNumber.empty()) {
        //cout << "core number : " << coreNumber << endl;
        return vector<string>();
    }
    string line;
    string name = "cpu"+coreNumber;
    ifstream stream  = Util::getStream(Path::basePath()+Path::statPath());

    while (getline(stream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            return values;
        }
    }
    return vector<string>();
}

float ProcessParser::getSysRamPercent()
{
    string line;
    string name1 = "MemAvailable:";
    string name2 = "MemFree";
    string name3 = "Buffers:";

    string value;
    int result;
    ifstream stream = Util::getStream((Path::basePath() + Path::memInfoPath()));
    float total_mem = 0;
    float free_mem = 0;
    float buffers = 0;
    while (getline(stream, line)) {
        if (total_mem != 0 && free_mem != 0 ) 
            break;
        if (line.compare(0, name1.size(), name1) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            total_mem = stof(values[1]);
        }
        if (line.compare(0, name2.size(), name2) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            free_mem = stof(values[1]);
        }
        if (line.compare(0, name3.size(), name3) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            buffers = stof(values[1]);
        }
    }
    return float(100.0  * (1 - ( free_mem / ( total_mem - buffers ))));
}

string ProcessParser::getSysKernelVersion()
{
    string line;
    string name = "Linux version";
    ifstream stream =  Util::getStream(Path::basePath() + Path::versionPath());
    while( getline(stream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            return values[2];
        }
    }
    return "";
}

int ProcessParser::getNumberOfCores()
{
    string line;
    string name = "cpu cores";
    ifstream stream = Util::getStream(Path::basePath() + "cpuinfo");
    while (getline(stream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            return stoi(values[3]);
        }
    }
    return 0;

}
int ProcessParser::getTotalThreads()
{
    string line;
    int result = 0;
    string name = "Threads:";
    vector<string> _list = ProcessParser::getPidList();
    for (int i = 0; i < _list.size(); i++) {
        string pid = _list[i];
        if (isPidExisting(pid)) {
            ifstream stream = Util::getStream(Path::basePath() + pid + Path::statusPath());
            while (getline(stream, line)) {
                if (line.compare(0, name.size(), name) == 0) {
                    istringstream buf(line);
                    istream_iterator<string> beg(buf), end;
                    vector<string> values(beg, end);
                    result += stoi(values[1]);
                    break;
                }
            }

        }
    }
    return result;

}
int ProcessParser::getTotalNumberOfProcesses()
{
    string line;
    int result = 0;
    string name = "processes";
    ifstream stream = Util::getStream(Path::basePath() + Path::statPath());
    while (getline(stream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result += stoi(values[1]);
            break;
        }
    }
    return result;
}
int ProcessParser::getNumberOfRunningProcesses()
{
    string line;
    int result = 0;
    string name = "procs_running";
    ifstream stream = Util::getStream(Path::basePath() + Path::statPath());
    while (getline(stream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
            istringstream buf(line);
            istream_iterator<string> beg(buf), end;
            vector<string> values(beg, end);
            result += stoi(values[1]);
            break;
        }
    }
    return result;
}
string ProcessParser::getOSName()
{
    string line;
    string name = "PRETTY_NAME=";
    //cout << "get out here" << endl;
    ifstream stream = Util::getStream(string("/etc/os-release"));
    //cout << "get out here" << endl;

    while (getline(stream, line)) {
        //cout << line << endl;
        if (line.compare(0, name.size(), name) == 0) {
            size_t found = line.find("=");
            found++;
            string result = line.substr(found);
            result.erase(remove(result.begin(), result.end(), '"'), result.end());
            return result;
        }
    }
    return "";

}
float get_sys_active_cpu_time(vector<string> values) {
    //cout << "get sys active cpu time :" << values.size() << endl;
    return (stof(values[S_USER]) +
            stof(values[S_NICE]) +
            stof(values[S_SYSTEM]) +
            stof(values[S_IRQ]) +
            stof(values[S_SOFTIRQ]) +
            stof(values[S_STEAL]) +
            stof(values[S_GUEST]) +
            stof(values[S_GUEST_NICE]));
}

float get_sys_idle_cpu_time(vector<string> values) 
{
    return (stof(values[S_IDLE]) + stof(values[S_IOWAIT])); 
}

std::string ProcessParser::PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2)
{
    //cout << "PrintCpuStats:" << "values1:" << values1.size() << " values2:" << values2.size() << endl; 
    //for (auto ss : values1) {
       // cout << "PrintCpuStats:" << ss << endl;
   // }
   // for (auto ss : values2) {
       // cout << "PrintCpuStats:" << ss << endl;
   // }
    float activeTime = get_sys_active_cpu_time(values2) - get_sys_active_cpu_time(values1);
    float idleTime = get_sys_idle_cpu_time(values2) - get_sys_idle_cpu_time(values1);
    float totalTime = activeTime + idleTime;
    float result = 100.0 * (activeTime / totalTime);
    //cout << "result:" << result << endl;
    return to_string(result);
}
bool ProcessParser::isPidExisting(string pid)
{
    string path = Path::basePath() + pid;
    DIR *dir;
    if (dir = opendir(path.c_str())) {
        closedir(dir);
        return true;
    } else {
        return false;
    }
}
#endif