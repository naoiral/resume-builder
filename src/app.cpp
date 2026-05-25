#include "app.h"
#include "data/json_io.h"
#include "export/pdf_export.h"
#include "ui/ui_preview.h"
#include <fstream>
#include <iostream>
#include <algorithm>

namespace resume {

App& App::instance() {
    static App app;
    return app;
}

void App::init() {
    current_view = AppView::Welcome;
    modified = false;
}

void App::shutdown() {
    autosave.stop();
    destroy_textures();
}

void App::push_undo() {
    undo_stack_.push_back({to_json_string(data)});
    if (undo_stack_.size() > MAX_UNDO) {
        undo_stack_.erase(undo_stack_.begin());
    }
    redo_stack_.clear();
    modified = true;
}

void App::undo() {
    if (undo_stack_.empty()) return;
    redo_stack_.push_back({to_json_string(data)});
    from_json_string(data, undo_stack_.back().json_snapshot);
    undo_stack_.pop_back();
}

void App::redo() {
    if (redo_stack_.empty()) return;
    undo_stack_.push_back({to_json_string(data)});
    from_json_string(data, redo_stack_.back().json_snapshot);
    redo_stack_.pop_back();
}

bool App::can_undo() const { return !undo_stack_.empty(); }
bool App::can_redo() const { return !redo_stack_.empty(); }

void App::new_resume() {
    data = ResumeData();
    current_file.clear();
    modified = false;
    undo_stack_.clear();
    redo_stack_.clear();
    current_view = AppView::Editor;
    create_demo_resume();
    set_status("已创建新简历");
}

bool App::open_file(const std::string& path) {
    if (load_from_file(data, path)) {
        current_file = path;
        modified = false;
        undo_stack_.clear();
        redo_stack_.clear();
        current_view = AppView::Editor;

        // Update selected template
        for (int i = 0; i < 4; i++) {
            if (data.template_name == template_names[i] ||
                data.template_name == std::to_string(i)) {
                selected_template = i;
                break;
            }
        }

        // Start autosave
        if (!current_file.empty()) {
            autosave.start(current_file, [this]() { return this->data; });
        }

        set_status("已打开: " + path);
        return true;
    }
    set_status("打开失败: " + path);
    return false;
}

bool App::save_file() {
    if (current_file.empty()) return false;
    data.last_modified = current_iso8601();
    if (save_to_file(data, current_file)) {
        modified = false;
        set_status("已保存");
        return true;
    }
    set_status("保存失败");
    return false;
}

bool App::save_file_as(const std::string& path) {
    data.last_modified = current_iso8601();
    if (save_to_file(data, path)) {
        current_file = path;
        modified = false;
        autosave.start(current_file, [this]() { return this->data; });
        set_status("已保存到: " + path);
        return true;
    }
    set_status("保存失败");
    return false;
}

bool App::export_pdf(const std::string& path) {
    bool ok = export_resume_pdf(data, path);
    if (ok) {
        set_status("PDF已导出: " + path);
    } else {
        set_status("PDF导出失败");
    }
    return ok;
}

void App::set_status(const std::string& msg) {
    status_message = msg;
    status_timer = 5.0f;
}

void App::create_demo_resume() {
    data.personal.name = "张三";
    data.personal.phone = "138-0000-0000";
    data.personal.email = "zhangsan@example.com";
    data.personal.objective = "C++软件开发工程师";
    data.personal.summary = "3年C++开发经验，熟悉Qt框架和跨平台开发，有良好的代码规范和团队协作能力。";

    Education edu;
    edu.school = "北京大学";
    edu.major = "计算机科学与技术";
    edu.degree = "本科";
    edu.start_date = "2018-09";
    edu.end_date = "2022-06";
    edu.gpa = "3.8/4.0";
    edu.courses = "数据结构、操作系统、计算机网络、数据库原理、算法设计";
    data.education.push_back(edu);

    WorkExperience work;
    work.company = "腾讯科技";
    work.title = "C++开发工程师";
    work.start_date = "2022-07";
    work.end_date = "至今";
    work.duties = {
        "负责后台服务核心模块的设计与开发",
        "优化服务性能，QPS提升30%",
        "参与技术方案评审，推动代码规范化"
    };
    data.work.push_back(work);

    Project proj;
    proj.name = "分布式文件存储系统";
    proj.start_date = "2023-01";
    proj.end_date = "2023-06";
    proj.description = "基于C++开发的分布式文件存储系统，支持PB级数据存储";
    proj.tech_stack = "C++17, gRPC, RocksDB, etcd";
    proj.duties = {
        "设计并实现分布式一致性协议",
        "开发客户端SDK，支持多语言调用",
        "完成性能优化，读写延迟降低40%"
    };
    data.projects.push_back(proj);

    SkillCategory skill_cat;
    skill_cat.category = "编程语言";
    skill_cat.items = {{"C++", SkillLevel::Expert}, {"Python", SkillLevel::Proficient}, {"Go", SkillLevel::Familiar}};
    data.skills.push_back(skill_cat);

    SkillCategory skill_cat2;
    skill_cat2.category = "技术框架";
    skill_cat2.items = {{"Qt", SkillLevel::Expert}, {"gRPC", SkillLevel::Proficient}, {"Redis", SkillLevel::Familiar}};
    data.skills.push_back(skill_cat2);

    Certificate cert;
    cert.name = "CET-6 英语六级";
    cert.date = "2020-06";
    data.certificates.push_back(cert);

    data.template_name = "经典模板";
}

} // namespace resume
