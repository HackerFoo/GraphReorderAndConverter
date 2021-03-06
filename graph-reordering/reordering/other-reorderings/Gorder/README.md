> Welcome to Gorder!

> Gorder is intended to generate the node ordering that preserves the graph data locality. The node ordering is used to renumber the node IDs in the original dataset and a new dataset with the new node IDs is the output of the software. The new dataset keeps the same topological structure with the original dataset. Please refer to our paper "Speedup Graph Processing by Graph Ordering" for more details (http://dl.acm.org/citation.cfm?id=2915220).

The software can be run as followed,

```zsh
./Gorder <filename> [-w [w]]
```

For example,

```zsh
./Gorder LiveJournal.txt -w 5 (LiveJournal.txt is the input file)
```

> The default value of parameter w is `5` if the software is run without setting the parameter -w.
An example of the format of the input file is as followed,

```
0       1
0       2
1       3
...
```

> Generally speaking, the format of the input file is similar with the one used in SNAP (http://snap.stanford.edu/data/). Each line is an edge of the graph and there should be M lines in the input file if the graph has M edges. The first integer of each line denotes the start node of the edge and the second integer denotes the end node of the edge. No duplicate edges are allowed. The node IDs of the dataset should be continuous and start with 0.

> Tested on Linux system using GCC 4.9.2 and GCC 5.3.0.

> Please cite our paper if the software is used for research works.
@inproceedings{Wei:2016:SGP:2882903.2915220,
 author = {Wei, Hao and Yu, Jeffrey Xu and Lu, Can and Lin, Xuemin},
 title = {Speedup Graph Processing by Graph Ordering},
 booktitle = {Proceedings of the 2016 International Conference on Management of Data},
 series = {SIGMOD '16},
 year = {2016},
 keywords = {CPU performance, graph algorithms, graph ordering},
}

> If you have any further questions, feel free to contact me via email, weihaohal@gmail.com. My homepage is http://www1.se.cuhk.edu.hk/~hwei.

