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
private:
    int deviceId;
    int groupId;
    std::atomic<bool> busy;
    std::atomic<bool> running;
    std::thread workerThread;
    Request* currentRequest;
    std::mutex mtx;
    std::condition_variable cv;
    int processingTime;

public:
    Device(int id, int group)
        : deviceId(id)
        , groupId(group)
        , busy(false)
        , running(false)
        , currentRequest(nullptr)
        , processingTime(0) {}


    void start() {
        running = true;
        workerThread = std::thread([this]() {
            while (running) {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this]() {
                    return !running || (currentRequest != nullptr);
                });

                if (!running) {
                    std::cout << "Device " << deviceId << " stopping...\n";
                    break;
                }

                if (currentRequest) {
                    busy = true;

                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_int_distribution<> dis(1000, 5000);
                    processingTime = dis(gen);


                    while (processingTime > 0 && running) {
                        lock.unlock();
                        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
                        processingTime -= 100;
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
        std::cout << "Device " << deviceId << " stopped.\n";
    }

    void processRequest(Request* request) {
        std::lock_guard<std::mutex> lock(mtx);
        currentRequest = request;
        cv.notify_one();
    }


    bool isBusy() const { return busy; }
    int getGroupId() const { return groupId; }
    int getDeviceId() const { return deviceId; }
    int getRemainingTime() const { return processingTime; }
    const Request* getCurrentRequest() const { return currentRequest; }
};

#endif //DEVICE_H
