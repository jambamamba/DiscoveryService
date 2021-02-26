#pragma once

#include <string>

struct DiscoveryData;

namespace Utils
{
std::string ParseTokenValue(const std::string &line, const std::string &&token);
bool ReadDeviceId(DiscoveryData &data);
inline bool FileExists (const std::string& name);
void SaveServerIpAddress(const std::string &ip);

};
