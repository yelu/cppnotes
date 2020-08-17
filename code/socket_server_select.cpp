// https://www.ibm.com/support/knowledgecenter/ssw_ibm_i_72/rzab6/xnonblock.htm

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>

int main(int argc, char **argv)
{
    // create an AF_INET stream socket to receive incoming connections on
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        printf("socket() failed\n");
        return -1;
    }

    // allow socket descriptor to be reuseable
    int on = 1;
    if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0) {
        printf("setsockopt() failed\n");
        close(listen_fd);
        return -1;
    }

    // set socket to be nonblocking. all of the sockets for
    // the incoming connections will also be nonblocking since
    // they will inherit that state from the listening socket.
    if(ioctl(listen_fd, FIONBIO, (char *)&on) < 0) {
        printf("ioctl() failed\n");
        close(listen_fd);
        return -1;
    }

    // bind the socket
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(8888);
    if(bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("bind() failed\n");
        close(listen_fd);
        return -1;
    }

    // listen back log
    if(listen(listen_fd, 32) < 0) {
        printf("listen() failed\n");
        close(listen_fd);
        return -1;
    }

    // initialize the master fd_set
    fd_set master_set;
    FD_ZERO(&master_set);
    int max_fd = listen_fd;
    FD_SET(listen_fd, &master_set);

    // loop waiting for incoming connects or for incoming data on any of the connected sockets.   
    for(;;) {
        fd_set working_set;
        memcpy(&working_set, &master_set, sizeof(master_set));

        // call select() and wait 3 minutes for it to complete.
        if(select(max_fd + 1, &working_set, NULL, NULL, NULL) <= 0) {
            perror("select() failed\n");
            break;
        }

        // one or more descriptors are readable. need to determine which ones they are.
        for (int i = 0; i <= max_fd; i++) {
            // check to see if this descriptor is ready
            if (!FD_ISSET(i, &working_set)) { continue; }

            // check to see if this is the listening socket
            if (i == listen_fd) {
                int new_fd = -1;
                if((new_fd = accept(listen_fd, NULL, NULL)) < 0) {
                    perror("accept() failed\n");
                    continue;
                }
                printf("new incoming connection on %d\n", new_fd);
                FD_SET(new_fd, &master_set);
                if (new_fd > max_fd) { max_fd = new_fd; }
            } else {
                char buffer[1024];
                int nbytes = 0;
                // receive all incoming data
                if((nbytes = recv(i, buffer, sizeof(buffer), 0)) <= 0) {
                    printf("connection closed or recv() failed\n");
                    close(i);
                    FD_CLR(i, &master_set);
                    if (i == max_fd) {
                        while (!FD_ISSET(max_fd, &master_set))
                            max_fd -= 1;
                    }
                    continue;
                }
                buffer[nbytes] = '\0';
                printf("recv msg: %s\n", buffer);
            } // end of existing connection is readable          
        } // end of loop through selectable descriptors
    } 

    // clean up
    for (int i = 0; i <= max_fd; i++)
    {
        if (FD_ISSET(i, &master_set)) { close(i); }
    }

    return 0;
}
