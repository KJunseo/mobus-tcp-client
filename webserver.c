#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024
#define SMALL_BUF 100

char* content_type(char* filename);
void error_handling(char* message);

int main(int argc, char *argv[]){
        int serv_sock, clnt_sock;
        socklen_t clnt_addr_size;
        int option=1;

        char buffer[BUF_SIZE];

        struct sockaddr_in serv_addr, clnt_addr;

        int n;

        if(argc!=2){
                printf("Usage: %s <port>\n", argv[0]);
                exit(1);
        }

        //create socket
        if((serv_sock=socket(PF_INET, SOCK_STREAM, 0))<0){
                error_handling("create socket error");
        }
        setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

        //initial setting
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family=AF_INET;
        serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
        serv_addr.sin_port=htons(atoi(argv[1]));

        //bind
        if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))<0){
                error_handling("bind error");
        }

        //listen
        if(listen(serv_sock, 5)==-1){
                error_handling("listen error");
        }
       clnt_addr_size = sizeof(clnt_addr);

        while(1){
                //accept
                if((clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size))<0){
                        error_handling("accept error");
                }

                bzero(buffer, BUF_SIZE);
                if((n=read(clnt_sock, buffer, BUF_SIZE))<0){
                        error_handling("reading error");
                }

                char* content_part=strstr(buffer, "name");

                char write_buffer[BUF_SIZE];
                bzero(write_buffer, BUF_SIZE);

                char* line=strtok(buffer, "\r\n");
                char* method=strtok(line, " ");
                char* path=strtok(NULL, " ");
                char* version=strtok(NULL, "");
                char* filename=strtok(path, "/");

                //if file path is '/' then filename is 'index.html'
                if(filename==NULL){
                        filename="index.html";
                }

                strcat(write_buffer, version); //HTTP/1.1

                //POST
                if(strcmp(method, "POST")==0 && strcmp(filename, "sample")==0){
                        char* name_part=strtok(content_part, "&");
                        char* snum_part=strtok(NULL, "&");

                        strtok(name_part, "=");
                        char* name=strtok(NULL, "=");

                        strtok(snum_part, "=");
                        char* snumber=strtok(NULL, "=");

                        char buf[SMALL_BUF];
                        FILE* sample = fopen("sample", "w");

                        sprintf(buf, "<html><body><h2>name=%s&snumber=%s</h2></body></html>", name, snumber);
                        fputs(buf, sample);
                        fclose(sample);
                }

               char* status="";
                char* type="";
                long file_size=0;
                char s_file_size[10];

                FILE* file=fopen(filename, "rb");

                //except '/', 'index.html', 'query.html', 'sample', -> 404 Not Found
                if(file==NULL){
                        status=" 404 Not Found\r\n";
                        char server[]="Server: Linux Web Server \r\n";
                        char cnt_len[]="Content-length:2048\r\n";
                        char cnt_type[]="Content-type:text/html\r\n\r\n";
                        char content[]="<html><body><h1>404 Not Found</h1></body></html>";

                        strcat(write_buffer, server);
                        strcat(write_buffer, cnt_len);
                        strcat(write_buffer, cnt_type);
                        strcat(write_buffer, content);
                }
                // '/', 'index.html', 'query.html', 'sample' -> 200 OK
                else{
                        status=" 200 OK\r\n";
                        strcat(write_buffer, status);

                        fseek(file, 0, SEEK_END);

                        file_size=ftell(file);

                        rewind(file);

                        sprintf(s_file_size, "%ld", file_size);

                        strcat(write_buffer, "Content-Length: ");
                        strcat(write_buffer, s_file_size);
                        strcat(write_buffer, "\r\n");

                        type=content_type(filename);
                        strcat(write_buffer, "Content-Type: ");
                        strcat(write_buffer, type);
                }
                strcat(write_buffer, "\r\n");

                if((n=write(clnt_sock, write_buffer, strlen(write_buffer)))<0){
                        error_handling("writing error");
                }

                //read file content
                if(file!=NULL){
                        char* content=(char*)malloc(sizeof(char)* file_size);
                        bzero(content, file_size);
                        fread(content, sizeof(char), file_size, file);

                        if((n=write(clnt_sock, content, file_size))<0){
                                error_handling("writing error");
                        }
                        free(content);
                }
        }
        close(clnt_sock);
        close(serv_sock);
        return 0;
}

void error_handling(char* message){
        fputs(message, stderr);
        fputc('\n', stderr);
        exit(1);
}

char* content_type(char* filename){
        if(strstr(filename, ".html")!=NULL){
                return "text/html\r\n";
        }else{
                return "text/html\r\n";
        }
}
