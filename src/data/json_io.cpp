#include "json_io.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;
namespace resume {

// PersonalInfo
void to_json(json& j, const PersonalInfo& p) {
    j = json{
        {"name", p.name}, {"phone", p.phone}, {"email", p.email},
        {"objective", p.objective}, {"summary", p.summary}, {"photo_path", p.photo_path}
    };
}
void from_json(const json& j, PersonalInfo& p) {
    j.at("name").get_to(p.name);
    if (j.contains("phone")) j.at("phone").get_to(p.phone);
    if (j.contains("email")) j.at("email").get_to(p.email);
    if (j.contains("objective")) j.at("objective").get_to(p.objective);
    if (j.contains("summary")) j.at("summary").get_to(p.summary);
    if (j.contains("photo_path")) j.at("photo_path").get_to(p.photo_path);
}

// Education
void to_json(json& j, const Education& e) {
    j = json{
        {"id", e.id}, {"school", e.school}, {"major", e.major}, {"degree", e.degree},
        {"start_date", e.start_date}, {"end_date", e.end_date},
        {"gpa", e.gpa}, {"courses", e.courses}, {"sort_order", e.sort_order}
    };
}
void from_json(const json& j, Education& e) {
    if (j.contains("id")) j.at("id").get_to(e.id);
    j.at("school").get_to(e.school);
    if (j.contains("major")) j.at("major").get_to(e.major);
    if (j.contains("degree")) j.at("degree").get_to(e.degree);
    if (j.contains("start_date")) j.at("start_date").get_to(e.start_date);
    if (j.contains("end_date")) j.at("end_date").get_to(e.end_date);
    if (j.contains("gpa")) j.at("gpa").get_to(e.gpa);
    if (j.contains("courses")) j.at("courses").get_to(e.courses);
    if (j.contains("sort_order")) j.at("sort_order").get_to(e.sort_order);
}

// WorkExperience
void to_json(json& j, const WorkExperience& w) {
    j = json{
        {"id", w.id}, {"company", w.company}, {"title", w.title},
        {"start_date", w.start_date}, {"end_date", w.end_date},
        {"duties", w.duties}, {"sort_order", w.sort_order}
    };
}
void from_json(const json& j, WorkExperience& w) {
    if (j.contains("id")) j.at("id").get_to(w.id);
    j.at("company").get_to(w.company);
    if (j.contains("title")) j.at("title").get_to(w.title);
    if (j.contains("start_date")) j.at("start_date").get_to(w.start_date);
    if (j.contains("end_date")) j.at("end_date").get_to(w.end_date);
    if (j.contains("duties")) j.at("duties").get_to(w.duties);
    if (j.contains("sort_order")) j.at("sort_order").get_to(w.sort_order);
}

// Project
void to_json(json& j, const Project& p) {
    j = json{
        {"id", p.id}, {"name", p.name},
        {"start_date", p.start_date}, {"end_date", p.end_date},
        {"description", p.description}, {"tech_stack", p.tech_stack},
        {"duties", p.duties}, {"sort_order", p.sort_order}
    };
}
void from_json(const json& j, Project& p) {
    if (j.contains("id")) j.at("id").get_to(p.id);
    j.at("name").get_to(p.name);
    if (j.contains("start_date")) j.at("start_date").get_to(p.start_date);
    if (j.contains("end_date")) j.at("end_date").get_to(p.end_date);
    if (j.contains("description")) j.at("description").get_to(p.description);
    if (j.contains("tech_stack")) j.at("tech_stack").get_to(p.tech_stack);
    if (j.contains("duties")) j.at("duties").get_to(p.duties);
    if (j.contains("sort_order")) j.at("sort_order").get_to(p.sort_order);
}

// SkillItem
void to_json(json& j, const SkillItem& s) {
    j = json{{"name", s.name}, {"level", static_cast<int>(s.level)}};
}
void from_json(const json& j, SkillItem& s) {
    j.at("name").get_to(s.name);
    if (j.contains("level")) s.level = static_cast<SkillLevel>(j.at("level").get<int>());
}

// SkillCategory
void to_json(json& j, const SkillCategory& s) {
    j = json{{"id", s.id}, {"category", s.category}, {"items", s.items}, {"sort_order", s.sort_order}};
}
void from_json(const json& j, SkillCategory& s) {
    if (j.contains("id")) j.at("id").get_to(s.id);
    j.at("category").get_to(s.category);
    if (j.contains("items")) j.at("items").get_to(s.items);
    if (j.contains("sort_order")) j.at("sort_order").get_to(s.sort_order);
}

// Certificate
void to_json(json& j, const Certificate& c) {
    j = json{{"id", c.id}, {"name", c.name}, {"date", c.date}, {"sort_order", c.sort_order}};
}
void from_json(const json& j, Certificate& c) {
    if (j.contains("id")) j.at("id").get_to(c.id);
    j.at("name").get_to(c.name);
    if (j.contains("date")) j.at("date").get_to(c.date);
    if (j.contains("sort_order")) j.at("sort_order").get_to(c.sort_order);
}

// ResumeData
void to_json(json& j, const ResumeData& d) {
    j = json{
        {"version", d.version}, {"template_name", d.template_name},
        {"personal", d.personal}, {"education", d.education},
        {"work", d.work}, {"projects", d.projects},
        {"skills", d.skills}, {"certificates", d.certificates},
        {"last_modified", d.last_modified}
    };
}
void from_json(const json& j, ResumeData& d) {
    if (j.contains("version")) j.at("version").get_to(d.version);
    if (j.contains("template_name")) j.at("template_name").get_to(d.template_name);
    if (j.contains("personal")) j.at("personal").get_to(d.personal);
    if (j.contains("education")) j.at("education").get_to(d.education);
    if (j.contains("work")) j.at("work").get_to(d.work);
    if (j.contains("projects")) j.at("projects").get_to(d.projects);
    if (j.contains("skills")) j.at("skills").get_to(d.skills);
    if (j.contains("certificates")) j.at("certificates").get_to(d.certificates);
    if (j.contains("last_modified")) j.at("last_modified").get_to(d.last_modified);
}

std::string to_json_string(const ResumeData& data) {
    json j = data;
    return j.dump(2);
}

bool from_json_string(ResumeData& data, const std::string& json_str) {
    try {
        json j = json::parse(json_str);
        data = j.get<ResumeData>();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return false;
    }
}

bool save_to_file(const ResumeData& data, const std::string& path) {
    try {
        std::ofstream f(path);
        if (!f.is_open()) return false;
        f << to_json_string(data);
        return true;
    } catch (...) {
        return false;
    }
}

bool load_from_file(ResumeData& data, const std::string& path) {
    try {
        std::ifstream f(path);
        if (!f.is_open()) return false;
        std::string content((std::istreambuf_iterator<char>(f)),
                            std::istreambuf_iterator<char>());
        return from_json_string(data, content);
    } catch (...) {
        return false;
    }
}

} // namespace resume
