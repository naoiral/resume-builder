#pragma once
#include "data/resume_model.h"
#include "data/autosave.h"
#include <string>
#include <vector>
#include <functional>

namespace resume {

enum class AppView { Welcome, Editor };

struct UndoState {
    std::string json_snapshot;
};

class App {
public:
    static App& instance();

    void init();
    void shutdown();

    // Data
    ResumeData data;
    std::string current_file;
    bool modified = false;

    // View
    AppView current_view = AppView::Welcome;

    // Template
    int selected_template = 0;  // 0=classic, 1=modern, 2=minimal, 3=creative
    const char* template_names[4] = {"经典模板", "技术岗模板", "实习模板", "社招模板"};

    // Text color (default black)
    float text_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};  // RGBA black

    // Border/line color (default black)
    float border_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};  // RGBA black

    // Auto save
    AutoSave autosave;

    // Undo/Redo
    void push_undo();
    void undo();
    void redo();
    bool can_undo() const;
    bool can_redo() const;

    // File operations
    void new_resume();
    bool open_file(const std::string& path);
    bool save_file();
    bool save_file_as(const std::string& path);

    // Export
    bool export_pdf(const std::string& path);

    // Status
    std::string status_message;
    float status_timer = 0;
    void set_status(const std::string& msg);

    // Demo data
    void create_demo_resume();

private:
    App() = default;
    std::vector<UndoState> undo_stack_;
    std::vector<UndoState> redo_stack_;
    static constexpr int MAX_UNDO = 50;
};

} // namespace resume
