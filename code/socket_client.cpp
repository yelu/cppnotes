// https://www.geeksforgeeks.org/socket-programming-cc/
// client side C/C++ program to demonstrate Socket programming

#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h>

#define PORT 8888

int main(int argc, char const *argv[]) 
{
    const char* msg = "hello from client";    
    int sock = -1;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("socket() error \n"); 
        return -1;
    } 

    struct sockaddr_in serv_addr; 
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
    
    // convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
    { 
        printf("address not supported \n"); 
        return -1; 
    } 

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("connection() failed \n"); 
        return -1; 
    } 
    send(sock, msg, strlen(msg), 0); 
    printf("msg sent\n");
    char buffer[1024] = {0};
    int nbytes = read(sock , buffer, 1024);
    printf("recv msg: %s\n", buffer); 
    return 0;
} 
