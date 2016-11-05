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
#include "HTTPServer.h"
#include <pthread.h>

using namespace std;

// command symbol table

enum _cmd {
    cunknown,
    cexit
};

_cmd str2cmd(string cmdstr) {
    if (cmdstr == "x" || cmdstr == "exit" || cmdstr == "quit")
        return cexit;
    return cunknown;
}

int main(int argc, char *argv[]) {
    string cmd;
    bool running = true;
    int portno = 80;
    // the first argument is always the work directory.
    if (argc == 2)
        // get port number from arguments
        portno = atoi(argv[1]);
    HTTPServer* server = new HTTPServer(portno);
    server->StartServer();
    // without the terminator, only the client can close the connection
    server->StartTerminator();
    while (running) {
        cin >> cmd;
        if (cmd != "")
            switch (str2cmd(cmd)) {
                case cexit:
                    running = false;
                    break;
                default:
                    cout << "Unrecognized command." << endl;
                    break;
            }
    }
    server->Stop();
    return 0;
}

