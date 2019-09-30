#include <iostream>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>

#include "debug.h"
#include "exception.h"
#include "dsserver.h"
#include "stdlib.h"
#include "stdio.h"


// void daemonize(const char *cmd) {
// 	int i, fd0, fd1, fd2;
// 	pid_t pid;
// 	struct rlimit rl;
// 	struct sigaction sa;

// 	//Clear file creation mask.
// 	umask(0);

// 	// Get maximum number of file descriptors.
// 	if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
// 		DEBUG_PRINT("%s: can’t get file limit", cmd);
// 		exit(-1);
// 	}

//  	//Become a session leader to lose controlling TTY.
// 	if ((pid = fork()) < 0) {
// 		DEBUG_PRINT("%s: can’t fork", cmd);
// 		exit(-1);
// 	}
// 	else if (pid != 0) {
// 		exit(0);
// 	}

// 	setsid();
// 	//Ensure future opens won’t allocate controlling TTYs.
 
// 	sa.sa_handler = SIG_IGN;
// 	sigemptyset(&sa.sa_mask);
//     sa.sa_flags = 0;
    
//     if (sigaction(SIGHUP, &sa, NULL) < 0) {
//         DEBUG_PRINT("%s: can’t ignore SIGHUP", cmd);
//         exit(-1);
//     }

//     if ((pid = fork()) < 0) {
//         DEBUG_PRINT("%s: can’t fork", cmd);
//         exit(-1);
//     }
//     else if (pid != 0)  {
// 		exit(0);
// 	}
    
// 	// Change the current working directory to the root so
// 	// we won’t prevent file systems from being unmounted.
//     if (chdir("/") < 0) {
//         DEBUG_PRINT("%s: can’t change directory to /", cmd);
//         exit(-1);
//     }

//     //Close all open file descriptors.
//     if (rl.rlim_max == RLIM_INFINITY) {
//         rl.rlim_max = 1024;
//     }

//     for (i = 0; i < rl.rlim_max; i++) {
//         close(i);
//     }
    

//     //attach file descriptors 0, 1, and 2 to /dev/null
//     fd0 = open("/dev/null", O_RDWR);
//     fd1 = dup(0);
//     fd2 = dup(0);
    
//     //Initialize the log file
//     openlog(cmd, LOG_CONS, LOG_DAEMON);
//     if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
//         syslog(LOG_ERR, "unexpected file descriptors %d %d %d", fd0, fd1, fd2);
// 		exit(1); 
// 	}
// }


// void sigterm(int signo) {
//     syslog(LOG_INFO, "got SIGTERM; exiting");
// 	exit(0); 
// }


// void sighup(int signo) {
//     syslog(LOG_INFO, "Re-reading configuration file");
// }


int main(int argc, char *argv[]) {

	if (argc<4) {
		std::cout << "Usage: " << std::endl;
		std::cout << "  kvdaemon [host] [port] [file]" << std::endl;
		return -1;
	}

	// char *cmd;
 //    struct sigaction sa;
 //    if ((cmd = strrchr(argv[0], '/')) == NULL)
 //        cmd = argv[0];
	// else cmd++;
    
 //    daemonize(cmd);
    
 //    //handle signals of interest
 //    sa.sa_handler = sigterm;
 //    sigemptyset(&sa.sa_mask);
 //    sigaddset(&sa.sa_mask, SIGHUP);
 //    sa.sa_flags = 0;
 //    if (sigaction(SIGTERM, &sa, NULL) < 0) {
 //        syslog(LOG_ERR, "can’t catch SIGTERM: %s", strerror(errno));
 //        exit(-1);
 //    }
    
 //    sa.sa_handler = sighup;
 //    sigemptyset(&sa.sa_mask);
 //    sigaddset(&sa.sa_mask, SIGTERM);
 //    sa.sa_flags = 0;
 //    if (sigaction(SIGHUP, &sa, NULL) < 0) {
 //    	syslog(LOG_ERR, "can’t catch SIGHUP: %s", strerror(errno));
 //    	exit(1); 
 //    }

    //rest of the daemon
	std::string host(argv[1]);
	int port = atoi(argv[2]);
	std::string file(argv[3]);

	try {
		dsserver s(host, port, file);
		s.serve();
		sleep(1);
		while (true) {
			if (s.is_running()) {
				sleep(1);
			}
			else {
				exit(0);
			}
		}
	}
	catch (exception &ex) {
		std::cerr << "Error initialize the serbver " << std::endl;
		exit(-1);
	}
}