//
//  header.h
//  Progra Avanzada
//
//  Created by Martin Helmut Dominguez Alvarez on 01/12/19.
//  Copyright © 2019 Martin Helmut Dominguez Alvarez. All rights reserved.
//

#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

//En MAC OSX
//#include <sys/syslimits.h>

//En Linux
#include <limits.h>

//SERVER
#define START       1

//CLIENTE ASK
#define FILES		101
#define DIREC   102
#define CLOSE		103

//SERVER RESPONSE ERROR
#define NOPERMS     201
#define NOTFOUD		202
#define INERROR     203
#define UNKNOW      204
#define ISDIR       205
#define UNKNOWD     206
#define NODIR       207

//SERVER RESPONSE OK
#define SENDFILE    301
#define SENDDIR     302

#define DEFAULT_PORT    6666
#define DEFAULT_IP      "127.0.0.1"


void snd_msg(int sfd, int code, char* data) {
	long length = strlen(data);
	
	write(sfd, &code, sizeof(code));
	write(sfd, &length, sizeof(length));
	write(sfd, data, length * sizeof(char));
}

#endif
