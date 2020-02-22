#include "util.hpp"

#include <numeric>
#include <algorithm>
#include <capnp/message.h>
#include <capnp/serialize.h>

void quit()
{
    system("pause");
    exit(0);
}

void align_malloc(void **memptr, size_t alignment, size_t size)
{
    int malloc_flag = posix_memalign(memptr, alignment, size);
    if (malloc_flag) {
        std::cerr << "posix_memalign: " << strerror(malloc_flag) << std::endl;
        quit();
    }
}

std::string extract_filename(const std::string full_filename)
{
    int pos = full_filename.find_last_of('.');
    return full_filename.substr(0, pos);
}

int arg_pos(char *str, int argc, char **argv)
{
  int a;
  for (a = 1; a < argc; a++) if (!strcmp(str, argv[a])) {
    if (a == argc - 1) {
      printf("Argument missing for %s\n", str);
      quit();
    }
    return a;
  }
  return -1;
}

EdgeVector load_graph(const std::string path)
{
    EdgeVector edge_vec;
    FILE *fp = fopen(path.c_str(), "r");
    if (fp == NULL) {
        std::cout << "fail to open " << path << std::endl;
        quit();
    }

    char line[512];
    while (fgets(line, 512, fp) != NULL) {
        if (line[0] == '#') continue;
        int u = 0, v = 0;
        const char *c = line;
        while (isdigit(*c))
            u = (u << 1) + (u << 3) + (*c++ - 48);
        c++;
        while (isdigit(*c))
            v = (v << 1) + (v << 3) + (*c++ - 48);
        edge_vec.push_back(std::make_pair(u, v));
    }    
    fclose(fp);
    
    return edge_vec;
}

std::vector<int> load_vertex_order(const std::string path)
{    
    FILE *fp = fopen(path.c_str(), "r");
    if (fp == NULL) {
        std::cout << "fail to open " << path << std::endl;
        quit();
    }

    EdgeVector id_pair;
    char line[512];
    while (fgets(line, 512, fp) != NULL) {
        if (line[0] == '#') continue;
        int u = 0, v = 0;
        const char *c = line;
        while (isdigit(*c))
            u = (u << 1) + (u << 3) + (*c++ - 48);
        c++;
        while (isdigit(*c))
            v = (v << 1) + (v << 3) + (*c++ - 48);
        id_pair.push_back(std::make_pair(u, v));
    }  
    fclose(fp);

    std::vector<int> order(id_pair.size());
    for (auto& p : id_pair)
        order[p.first] = p.second;

    return order;
}

void save_graph(const std::string path, const EdgeVector& edge_vec)
{
    FILE *fp = fopen(path.c_str(), "w");
    if (fp == NULL) {
        std::cout << "fail to create " << path << std::endl;
        quit();
    }

    for (auto& e : edge_vec) {
        fprintf(fp, "%d %d\n", e.first, e.second);
    }
    fclose(fp);
}

void save_newid(const std::string path, std::vector<int> org2newid)
{
    FILE *fp = fopen(path.c_str(), "w");
    if (fp == NULL) {
        std::cout << "fail to create " << path << std::endl;
        quit();
    }

    for (int i = 0; i < (int)org2newid.size(); ++i)
        fprintf(fp, "%d %d\n", i, org2newid[i]);
    fclose(fp);
}

bool edge_idpair_cmp(const Edge& a, const Edge& b)
{
    if(a.first == b.first) return a.second < b.second;
    else return a.first < b.first;
}

EdgeVector load_rr_graph(const ucap::RrGraph::Reader &rr_graph) {
  EdgeVector edges;
  auto rr_edges = rr_graph.getRrEdges().getEdges();
  edges.reserve(rr_edges.size());

  for(auto e : rr_edges) {
    int u = e.getSrcNode();
    int v = e.getSinkNode();
    if (u != v) {
        edges.push_back(std::make_pair(u, v));
    }
  }
  return edges;
}

void save_rr_graph(const std::vector<int> &order,
                   const ucap::RrGraph::Reader &in,
                   ucap::RrGraph::Builder &out) {

  int i;
  out.setToolComment(in.getToolComment());
  out.setToolName(in.getToolName());
  out.setToolVersion(in.getToolVersion());
  out.setChannels(in.getChannels());
  out.setSwitches(in.getSwitches());
  out.setSegments(in.getSegments());
  out.setBlockTypes(in.getBlockTypes());
  out.setConnectionBoxes(in.getConnectionBoxes());
  out.setGrid(in.getGrid());

  // find where nodes will go and store them
  auto nodesIn = in.getRrNodes().getNodes();
  auto nodesOut = out.getRrNodes().initNodes(nodesIn.size());
  std::vector<bool> written_nodes(nodesIn.size(), false);
  for(i = 0; i < nodesIn.size(); i++) {
    int u = order[i];
    assert(!written_nodes[u]);
    nodesOut.setWithCaveats(u, nodesIn[i]);
    nodesOut[u].setId(u);
    written_nodes[u] = true;
  }

  // check the order
  i = 0;
  for(auto nodeOut : nodesOut) {
    assert(i == nodeOut.getId());
    i++;
  }

  for(auto w : written_nodes) assert(w);

  // sort the edges by src then sink
  auto edgesIn = in.getRrEdges().getEdges();
  std::vector<int> edge_list(edgesIn.size());
  std::iota(std::begin(edge_list), std::end(edge_list), 0);
  std::sort(edge_list.begin(), edge_list.end(),
            [&](int ai, int bi) {
              const auto &a = edgesIn[ai];
              const auto &b = edgesIn[bi];
              return order[a.getSrcNode()] < order[b.getSrcNode()] ||
                                         (order[a.getSrcNode()] == order[b.getSrcNode()] &&
                                          order[a.getSinkNode()] < order[b.getSinkNode()]);
            });

  // store them
  i = 0;
  std::vector<bool> written_edges(edgesIn.size(), false);
  auto edgesOut = out.getRrEdges().initEdges(edgesIn.size());
  for(auto e : edge_list) {
    const auto &edgeIn = edgesIn[e];
    assert(!written_edges[e]);
    edgesOut.setWithCaveats(i, edgeIn);
    edgesOut[i].setSinkNode(order[edgeIn.getSinkNode()]);
    edgesOut[i].setSrcNode(order[edgeIn.getSrcNode()]);
    written_edges[e] = true;
    i++;
  }

  for(auto w : written_edges) assert(w);
}
