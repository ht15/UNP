#include<signal.h>

typedef void Sigfunc(int);

Sigfunc* mySignal(int signo, Sigfunc* func) { // 使用sigactin来实现signal.sigactio可以指定阻塞的信号集 并 可以设置restart等flag.
    struct sigaction act, oact;
    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    //if(signo == SIGALRM) {
#ifdef SA_INTERRUPT
    act.sa_flags |= SA_INTERRUPT;
#endif
    //} else {
        //act.sa_flags |= SA_RESTART; // 如果设置SA_RESTART则accept被SIGCHLD中断后 有些系统 不需要根据errno进行处理也会自动重启
    //    act.sa_flags |= SA_INTERRUPT;
    //}
    if(sigaction(signo, &act, &oact) < 0){
        return SIG_ERR;
    }
    return oact.sa_handler;
}
