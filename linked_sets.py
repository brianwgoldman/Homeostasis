import sys
from collections import defaultdict
from itertools import permutations


with open(sys.argv[1], 'r') as f:
    raw = f.read().strip().split('\n')

no_comments = [line.split('#')[0].strip() for line in raw]
lines = [line for line in no_comments if len(line) > 0]

header = lines[0].split()
frequencies = {pair: defaultdict(int) for pair in permutations(header, 2)}
for line in lines[1:]:
    values = line.split()
    labeled = zip(header, values)
    for first, second in permutations(labeled, 2):
        pair, values = zip(first, second)
        frequencies[pair][values] += 1

def identical(occurances):
    # Assumes something only occurs if it was seen once
    #print occurances.keys()
    first, second = zip(*occurances.keys())
    return len(set(first)) == len(set(second)) == len(occurances)

edges = defaultdict(list)
for data in frequencies.items():
    if identical(data[1]):
        edges[data[0][0]].append(data[0][1])
seen = set()
print "Linked Sets"
linked_sets = 0
for source, neighbors in edges.items():
    if source not in seen:
        seen.add(source)
        seen.update(neighbors)
        print ', '.join(sorted([source] + neighbors))
        linked_sets += 1
print "Free Variables"
free = {x for x in header if x not in seen}
print ', '.join(sorted(free))
print "Total:", len(header)
print len(seen), "variables in", linked_sets, "sets"
print "free", len(free)

