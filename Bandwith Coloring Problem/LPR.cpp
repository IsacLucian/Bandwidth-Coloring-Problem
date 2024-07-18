#include "LPR.h"

LPR::LPR(int NoNodes, int NoEdges, int NoColors, int PopulationSize, std::vector<std::vector<int>> Edges)
{
    this->NoNodes = NoNodes;
    this->NoEdges = NoEdges;
    this->NoColors = NoColors;
    this->PopulationSize = PopulationSize;
    this->Edges = Edges;
    PenaltyMatrix.resize(NoNodes);

    for (int line = 0; line < NoNodes; ++line)
    {
        PenaltyMatrix[line].resize(NoNodes);
        for (int col = 0; col < line; ++col)
            PenaltyMatrix[line][col] = 0;
    }

    InitializeVariables();
}
//------------------------------------------------------------------------------------------------

std::vector<int> LPR::Solve()
{
    std::random_device rd;
    std::mt19937 gen(rd());

    int MaxIterations = 2;
    int Iterations = 0;
    std::vector<int> BestSol, WorstSol;

    do
    {
        InitializePopulation();
        if (Iterations > 0)
        {
            int MaxConstraintViolation = INT_MAX;
            for (auto Sol : Population)
            {
                int Sum = SumConstraintViolations(Sol);
                if (Sum > MaxConstraintViolation)
                {
                    MaxConstraintViolation = Sum;
                    WorstSol = Sol;
                }
            }

            Population.insert(BestSol);
            Population.erase(WorstSol);
        }
        
        int MinConstraintViolation = INT_MAX;
        for (auto Sol : Population)
        {
            int Sum = SumConstraintViolations(Sol);
            if (Sum < MinConstraintViolation)
            {
                MinConstraintViolation = Sum;
                BestSol = Sol;
            }
        }

        std::set<std::pair<std::vector<int>, std::vector<int>>> PairSet;
        for (auto It1 = Population.begin(); It1 != Population.end(); ++It1)
            for (auto It2 = std::next(It1); It2 != Population.end(); ++It2)
                PairSet.insert({ *It1, *It2 });

        while (PairSet.size() > 0)
        {
            std::uniform_int_distribution<int> dist(0, PairSet.size() - 1);
            auto It = std::next(PairSet.begin(), dist(gen));
            std::pair<std::vector<int>, std::vector<int>> SelectedPair = *It;

            PairSet.erase(*It);
            std::vector<int> FirstChild = MixedPathRelinking(SelectedPair.first, SelectedPair.second);
            std::vector<int> SecondChild = MixedPathRelinking(SelectedPair.second, SelectedPair.first);

            Improvement_and_Updating(FirstChild, BestSol, PairSet);
            Improvement_and_Updating(SecondChild, BestSol, PairSet);

            if (SumConstraintViolations(BestSol) == 0)
                return BestSol;
        }

        ++Iterations;
    } while (Iterations < MaxIterations);

    return std::vector<int>();
}
//------------------------------------------------------------------------------------------------

void LPR::InitializeVariables()
{
    this->Alpha0 = 2000;
    this->Alpha = 10000;
    this->MaxPenaltyWeight = 30;
    this->ScalingFactor = 0.4f;
    this->Tmax = 50;
    this->Pmax = 15;
    std::vector<int> A = { 1, 2, 1, 4, 1, 2, 1, 8, 1, 2, 1, 4, 1, 2, 1 };

    TabuTenure.resize(Pmax);
    TabuTenureInterval.resize(Pmax);
    for (int Index = 0; Index < Pmax; ++Index)
    {
        TabuTenure[Index] = Tmax * A[Index] / 8;
        TabuTenureInterval[Index] = Tmax * A[Index] / 2;
    }

    AdjList.resize(NoNodes);
    for (int V1 = 0; V1 < NoNodes; ++V1)
    {
        std::vector<int> Ngh;
        for (int V2 = 0; V2 < NoNodes; ++V2)
        {
            if (Edges[V1][V2] > 0)
                Ngh.push_back(V2);
        }
        AdjList[V1] = Ngh;
    }
}
//------------------------------------------------------------------------------------------------

