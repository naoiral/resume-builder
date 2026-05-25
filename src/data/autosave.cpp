#include "autosave.h"
#include "json_io.h"
#include <chrono>
#include <iostream>

namespace resume {

AutoSave::AutoSave() = default;

AutoSave::~AutoSave() {
    stop();
}

void AutoSave::start(const std::string& filepath, std::function<ResumeData()> snapshot_fn) {
    std::lock_guard<std::mutex> lock(mutex_);
    filepath_ = filepath;
    snapshot_fn_ = std::move(snapshot_fn);
    if (!running_.exchange(true)) {
        thread_ = std::thread(&AutoSave::run, this);
    }
}

void AutoSave::stop() {
    running_ = false;
    if (thread_.joinable()) {
        thread_.join();
    }
}

void AutoSave::set_interval(int seconds) {
    interval_ = seconds > 0 ? seconds : 30;
}

void AutoSave::trigger_now() {
    if (!snapshot_fn_ || filepath_.empty()) return;
    auto data = snapshot_fn_();
    data.last_modified = current_iso8601();
    save_to_file(data, filepath_);
}

void AutoSave::run() {
    while (running_) {
        for (int i = 0; i < interval_ * 10 && running_; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        if (running_ && snapshot_fn_ && !filepath_.empty()) {
            trigger_now();
        }
    }
}

} // namespace resume
