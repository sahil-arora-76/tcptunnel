#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define RED "\033[31m"
#include <pthread.h>
#define NOCOLOR "\033[m"


void set_err(int errn, char *str);
void show_err(void);
struct ERR 
{
    int errn; 
    char what[300]; 
};
struct ERR err = { 0, { 0 }};


char buffer[1024] =  { 0 }; 


void set_err(int errn, char *str)
{
    err.errn = errn; 
    memcpy(err.what, str, strlen(str) + 1); 
    show_err();
    exit(1); 
}

void show_err(void) 
{
    if (err.errn != 0) 
    {
        fprintf(stderr, "%s Code: %d, Info: %s", RED, err.errn, err.what); 
    }
}

typedef struct addr 
{
    int fd;
    struct sockaddr_in addr;
} addr; 


//main structs 
addr client; 
addr server_remote; 
addr server_local;



void args_check(int argc, char **argv)
{
    if (!argv[1]) 
    {
        fprintf(stderr, "%smissing --local-port=%s\nUsage ./tunnel.o <localport> <remoteport> <remotehost>\n", RED, NOCOLOR);
        exit(1); 
    } else if (!argv[2] ) 
    {
        fprintf(stderr, "%smissing --remote-port=%s\nUsage ./tunnel.o <localport> <remoteport> <remotehost>\n", RED, NOCOLOR);
        exit(1);  
    } else if (!argv[3])
    {
        fprintf(stderr, "%smissing --remote-host=%s\nUsage ./tunnel.o <localport> <remoteport> <remotehost>\n", RED, NOCOLOR);
    }
}

void *read_data()
{
    int new_socket;

    int len = sizeof(server_local.addr);
    if ((new_socket = accept(server_local.fd, (struct sockaddr *)&server_local.addr, (socklen_t*)&len))<0)
    {
        char error[100] = "Failed: accept()";
        set_err(new_socket, error); 
    }
    char new_buffer[1024] =  { 0 }; 
    read(new_socket, new_buffer, 1024); 
    printf("%s\n", new_buffer);
    return NULL;
}


void *send_data()
{
    if (connect(client.fd, (struct sockaddr *)&client.addr, sizeof(client.addr)) < 0)
    {
        char error[100] = "Failed: connect()";
        set_err(client.fd, error); 
    }

    send(client.fd, buffer, strlen(buffer), 0);
    return NULL;
}


void build_client(int port, char *ip) 
{
    
    if ((client.fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        char error[100] = "Failed: socket()";
        set_err(client.fd, error);
    }

    client.addr.sin_family = AF_INET;
    client.addr.sin_port = htons(port);
       
    if(inet_pton(AF_INET, ip, &client.addr.sin_addr)<=0) 
    {
        char error[100] = "Failed: inet_pton()";
        set_err(-1, error);
    }
   
}

void build_serve(int port, char *ip, addr *address) 
{
    int opt = 1; 
    int sockfd;
    int rc;

    sockfd = socket(AF_INET, SOCK_STREAM, 0); 

    if (sockfd == 0) 
    {
        char error[100] = "Failed: socket()";
        set_err(sockfd, error); 
    }

    rc = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); 
    if (rc) 
    {
        char error[100] = "Failed: setsockopt()";
        set_err(rc, error); 
    }

    address->addr.sin_family = AF_INET;
    address->addr.sin_addr.s_addr = inet_addr(ip); 
    address->addr.sin_port = htons( port ); 
    address->fd = sockfd;
    rc = bind(address->fd, (struct sockaddr*)&(address->addr), sizeof(address->addr)); 
    if (rc < 0) 
    {
        char error[100] = "Failed: bind()"; 
        set_err(rc, error); 
    }
    
    rc = listen(address->fd, 5); 
    if (rc < 0) 
    {
        char error[100] = "Failed: listen()"; 
        set_err(rc, error);
    }
    
}


int main(int argc, char **argv)
{
    args_check(argc, argv);

    int local_port = atoi(argv[1]); 
    int remote_port = atoi(argv[2]); 
    char *remote_host = argv[3];
    char *local_host = "127.0.0.1"; 
    int new_sock; 
    pthread_t t1, t2; 
    
    build_serve(remote_port, remote_host, &server_remote);
    int len = sizeof(server_remote.addr);
    if ((new_sock = accept(server_remote.fd, (struct sockaddr *)&server_remote.addr, (socklen_t*)&len))<0)
    {
        char error[100] = "Failed: accept()";
        set_err(new_sock, error); 
    }
    read(new_sock, buffer, sizeof(buffer)); 
    close(server_remote.fd);
    build_client(local_port, local_host); 
    build_serve(local_port, local_host, &server_local); 

    pthread_create(&t1, NULL, &read_data, NULL);
    pthread_create(&t2, NULL, &send_data, NULL);
    pthread_join(t2, NULL);
    pthread_join(t1, NULL);
    
    return 0;

}