void LPR::InitializePopulation()
{
    std::vector<std::vector<int>> LargerPopulation;
    for (int Index = 0; Index < 3 * PopulationSize; ++Index)
    {
        std::vector<int> RandSol = GenerateRandomSolution();

        //TabuSearch(RandSol, false);
        TabuSearchImpr(RandSol, false);

        LargerPopulation.push_back(RandSol);
    }

    auto CompareLambda = [&](const std::vector<int> s1, const std::vector<int> s2) {
        return SumConstraintViolations(s1) < SumConstraintViolations(s2);
    };

    sort(LargerPopulation.begin(), LargerPopulation.end(), CompareLambda);
    LargerPopulation.resize(PopulationSize);

    Population.clear();
    for (auto Sol : LargerPopulation)
        Population.insert(Sol);
}
//------------------------------------------------------------------------------------------------

int LPR::SumConstraintViolations(std::vector<int> Solution)
{
    if (Solution.size() == 0)
        return INT_MAX;

    int Sum = 0;
    for (int v1 = 0; v1 < NoNodes; ++v1)
    {
        for (int v2 = 0; v2 < v1; ++v2)
        {
            Sum = Sum + std::max(0, Edges[v1][v2] - std::abs(Solution[v1] - Solution[v2]));
        }
    }

    return Sum;
}
//------------------------------------------------------------------------------------------------

int LPR::AugmentedSumConstraintViolations(std::vector<int> Solution)
{
    if (Solution.size() == 0)
        return INT_MAX;

    int Sum = SumConstraintViolations(Solution);
    for (int v1 = 0; v1 < NoNodes; ++v1)
    {
        for (int v2 = 0; v2 < v1; ++v2)
        {
            if (std::abs(Solution[v1] - Solution[v2]) < Edges[v1][v2])
            {
                Sum = Sum + PenaltyMatrix[v1][v2];
            }
        }
    }
    return Sum;
}
//------------------------------------------------------------------------------------------------

int LPR::DistanceHamming(std::vector<int> Solution)
{

    int MinCount = INT_MAX;
    for (auto CurrSol : Population)
    {
        int Count = 0;
        for (int Index = 0; Index < Solution.size(); ++Index)
            if (CurrSol[Index] != Solution[Index])
                ++Count;

        MinCount = std::min(MinCount, Count);
    }

    return MinCount;
}
//------------------------------------------------------------------------------------------------

std::vector<int> LPR::GenerateRandomSolution()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, NoColors);

    std::vector<int> Solution;
    Solution.resize(NoNodes);

    for (int node = 0; node < NoNodes; ++node)
    {
        Solution[node] = dist(gen);
    }

    return Solution;
}
//------------------------------------------------------------------------------------------------

std::vector<int> LPR::MixedPathRelinking(std::vector<int> FirstParent, std::vector<int> SecondParent)
{
    std::vector<int> DiffPos;
    for (int Index = 0; Index < NoNodes; ++Index)
        if (FirstParent[Index] != SecondParent[Index])
            DiffPos.push_back(Index);

    int DiffPosLen = DiffPos.size();
    std::vector<int> Last, PrevLast, TempSol;
    int SumConstraintsLast, SumConstraintsPrevLast, TempSum;

    PrevLast = FirstParent;
    Last = SecondParent;
    SumConstraintsPrevLast = SumConstraintViolations(PrevLast);
    SumConstraintsLast = SumConstraintViolations(Last);

    int CurrentLen = 2;
    while (DiffPos.size() > 0)
    {
        std::vector<int> CurrentChoice;
        if (CurrentLen % 2 == 0)
            CurrentChoice = SecondParent;
        else
            CurrentChoice = FirstParent;

        int BestSubstitutionCost = INT_MAX;
        int BestSubstitutionIndex = INT_MAX;

        for (int Index = 0; Index < DiffPos.size(); ++Index)
        {
            int CurrentDiffNode = DiffPos[Index];
            int AuxSum = SumConstraintsPrevLast;
            for (int Neighbour : AdjList[CurrentDiffNode])
            {
                AuxSum = AuxSum - std::max(0, Edges[CurrentDiffNode][Neighbour] - std::abs(PrevLast[CurrentDiffNode] - PrevLast[Neighbour]))
                        + std::max(0, Edges[CurrentDiffNode][Neighbour] - std::abs(CurrentChoice[CurrentDiffNode] - CurrentChoice[Neighbour]));
            }

            if (AuxSum < BestSubstitutionCost)
            {
                BestSubstitutionCost = AuxSum;
                BestSubstitutionIndex = Index;
            }
        }
        
        TempSol = PrevLast;
        TempSol[DiffPos[BestSubstitutionIndex]] = CurrentChoice[DiffPos[BestSubstitutionIndex]];
        PrevLast = Last;
        Last = TempSol;

        TempSum = BestSubstitutionCost;
        SumConstraintsPrevLast = SumConstraintsLast;
        SumConstraintsLast = TempSum;

        DiffPos.erase(DiffPos.begin() + BestSubstitutionIndex);
        ++CurrentLen;
    }
    return Last;
}
//------------------------------------------------------------------------------------------------

