#include "porder.hpp"
#include "utils/util.hpp"
#include <capnp/message.h>
#include <capnp/serialize.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

POrder porder;

int main(int argc, char* argv[])
{
    if (argc == 1) {
        printf("Please provide graph file path.\n");
        quit();
    }

    std::string graph_file_path(argv[1]);
    std::string graph_name = extract_filename(graph_file_path);
    std::string order_option = "gro";
    int i;
    if ((i = arg_pos((char *)"-order", argc, argv)) > 0)
        order_option = std::string(argv[i + 1]);
    else if ((i = arg_pos((char *)"-ratio", argc, argv)) > 0) {
        EdgeVector edge_vec = load_graph(graph_file_path);
        porder.load_org_graph(edge_vec);
        porder.comp_ratio();
        return 0;
    }
        
    struct timeval time_start;
    struct timeval time_end;

    gettimeofday(&time_start, NULL);
    int fd_in = open(graph_file_path.c_str(), O_RDONLY);
    if (fd_in < 0) {
        std::cout << "Failed to open " << graph_file_path << std::endl;
        quit();
    }

    ::capnp::ReaderOptions opts = ::capnp::ReaderOptions();
    opts.traversalLimitInWords = 1ul << 30;
    ::capnp::StreamFdMessageReader messageIn(fd_in, opts);
    auto rrIn = messageIn.getRoot<ucap::RrGraph>();
    EdgeVector edge_vec = load_rr_graph(rrIn);
    gettimeofday(&time_end, NULL);
    double read_time = (time_end.tv_sec - time_start.tv_sec) * 1000.0 + (time_end.tv_usec - time_start.tv_usec) / 1000.0;
    printf("read_time=%.3fms\n", read_time);

    gettimeofday(&time_start, NULL);
    porder.load_org_graph(edge_vec);
    gettimeofday(&time_end, NULL);
    double build_time = (time_end.tv_sec - time_start.tv_sec) * 1000.0 + (time_end.tv_usec - time_start.tv_usec) / 1000.0;
    printf("build_time=%.3fms\n", build_time);

    // porder.leaf_node_count();
    // porder.select_bignode(0.8);
    EdgeVector new_edge_vec;
    gettimeofday(&time_start, NULL);

    std::string suffix = "GRO";
    if (order_option == "hybrid") {
        suffix = "Horder";
        new_edge_vec = porder.hybrid_bfsdeg();
    } else if (order_option == "mloggapa") {
        suffix = "MLOGGAPAorder";
        new_edge_vec = porder.mloggapa_order();       
    } else if (order_option == "metis") {
        suffix = "METISorder";
        new_edge_vec = porder.metis_order();
    } else if (order_option == "slashburn") {
        suffix = "SBorder";
        new_edge_vec = porder.slashburn_order();        
    } else if (order_option == "bfsr") {
        suffix = "BFSRorder";
        new_edge_vec = porder.bfsr_order();        
    } else if (order_option == "dfs") {
        suffix = "DFSorder";
        new_edge_vec = porder.dfs_order();        
    } else {
        order_option = "gro";
        suffix = "GRO";
        new_edge_vec = porder.greedy_mheap(); 
    }

    printf("order_algo=%s graph_file=%s\n", order_option.c_str(), graph_name.c_str());
    gettimeofday(&time_end, NULL);
    double reorder_time = (time_end.tv_sec - time_start.tv_sec) * 1000.0 + (time_end.tv_usec - time_start.tv_usec) / 1000.0;    
    if (reorder_time > 10000.0) printf("reorder_time=%.3fs\n", reorder_time / 1000.0);
    else printf("reorder_time=%.3fms\n", reorder_time);

    porder.comp_ratio();

    gettimeofday(&time_start, NULL);
    std::string output_filename = graph_name + "." + suffix + ".bin";
    int fd_out = open(output_filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd_out < 0) {
        std::cout << "Fail to open " << output_filename << std::endl;
        quit();
    }

    ::capnp::MallocMessageBuilder messageOut;
    auto rrOut = messageOut.initRoot<ucap::RrGraph>();
    save_rr_graph(porder.org2newid, rrIn, rrOut);
    writeMessageToFd(fd_out, messageOut);
    gettimeofday(&time_end, NULL);
    double write_time = (time_end.tv_sec - time_start.tv_sec) * 1000.0 + (time_end.tv_usec - time_start.tv_usec) / 1000.0;
    printf("write_time=%.3fms\n", write_time);

    return 0;
}


