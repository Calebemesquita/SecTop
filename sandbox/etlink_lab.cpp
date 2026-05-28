/*
Esse codigo possui uma quantidade muito grande de comentarios pois ele foi feito para brincar é testar conhecimento alem de adiquirilo e entender o coidgo
O codigo real provavelmente deve estar mais bem docummentado

esse visa apenas a compreenção
*/



#include <iostream>

#include <linux/netlink.h> // para cabeçario ali
#include <linux/inet_diag.h> // inet_diag_req_v2
#include <linux/tcp.h>
#include <sys/socket.h>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/sock_diag.h>
#include <linux/inet_diag.h>
#include <arpa/inet.h> // Para inet_ntop e ntohs
#include <unistd.h>


// [ nlmsghdr ][ inet_diag_req_v2 ] Pacote que vou enviar ao kernel
struct NetlinkRequest{
    struct nlmsghdr nlmh; // cabeçario da mensagem netlinkRequest
    /*
    struct nlmsghdr {
        __u32 nlmsg_len;   // tamanho total da mensagem
        __u16 nlmsg_type;  // tipo da mensagem
        __u16 nlmsg_flags; // flags (request, dump, etc.)
        __u32 nlmsg_seq;   // sequência
        __u32 nlmsg_pid;   // PID do processo
    }; 
    
    req.nlh.nlmsg_len = sizeof(NetlinkRequest);
    req.nlh.nlmsg_type = SOCK_DIAG_BY_FAMILY;
    req.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;

*/

    struct inet_diag_req_v2 req;
    /*
    struct inet_diag_req_v2 {
        __u8  sdiag_family;   // AF_INET ou AF_INET6
        __u8  sdiag_protocol; // IPPROTO_TCP ou UDP
        __u8  idiag_ext;      // extensões
        __u8  pad;
        __u32 idiag_states;   // estados das conexões
        struct inet_diag_sockid id; // filtro (porta/IP)
    };

    
    */
};
#define TCPF_ALL 0xFFF