void LPR::TabuSearchImpr(std::vector<int>& Solution, bool IsAugmented)
{
    int IntervalIteration = 0;
    int Interval = 0;
    int CurrentIteration = 0;
    int CurrentDepth = 0;
    std::vector<int> BestSol = Solution;
    std::map<std::pair<int, int>, int> TabuTable;

    int LowestConstraintViolation = SumConstraintViolations(Solution);
    int SolutionCost, MaxDepth;
    if (IsAugmented)
    {
        SolutionCost = AugmentedSumConstraintViolations(Solution);
        MaxDepth = Alpha0;
    }
    else
    {
        SolutionCost = SumConstraintViolations(Solution);
        MaxDepth = Alpha;
    }

    std::vector<std::vector<int>> ColorChangeSum;
    std::vector<std::vector<int>> ColorChangeWeightSum;
    InitializePrecalcMatrixes(Solution, ColorChangeSum, ColorChangeWeightSum, IsAugmented);
    
    int NoRandCandidates = 100;
    int BestCandidateValue;
    int BestCandidateValueTabu;
    std::vector<std::pair<int, int>> BestCandidateList;
    std::vector<std::pair<int, int>> BestCandidateListTabu;
    std::pair<int, int> CurrChoice;
    std::pair<int, int> BestCandidate;

    while (CurrentDepth < MaxDepth)
    {
        if (LowestConstraintViolation == 0)
        {
            Solution = BestSol;
            return;
        }

        BestCandidateValue = INT_MAX;
        BestCandidateValueTabu = INT_MAX;

        BestCandidateList.clear();
        BestCandidateListTabu.clear();
        for (int Node = 0; Node < NoNodes; ++Node)
        {
            int Skip = false;

            if (ColorChangeSum[Node][Solution[Node]] == 0)
            {
                if (IsAugmented)
                {
                    if (ColorChangeWeightSum[Node][Solution[Node]] == 0)
                        Skip = true;
                }
                else Skip = true;
            }

            if (Skip)
                continue;

            for (int NewColor = 1; NewColor <= NoColors; ++NewColor)
            {
                if (Solution[Node] == NewColor)
                    continue;

                CurrChoice = { Node, NewColor };

                bool IsTabu = false;
                if (TabuTable.count(CurrChoice))
                {
                    int OutIteration = TabuTable[CurrChoice];
                    if (OutIteration > CurrentIteration)
                    {
                        IsTabu = true;
                    }
                    else
                    {
                        TabuTable.erase(CurrChoice);
                    }
                }

                int Delta = ColorChangeSum[Node][Solution[Node]] - ColorChangeSum[Node][NewColor];
                if (IsAugmented)
                    Delta += ColorChangeWeightSum[Node][Solution[Node]] - ColorChangeWeightSum[Node][NewColor];

                if (!IsTabu)
                {
                    if (SolutionCost - Delta < BestCandidateValue)
                    {
                        BestCandidateValue = SolutionCost - Delta;
                        BestCandidateList.clear();
                        BestCandidateList.push_back(CurrChoice);
                    }
                    else if (SolutionCost - Delta == BestCandidateValue && BestCandidateList.size() < NoRandCandidates)
                    {
                        BestCandidateList.push_back(CurrChoice);
                    }
                }
                else
                {
                    if (SolutionCost - Delta < BestCandidateValueTabu)
                    {
                        BestCandidateValueTabu = SolutionCost - Delta;
                        BestCandidateListTabu.clear();
                        BestCandidateListTabu.push_back(CurrChoice);
                    }
                    else if (SolutionCost - Delta == BestCandidateValueTabu && BestCandidateListTabu.size() < NoRandCandidates)
                    {
                        BestCandidateListTabu.push_back(CurrChoice);
                    }
                }
            }
        }

        if (BestCandidateList.size() == 0 && BestCandidateListTabu.size() == 0)
        {
            Solution = BestSol;
            return;
        }

        if (BestCandidateList.size() == 0 || BestCandidateValueTabu < std::min(BestCandidateValue, LowestConstraintViolation))
        {
            //Aspiration
            BestCandidate = BestCandidateListTabu[rand() % BestCandidateListTabu.size()];
            BestCandidateValue = BestCandidateValueTabu;
        }
        else
        {
            BestCandidate = BestCandidateList[rand() % BestCandidateList.size()];
        }

        TabuTable[BestCandidate] = CurrentIteration + TabuTenure[Interval] + rand() % 3;
        ++IntervalIteration;
        if (IntervalIteration > TabuTenureInterval[Interval])
        {
            Interval = (Interval + 1) % Pmax;
            IntervalIteration = 0;
        }

        SolutionCost = BestCandidateValue;
        UpdatePrecalcMatrixes(Solution, BestCandidate, ColorChangeSum, ColorChangeWeightSum, IsAugmented);
        Solution[BestCandidate.first] = BestCandidate.second;


        if (BestCandidateValue < LowestConstraintViolation)
        {
            LowestConstraintViolation = BestCandidateValue;
            BestSol = Solution;
            CurrentDepth = 0;
        }
        else
        {
            ++CurrentDepth;
        }

        ++CurrentIteration;
    }

    Solution = BestSol;
}
//------------------------------------------------------------------------------------------------

