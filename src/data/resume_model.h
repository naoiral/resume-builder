#pragma once
#include <string>
#include <vector>
#include <ctime>
#include <random>
#include <sstream>
#include <iomanip>

namespace resume {

inline std::string generate_id() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);
    std::ostringstream oss;
    oss << std::hex << std::setfill('0')
        << std::setw(8) << dist(gen) << "-"
        << std::setw(4) << (dist(gen) & 0xFFFF) << "-"
        << std::setw(4) << ((dist(gen) & 0x0FFF) | 0x4000) << "-"
        << std::setw(4) << ((dist(gen) & 0x3FFF) | 0x8000) << "-"
        << std::setw(8) << dist(gen) << std::setw(4) << (dist(gen) & 0xFFFF);
    return oss.str();
}

inline std::string current_iso8601() {
    auto now = std::time(nullptr);
    auto* tm = std::gmtime(&now);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", tm);
    return buf;
}

enum class SkillLevel { Know, Familiar, Proficient, Expert };

inline const char* skill_level_str(SkillLevel l) {
    switch(l) {
        case SkillLevel::Know: return "了解";
        case SkillLevel::Familiar: return "熟悉";
        case SkillLevel::Proficient: return "掌握";
        case SkillLevel::Expert: return "熟练";
    }
    return "了解";
}

struct PersonalInfo {
    std::string name;
    std::string phone;
    std::string email;
    std::string objective;  // 求职意向
    std::string summary;    // 个人简介
    std::string photo_path;
};

struct Education {
    std::string id;
    std::string school;
    std::string major;       // 专业
    std::string degree;      // 学历
    std::string start_date;
    std::string end_date;
    std::string gpa;
    std::string courses;     // 主修课程
    int sort_order = 0;

    Education() : id(generate_id()) {}
};

struct WorkExperience {
    std::string id;
    std::string company;
    std::string title;       // 职位
    std::string start_date;
    std::string end_date;
    std::vector<std::string> duties;  // 工作内容（多条）
    int sort_order = 0;

    WorkExperience() : id(generate_id()) {}
};

struct Project {
    std::string id;
    std::string name;
    std::string start_date;
    std::string end_date;
    std::string description;
    std::string tech_stack;           // 技术栈
    std::vector<std::string> duties;  // 个人职责与成果
    int sort_order = 0;

    Project() : id(generate_id()) {}
};

struct SkillItem {
    std::string name;
    SkillLevel level = SkillLevel::Familiar;
};

struct SkillCategory {
    std::string id;
    std::string category;             // 分类名称
    std::vector<SkillItem> items;
    int sort_order = 0;

    SkillCategory() : id(generate_id()) {}
};

struct Certificate {
    std::string id;
    std::string name;
    std::string date;
    int sort_order = 0;

    Certificate() : id(generate_id()) {}
};

struct ResumeData {
    std::string version = "1.0";
    std::string template_name = "classic";
    PersonalInfo personal;
    std::vector<Education> education;
    std::vector<WorkExperience> work;
    std::vector<Project> projects;
    std::vector<SkillCategory> skills;
    std::vector<Certificate> certificates;
    std::string last_modified;

    ResumeData() : last_modified(current_iso8601()) {}
};

} // namespace resume
