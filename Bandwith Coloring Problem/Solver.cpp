#include "Solver.h"

Solver::Solver(std::string InstancesPath)
{   
    this->InstancesPath = InstancesPath;
    std::string OutputPath = InstancesPath + "\\Output";
    std::string OutStatsPath = OutputPath + "\\stats.csv";
}
//------------------------------------------------------------------------------------------------

void Solver::Solve()
{
    std::ofstream FoutStats(OutStatsPath);
    FoutStats << "Filename, SR, Average Success Time, Average Execution Time\n";

    std::vector<std::string> FileNames;
    for (const auto& entry : std::filesystem::directory_iterator(InstancesPath)) {
        if (entry.is_regular_file()) {
            FileNames.push_back(entry.path().string());
        }
    }

    auto ExecutionTimeStart = std::chrono::high_resolution_clock::now();
    cv::parallel_for_(cv::Range(0, FileNames.size()), [&](const cv::Range& range) {
        for (int Index = range.start; Index < range.end; ++Index) {
            //for(int Index = 0; Index < FileNames.size(); ++Index) {
            int NoNodes = 0;
            int NoEdges = 0;
            int KBest = 0;
            std::vector<std::vector<int>> Graph;
            ReadData(FileNames[Index], NoNodes, NoEdges, KBest, Graph);

            int BestNoColors = KBest;
            std::vector<int> BestSol;

            std::filesystem::path Path(FileNames[Index]);
            std::string FileNameWithoutExtension = Path.stem().string();
            std::string Extension = Path.extension().string();
            std::string LogPath = OutputPath + "\\" + FileNameWithoutExtension + ".log";
            std::string OutTmpPath = OutputPath + "\\" + FileNameWithoutExtension + ".tmp";

            std::ofstream Fout(LogPath);
            int Instances = 20;
            int NoSuccess = 0;
            double TotalTimeSuccess = 0;
            auto TotalTimeStart = std::chrono::high_resolution_clock::now();
            for (int It = 0; It < Instances; ++It)
            {
                Fout << "Process For Instance = " << It << "\n";
                auto LocalTimeStart = std::chrono::high_resolution_clock::now();
                LPR Solver(NoNodes, NoEdges, KBest, PopulationSize, Graph);
                std::vector<int> Solution = Solver.Solve();
                auto LocalTimeEnd = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> Duration = LocalTimeEnd - LocalTimeStart;
                if (Solution.size() > 0)
                {
                    ++NoSuccess;
                    TotalTimeSuccess += Duration.count();
                    for (auto El : Solution)
                        Fout << El << " ";
                    Fout << "\nSuccess ---> ";
                    BestSol = Solution;
                }
                else Fout << "\nFail ---> ";
                Fout << "Execution Time: " << Duration.count() << " seconds\n\n";
            }
            auto TotalTimeEnd = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> TotalTime = TotalTimeEnd - TotalTimeStart;
            Fout << "Total Execution Time: " << TotalTime.count() << " seconds\n\n";

            FoutStats << std::fixed << std::setprecision(2)
                << FileNameWithoutExtension << ", " + std::to_string(NoSuccess) + "/" + std::to_string(Instances) + ", ";
            if (NoSuccess == 0)
                FoutStats << "-" << ", " << "-" << "\n";
            else
                FoutStats << 1.0 * TotalTimeSuccess / NoSuccess << ", " << TotalTime.count() << "\n";

            if (NoSuccess == 0)
            {
                std::cout << FileNames[Index] + " ---> Fail\n";
            }
            else
            {
                std::cout << FileNames[Index] + " ---> Success\n";
                ComputeGraphImge(NoNodes, Graph, BestSol, OutTmpPath);
            }
        }
    });
    auto ExecutionTimeEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> TotalTime = ExecutionTimeEnd - ExecutionTimeStart;

    FoutStats << std::fixed << std::setprecision(3) << "\n\nTotal Execution Time:," << std::to_string(TotalTime.count()) + " seconds";
}
//------------------------------------------------------------------------------------------------

void Solver::ReadData(std::string FileName, int& NoNodes, int& NoEdges, int& KBest, std::vector<std::vector<int>>& Graph)
{
    std::ifstream fin(FileName);
    std::string line;

    while (std::getline(fin, line))
    {
        std::vector<std::string> tokens;
        auto newEnd = std::unique(line.begin(), line.end(),
            [](char a, char b) { return std::isspace(a) && std::isspace(b); });

        line.erase(newEnd, line.end());

        size_t spacePos = line.find(' ');
        size_t startPos = 0;

        while (spacePos != std::string::npos) {
            std::string word = line.substr(startPos, spacePos - startPos);
            tokens.push_back(word);

            startPos = spacePos + 1;
            spacePos = line.find(' ', startPos);
        }

        std::string lastWord = line.substr(startPos);
        tokens.push_back(lastWord);

        if (tokens[0] == "c" || tokens[0] == "n")
            continue;

        if (tokens[0] == "p")
        {
            NoNodes = std::stoi(tokens[2]);
            NoEdges = std::stoi(tokens[3]);
            KBest = std::stoi(tokens[4]);

            Graph = std::vector<std::vector<int>>(NoNodes, std::vector<int>(NoNodes, 0));
        }

        if (tokens[0] == "e")
        {
            int v1 = std::stoi(tokens[1]) - 1;
            int v2 = std::stoi(tokens[2]) - 1;
            int d = std::stoi(tokens[3]);

            if (v1 == v2)
                continue;

            Graph[v1][v2] = d;
            Graph[v2][v1] = d;
        }
    }
}
//------------------------------------------------------------------------------------------------

void Solver::ComputeGraphImge(int NoNodes, std::vector<std::vector<int>> Graph, std::vector<int> BestSol, std::string TempPath)
{
    std::string EdgesForImage = "[";
    bool First = true;
    for (int v1 = 0; v1 < NoNodes; ++v1)
    {
        for (int v2 = 0; v2 < v1; ++v2)
        {

            if (Graph[v1][v2] > 0)
            {
                if (First == false)
                    EdgesForImage = EdgesForImage + ", ";
                else
                    First = false;

                EdgesForImage = EdgesForImage + "[" + std::to_string(v1) + ", " + std::to_string(v2) + ", { \'label\': " + std::to_string(Graph[v1][v2]) + " }]";
            }
        }
    }
    EdgesForImage = EdgesForImage + "]";

    std::string ColorsForImage = "{";

    First = true;
    for (int Node = 0; Node < BestSol.size(); ++Node)
    {
        if (First == false)
            ColorsForImage = ColorsForImage + ", ";
        else
            First = false;

        ColorsForImage = ColorsForImage + std::to_string(Node) + ": { \'value\': " + std::to_string(BestSol[Node]) + " }";
    }

    ColorsForImage = ColorsForImage + "}";
    std::ofstream Fout(TempPath);
    Fout << EdgesForImage << "\n" << ColorsForImage << "\n";
    Fout.close();

    std::string Command = "python Scripts/generate_graph.py \"" + TempPath + "\"";
    int result = system(Command.c_str());

    if (result == 0)
    {
        std::filesystem::remove(TempPath);
    }
}
