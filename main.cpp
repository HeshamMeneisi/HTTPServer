#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include "ServerThread.h"
#include <pthread.h>
#include<list>

using namespace std;

// command symbol table
enum _cmd
{
    cunknown,
    cexit
};
// port number and main socket
int portno, sockfd;
// a list of active clients
list<ServerThread*> aclients;

_cmd str2cmd(string cmdstr)
{
    if(cmdstr == "x" || cmdstr == "exit" || cmdstr == "quit")
        return cexit;
    return cunknown;
}
void error(string msg)
{
    cerr << msg << endl;
    exit(1);
}
void msg(string msg)
{
    cout << msg << endl;
}
// the terminator logic, run in a separate thread
void *tfunc(void*)
{
    while(1)
    {
        sleep(0.01);
        time_t now = time(NULL);
        int nc = aclients.size();
        int ttl = 5/(double)(nc>0?nc:1); //in seconds
        for(list<ServerThread*>::iterator list_iter = aclients.begin(); list_iter != aclients.end();)
        {
            ServerThread* st = *list_iter;
            int runningtime = difftime(now, st->GetStartTime());
            if(st->IsMarked() || (runningtime >= ttl && st->Terminate(false)))
            {
                if(st->IsMarked())
                    cout << "Client on socket " << st->GetSocket() << " self terminated"<< endl;
                else
                    cout << "Terminated client on socket " << st->GetSocket() <<endl;
                delete st;
                list_iter = aclients.erase(list_iter);
            }
            else
            {
                list_iter++;
            }
        }
    }
}
void* sfunc(void*)
{
    sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    pthread_t terminator;

    // socket(int domain, int type, int protocol)
    sockfd =  socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("Opening socket");

    // clear the struct's memory
    bzero((char *) &serv_addr, sizeof(serv_addr));

    // setup the host_addr structure
    serv_addr.sin_family = AF_INET;

    // automatically be filled with current host's IP address which is localhost
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    // convert short to network port order
    serv_addr.sin_port = htons(portno);
    // to avoid waiting for limbo connection when debugging
    int en = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(int));

    // bind(int fd, struct sockaddr *local_addr, socklen_t addr_length)
    // This bind() call will bind  the socket to the current IP address on our port
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("On binding");
    // This listen() function will start filling the backlog queue with clients
    // The maximum size for the backlog queue to 1
    listen(sockfd,1);

    clilen = sizeof(cli_addr);

    // create the terminator thread
    pthread_create(&terminator, NULL, tfunc, NULL);

    msg("Server is online!");
    while(true)
    {
        // The accept() function blocks untill the backlog has elements
        // It then stores the info of the client socket in newsockfd
        int csockn = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
        if (csockn < 0)
            msg("Failed on accept.");

        printf("Incoming connection from %s port %d\n",
               inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
        ServerThread* clt = new ServerThread(csockn);
        clt->Serve();
        aclients.push_front(clt);
    }
}
int main(int argc, char *argv[])
{
    pthread_t server;
    // the first argument is always the work directory.
    if (argc < 2)
        portno = 80;
    else
        // get port number from arguments
        portno = atoi(argv[1]);
    // start the server thread
    pthread_create(&server, NULL, sfunc, NULL);
    string cmd;
    bool running = true;
    while(running)
    {
        cin >> cmd;
        if(cmd!="")
            switch(str2cmd(cmd))
            {
            case cexit:
                running = false;
                break;
            default:
                msg("Unrecognized command.");
                break;
            }
    }
    close(sockfd);
    return 0;
}