void LPR::TabuSearch(std::vector<int>& Solution, bool IsAugmented)
{
    int IntervalIteration = 0;
    int Interval = 0;
    int CurrentIteration = 0;
    int CurrentDepth = 0;
    std::vector<int> BestSol = Solution;
    std::map<std::pair<int, int>, int> TabuTable;

    int LowestConstraintViolation;
    if (IsAugmented)
        LowestConstraintViolation = AugmentedSumConstraintViolations(Solution);
    else
        LowestConstraintViolation = SumConstraintViolations(Solution);

    while (CurrentDepth < Alpha)
    {
        if (LowestConstraintViolation == 0)
        {
            Solution = BestSol;
            return;
        }

        int BestCandidateValue = INT_MAX;
        std::vector<std::pair<int, int>> BestCandidateList;

        for (int Node = 0; Node < NoNodes; ++Node)
        {
            for (int Neighbour = 0; Neighbour < NoNodes; ++Neighbour)
            {
                if (Edges[Node][Neighbour] > 0 && std::abs(Solution[Node] - Solution[Neighbour]) < Edges[Node][Neighbour])
                {
                    for (int NewColor = 1; NewColor <= NoColors; ++NewColor)
                    {
                        if (Solution[Node] == NewColor)
                            continue;

                        std::pair<int, int> CurrChoice = { Node, NewColor };

                        if (TabuTable.count(CurrChoice))
                        {
                            int OutIteration = TabuTable[CurrChoice];
                            if (OutIteration >= CurrentIteration)
                            {
                                continue;
                            }
                            else
                            {
                                TabuTable.erase(CurrChoice);
                            }
                        }

                        std::vector<int> TmpSol = Solution;
                        TmpSol[CurrChoice.first] = CurrChoice.second;
                        int TmpCost;
                        if (IsAugmented)
                            TmpCost = AugmentedSumConstraintViolations(TmpSol);
                        else
                            TmpCost = SumConstraintViolations(TmpSol);

                        if (TmpCost < BestCandidateValue)
                        {
                            BestCandidateValue = TmpCost;
                            BestCandidateList.clear();
                            BestCandidateList.push_back(CurrChoice);
                        }
                        else if (TmpCost == BestCandidateValue)
                        {
                            BestCandidateList.push_back(CurrChoice);
                        }
                    }
                }
            }
        }

        if (BestCandidateList.size() == 0)
        {
            Solution = BestSol;
            return;
        }

        std::pair<int, int> BestCandidate = BestCandidateList[rand() % BestCandidateList.size()];
        TabuTable[BestCandidate] = CurrentIteration + TabuTenure[Interval] + rand() % 3;
        ++IntervalIteration;
        if (IntervalIteration > TabuTenureInterval[Interval])
        {
            Interval = (Interval + 1) % Pmax;
            IntervalIteration = 0;
        }

        Solution[BestCandidate.first] = BestCandidate.second;


        if (BestCandidateValue < LowestConstraintViolation)
        {
            LowestConstraintViolation = BestCandidateValue;
            BestSol = Solution;
            CurrentDepth = 0;
        }
        else
            ++CurrentDepth;

        ++CurrentIteration;
    }

    Solution = BestSol;
}
//------------------------------------------------------------------------------------------------

