#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

int sock;

void DieWithError(char *errorMessage)
{
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

    struct sockaddr_in servAddr;
    unsigned short servPort;
    char *servIP;
    pid_t pid;
    int bytesRcvd;

    if ((argc < 3) || (argc > 4))
    {
        fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n",
                argv[0]);
        exit(1);
    }

    servIP = argv[1];
    pid = getpid();
    servPort = atoi(argv[2]);

    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_UDP)) < 0)
    {
        DieWithError("socket() failed");
    }

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(servIP);
    servAddr.sin_port = htons(servPort);

    if (connect(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        DieWithError("connection failed");
    }

    printf("Student %d start answering\n", getpid());

    if (send(sock, &pid, sizeof(int), 0) != sizeof(int))
    {
        DieWithError("send() in student sent a different number of bytes than expected");
    }

    if ((bytesRcvd = recv(sock, &pid, sizeof(int), 0)) <= 0)
    {
        DieWithError("recv() failed or connection closed prematurely");
    }
    printf("Student %d pass exam\n", pid);
    close(sock);
    exit(0);
}