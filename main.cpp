#include <iostream>
#include <unistd.h>

#include "DiscoveryClient.h"
#include "DiscoveryService.h"

int main(int argc, char** argv)
{
    std::string device_id_file("/usr/local/etc/medussa.id");
    DiscoveryService discovery(device_id_file);
    DiscoveryClient client(device_id_file);
    while(true)
    {
        client.Discover();
        std::cout << "Waiting\n";
        usleep(3 * 1000 * 1000);
    }
    return 0;
}
