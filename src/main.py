import numpy as np
import networkx as nx
import cPickle as cp
import random
import ctypes
import os
import sys
from tqdm import tqdm

sys.path.append( '%s/mvc_lib' % os.path.dirname(os.path.realpath(__file__)) )
from mvc_lib import MvcLib

n_valid = 100

def gen_graph(opt):
    max_n = int(opt['max_n'])
    min_n = int(opt['min_n'])
    max_w = int(opt['max_w'])
    min_w = int(opt['min_w'])
    cur_n = np.random.randint(max_n - min_n + 1) + min_n
    if opt['g_type'] == 'erdos_renyi':
        g = nx.erdos_renyi_graph(n = cur_n, p = 0.15)
    elif opt['g_type'] == 'powerlaw':
        g = nx.powerlaw_cluster_graph(n = cur_n, m = 4, p = 0.05)
    elif opt['g_type'] == 'barabasi_albert':
        g = nx.barabasi_albert_graph(n = cur_n, m = 4)
    nx.set_node_attributes(g, dict(zip(range(cur_n), np.random.randint(min_w, max_w, cur_n))),
                           'weight')
    return g

def gen_new_graphs(opt):
    print 'generating new training graphs'
    sys.stdout.flush()
    api.ClearTrainGraphs()
    for i in tqdm(range(1000)):
        g = gen_graph(opt)
        api.InsertGraph(g, is_test=False)

def PrepareValidData(opt):
    print 'generating validation graphs'
    sys.stdout.flush()
    for i in tqdm(range(n_valid)):
        g = gen_graph(opt)
        api.InsertGraph(g, is_test=True)

def find_model_file(opt):
    max_n = int(opt['max_n'])
    min_n = int(opt['min_n'])
    log_file = None
    if max_n < 100:
	return None
    if min_n == 50 and max_n == 100:
        return None
    elif min_n == 100 and max_n == 200:
        n1 = 50
        n2 = 100
    else:
        n1 = min_n - 100
        n2 = max_n - 100

    log_file = '%s/log-%d-%d.txt' % (opt['save_dir'], n1, n2)
    if not os.path.isfile(log_file):
	return None
    best_r = -1000000
    best_it = -1
    with open(log_file, 'r') as f:
        for line in f:
            if 'average' in line:
                line = line.split(' ')
                it = int(line[1].strip())
                r = -float(line[-1].strip())
                if it > 10000 and r > best_r:
                    best_r = r
                    best_it = it
    if best_it < 0:
        return None
    print best_it, best_r
    return '%s/nrange_%d_%d_iter_%d.model' % (opt['save_dir'], n1, n2, best_it)
    
if __name__ == '__main__':
    api = MvcLib(sys.argv)
    
    opt = {}
    for i in range(1, len(sys.argv), 2):
        opt[sys.argv[i][1:]] = sys.argv[i + 1]

    model_file = find_model_file(opt)
    if model_file is not None:
        print 'loading', model_file
        sys.stdout.flush()
        api.LoadModel(model_file)
    
    PrepareValidData(opt)

    # startup
    gen_new_graphs(opt)
    for i in range(10):
        api.lib.PlayGame(100, ctypes.c_double(1.0))
    api.TakeSnapshot()

    eps_start = 1.0
    eps_end = 0.05
    eps_step = 10000.0
    for iter in range(int(opt['max_iter'])):
        if iter and iter % 5000 == 0:
            gen_new_graphs(opt)
        eps = eps_end + max(0., (eps_start - eps_end) * (eps_step - iter) / eps_step)
        if iter % 10 == 0:
            api.lib.PlayGame(10, ctypes.c_double(eps))

        if iter % 300 == 0:
            frac = 0.0
            frac_fastwvc = 0.0
            for idx in range(n_valid):
                frac += api.lib.Test(idx)
                val_fastwvc, _ = api.FastWVC(True, idx, 0, 60, 2, False)
                frac_fastwvc += val_fastwvc
            print 'iter', iter, 'eps', eps, 'average size of vc: ', frac / n_valid, 'FastWVC average:', frac_fastwvc / n_valid
            sys.stdout.flush()
            model_path = '%s/nrange_%d_%d_iter_%d.model' % (opt['save_dir'], int(opt['min_n']), int(opt['max_n']), iter)
            api.SaveModel(model_path)

        if iter % 1000 == 0:
            api.TakeSnapshot()

        api.lib.Fit()
