#include "UETT.h"

UETT::UETT(const std::string& filename)
{
    Path = filename;
    OutputPath = fs::path(Path).parent_path() / "Output";
    std::string FileNameWithoutExtension = fs::path(Path).stem().string();
    OutputPath_graph = OutputPath / (FileNameWithoutExtension + "_graph.tmp");
    OutputPath_timetable = OutputPath / (FileNameWithoutExtension + "_timetable.tmp");

    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file");
    }

    json j;
    file >> j;

    ParseJson(j);
    CreateGraph();
}

void UETT::Solve()
{
    int NoNodes = 0, NoEdges = 0;
    std::vector<std::vector<int>> Edges;
    
    int it = 0;
    for (const auto& node : Graph) {
        int_to_str[it] = node.first;
        str_to_int[node.first] = it;
        ++it;
        ++NoNodes;
        NoEdges += node.second.size();
    }
    
    NoEdges /= 2;
    Edges.resize(NoNodes);
    it = 0;
    for (const auto& node : Graph) {
        Edges[str_to_int[node.first]].resize(NoNodes);
        for (const auto& neighbor : node.second) {
            int cost_1;
            if (Difficulties[node.first] == "easy")
                cost_1 = 1;
            else if (Difficulties[node.first] == "medium")
                cost_1 = 2;
            else
                cost_1 = 3;

            int cost_2;
            if (Difficulties[neighbor] == "easy")
                cost_2 = 1;
            else if (Difficulties[neighbor] == "medium")
                cost_2 = 2;
            else
                cost_2 = 3;

            Edges[str_to_int[node.first]][str_to_int[neighbor]] = std::min(cost_1, cost_2);
        }
        ++it;
    }
    LPR solution(NoNodes, NoEdges, 15, 20, Edges);
    std::vector<int> cols = solution.Solve();
    if (cols.size() > 0)
    {
        Solver::ComputeGraphImge(NoNodes, Edges, cols, OutputPath_graph.string());
        ComputeTimetableImage(cols, OutputPath_timetable.string());
    }
}


void UETT::ParseJson(const json& j)
{
    for (auto& [key, value] : j["mandatory_exams"].items()) {
        Difficulties[value[0]] = value[1];
    }

    for (auto& [key, value] : j["optional_packs"].items()) {
        for (const auto& exam : value[0]) {
            Difficulties[exam] = value[1];
        }
    }

    for (auto& [key, value] : j["students"].items()) {
        for (const auto& exam : value) {
            ExamToStudents[exam].insert(key);
        }
    }
}

void UETT::CreateGraph()
{
    for (const auto& [exam1, students1] : ExamToStudents) {
        for (const auto& [exam2, students2] : ExamToStudents) {
            if (exam1 != exam2) {
                std::vector<std::string> sharedStudents;
                std::set_intersection(students1.begin(), students1.end(),
                    students2.begin(), students2.end(),
                    std::back_inserter(sharedStudents));
                if (!sharedStudents.empty()) {
                    Graph[exam1].insert(exam2);
                }
            }
        }
    }

}

void UETT::ComputeTimetableImage(std::vector<int> solution, std::string TempPath)
{
    bool First = true;
    int it = 0;

    std::string ExamsForImage = "[";
    std::string StartForImage = "[";
    std::string EndForImage = "[";
    std::string ColorsForImage = "[";

    std::vector<std::pair<std::string, std::string>> timeIntervals = {
        {"08:00", "10:00"},
        {"10:00", "12:00"},
        {"12:00", "14:00"},
        {"14:00", "16:00"},
        {"16:00", "18:00"},
        {"18:00", "20:00"}
    };

    for (auto col : solution)
    {
        if (First == false)
        {
            ExamsForImage = ExamsForImage + ", ";
            ColorsForImage = ColorsForImage + ", ";
            StartForImage = StartForImage + ", ";
            EndForImage = EndForImage + ", ";
        }
        else
            First = false;

        ColorsForImage = ColorsForImage + std::to_string(col);
        ExamsForImage = ExamsForImage + '\'' + int_to_str[it] + '\'';
        ++it;

        int randomIndex = std::rand() % timeIntervals.size();

        std::pair<std::string, std::string> randomInterval = timeIntervals[randomIndex];

        StartForImage = StartForImage + '\'' + randomInterval.first + '\'';
        EndForImage = EndForImage + '\'' + randomInterval.second + '\'';
    }

    ExamsForImage = ExamsForImage + "]";
    ColorsForImage = ColorsForImage + "]";
    StartForImage = StartForImage + "]";
    EndForImage = EndForImage + "]";

    std::ofstream Fout(TempPath);
    Fout << ExamsForImage << "\n" << ColorsForImage << "\n" << StartForImage << "\n" << EndForImage << "\n";
    Fout.close();

    std::string Command = "python Scripts/generate_timetable.py \"" + TempPath + "\"";
    int result = system(Command.c_str());

    if (result == 0)
    {
        std::filesystem::remove(TempPath);
    }
}
