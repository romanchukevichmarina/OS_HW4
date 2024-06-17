#include <stdio.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

int sock;

void DieWithError(char *errorMessage){
    close(sock);
    perror(errorMessage);
    exit(0);
}

void sigfunc(int sig)
{
    if (sig != SIGINT && sig != SIGTERM)
    {
        return;
    }

    close(sock);
    printf("disconnected\n");
    exit(0);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, sigfunc);
    signal(SIGTERM, sigfunc);

    struct sockaddr_in echoServAddr;
    unsigned short echoServPort;
    char *servIP;
    pid_t pid;
    int bytesRcvd;

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <Server IP> <Echo Port>\n",
                argv[0]);
        exit(-1);
    }

    servIP = argv[1];
    echoServPort = atoi(argv[2]);

    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);
    echoServAddr.sin_port = htons(echoServPort);

    if (connect(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("connect() failed");

    printf("Teacher is ready listen students\' answers\n");

    for (;;)
    {
        printf("Teacher is sleeping.\n");

        if ((bytesRcvd = recv(sock, &pid, sizeof(int), 0)) <= 0)
            DieWithError("recv() failed or connection closed prematurely");

        printf("Student %d is answering his ticket\n", pid);

        sleep(rand() % 10);

        if (send(sock, &pid, sizeof(int), 0) != sizeof(int))
            DieWithError("send() sent a different number of bytes than expected");

        printf("Student %d finished the exam\n", pid);
    }
}