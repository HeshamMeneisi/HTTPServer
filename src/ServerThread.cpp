#include "ServerThread.h"
#include <iostream>
#include <pthread.h>
#include <ctime>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>

using namespace std;

// entry point for all threads
void* run(void* obj)
{
    ((ServerThread*)obj)->Run();
    // when Run() finishes, mark for termination
    ((ServerThread*)obj)->MarkFT();
}
bool getExt(const char* path,char* ext)
{
    int l = strlen(path), i=l;
    while(i>0 && path[--i]!='.' && path[i]!='/');
    if(i==0 || path[i] == '/') return false;
    for(int j = 0; i<l; ext[j++] = path[++i]);
    return true;
}
const char* term = "\r\n\r\n";
///////////////////////////////////////////////////////////
ServerThread::ServerThread(int socket)
{
    this->socket = socket;
    marked = serving = false;
}
ServerThread::~ServerThread()
{
}
void ServerThread::Serve()
{
    pthread_create(&tid, NULL, run, this);
    start = time(NULL);
}
bool ServerThread::Terminate(bool force)
{
    if(serving&&!force)return false;
    pthread_cancel(tid);
    close(socket);
    return true;
}

void ServerThread::Run()
{
    int rd = 0;
    while(1)
    {
        serving = false;
        rd = read(socket,buffer,BUFFERLENGTH-1);
        if(rd <= 0)
        {
            close(socket);
            MarkFT();
            break;
        }
        serving = true;
        char method[8], path[261], version[8], ext[5];
        sprintf(path,"%s",WORKDIR);
        int i=0;
        for(; i<rd && strcmp(term,buffer+i); i++)header[i]=buffer[i];
        header[i] = '\0';
        sscanf(header,"%s %s %s",method,path+WDLEN,version);
        getExt(path,ext);
        if(!strlen(ext))strcat(path,"/index.html");
        //cout << "m=" << method << endl << "p=" << path << endl << "v=" << version << endl;
        if(!strcmp("GET",method))
        {
            FILE * file = fopen(path, "r");
            if(file)
            {
                fseek(file,0,SEEK_END);
                int size = ftell(file);
                fseek(file,0,0);
                // send 200
                sprintf(buffer,"%s%d\r\n\r\n", "HTTP/1.1 200 OK\r\nContent-Length: ",size);
                send(socket, buffer, strlen(buffer), 0);
                cout << "200 " << path << endl;
                do
                {
                    rd = fread(buffer, sizeof(char), BUFFERLENGTH, file);
                    send(socket, buffer, rd, 0);
                }
                while(rd > 0);
                fclose(file);
            }
            else
            {
                // send 404
                sprintf(buffer,"%s\r\n\r\n","HTTP/1.1 404 Not Found");
                send(socket, buffer, strlen(buffer), 0);
                cout << "404 " << path << endl;
            }
        }
        else if(!strcmp("POST",method))
        {
            FILE * file = fopen(path, "w+");
            if(file)
            {
                // send 200
                sprintf(buffer,"%s\r\n\r\n", "HTTP/1.1 200 OK");
                send(socket, buffer, strlen(buffer), 0);
            }
            fclose(file);
        }
    }
    MarkFT();
}
