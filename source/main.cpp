#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <string.h>
#include <assert.h>

void addSig(int sig, void(handler)(int))
{
    struct sigaction sa;
    bzero(&sa,sizeof(sa)); // 初始化
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    
    assert(sigaction(sig,&sa,NULL) != -1);
}

int main(int argc, char* argv[])
{
    if(argc <= 1)
    {
        printf("usage: %s port_number\n", basename(argv[0]));
        return 1;
    }

    int port = atoi(argv[1]);
    addSig(SIGPIPE,SIG_IGN);


    return 0;
}