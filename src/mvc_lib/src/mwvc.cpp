#include <chrono>
#include <sstream>
#include <iostream>

#include <fast_wvc.h>

using namespace std;

int main(int argc, char *argv[])
{
    unsigned int seed;
    unsigned int cutoff_time;
    unsigned int mode;
    
    if (argc == 1)
    {
        cout << "FastWVC - a Minimum Weighted Vertex Cover Problem solver." << endl;
        cout << "Usage: ./mwvc [Graph file] [Seed] [Cutoff time] [CC mode]" << endl;
        return 1;
    }

    if (argc < 5)
    {
        cerr << "Missing argument(s)." << endl;
        cout << "Usage: ./mwvc [Graph file] [Seed] [Cutoff time] [CC mode]" << endl;
        return 1;
    }

    stringstream ss;
    ss << argv[2];
    ss >> seed;
    ss.clear();
    ss << argv[3];
    ss >> cutoff_time;
    ss.clear();
    ss << argv[4];
    ss >> mode;
    ss.clear();

    Graph g = BuildInstance(argv[1]);
    // BuildInstance aborts on failure.

    if (seed < 0U || seed > ~0U)
    {
        seed = 10;
    }

    if (cutoff_time < 0 || cutoff_time > (int)(~0U>>1))
    {
        cutoff_time = 1000;
    }

    if (mode < 0 || mode > 3)
    {
        mode = 0;
    }

    srand(seed);
    g.seed = seed;
    g.mode = mode;

    cout << argv[1];

    std::chrono::steady_clock::time_point deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(cutoff_time);

    ConstructVC(g);
    LocalSearch(g, deadline);

    if (CheckSolution(g) == 1)
    {
        cout << ", " << g.now_weight << endl;
    }
    else
    {
        cout << ", the solution is wrong." << endl;
    }

    return 0;
}
