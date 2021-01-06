#ifndef WORKER_H
#define WORKER_H
#pragma once
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <condition_variable>
#include <iostream>

namespace pipeline3D {

    class SpinLockMutex {
        public:
            SpinLockMutex () { f_ .clear(); }
            void  lock() { while(f_.test_and_set ()){} }
            void  unlock () { f_.clear(); }
        private:
            std::atomic_flag f_;
    };	    

    const unsigned int max_hardware = std::thread::hardware_concurrency();
    class Worker {
        private:
            std::atomic<unsigned int> used_workers {0};
            unsigned int max_workers {max_hardware};
            std::mutex worker_mutex;
            std::condition_variable worker_cv;

        public:

            std::mutex public_mutex;
            Worker () = default;
            Worker (unsigned int max) {
                used_workers = 0;
                if (max < max_hardware)
                    max_workers = max;    
                else 
                    max_workers = max_hardware;
            }
            Worker(const Worker & wr) : max_workers(wr.max_workers) {}

            inline unsigned int getMaxWorkers () {
                return max_workers;
            }
            inline void forceMaxWorkers (const unsigned int w){
                if (w > max_hardware)
                    std::cout << "WARNING! OPTIMAL NUMBER OF WORKER-THREADS: " << max_hardware << "\n";
                max_workers = w;
            }

            inline void addWorker(){
                std::unique_lock<std::mutex> lock(worker_mutex);
                worker_cv.wait(lock, [this]{return (used_workers < max_workers);});
                used_workers.fetch_add(1, std::memory_order_consume);
            }
            inline void removeWorker(){
                std::lock_guard<std::mutex> lock(worker_mutex);
                used_workers.fetch_sub(1, std::memory_order_release);
                worker_cv.notify_one();
            }

    };

};
#endif // SCENE_H
