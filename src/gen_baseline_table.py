import main
import sys

SIZES = [
    (15, 20),
    (20, 50),
    (50, 100),
    (100, 250),
    (250, 500),
    (500, 1000),
    (1000, 1500)
]

WEIGHT_RANGES = [(1, 100)]

TYPES = [
    'erdos_renyi',
    'powerlaw',
    'barabasi_albert'
    ]

NUM_GRAPHS_PER_CATEGORY = 100

# Note that whichever of these happens *first* stops FastWVC.
MAX_ITERATIONS = 10
TIMEOUT_SECONDS = 600 

api = main.MvcLib(sys.argv)

column_headers = ['Graph Type \ min_n,max_n'] + ['{},{}'.format(min_n, max_n) for min_n, max_n in SIZES]
row_headers = [' '.join([word.capitalize() for word in t.split('_')]) for t in TYPES]

table = []
for t in TYPES:
    for min_w, max_w in WEIGHT_RANGES:
        row = []
        for min_n, max_n in SIZES:
            opt = dict(g_type=t, min_w=min_w, max_w=max_w, min_n=min_n,
                       max_n=max_n)
            total_weight = 0
            api.ClearTrainGraphs()
            for i in range(NUM_GRAPHS_PER_CATEGORY):
                g = main.gen_graph(opt)
                api.InsertGraph(g, is_test=False)
                w, cover = api.FastWVC(False, i, max_n, TIMEOUT_SECONDS,
                                       MAX_ITERATIONS)
                total_weight += w
            row.append(total_weight / NUM_GRAPHS_PER_CATEGORY)
        table.append(row)

for h in column_headers:
    print h, '\t',
print
for i, row in enumerate(table):
    print row_headers[i], '\t',
    for el in row:
        print el, '\t',
    print
