#pragma once

#include <iostream>
#include <string>
#include <map>
#include <set>
#include <fstream>
#include <filesystem>
#include "json.hpp"
#include "LPR.h"
#include "Solver.h"

namespace fs = std::filesystem;

using json = nlohmann::json;

class UETT
{
    std::string Path;
    fs::path OutputPath;
    fs::path OutputPath_graph;
    fs::path OutputPath_timetable;
public:
    UETT(const std::string& filename);

    void Solve();
private:
    std::map<std::string, std::string> Difficulties;
    std::map<std::string, std::set<std::string>> ExamToStudents;
    std::map<std::string, std::set<std::string>> Graph;
    std::map<int, std::string> int_to_str;
    std::map<std::string, int> str_to_int;

    void ParseJson(const json& j);
    void CreateGraph();
    void ComputeTimetableImage(std::vector<int> solution, std::string TempPath);
};

