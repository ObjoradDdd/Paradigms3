#include <iostream>
#include "queueing.h"
#include <thread>
#include <chrono>
#include <cmath>

int main() {
    int groupCount, devicesPerGroup, queueSize;
    

    cout << "Enter number of groups (n > 2): ";
    cin >> groupCount;
    
    if (groupCount <= 2) {
        cout << "Number of groups must be greater than 2!\n";

    }
    
    cout << "Enter devices per group (m > 2): ";
    cin >> devicesPerGroup;
    
    if (devicesPerGroup <= 2) {
        cout << "Number of devices must be greater than 2!\n";
    }
    
    cout << "Enter queue capacity: ";
    cin >> queueSize;
    
    if (queueSize <= 0) {
        cout << "Queue capacity must be positive!\n";
    }

    auto* system = new queueing(groupCount, devicesPerGroup, queueSize);
    system->start();

    while (true) {
        system->printStatus();
        this_thread::sleep_for(chrono::seconds(4));
    }

}
