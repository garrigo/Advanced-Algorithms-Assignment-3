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

// https://www.codeguru.com/cpp/sample_chapter/article.php/c13533/Why-Too-Many-Threads-Hurts-Performance-and-What-to-do-About-It.htm
// https://www.reddit.com/r/cpp_questions/comments/5ih1g8/how_do_i_limit_the_number_of_threads_used_by/
namespace pipeline3D {


class SpinLockMutex {
    public:
        SpinLockMutex () { f_ .clear(); }
        void  lock() { while(f_.test_and_set ()){} }
        void  unlock () { f_.clear(); }
    private:
        std::atomic_flag f_;
};	    
class Worker {
    private:
        std::atomic<unsigned int> used_workers_{0} ;
        unsigned int max_workers_ = std::thread::hardware_concurrency();
        std::mutex m_;
        std::condition_variable cv_;

    public:

        std::mutex public_mutex;
        Worker () = default;
        Worker (unsigned int max) {
            if (max < std::thread::hardware_concurrency())
                max_workers_ = max;    
            else 
                max_workers_ = std::thread::hardware_concurrency();
        }
        Worker(const Worker & wr) : max_workers_(wr.max_workers_) {}

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
            // used_workers_++;
            used_workers_.fetch_add(1, std::memory_order_release);
            // std::cout << "ADDED WORKER, used : " << used_workers_ << "\n";
        }
        inline void removeWorker(){
            // used_workers_--;
            used_workers_.fetch_sub(1, std::memory_order_release);
            cv_.notify_one();
            // std::cout << "REMOVED WORKER, used : " << used_workers_ << "\n";
        }

};

};
#endif // SCENE_H
