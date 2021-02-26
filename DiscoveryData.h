#pragma once

#include <memory>

const static uint16_t MAX_QUEUE_SIZE = 30;
const static uint16_t SERVER_PORT = 5555;
struct DiscoveryData
{
    const static uint32_t DISCOVERY_VERSION = 1;
    unsigned char m_sync_byte[4] = {0xca, 0xfe, 0xba, 0xbe};
    uint32_t m_version = DISCOVERY_VERSION;
    char m_serial_number[32] = "?";
    char m_name[64] = "?";
};
