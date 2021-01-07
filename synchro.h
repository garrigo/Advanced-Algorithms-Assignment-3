#ifndef SYNCHRO_H
#define SYNCHRO_H
#include<mutex>
#include<condition_variable>
#include<atomic>
#include<thread>
#include<iostream>

class Synchronizer{
    private:
        std::condition_variable cv_;
        unsigned short int nThreads_ = std::thread::hardware_concurrency();
        std::atomic<unsigned short int> usedThreads_{0};
        bool canAdd = true;

    public:
        Synchronizer() = default;

        Synchronizer(unsigned short int nThreads){
            if (nThreads < nThreads_)
                nThreads_ = nThreads;
        }
        Synchronizer(const Synchronizer & synch) : nThreads_(synch.nThreads_) {}

        inline void setNThreads(unsigned short int nThreads){this->nThreads_=nThreads;}

        inline void adder(){
            std::unique_lock<std::mutex> lock (mtx_);
			cv_.wait(lock, [this](){return canAdd;});
           
            // add 1 to used threads number in an atomic way
            usedThreads_.fetch_add(1, std::memory_order_release);
            canAdd = (nThreads_ > usedThreads_);

            //std::cout << "New thread correctly loaded."<<std::endl;

        }

        inline void remover(){
            std::unique_lock<std::mutex> lock (mtx_);
            
            // decrease 1 to used threads number in an atomic way
            usedThreads_.fetch_sub(1, std::memory_order_release);
            canAdd = (nThreads_ > usedThreads_);
            cv_.notify_one();

            //std::cout<< "Thread removed."<<std::endl;
        }

        inline unsigned short int getNThreads(){return nThreads_;}

        std::mutex mtx_;
};
#endif // SYNCHRO_H