void LPR::TwoPhaseTabuSearch(std::vector<int>& Solution)
{
    //TabuSearch(Solution, true);
    //TabuSearch(Solution, false);
    TabuSearchImpr(Solution, true);
    TabuSearchImpr(Solution, false);
}
//------------------------------------------------------------------------------------------------

void LPR::Improvement_and_Updating(std::vector<int>& CurrentSol, std::vector<int>& BestSol, std::set<std::pair<std::vector<int>, std::vector<int>>>& PairSet)
{
    TwoPhaseTabuSearch(CurrentSol);
    UpdatePenaltyMatrix(CurrentSol);   
    
    if (SumConstraintViolations(CurrentSol) < SumConstraintViolations(BestSol))
        BestSol = CurrentSol;
    
    int MaxConstraintViolation = 0;
    std::vector<int> WorstSol;
    for (auto Sol : Population)
    {
        int Sum = SumConstraintViolations(Sol);
        if (Sum > MaxConstraintViolation)
        {
            MaxConstraintViolation = Sum;
            WorstSol = Sol;
        }
    }

    if (SumConstraintViolations(CurrentSol) < SumConstraintViolations(WorstSol) &&
        DistanceHamming(CurrentSol) > 0.1f * NoNodes)
    {
        Population.insert(CurrentSol);
        Population.erase(WorstSol);
        for (auto KSol : Population)
        {
            if (PairSet.find({ WorstSol, KSol }) != PairSet.end())
            {
                PairSet.erase({ WorstSol, KSol });
                PairSet.insert({ CurrentSol, KSol });
            }

            if (PairSet.find({ KSol, WorstSol }) != PairSet.end())
            {
                PairSet.erase({ KSol, WorstSol });
                PairSet.insert({ KSol, CurrentSol });
            }

        }

    }
}
//------------------------------------------------------------------------------------------------

void LPR::UpdatePenaltyMatrix(std::vector<int> Solution)
{
    int MaxPenalty = 0;
    for (int v1 = 0; v1 < NoNodes; ++v1)
    {
        for (int v2 = 0; v2 < v1; ++v2)
        {
            if (Edges[v1][v2] > 0 && std::abs(Solution[v1] - Solution[v2]) < Edges[v1][v2])
            {
                ++PenaltyMatrix[v1][v2];
                ++PenaltyMatrix[v2][v1];
            }

            MaxPenalty = std::max(MaxPenalty, PenaltyMatrix[v1][v2]);
        }
    }

    if (MaxPenalty > MaxPenaltyWeight)
    {
        for (int v1 = 0; v1 < NoNodes; ++v1)
        {
            for (int v2 = 0; v2 < v1; ++v2)
            {
                PenaltyMatrix[v1][v2] = std::floor(ScalingFactor * PenaltyMatrix[v1][v2]);
                PenaltyMatrix[v2][v1] = std::floor(ScalingFactor * PenaltyMatrix[v2][v1]);
            }
        }
    }
}
//------------------------------------------------------------------------------------------------

