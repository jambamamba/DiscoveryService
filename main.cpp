#include <iostream>
#include <unistd.h>

#include "DiscoveryClient.h"
#include "DiscoveryService.h"

int main(int argc, char** argv)
{
    DiscoveryService discovery;
    DiscoveryClient client;
    while(true)
    {
        client.Discover();
        std::cout << "Waiting\n";
        usleep(3 * 1000 * 1000);
    }
    return 0;
}
