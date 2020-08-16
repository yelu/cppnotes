#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addr


int main(int argc, char **argv)
{
    // create socket
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd < 0) { return 1; }
    struct sockaddr_in svr_addr;
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    svr_addr.sin_port = htons (8888);

    // bind
    if(bind(listen_fd, (sockaddr*)&svr_addr, sizeof(svr_addr)) < 0){ return 1; }

    // listen
    if(listen(listen_fd, 3 /*backlog*/) < 0) { return 1; }

    for ( ; ; ) 
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        
        // accept
        int conn_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
        if(conn_fd < 0) { continue; }

        // read and send data
        int data_recv = 0;
        if(read(conn_fd, &data_recv, sizeof(data_recv)) != sizeof(data_recv)) { continue; }
        write(conn_fd, &data_recv, sizeof(data_recv));
        close(conn_fd);
    }
    close(listen_fd);
    return 0;
}