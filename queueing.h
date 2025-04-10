#ifndef QUEUEING_H
#define QUEUEING_H
#include <queue>
#include <vector>
#include "device.h"
#include "request.h"

using namespace std;

class queueing {
private:
    struct RequestComparator {
        bool operator()(const Request* a, const Request* b) {
            return a->priority < b->priority;
        }
    };

    std::vector<std::vector<Device*>> devices;
    std::priority_queue<Request*, std::vector<Request*>, RequestComparator> requestQueue;
    std::mutex queueMutex;
    std::condition_variable queueCV;
    std::atomic<bool> running;
    int queueCapacity;
    std::thread generatorThread;
    std::thread dispatcherThread;

public:

    queueing(int groupCount, int devicesPerGroup, int queueSize)
        : queueCapacity(queueSize)
        , running(false) {
        devices.resize(groupCount);
        for (int i = 0; i < groupCount; ++i) {
            for (int j = 0; j < devicesPerGroup; ++j) {
                devices[i].push_back(new Device(j, i));
            }
        }
    }


    void start() {
        running = true;
        

        for (auto& group : devices) {
            for (auto* device : group) {
                device->start();
            }
        }
        

        generatorThread = std::thread(&queueing::generateRequests, this);
        dispatcherThread = std::thread(&queueing::dispatchRequests, this);
    }


    void generateRequests() {
        std::random_device rd;
        std::mt19937 gen(rd());
        int requestId = 0;
        
        while (running) {
            std::unique_lock<std::mutex> lock(queueMutex);
            
            if (requestQueue.size() < queueCapacity) {

                std::uniform_int_distribution<> groupDis(0, devices.size() - 1);
                std::uniform_int_distribution<> prioDis(1, 3);
                
                auto* request = new Request(
                    groupDis(gen),
                    prioDis(gen),
                    ++requestId
                );
                
                requestQueue.push(request);
                queueCV.notify_one();
                
                std::cout << "New request: ID=" << request->requestId
                          << " Group=" << request->groupId 
                          << " Priority=" << request->priority << std::endl;
            }
            
            lock.unlock();

            std::uniform_int_distribution<> delayDis(100, 1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(delayDis(gen)));
        }
    }

    void dispatchRequests() {
        while (running) {
            std::unique_lock<std::mutex> lock(queueMutex);
            
            queueCV.wait(lock, [this]() {
                return !running || !requestQueue.empty();
            });
            
            if (!running) break;
            
            if (!requestQueue.empty()) {
                auto* request = requestQueue.top();

                for (auto* device : devices[request->groupId]) {
                    if (!device->isBusy()) {
                        requestQueue.pop();
                        device->processRequest(request);
                        break;
                    }
                }
            }
            
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }


    void printStatus() {
        std::lock_guard<std::mutex> lock(queueMutex);

        std::cout << "Queue: " << requestQueue.size() << "/" 
                  << queueCapacity << std::endl;
        
        for (size_t i = 0; i < devices.size(); ++i) {
            std::cout << "\nGroup " << i << ":\n";
            for (auto* device : devices[i]) {
                std::cout << "Device " << device->getDeviceId() << ": ";
                if (device->isBusy()) {
                    auto* req = device->getCurrentRequest();
                    std::cout << "Busy (Request ID=" << req->requestId
                              << ", Priority=" << req->priority 
                              << ", Time=" << device->getRemainingTime() 
                              << "ms)";
                } else {
                    std::cout << "Free";
                }
                std::cout << std::endl;
            }
        }
    }

    void stop() {
        running = false;
        
        if (generatorThread.joinable()) {
            generatorThread.join();
        }
        if (dispatcherThread.joinable()) {
            dispatcherThread.join();
        }
        
        for (auto& group : devices) {
            for (auto* device : group) {
                device->stop();
            }
        }
    }


    ~queueing() {
        stop();

        for (auto& group : devices) {
            for (auto* device : group) {
                delete device;
            }
        }
        while (!requestQueue.empty()) {
            delete requestQueue.top();
            requestQueue.pop();
        }
    }
};


#endif //QUEUEING_H
