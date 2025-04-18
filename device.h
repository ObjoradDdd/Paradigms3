//
// Created by Atrun on 10.04.2025.
//
#ifndef DEVICE_H
#define DEVICE_H
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <random>
#include <cmath>
#include <iostream>
#include "request.h"

using namespace std;

class Device {
    int deviceId;
    int groupId;
    atomic<bool> busy;
    atomic<bool> running;
    thread workerThread;
    Request *currentRequest;
    mutex mtx;
    condition_variable cv;
    int processingTime;

public:
    Device(int id, int group)
        : deviceId(id)
          , groupId(group)
          , busy(false)
          , running(false)
          , currentRequest(nullptr)
          , processingTime(0) {
    }


    void start() {
        running = true;
        workerThread = thread([this]() {
            while (running) {
                unique_lock<mutex> lock(mtx);
                cv.wait(lock, [this]() {
                    return !running || (currentRequest != nullptr);
                });

                if (!running) {
                    cout << "Device " << deviceId << " stopping...\n";
                    break;
                }

                if (currentRequest) {
                    busy = true;

                    random_device rd;
                    mt19937 gen(rd());
                    uniform_int_distribution<> dis(1000, 5000);
                    processingTime = dis(gen);


                    while (processingTime > 0 && running) {
                        lock.unlock();
                        this_thread::sleep_for(chrono::milliseconds(3000));
                        processingTime -= 3000;
                        lock.lock();
                    }

                    delete currentRequest;
                    currentRequest = nullptr;
                    busy = false;
                }
            }
        });
    }


    void stop() {
        running = false;
        cv.notify_one();
        if (workerThread.joinable()) {
            workerThread.join();
        }
        cout << "Device " << deviceId << " stopped.\n";
    }

    void processRequest(Request *request) {
        lock_guard<mutex> lock(mtx);
        currentRequest = request;
        cv.notify_one();
    }


    bool isBusy() const { return busy; }
    int getGroupId() const { return groupId; }
    int getDeviceId() const { return deviceId; }
    int getRemainingTime() const { return processingTime; }
    const Request *getCurrentRequest() const { return currentRequest; }
};

#endif //DEVICE_H
