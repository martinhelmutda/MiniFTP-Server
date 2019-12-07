//
//  server.c
//  Progra Avanzada
//
//  Created by Martin Helmut Dominguez Alvarez on 01/12/19.
//  Copyright © 2019 Martin Helmut Dominguez Alvarez. All rights reserved.
//

#include "header.h"

void server(char* ip, int port, char* program);
void serves_client(int sock, FILE *log_file);
int is_directory(const char *path);

char *root;
char ip[15];


int main(int argc, char* argv[]) {
    
    int port;
    int fd;
    
    strcpy(ip, DEFAULT_IP);
    port = DEFAULT_PORT;
    DIR* root_dir;
    
    if (argc != 4){
        printf("usage: %s dir_ip  port  root_directory \n", argv[0]);
        return -1;
    }else{
//    ---- Check ARGS ----
        if (strcmp(argv[1], " ") != 0) {
            strcpy(ip, argv[1]);
        } else {
            printf("usage: %s dir_ip  port  root_directory \n", argv[0]);
            return -1;
        }
  
        
        if (strcmp(argv[2], " ") != 0) {
            port = atoi(argv[2]);
            if (port < 5000) {
                printf("%s: The port must be greater than 5000.\n", argv[0]);
                return -1;
            }
        } else {
            printf("usage: %s  dir_ip  port  root_dir \n", argv[0]);
            return -1;
        }
//    ---- Check ARGS ----
        
        
//    ---- Set Server Root ---
        root = argv[3];
        if ((root_dir = opendir(root)) == NULL ){
            perror(argv[0]);
            return -1;
        }
        closedir(root_dir);
//    ---- Set Server Root ---
    
        
//    ---- Create Log ---
        if ((fd = open("log_file.txt", O_TRUNC | O_CREAT, 0600 )) < 0){
            perror(argv[0]);
            return -1;
        }
        close(fd);
//    ---- Create Log ---
    }

//    ---- Start Server --- :D
    server(ip, port, argv[0]);
    return 0;
}



