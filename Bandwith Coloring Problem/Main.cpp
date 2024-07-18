#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <filesystem>
#include <map>
#include <opencv2/opencv.hpp>

#include "Solver.h"

#include "UETT.h"

int main()
{

    std::string Instance = R"(C:\Users\lucian.isac\source\repos\Bandwith Coloring Problem\Bandwith Coloring Problem\Instances\UETT_Instances\generated_json)";

    for (const auto& entry : fs::directory_iterator(Instance)) {
        if (entry.is_regular_file()) {
            UETT solv(entry.path().string());
            solv.Solve();
        }
    }
    
    return 0;

    std::string InstancesPath = "D:\\BCP_Instances";
    Solver sol(InstancesPath);
    sol.Solve();
}
