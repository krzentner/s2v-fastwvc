#!/usr/bin/env python2.7

import main
import sys
import itertools

SIZES = [
    (15, 20),
    # (20, 35),
    # (35, 50),
    (20, 50),
    (50, 100)
    # (50, 65),
    # (65, 80),
    # (80, 100),
]

WEIGHT_RANGES = [(1, 100)]

TYPES = [
    'erdos_renyi',
    'powerlaw',
    'barabasi_albert'
    ]

MAX_ITERATIONS = [
    0,
    1,
    10,
    100,
    1000
    ]

NUM_GRAPHS_PER_CATEGORY = 100

# Note that this is large enough to basically ignore.
TIMEOUT_SECONDS = 600 

api = main.MvcLib(sys.argv)

column_headers = ['Graph Type-min_n-max_n vs FastWVC iterations'] + MAX_ITERATIONS
n_ranges = ['{}-{}'.format(min_n, max_n) for min_n, max_n in SIZES]
graph_type_names = [' '.join([word.capitalize() for word in t.split('_')]) for t in TYPES]
row_headers = ['{}-{}-{}'.format(g_type_name, n_range, use_randvc)
               for g_type_name in graph_type_names
               for use_randvc in ('R', 'C')
               for n_range in n_ranges]


table = []
for t in TYPES:
    for use_randvc in (True, False):
        for min_w, max_w in WEIGHT_RANGES:
            for min_n, max_n in SIZES:
                opt = dict(g_type=t, min_w=min_w, max_w=max_w, min_n=min_n,
                        max_n=max_n)
                total_weight_per_iteration = [0] * len(MAX_ITERATIONS)
                api.ClearTrainGraphs()
                for i in range(NUM_GRAPHS_PER_CATEGORY):
                    g = main.gen_graph(opt)
                    api.InsertGraph(g, is_test=False)
                    for j, iterations in enumerate(MAX_ITERATIONS):
                        w, cover = api.FastWVC(False, i, max_n, TIMEOUT_SECONDS,
                                               iterations, use_randvc)
                        total_weight_per_iteration[j] += w
                table.append([total_w / NUM_GRAPHS_PER_CATEGORY for total_w in
                            total_weight_per_iteration])

for h in column_headers[:-1]:
    print h, ',',
print column_headers[-1]
for i, row in enumerate(table):
    print row_headers[i], ',',
    for el in row[:-1]:
        print el, ',',
    print row[-1]
