#ifndef SERVERTHREAD_H
#define SERVERTHREAD_H

#include <ctime>
#include <pthread.h>
#define BUFFERLENGTH 8192
#define WORKDIR "/root/cdocs"
#define WDLEN 11
class ServerThread
{
private:
    int socket;
    pthread_t tid;
    std::time_t start;
    char buffer[BUFFERLENGTH];
    char header[BUFFERLENGTH];
    bool marked;
    bool serving;
public:
    ServerThread(int socket);
    ~ServerThread();
    void Serve();
    void Run();
    bool Terminate(bool force);
    int GetSocket()
    {
        return this->socket;
    }
    time_t GetStartTime()
    {
        return this->start;
    }
    pthread_t GetThreadID()
    {
        return this->tid;
    }
    void MarkFT()
    {
        marked = true;
    }
    bool IsMarked()
    {
        return marked;
    }
};

#endif // SERVERTHREAD_H
