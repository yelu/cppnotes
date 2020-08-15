#include "unp.h"

/*
int ReadLine(int fd, void* buff, size_t maxlen)
{
	for (int n = 0; n < maxlen; n++) 
    {
        char ch;
        int bytesRecv = read(fd, buff, 1);
		if ( bytesRecv != 1)  {break;}
        if ( bytesRecv == 1 && c == '\n') {break;}
	}

	*buff = 0;	/* null terminate like fgets() */
	return 0;
}

ssize_t	 WriteLine(int fd, const void* buff, size_t n)
{
	size_t nLeft = n;
	ssize_t nwritten;
	const char* ptr = buff;

	while (nLeft > 0) {
		if ( (nwritten = write(fd, ptr, nLeft)) <= 0) {
			if (errno == EINTR)
				nwritten = 0;		/* and call write() again */
			else
				return(-1);			/* error */
		}

		nLeft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}*/

int main(int argc, char **argv)
{
    int listenFd, connFd;
    pid_t childpid;
    socklen_t clientAddrLen;
    struct sockaddr_in clientAddr, svrAddr;
    int srvListenPort = 9877;
    
    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd < 0) { return 1; }
    bzero(&svrAddr, sizeof(svrAddr));
    svrAddr.sin_family = AF_INET;
    svrAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    svrAddr.sin_port = htons (srvListenPort);
    if(bind(listenFd, (sockaddr*)&svrAddr, sizeof(svrAddr)) < 0){ return 1;}
    if(listen(listenFd, LISTENQ) < 0) {return 1;}
    char buff[1024];
    for ( ; ; ) 
    {
        clientAddrLen = sizeof(clientAddr);
        connFd = accept(listenFd, (SA *) &clientAddr, &clientAddrLen);
        if(connFd < 0) {continue;}
        // read and send data.
        int dataRecv = 0;
        if(read(connFd, &dataRecv, sizeof(dataRecv)) != sizeof(dataRecv)) {continue;}
        write(connFd, &dataRecv, sizeof(dataRecv));
        close(connFd);
    }
    close(listenFd);
    return 0;
}