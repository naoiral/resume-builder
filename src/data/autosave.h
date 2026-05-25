#pragma once
#include "resume_model.h"
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>

namespace resume {

class AutoSave {
public:
    AutoSave();
    ~AutoSave();

    void start(const std::string& filepath, std::function<ResumeData()> snapshot_fn);
    void stop();
    void set_interval(int seconds);
    void trigger_now();

private:
    void run();
    std::thread thread_;
    std::mutex mutex_;
    std::atomic<bool> running_{false};
    std::atomic<int> interval_{30};
    std::string filepath_;
    std::function<ResumeData()> snapshot_fn_;
};

} // namespace resume