void LPR::InitializePrecalcMatrixes(std::vector<int> Solution, std::vector<std::vector<int>>& ColorChangeSum, std::vector<std::vector<int>>& ColorChangeWeightSum, bool IsAugmented)
{
    ColorChangeSum.resize(NoNodes);
    for (int Node = 0; Node < NoNodes; ++Node)
    {
        ColorChangeSum[Node].resize(NoColors + 1);
        for (int NewColor = 1; NewColor <= NoColors; ++NewColor)
        {
            ColorChangeSum[Node][NewColor] = 0;
            for (auto Neighbour : AdjList[Node])
            {
                ColorChangeSum[Node][NewColor] += std::max(0, Edges[Node][Neighbour] - std::abs(Solution[Neighbour] - NewColor));
            }
        }
    }

    if (IsAugmented)
    {
        ColorChangeWeightSum.resize(NoNodes);
        for (int Node = 0; Node < NoNodes; ++Node)
        {
            ColorChangeWeightSum[Node].resize(NoColors + 1);
            for (int NewColor = 1; NewColor <= NoColors; ++NewColor)
            {
                ColorChangeWeightSum[Node][NewColor] = 0;
                for (auto Neighbour : AdjList[Node])
                {
                    if(std::abs(Solution[Neighbour] - NewColor) < Edges[Node][Neighbour])
                        ColorChangeWeightSum[Node][NewColor] += PenaltyMatrix[Node][Neighbour];
                }
            }
        }
    }
}
//------------------------------------------------------------------------------------------------

void LPR::UpdatePrecalcMatrixes(std::vector<int> Solution, std::pair<int, int> BestCandidate, std::vector<std::vector<int>>&ColorChangeSum, std::vector<std::vector<int>>& ColorChangeWeightSum, bool IsAugmented)
{
    int Start, End;
    int OldColor = Solution[BestCandidate.first];
    for (auto Neighbour : AdjList[BestCandidate.first])
    {
        Start = std::max(1, OldColor - Edges[BestCandidate.first][Neighbour] + 1);
        End = std::min(NoColors, OldColor + Edges[BestCandidate.first][Neighbour] - 1);
        for (int NewColor = Start; NewColor <= End; ++NewColor)
        {
            ColorChangeSum[Neighbour][NewColor] -= (Edges[Neighbour][BestCandidate.first] - std::abs(OldColor - NewColor));
        }

        Start = std::max(1, BestCandidate.second - Edges[BestCandidate.first][Neighbour] + 1);
        End = std::min(NoColors, BestCandidate.second + Edges[BestCandidate.first][Neighbour] - 1);

        for (int NewColor = Start; NewColor <= End; ++NewColor)
        {
            ColorChangeSum[Neighbour][NewColor] += (Edges[Neighbour][BestCandidate.first] - std::abs(BestCandidate.second - NewColor));
        }

    }

    if (IsAugmented)
    {
        for (auto Neighbour : AdjList[BestCandidate.first])
        {
            Start = std::max(1, OldColor - Edges[BestCandidate.first][Neighbour] + 1);
            End = std::min(NoColors, OldColor + Edges[BestCandidate.first][Neighbour] - 1);
            for (int NewColor = Start; NewColor <= End; ++NewColor)
            {
                ColorChangeWeightSum[Neighbour][NewColor] -= PenaltyMatrix[Neighbour][BestCandidate.first];
            }

            Start = std::max(1, BestCandidate.second - Edges[BestCandidate.first][Neighbour] + 1);
            End = std::min(NoColors, BestCandidate.second + Edges[BestCandidate.first][Neighbour] - 1);

            for (int NewColor = Start; NewColor <= End; ++NewColor)
            {
                ColorChangeWeightSum[Neighbour][NewColor] += PenaltyMatrix[Neighbour][BestCandidate.first];
            }
        }
    }
}
//------------------------------------------------------------------------------------------------