int main (){
    // int socket(int domain, int type, int protocol);
    // AF_NETLINK -> quero falar com kernel, não com a rede
    // pode ser tbm AF_UNIX processos locais ou AF_INET rede
    // 
    // SOCK_RAW -> aceSso cru as mensagems
    //
    // NETLINK_SOCK_DIAG -> define qual subsistema quero acessar, os sockets
    int nl_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_SOCK_DIAG);


    if(nl_fd < 0){
        std::cerr << "Erro ao criar sokcet Netlink, acho que tu ta linux mesmo?" << std::endl;
    }


    NetlinkRequest request;
    
    // passo endereço da request
    // limpo os campos com 0
    // passo tamanho da request
    // evitando inicializar com lixo
    memset(&request, 0, sizeof(request));

    request.nlmh.nlmsg_len = sizeof(request);
    request.nlmh.nlmsg_type = SOCK_DIAG_BY_FAMILY; // informações do ssocket por familia ivp4 e ipv6
    request.nlmh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP; // digo que é uma request é que eu quero tudo (dump)
    
    
    request.req.sdiag_family = AF_INET; // so quero IPv4 dps boto IPV6 com AF_NET6
    request.req.sdiag_protocol = IPPROTO_TCP; // APENAS TCP
    request.req.idiag_states = 0xFFF; // todos os estados LISTEN, ESTABLISHED, etc


    // sockaddr_in -> IPv4
    // socketaddr_un -> UNix sokcet
    // nl netlink
    struct sockaddr_nl sa; // cria uam struct de endereço espesifica do netLink
    memset(&sa, 0, sizeof(sa));

    sa.nl_family = AF_NETLINK; // endereço para comunicação via kernel com netlink

    // envia requisisção ao kernel
   
    /*
    [ Seu programa ]
        |
        | sendto()
        v
    [ Kernel Netlink subsystem ]
        |
        | interpreta nlmsghdr + inet_diag_req_v2
        v
    [ Sistema de sockets do kernel ]
    */
    // sendto é uma syscall
    // sendto(MeuSocket, DadosDaRequest, TamanhDaRequest, padrão, destino(kernel), tamanho_destino)
    
    if(sendto(nl_fd, &request, sizeof(request), 0, (struct sockaddr*)&sa, sizeof(sa)) < 0){
        std::cerr << "Erro no sendto" << std::endl;
        close(nl_fd);
        return 1;
    }

    std::cout << "[+] Requisicao enviada! Lendo resposta do Kernel...\n" << std::endl;
    std::cout << "ESTADO\tIP LOCAL:PORTA\t\tIP REMOTO:PORTA\t\tINODE\n";
    std::cout << "----------------------------------------------------------------------\n";


    char buffer[8192]; // cada char é 1Byte ent criei  no total tenho 8192 Bytes ou 8KB
    // [bloco char numero 0][??][??][??][??][??]...[char numero 8191]
    bool done = false;

    /*
    [ bloco 1 ]
    [ bloco 2 ]
    [ bloco 3 ]
    ...
    */

    while(!done){
        // recebe bloco do kernel
        // recv é uma Syscall
        // ssize_t tem 8Bytes
        ssize_t bytes_read = recv(nl_fd, buffer, sizeof(buffer), 0);
        /*oq chega aqui?
        [ nlmsghdr ][ dados ]
        [ nlmsghdr ][ dados ]
        ...
        */
                        // parei aquii
        if(bytes_read <= 0){
            break;
        }

        struct nlmsghdr *h = (struct nlmsghdr *)buffer;
        /*
        h → interpreta assim:
            nlmsg_len   (4 bytes)
            nlmsg_type  (2 bytes)
            nlmsg_flags (2 bytes)
            nlmsg_seq   (4 bytes)
            nlmsg_pid   (4 bytes)
        */
        for(; NLMSG_OK(h, bytes_read); h = NLMSG_NEXT(h, bytes_read)){

            // se no type tiver um NSMSG_DONE significa que acabou tudo 
            if(h->nlmsg_type == NLMSG_DONE){
                done = true;
                break;
            }
            // se tiver no type um error signifca que bixou ne kkk
            if(h->nlmsg_type == NLMSG_ERROR){
                std::cerr << "Erro retornado pelo Kernel!" << std::endl;
                done = true;
                break;
            }

            // Só processa se for uma mensagem de diagnóstico válida
            if(h->nlmsg_type != SOCK_DIAG_BY_FAMILY) {
                continue;
            }


            // NLMSG_DATA é um truque do kernel ele sabe que, o CABEÇARIO da mensagem tem 16Bytes ent ele simplismente pula eles vai para os dados
            //(comvertemos a saida para tipo um pontiero para struct inet_diag_msg)
            struct inet_diag_msg *diag = (struct inet_diag_msg *)NLMSG_DATA(h);

            // Filtra apenas IPv4 pois so pedimos ipv4
            if(diag->idiag_family != AF_INET) {
                continue;
            }

            // uint16_t vai ter 2bytes ou seja 16bits
            uint16_t src_port = ntohs(diag->id.idiag_sport);
            uint16_t dst_port = ntohs(diag->id.idiag_dport);

            char src_ip[INET_ADDRSTRLEN]; // criei um buffer de 16bytes (INET_ADDRSTRLEN) => suficiente par um IP 
            char dst_ip[INET_ADDRSTRLEN]; // um Ip vai ter 4 bytes ou seja 32 bits porem com os pontos é a porta pegamos 16Bytes logo => 128bits
            
            // Converte IPs para string legível
            inet_ntop(diag->idiag_family, diag->id.idiag_src, src_ip, sizeof(src_ip));
            inet_ntop(diag->idiag_family, diag->id.idiag_dst, dst_ip, sizeof(dst_ip));

            // Mapeamento simples de estados (opcional, para ficar mais legível que apenas números)
            const char* state_str = "";
            switch(diag->idiag_state){
                case 1: state_str = "ESTABLISHED"; break;
                case 2: state_str = "SYN_SENT"; break;
                case 3: state_str = "SYN_RECV"; break;
                case 4: state_str = "FIN_WAIT1"; break;
                case 5: state_str = "FIN_WAIT2"; break;
                case 6: state_str = "TIME_WAIT"; break;
                case 7: state_str = "CLOSE"; break;
                case 8: state_str = "CLOSE_WAIT"; break;
                case 9: state_str = "LAST_ACK"; break;
                case 10: state_str = "LISTEN"; break;
                case 11: state_str = "CLOSING"; break;
                default: state_str = "UNKNOWN"; break;
            }

            std::cout << state_str << "\t"
                      << src_ip << ":" << src_port << "\t\t"
                      << dst_ip << ":" << dst_port << "\t\t"
                      << diag->idiag_inode << "\n";
        }
    }

    close(nl_fd);
    return 0;
    
}

    




