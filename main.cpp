#include <iostream>
#include "queueing.h"
#include <thread>
#include <chrono>
#include <cmath>

int main() {
    int groupCount, devicesPerGroup, queueSize;
    

    std::cout << "Enter number of groups (n > 2): ";
    std::cin >> groupCount;
    
    if (groupCount <= 2) {
        std::cout << "Number of groups must be greater than 2!\n";

    }
    
    std::cout << "Enter devices per group (m > 2): ";
    std::cin >> devicesPerGroup;
    
    if (devicesPerGroup <= 2) {
        std::cout << "Number of devices must be greater than 2!\n";
    }
    
    std::cout << "Enter queue capacity: ";
    std::cin >> queueSize;
    
    if (queueSize <= 0) {
        std::cout << "Queue capacity must be positive!\n";
    }

    auto* system = new queueing(groupCount, devicesPerGroup, queueSize);
    system->start();

    while (true) {
        system->printStatus();
        std::this_thread::sleep_for(std::chrono::seconds(4));
    }

}
