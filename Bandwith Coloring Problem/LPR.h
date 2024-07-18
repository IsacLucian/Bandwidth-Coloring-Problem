#pragma once
#include <iostream>
#include <vector>
#include <set>
#include <random>
#include <queue>
#include <functional>
#include <chrono>
#include <map>
#include <limits.h>

#include <cassert>


class LPR
{
private:
    int NoNodes;
    int NoEdges;
    int NoColors;
    int PopulationSize;
    int Alpha;
    int Alpha0;
    int Tmax;
    int MaxPenaltyWeight;
    int Pmax;
    float ScalingFactor;
    std::vector<int> TabuTenure;
    std::vector<int> TabuTenureInterval;
    std::vector<std::vector<int>> Edges;
    std::vector<std::vector<int>> AdjList;
    std::vector<std::vector<int>> PenaltyMatrix;
    std::set<std::vector<int>> Population;

public:
    LPR(int Nodes, int NoEdges, int NoColors, int PopulationSize, std::vector<std::vector<int>> Edges);

    std::vector<int> Solve();

private:
    void InitializeVariables();
    void InitializePopulation();
    void TabuSearchImpr(std::vector<int>& Solution, bool IsAugmented);
    void TabuSearch(std::vector<int>& Solution, bool IsAugmented);
    void TwoPhaseTabuSearch(std::vector<int>& Solution);
    void Improvement_and_Updating(std::vector<int>& CurrentSol, std::vector<int>& BestSol, std::set<std::pair<std::vector<int>, std::vector<int>>>& PairSet);
    void UpdatePenaltyMatrix(std::vector<int> Solution);
    void InitializePrecalcMatrixes(std::vector<int> Solution, std::vector<std::vector<int>>& ColorChangeSum, std::vector<std::vector<int>>& ColorChangeWeightSum, bool IsAugmented);
    void UpdatePrecalcMatrixes(std::vector<int> Solution, std::pair<int, int> BestCandidate, std::vector<std::vector<int>>& ColorChangeSum, std::vector<std::vector<int>>& ColorChangeWeightSum, bool IsAugmented);
    int SumConstraintViolations(std::vector<int> Solution);
    int AugmentedSumConstraintViolations(std::vector<int> Solution);
    int DistanceHamming(std::vector<int> Solution);
    std::vector<int> GenerateRandomSolution();
    std::vector<int> MixedPathRelinking(std::vector<int> FirstParent, std::vector<int> SecondParent);
};

