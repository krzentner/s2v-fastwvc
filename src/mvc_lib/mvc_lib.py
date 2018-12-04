import ctypes
import networkx as nx
import numpy as np
import os
import sys

class MvcLib(object):

    def __init__(self, args):
        dir_path = os.path.dirname(os.path.realpath(__file__))        
        self.lib = ctypes.CDLL('%s/build/dll/libmvc.so' % dir_path)

        self.lib.Fit.restype = ctypes.c_double
        self.lib.Test.restype = ctypes.c_double
        self.lib.GetSol.restype = ctypes.c_double
        self.lib.FastWVC.restype = ctypes.c_double
        arr = (ctypes.c_char_p * len(args))()
        arr[:] = args
        self.lib.Init(len(args), arr)
        self.ngraph_train = 0
        self.ngraph_test = 0

    def __CtypeNetworkX(self, g):
        edges = g.edges()
        e_list_from = (ctypes.c_int * len(edges))()
        e_list_to = (ctypes.c_int * len(edges))()
        nodes = g.nodes()
        v_weights = (ctypes.c_int * len(nodes))()

        if len(edges):
            a, b = zip(*edges)        
            e_list_from[:] = a
            e_list_to[:] = b

        for n in nodes:
            v_weights[n] = nodes[n]['weight']

        return (len(g.nodes()), len(edges), ctypes.cast(e_list_from,
                ctypes.c_void_p), ctypes.cast(e_list_to, ctypes.c_void_p),
                ctypes.cast(v_weights, ctypes.c_void_p)) 

    def TakeSnapshot(self):
        self.lib.UpdateSnapshot()

    def ClearTrainGraphs(self):
        self.ngraph_train = 0
        self.lib.ClearTrainGraphs()

    def InsertGraph(self, g, is_test):
        n_nodes, n_edges, e_froms, e_tos, v_weights = self.__CtypeNetworkX(g)
        if is_test:
            t = self.ngraph_test
            self.ngraph_test += 1
        else:
            t = self.ngraph_train
            self.ngraph_train += 1

        self.lib.InsertGraph(is_test, t, n_nodes, n_edges, e_froms, e_tos, v_weights)
    
    def LoadModel(self, path_to_model):
        p = ctypes.cast(path_to_model, ctypes.c_char_p)
        self.lib.LoadModel(p)

    def SaveModel(self, path_to_model):
        p = ctypes.cast(path_to_model, ctypes.c_char_p)
        self.lib.SaveModel(p)

    def GetSol(self, gid, maxn):
        sol = (ctypes.c_int * (maxn + 10))()
        val = self.lib.GetSol(gid, sol)
        return val, sol

    def FastWVC(self, is_test, gid, maxn, timeout_seconds, max_steps):
        sol = None
        if maxn != 0:
            sol = (ctypes.c_int * (maxn + 10))()
        val = self.lib.FastWVC(is_test, gid, sol, timeout_seconds, max_steps)
        return val, sol

if __name__ == '__main__':
    f = MvcLib(sys.argv)