int is_directory(const char *path) {
   struct stat statbuf;
   if (stat(path, &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}

//Enviar respuestas al cliente
void sender(int code,long length, char *text_msg, int sock){
    write(sock , &code, sizeof(code)); //code
    write(sock , &length , sizeof(length)); //length
    write(sock, text_msg, length * sizeof(char)); //text_msg
}

void server(char* ip, int port, char* prog) {
    struct sockaddr_in server_info;
    struct sockaddr_in client_info;
    FILE *log_file;
    
    int sfd, nsfd, pid;
    
    if ( (sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror(prog);
        exit(-1);
    }

    server_info.sin_family = AF_INET;
    server_info.sin_addr.s_addr = inet_addr(ip);
    server_info.sin_port = htons(port);
    if ( bind(sfd, (struct sockaddr *) &server_info, sizeof(server_info)) < 0 ) {
        perror(prog);
        exit(-1);
    }

    listen(sfd, 1);
    
    //   ---- OPEN LOG ---
        if((log_file= fopen("log_file.txt", "w")) < 0) printf("Error abriendo archivo");
    //   ---- OPEN LOG ---
    
    while (1) {
        unsigned int len = sizeof(client_info);
        if ( (nsfd = accept(sfd, (struct sockaddr *) &client_info, &len)) < 0 ) {
            perror(prog);
            exit(-1);
        }

        if ( (pid = fork()) < 0 ) {
            perror(prog);
        } else if (pid == 0) {
            close(sfd);
            serves_client(nsfd, log_file); //SERVES_CLIENT
            exit(0);
        } else {
            close(nsfd);
        }
    }
    
    //   ---- CLOSE LOG ---
        fclose(log_file);
    //   ---- CLOSE LOG ---
    

}

void serves_client(int sock, FILE *log_file) {
    
    char *connection_detail = (char *) malloc(100);
    struct dirent* direntry;
    long length;
    int code;
    char *data, *list_dir;
    int fd, fd_in, fd_out;
     char *text_msg;
    DIR  *directorio;
    ssize_t bytes;
    off_t size;
    
    char *buffer;
    char *filecpy = (char *) malloc(100);
    char *path = (char *) malloc(100);
    char *check_slash = (char *) malloc(2);
   
    time_t now = time(NULL);
    char * start_t = ctime(&now);
    unsigned long length_t;
    char *time_data;

    
    strcat(connection_detail, ip);
    strcat(connection_detail, " ");
    
    
    length_t = strlen(start_t);
    time_data = (char * ) malloc(length_t);
    strncpy(time_data, start_t, length_t - 2);
    strcat(connection_detail, time_data);

    code = START;
    length = 4;
    text_msg = "Bienvenido al súper servidor FTP";
    sender(code, length, text_msg, sock);
    
    fprintf(log_file, "CONEXIÓN INICIADA: %s %d \n %s\n", connection_detail, code, text_msg);

    do{
        read(sock, &code, sizeof(code));
        read(sock, &length, sizeof(length));
        data = (char*) malloc(length * sizeof(char) + 1);
        read(sock, data, length * sizeof(char));
        strncpy(check_slash, data, 1);
        data[length] = '\0'; // Destructor de basura
        
// ---- LOG CLIENTE ----
        fprintf(log_file, "CLIENTE - Origen %s: \n\t Mensaje: %d - %s\n", connection_detail, code, data);
// ---- LOG CLIENTE ----
        
        //Create complete path
        strcpy(path, root);
        strcat(path, data);
        
        switch(code){
        /*
         Code send by client
            case FILES
            case DIR
            case CLOSE
            default UNKNOW
         */
            case FILES:

                printf("File: %s \n", data);
                 if( (strstr(data, "..") != NULL) || (strcmp(check_slash,"/")) != 0) {
                    //ERROR 203
                    code = INERROR;
                    text_msg = "Error interno";
                    length = strlen(text_msg);
                    sender(code, length, text_msg, sock);
                    break;
                }

                if( access( path, F_OK ) != -1 ){
                    if((is_directory(path))){ //Checa el directorio
                    //ERROR 205
                        code = ISDIR;
                        text_msg = "La ruta corresponde a un directorio";
                        length = strlen(text_msg);
                        sender(code, length, text_msg, sock);
                    }else{
                        if ((fd_in = open(path, O_RDONLY ) )> 0){ //Si el archivo se puede leer
                         ///  manejo de archivos
                            strcpy(filecpy, "copy of ");
                            strcat(filecpy, data + 1);

                            if((fd_out = open(filecpy, O_WRONLY | O_TRUNC | O_CREAT, 0666)) < 0){
                                printf("Error abriendo el archivo");
                            }

                            size = lseek(fd_in, 0, SEEK_END); //Checa el tamaño del archivo
                            lseek(fd_in, 0, SEEK_SET);
                            buffer = (char*) malloc(size * sizeof(char));

                            if ( (bytes = read(fd_in, buffer, size)) != 0 ) write(fd_out, buffer, bytes);

                            free(buffer); //Libera buffer
                            close(fd_in);
                            close(fd_out);
                        ///  manejo de archivos
                            
                        /// manejo de sock
                            //CODE 301
                            code = SENDFILE;
                            text_msg = "Enviar archivo";
                            length = strlen(text_msg);
                            sender(code, length, text_msg, sock);
                        /// manejo de sock
                        } else{
                            //ERROR 201
                            code = NOPERMS;
                            text_msg = "Permiso denegado";
                            length = strlen(text_msg);
                            sender(code, length, text_msg, sock);
                        }
                    }
                }else{
                    //ERROR 202
                    code = NOTFOUD;
                    text_msg = "No se ha encontrado el archivo";
                    length = strlen(text_msg);
                    sender(code, length, text_msg, sock);
                }
                break;

                
            case DIREC:
                
                printf("Directory: %s \n", data);
                
                //Checar
                 if( (strstr(data, "..") != NULL) || (strcmp(check_slash,"/")) != 0) {
                      //ERROR 203
                      code = INERROR;
                      text_msg = "Error interno";
                      length = strlen(text_msg);
                      sender(code, length, text_msg, sock);
                      break;
                  }

                if( (directorio = opendir(path)) ){ //Si es un directorio
                    //Busca o crea el archivo
                    remove("ls.txt");
                    fd_out = open("ls.txt", O_WRONLY | O_TRUNC | O_CREAT, 0666);

                    list_dir = (char *) malloc(1000);
                    strcpy(list_dir, "CONTENIDO: \n");
                    while((direntry = readdir(directorio)) != NULL){ //Read content
                        if(strcmp(direntry->d_name, ".") != 0 &&  strcmp(direntry->d_name, "..") != 0 ){ //If not is
                            printf("Archivo obtenido: %s\n", direntry->d_name);
                            strcat(list_dir, direntry->d_name);
                            strcat(list_dir, "\n"); //Add to listdir
                        }
                    }

                    write(fd_out, list_dir, strlen(list_dir) * sizeof(char)); //write list dir in fd_out
                    close(fd_out);
                    
                    //CODE 302
                    code = SENDDIR;
                    length = strlen(list_dir);
                    
                    sender(code, length, list_dir, sock); //Send list again through socket
                    
                    /// gestión de archivos y directorios
                    closedir(directorio);
                    free(list_dir);
                    
                }else{ //Si no es un directorio
                    
                    if ((fd = open(path, O_WRONLY | O_RDONLY ) ) > 0){
                        //ERROR 207
                        code = NODIR;
                        text_msg = "No es un directorio";
                        length = strlen(text_msg);
                        
                        sender(code, length, text_msg, sock);
                        close(fd);
                        
                    }else{
                        //ERROR 206
                        code = UNKNOWD;
                        text_msg = "No se encontró el directorio";
                        length = strlen(text_msg);
                        
                        sender(code, length, text_msg, sock);
                    }
                }
                break;
            
            case CLOSE: //Bye
                break;

            default:
                code = UNKNOW; // ERROR 204
                text_msg = "Comando desconocido";
                length = strlen(text_msg);
                
                sender(code, length, text_msg, sock);
                break;
        }

//     ---- LOG SERVER ---
        if(code != CLOSE){
            fprintf(log_file, "SERVIDOR - Ubicacion %s: \n\t Mensaje: %d - %s\n", connection_detail, code, text_msg);
        }
//    ---- LOG SERVER ---
        
        free(data);
        
    }while(code != CLOSE);
    printf("Fin de la transmisión");
    close(sock);
}
