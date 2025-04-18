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

    vector<vector<Device*>> devices;
    priority_queue<Request*, vector<Request*>, RequestComparator> requestQueue;
    mutex queueMutex;
    condition_variable queueCV;
    atomic<bool> running;
    int queueCapacity;
    thread generatorThread;
    thread dispatcherThread;

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
        

        generatorThread = thread(&queueing::generateRequests, this);
        dispatcherThread = thread(&queueing::dispatchRequests, this);
    }


    void generateRequests() {
        random_device rd;
        mt19937 gen(rd());
        int requestId = 0;
        
        while (running) {
            unique_lock<mutex> lock(queueMutex);
            
            if (requestQueue.size() < queueCapacity) {

                uniform_int_distribution<> groupDis(0, devices.size() - 1);
                uniform_int_distribution<> prioDis(1, 3);
                
                auto* request = new Request(
                    groupDis(gen),
                    prioDis(gen),
                    ++requestId
                );
                
                requestQueue.push(request);
                queueCV.notify_one();
                
                cout << "New request: ID=" << request->requestId
                          << " Group=" << request->groupId 
                          << " Priority=" << request->priority << endl;
            }
            
            lock.unlock();

            uniform_int_distribution<> delayDis(100, 1000);
            this_thread::sleep_for(chrono::milliseconds(delayDis(gen)));
        }
    }

    void dispatchRequests() {
        while (running) {
            unique_lock<mutex> lock(queueMutex);
            
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
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }


    void printStatus() {
        lock_guard<mutex> lock(queueMutex);

        cout << "Queue: " << requestQueue.size() << "/" 
                  << queueCapacity << endl;
        
        for (size_t i = 0; i < devices.size(); ++i) {
            cout << "\nGroup " << i << ":\n";
            for (auto* device : devices[i]) {
                cout << "Device " << device->getDeviceId() << ": ";
                if (device->isBusy()) {
                    auto* req = device->getCurrentRequest();
                    cout << "Busy (Request ID=" << req->requestId
                              << ", Priority=" << req->priority 
                              << ", Time=" << device->getRemainingTime() 
                              << "ms)";
                } else {
                    cout << "Free";
                }
                cout << endl;
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
