#include <iostream>
#include "queueing.h"
#include <thread>
#include <chrono>
#include <cmath>

int main() {
    int groupCount, devicesPerGroup, queueSize;
    


    cin >> groupCount;
    

    cin >> devicesPerGroup;
    


    cin >> queueSize;
    

    auto* system = new queueing(groupCount, devicesPerGroup, queueSize);
    system->start();

    while (true) {
        system->printStatus();
        this_thread::sleep_for(chrono::seconds(4));
    }

}
