#include <stdio.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <stdlib.h>
#include <string.h> 
#include <unistd.h> 
#include <signal.h>

#define MAXPENDING 5 

int servStudSock;
int servTeachSock;
struct sockaddr_in teacherAddr;

void DieWithError(char *errorMessage){
    close(servStudSock);
    close(servTeachSock);
    perror(errorMessage);
    exit(0);
}

int createSocket(int port, in_addr_t servInAddr)
{
    int servSock;
    struct sockaddr_in servAddr;

    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = servInAddr;
    servAddr.sin_port = htons(port);

    if (bind(servSock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
        DieWithError("bind() failed");

    return servSock;
}

void HandleUDPClient()
{
    unsigned int studentLen;
    struct sockaddr_in echoStudAddr;
    studentLen = sizeof(echoStudAddr);

    pid_t pid;
    int recvMsgSize;
    if ((recvMsgSize = recvfrom(servStudSock, &pid, sizeof(int), 0, (struct sockaddr *)&echoStudAddr, &studentLen)) < 0)
    {
        DieWithError("recvfrom() from student failed");
    }
    printf("Handling %s\n", inet_ntoa(echoStudAddr.sin_addr));

    printf("Student %d waiting for his turn\n", pid);

    if (sendto(servTeachSock, &pid, sizeof(int), 0, (struct sockaddr *)&teacherAddr, sizeof(teacherAddr)) != sizeof(int))
    {
        DieWithError("sendto() sent to the teacher a different number of bytes than expected");
    }
    printf("Student %d leaves the exam\n", pid);

    if ((recvMsgSize = recvfrom(servTeachSock, &pid, sizeof(int), 0, (struct sockaddr *)NULL, 0)) < 0)
    {
        DieWithError("recvfrom() from teacher failed");
    }

    if (sendto(servStudSock, &pid, sizeof(int), 0, (struct sockaddr *)&echoStudAddr, sizeof(echoStudAddr)) != sizeof(int))
    {
        DieWithError("sendto() sent to the student a different number of bytes than expected");
    }
}

void sigfunc(int sig)
{
    if (sig != SIGINT && sig != SIGTERM)
    {
        return;
    }
    close(servStudSock);
    close(servTeachSock);
    printf("disconnected\n");
    exit(0);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, sigfunc);
    signal(SIGTERM, sigfunc);

    int studentSock;
    int teacherSock;
    unsigned int servStudentPort;
    unsigned int servTeacherPort;

    if (argc != 4) 
    {
        fprintf(stderr, "Usage:  %s <Server Address> <Port for Students> <Port for Teacher>\n", argv[0]);
        exit(1);
    }

    in_addr_t servAddr;
    if ((servAddr = inet_addr(argv[1])) < 0)
    {
        DieWithError("Invalid address");
    }

    servStudentPort = atoi(argv[2]);
    servTeacherPort = atoi(argv[3]);

    servStudSock = createSocket(servStudentPort, servAddr);
    servTeachSock = createSocket(servTeacherPort, servAddr);

    int recvMsgSize;
    int h;
    unsigned int studentLen;
    studentLen = sizeof(teacherAddr);
    if ((recvMsgSize = recvfrom(servTeachSock, &h, sizeof(int), 0, (struct sockaddr *)&teacherAddr, &studentLen)) < 0)
    {
        DieWithError("recvfrom() failed");
    }
    else
    {
        printf("Teacher is waiting for students\n");
    }

    for (;;)
    {
        HandleUDPClient();
    }
}