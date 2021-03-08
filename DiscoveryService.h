#pragma once

#include <memory>

#include "DiscoveryData.h"
#include "SimpleUdpSocket.h"

class NetworkServerInterface;
//! \addtogroup Services
//! \{
class DiscoveryService
{
public:
    DiscoveryService(const std::string &device_id_file);
    virtual ~DiscoveryService();
    void UpdateDeviceId(const std::string &device_id_file);
protected:
    void HandleUdpDatagram(const uint8_t *dataFromClient, size_t dataLen, const sockaddr_in &sa);

    SimpleUdpSocket m_udp_socket;
    DiscoveryData m_discover_message;
};

//! \}
