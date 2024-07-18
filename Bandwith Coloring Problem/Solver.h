#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <filesystem>
#include <map>
#include <opencv2/opencv.hpp>

#include "LPR.h"

class Solver
{
    const int PopulationSize = 20;
    std::string InstancesPath;
    std::string OutputPath;
    std::string OutStatsPath;
public:
    Solver(std::string InstancesPath);
    void Solve();

    static void ComputeGraphImge(int NoNodes, std::vector<std::vector<int>> Graph, std::vector<int> BestSol, std::string TempPath);
private:
    void ReadData(std::string FileName, int& NoNodes, int& NoEdges, int& KBest, std::vector<std::vector<int>>& Graph);

};

