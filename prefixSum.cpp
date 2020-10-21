#include <condition_variable>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <thread>
#include <string>
#include <vector>


class ParallelPrefixSum {

public: 

        ParallelPrefixSum(uint32_t num_nums, uint32_t num_threads) : 
                num_nums{num_nums}, // this->num_nums = num_nums
                num_threads{num_threads}
        {
                srand(1);
                for(uint32_t i = 0; i < num_nums; ++i) {
                        nums.push_back(rand() % 10);
                        check_nums.push_back(nums[i]);
                }
                for(uint32_t i = 0; i < num_threads - 1; ++i) {
                        partial_sums.push_back(-1);
                }
        }

        void run_sequential() {
                start = std::chrono::high_resolution_clock::now();
                prefix_sum(check_nums, 0, check_nums.size());
                end = std::chrono::high_resolution_clock::now();
        }

        void run_parallel() {
                start = std::chrono::high_resolution_clock::now();
                std::vector<std::thread> threads;
                for(uint32_t pid = 0; pid < num_threads; ++pid) {
                        threads.emplace_back([this, pid]{this->worker(pid);});
                }
                for(auto& thread : threads) {
                        thread.join();
                }
                end = std::chrono::high_resolution_clock::now();
        }

        double get_time() {
                std::chrono::duration<double> elapsed = end-start;
                return elapsed.count();
        }

        bool verify() {
                for(uint64_t i = 0; i < num_nums; ++i) {
                        if(nums[i] != check_nums[i]) {
                                return false;
                        }
                }
                return true;
        }

        void print_nums() {
                for( auto num : nums) {
                        std::cout << num << " ";
                }
                std::cout << std::endl;
        }

private:
        void prefix_sum(std::vector<int64_t>& nums, uint64_t start, uint64_t end) {
                for(uint64_t i = start + 1; i < end; ++i) {
                        nums[i] += nums[i-1];
                }
        }

        void worker(uint32_t pid) {
                uint64_t chunk = num_nums / num_threads;
                uint64_t start = pid * chunk;
                uint64_t end = start + chunk;
                if(pid == num_threads - 1) {
                        end = num_nums;
                }
                // calculate your prefix sum for your chunk
                prefix_sum(nums, start, end);
                int64_t carried_sum = 0;
                // wait until you get the prefix sum from the left!
                if(pid > 0) {
                        std::unique_lock<std::mutex> lock(mutex);
                        condition_variable.wait(lock, [&]{return partial_sums[pid -1] >= 0;});
                        carried_sum += partial_sums[pid -1];
                }
                // now pass your shit to the right!
                if(pid < num_threads - 1) {
                        std::unique_lock<std::mutex> lock(mutex);
                        partial_sums[pid] = carried_sum + nums[end - 1];
                        lock.unlock();
                        condition_variable.notify_all();
                }
                // after you have
                if(pid>0) {
                        for(uint64_t i = start; i < end; ++i) {
                                nums[i] += carried_sum;
                        }
                }
        }

        const uint32_t num_threads;
        const uint32_t num_nums;
        std::vector<int64_t> nums;
        std::vector<int64_t> check_nums;
        std::vector<int64_t> partial_sums;
        std::mutex mutex;
        std::condition_variable condition_variable;
        std::chrono::time_point<std::chrono::high_resolution_clock> start;
        std::chrono::time_point<std::chrono::high_resolution_clock> end;
};
                        


int main(int argc, const char * argv[]) {
        const uint32_t log_num_nums = 22;
        const uint32_t num_threads = 4;
        const uint32_t num_nums =1 << log_num_nums;
        ParallelPrefixSum pps(num_nums, num_threads);
        pps.run_sequential();
        const double sequential_time = pps.get_time();
        pps.run_parallel();
        const double parallel_time = pps.get_time();
        const double speedup = sequential_time / parallel_time;
        std::string correct = pps.verify() ? "" : "in";
        std::cout << "Results are " << correct << "correct." << std::endl;
        std::cout << "Sequential execution time: " << sequential_time << " seconds." << std::endl;
        std::cout << "Parallel execution time: " << parallel_time << " seconds." << std::endl;
        std::cout << "Speedup: " << speedup << "." << std::endl;
        return 0;
}
