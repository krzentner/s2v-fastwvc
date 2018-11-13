#ifndef FAST_WVC_H
#define FAST_WVC_H

#include <string>
#include <chrono>
#include <graph.h>

Graph BuildInstance(std::string);
void ResetRemoveCand(Graph &);
void Uncover(Graph &, int);
void Cover(Graph &, int);
void Add(Graph &, int);
void Remove(Graph &, int);
int UpdateTargetSize(Graph &);
int ChooseRemoveV(Graph &);
int ChooseAddV(Graph &, int, int);
void UpdateBestSolution(Graph &, Graph &);
void RemoveRedundant(Graph &);
void ConstructVC(Graph &);
int CheckSolution(Graph &);
void ForgetEdgeWeights(Graph &);
void UpdateEdgeWeight(Graph &);
void LocalSearch(Graph &, std::chrono::steady_clock::time_point deadline);

#endif // FAST_WVC_H
