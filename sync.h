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
    //Custom Mutex, slightly more efficient when used in the fragment computation
    class SpinLockMutex {
        public:
            SpinLockMutex () { f_ .clear(); }
            void  lock() { while(f_.test_and_set ()){} }
            void  unlock () { f_.clear(); }
        private:
            std::atomic_flag f_;
    };	    

    const unsigned int max_hardware = std::thread::hardware_concurrency();

    //Class that handles available worker-threads according to the user-defined maximum workers
    class WorkerHandler {
        private:

            unsigned int used_workers {0};
            unsigned int max_workers {max_hardware};
            std::mutex worker_mutex;
            std::condition_variable worker_cv;

        public:

            WorkerHandler () = default;
            WorkerHandler (unsigned int max) {
                used_workers = 0;
                if (max < max_hardware)
                    max_workers = max;    
                else 
                    max_workers = max_hardware;
            }
            WorkerHandler(const WorkerHandler & wr) : max_workers(wr.max_workers) {}

            inline unsigned int getMaxWorkers () {
                return max_workers;
            }
            //Method that allows to force the maximum number of workers despite the hardware limit of physical threads
            inline void forceMaxWorkers (const unsigned int max){
                if (max > max_hardware)
                    std::cout << "WARNING! Optimal number of worker-threads: " << max_hardware << "\n";
                max_workers = max;
            }
            //Waits for a free worker slot if the used workers counter is equal to the maximum number of workers; when released increments the former atomically (worker mutex)
            inline void addWorker(){
                std::unique_lock<std::mutex> lock(worker_mutex);
                worker_cv.wait(lock, [this]{return (used_workers < max_workers);});
                used_workers++;
            }
            //Locks the worker mutex, decrements the used workers counter and notifies an addWorker() to release it via condition variable
            inline void removeWorker(){
                std::lock_guard<std::mutex> lock(worker_mutex);
                used_workers--;
                worker_cv.notify_one();
            }

    };

};
#endif // SCENE_H
