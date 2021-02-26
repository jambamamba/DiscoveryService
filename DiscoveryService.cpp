#include "DiscoveryService.h"

#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <regex>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>

namespace {

const char DeviceIdFile[] = "/usr/local/etc/medussa.id";
const char ServerIpFile[] = "/tmp/medussa.ip";

std::string parseTokenValue(const std::string &line, const std::string &&token)
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
bool readDeviceId(DiscoveryData &data)
{
    FILE* fp = fopen(DeviceIdFile, "rt");
    if(!fp)
    {
        std::cout << "Failed to open " << DeviceIdFile
                  << ", errno: " << errno
                  << ", " << strerror(errno)
                  << "\n";
        return false;
    }
    char line[128] = {0};
    while(fgets(line, 128, fp))
    {
        std::string serial = parseTokenValue(line, "serialnumber");
        if(serial.size())
        {
//            std::cout << "#### serial " << serial << "\n";
            memcpy(data.m_serial_number, serial.c_str(), serial.size());
        }

        std::string name = parseTokenValue(line, "name");
        if(name.size())
        {
//            std::cout << "#### name " << name << "\n";
            memcpy(data.m_name, name.c_str(), name.size());
        }
    }
    fclose(fp);
    return true;
}
inline bool fileExists (const std::string& name)
{
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}
void saveServerIpAddress(const std::string &ip)
{
    if(fileExists(ServerIpFile))
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
}//namespace

//-------------------------------------------------------------------------------
DiscoveryService::DiscoveryService()
    : m_udp_socket(SERVER_PORT,
                   sizeof(DiscoveryData),
                   MAX_QUEUE_SIZE,
                   "DiscoveryService",
                   false,
                   [this](const uint8_t *dataFromClient, size_t dataLen, const sockaddr_in &sa){
        HandleUdpDatagram(dataFromClient, dataLen, sa);
    })
{
    readDeviceId(m_discover_message);
    std::cout << "Device Name: " << m_discover_message.m_name << ", Device Serial Number: " << m_discover_message.m_serial_number << "\n";
}
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
DiscoveryService::~DiscoveryService()
{
    m_udp_socket.Stop();
    std::cout << "dtor\n";
}
//-------------------------------------------------------------------------------
void DiscoveryService::HandleUdpDatagram(const uint8_t *dataFromClient, size_t dataLen, const sockaddr_in &sa)
{
    auto msg = reinterpret_cast<const DiscoveryData*>(dataFromClient);
    std::cout << "Received discovery datagram from " <<
                                 inet_ntoa(sa.sin_addr) << ":" << ntohs(sa.sin_port) << "\n";
    if(msg->m_version != DiscoveryData::DISCOVERY_VERSION)
    {
        std::cout << "Failed: Incompatible Discovery message received\n";
        return;
    }
    saveServerIpAddress(inet_ntoa(sa.sin_addr));
    m_udp_socket.SendToClient(sa, reinterpret_cast<char*>(&m_discover_message));
}
