#ifndef WORKER_H
#define WORKER_H
#pragma once
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <condition_variable>
#include<iostream>
#include <chrono>


namespace pipeline3D {
enum Parallelization {None = 0, Obj = 1, Triang = 2, Scanline =  3, Frag = 4};
short int a;
class Worker {
    private:
        std::atomic<unsigned int> used_workers_ ;
        unsigned int max_workers_ = std::thread::hardware_concurrency();
        std::mutex m_;
        std::condition_variable cv_;

    public:
        std::mutex public_mutex;
        Worker () = default;
        Worker (unsigned int max) {
            used_workers_ = 0;
            if (max < std::thread::hardware_concurrency())
                max_workers_ = max;    
            else 
                max_workers_ = std::thread::hardware_concurrency();
        }
        Worker(const Worker & wr) :  used_workers_(0), max_workers_(wr.max_workers_) {}

        inline unsigned int getMaxWorkers () {
            return max_workers_;
        }
        inline void setMaxWorkers (const unsigned int w){
            if (w < std::thread::hardware_concurrency())
                max_workers_ = w;    
            else 
                max_workers_ = std::thread::hardware_concurrency();           
        }

        inline void addWorker(){
            std::unique_lock<std::mutex> lock(m_);
            cv_.wait(lock, [this]{return (used_workers_ < max_workers_);});
            used_workers_++;
            // std::cout << "ADDED WORKER, used : " << used_workers_ << "\n";
        }
        inline void removeWorker(){
            used_workers_--;
            cv_.notify_one();
            // std::cout << "REMOVED WORKER, used : " << used_workers_ << "\n";
        }

};

};
#endif // SCENE_H
