#ifndef MVC_LIB_H
#define MVC_LIB_H

extern "C" int Init(const int argc, const char** argv);

extern "C" int InsertGraph(bool isTest, const int g_id, const int num_nodes, const int num_edges, const int* edges_from, const int* edges_to, const int* node_weights);

extern "C" int LoadModel(const char* filename);

extern "C" int SaveModel(const char* filename);

extern "C" int UpdateSnapshot();

extern "C" int ClearTrainGraphs();

extern "C" int PlayGame(const int n_traj, const double eps);

extern "C" double Fit();
extern "C" double Test(const int gid);

extern "C" double GetSol(const int gid, int* sol);

extern "C" double FastWVC(bool isTest, const int gid, int* sol, int timeout_seconds, int max_steps,
                          bool use_randvc);

#endif
