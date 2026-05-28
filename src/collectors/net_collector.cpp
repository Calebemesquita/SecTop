#include "net_collector.hpp"

#include <stdexcept>
#include <cstring>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/sock_diag.h>
#include <linux/inet_diag.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace sectop{

    struct NetlinkRequest{
        struct nlmsghdr nlmh;
        struct inet_diag_req_v2 req;
    };


    void NetCollector::collect(){
        nl_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_INET_DIAG);
        if(nl_fd < 0){
            throw std::runtime_error("Erro ao criar socket");
        }
    }

    NetCollector::~NetCollector() {
        if (nl_fd >= 0) {
            close(nl_fd);
        }
    }

    std::string NetCollector::get_state_string(uint8_t state) {
        switch(state){
            case 1: return "ESTABLISHED";
            case 2: return "SYN_SENT";
            case 3: return "SYN_RECV";
            case 4: return "FIN_WAIT1";
            case 5: return "FIN_WAIT2";
            case 6: return "TIME_WAIT";
            case 7: return "CLOSE";
            case 8: return "CLOSE_WAIT";
            case 9: return "LAST_ACK";
            case 10: return "LISTEN";
            case 11: return "CLOSING";
            default: return "UNKNOWN";
        }
    }


    // REFAZER DAQUI PARA BAIXO
    void NetCollector::collect() {
        current_connections.clear();

        NetlinkRequest request;
        memset(&request, 0, sizeof(request));

        request.nlmh.nlmsg_len = sizeof(request);
        request.nlmh.nlmsg_type = SOCK_DIAG_BY_FAMILY;
        request.nlmh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
        
        request.req.sdiag_family = AF_INET;
        request.req.sdiag_protocol = IPPROTO_TCP;
        request.req.idiag_states = 0xFFF; 

        struct sockaddr_nl sa;
        memset(&sa, 0, sizeof(sa));
        sa.nl_family = AF_NETLINK;

        if (sendto(nl_fd, &request, sizeof(request), 0, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
            throw std::runtime_error("NetCollector: Erro ao enviar requisicao ao kernel (sendto).");
        }

        char buffer[8192];
        bool done = false;

        while (!done) {
            ssize_t bytes_read = recv(nl_fd, buffer, sizeof(buffer), 0);
            if (bytes_read <= 0) break;

            struct nlmsghdr *h = (struct nlmsghdr *)buffer;

            for (; NLMSG_OK(h, bytes_read); h = NLMSG_NEXT(h, bytes_read)) {
                if (h->nlmsg_type == NLMSG_DONE) {
                    done = true;
                    break;
                }
                if (h->nlmsg_type == NLMSG_ERROR) {
                    throw std::runtime_error("NetCollector: Erro retornado pelo Kernel no pacote Netlink.");
                }
                if (h->nlmsg_type != SOCK_DIAG_BY_FAMILY) {
                    continue;
                }

                struct inet_diag_msg *diag = (struct inet_diag_msg *)NLMSG_DATA(h);

                if (diag->idiag_family != AF_INET) {
                    continue;
                }

                Connection conn;
                conn.local_port = ntohs(diag->id.idiag_sport);
                conn.remote_port = ntohs(diag->id.idiag_dport);
                conn.inode = diag->idiag_inode;
                conn.state = get_state_string(diag->idiag_state);

                char src_ip[INET_ADDRSTRLEN];
                char dst_ip[INET_ADDRSTRLEN];
                inet_ntop(diag->idiag_family, diag->id.idiag_src, src_ip, sizeof(src_ip));
                inet_ntop(diag->idiag_family, diag->id.idiag_dst, dst_ip, sizeof(dst_ip));

                conn.local_ip = src_ip;
                conn.remote_ip = dst_ip;

                current_connections.push_back(conn);
            }
        }
    }

    const std::vector<Connection>& NetCollector::get_connections() const {
        return current_connections;
    }



}



