#include "Utils.h"

#include <iostream>
#include <sys/stat.h>
#include <regex>

#include "DiscoveryData.h"

const char ServerIpFile[] = "/tmp/medussa.ip";

namespace Utils
{

std::string ParseTokenValue(const std::string &line, const std::string &&token)
{
    std::string str(line);
    std::string rxstr = token + ":([a-zA-Z0-9-`~!@#$%^&*()[{\\]};'\",.<>\\\\?|\\/]*)";
    std::regex rx(rxstr);

    std::smatch matches;
    std::regex_search(str, matches, rx);
    int idx = 0;
    for(auto match: matches)
    {
        if(idx == 1) { return match; }
        idx++;
    }
    return "";
}
bool ReadDeviceId(DiscoveryData &data, const std::string &device_id_file)
{
    FILE* fp = fopen(device_id_file.c_str(), "rt");
    if(!fp)
    {
        std::cout << "Failed to open " << device_id_file.c_str()
                  << ", errno: " << errno
                  << ", " << strerror(errno)
                  << "\n";
        return false;
    }
    char line[128] = {0};
    while(fgets(line, 128, fp))
    {
        std::string serial = Utils::ParseTokenValue(line, "serialnumber");
        if(serial.size())
        {
//            std::cout << "#### serial " << serial << "\n";
            memcpy(data.m_serial_number, serial.c_str(), serial.size());
        }

        std::string name = Utils::ParseTokenValue(line, "name");
        if(name.size())
        {
//            std::cout << "#### name " << name << "\n";
            memcpy(data.m_name, name.c_str(), name.size());
        }
    }
    fclose(fp);
    return true;
}
inline bool FileExists (const std::string& name)
{
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}
void SaveServerIpAddress(const std::string &ip)
{
    if(Utils::FileExists(ServerIpFile))
    {
        return;
    }
    FILE* fp = fopen(ServerIpFile, "wt");
    if(!fp)
    {
        std::cout << "Failed to open " << ServerIpFile
                  << ", errno: " << errno
                  << ", " << strerror(errno)
                  << "\n";
        return;
    }
    fprintf(fp, "%s", ip.c_str());
    fclose(fp);
}
}//Utils
