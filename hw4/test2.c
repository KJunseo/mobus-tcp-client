#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#define MB_PORT 502

void error_handling(char *message);
void show_function();
void read_coils(int sock);
void write_multiple_coils(int sock);
void read_holding_registers(int sock);
void write_multiple_registers(int sock);
ssize_t receive(int sock, uint8_t *buffer);

typedef enum {false, true} bool;

int _msg_id=1;
int _slave_id=1;
int main(int argc, char **argv){
        int sock;
        int menu;
        struct sockaddr_in serv_addr;

        if(argc!=2){
                printf("Usage : %s <Modbus_tcp_server_ip>\n", argv[0]);
                exit(1);
        }

        sock=socket(PF_INET, SOCK_STREAM, 0);

        if(sock==-1) error_handling("socket() error");

        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family=AF_INET;
        serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
        serv_addr.sin_port=htons(MB_PORT);

        if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1){
                printf(" %s\n",strerror(errno));
                error_handling("connect() error");
        }

        while(1){
                show_function();
                printf("Select a function : ");
                scanf("%d", &menu);

                if(menu==100){
                        printf("Quit\n\n");
                        break;
                }
switch(menu){
                        case 1:
                                read_coils(sock);
                                break;
                        case 2:
                                write_multiple_coils(sock);
                                break;
                        case 3:
                                read_holding_registers(sock);
                                break;
                        case 4:
                                write_multiple_registers(sock);
                                break;
                        default:
                                printf("Wrong selection. Try again\n\n");
                                break;
                }
        }

        close(sock);
        return 0;
}

void error_handling(char *message){
        fputs(message, stderr);
        fputc('\n', stderr);
        exit(1);
}

void show_function(){
        printf("Which function do you want?\n");
        printf("----------------------------------------------\n");
        printf("[1] : Read Coils\n");
        printf("[2] : Write Multiple Coils\n");
        printf("[3] : Read Holding Registers\n");
        printf("[4] : Write multiple (Holding) Registers\n");
        printf("[100] : Quit\n");
        printf("----------------------------------------------\n");
}

void read_coils(int sock){
        uint8_t to_send[12];
        int address;
        int amount;
        uint8_t to_recv[260];
        int i;

        printf("Enter the Start Address: ");
        scanf("%d", &address);
        printf("Enter the number of coils to be read: ");
        scanf("%d", &amount);

        to_send[0]=(uint8_t)_msg_id>>8;
        to_send[1]=(uint8_t)(_msg_id & 0x00FF);
to_send[2]=0;
        to_send[3]=0;
        to_send[4]=0;
        to_send[5]=6;
        to_send[6]=(uint8_t)_slave_id;
        to_send[7]=(uint8_t)0x01;
        to_send[8]=(uint8_t)(address>>8);
        to_send[9]=(uint8_t)(address & 0x00FF);
        to_send[10]=(uint8_t)(amount>>8);
        to_send[11]=(uint8_t)(amount & 0x00FF);

        _msg_id++;
        send(sock, to_send, 12, 0);

        ssize_t k = receive(sock, to_recv);

        printf("Function code: %hhu\n", to_recv[7]);
        printf("Byte Count: %hhu\n", to_recv[8]);
        printf("Output Status\n");
        for(i=9;i<(int)k;i++){
                printf("%hhu ", to_recv[i]);
        }
        printf("\n\n");
}

void write_multiple_coils(int sock){
        int address;
        int amount;
        uint8_t to_recv[260];
        int i;

        printf("Enter the Start Address: ");
        scanf("%d", &address);
        printf("Enter the number of coils to be write: ");
        scanf("%d", &amount);

        bool write_cols[amount];
        uint16_t temp[amount];

        for(i=0;i<amount;i++){
                printf("[%d]Input bool(0:false, 1:true): ",i);
                scanf("%u", &write_cols[i]);
                temp[i]=(uint16_t)write_cols[i];
        }

        printf("write coils: ");
        for(i=0;i<amount;i++){
                if(temp[i]==1){
                        printf(" [%s]", "true");
                }else{
                        printf(" [%s]", "false");
                }
        }


uint8_t to_send[14+(amount-1)/8];
        to_send[0]=(uint8_t)_msg_id>>8;
        to_send[1]=(uint8_t)(_msg_id & 0x00FF);
        to_send[2]=0;
        to_send[3]=0;
        to_send[4]=0;
        to_send[5]=(uint8_t)(7+(amount-1)/8);
        to_send[6]=(uint8_t)_slave_id;
        to_send[7]=(uint8_t)0x0F;
        to_send[8]=(uint8_t)(address>>8);
        to_send[9]=(uint8_t)(address & 0x00FF);
        to_send[10]=(uint8_t)(amount>>8);
        to_send[11]=(uint8_t)(amount & 0x00FF);
        to_send[12]=(uint8_t)((amount+7)/8);
        for(i=0;i<amount;i++){
                to_send[13+(i-1)/8]+=(uint8_t)(temp[i]<<(i%8));
        }

        _msg_id++;
        send(sock, to_send, 14+(amount-1)/8, 0);

        ssize_t k=receive(sock, to_recv);

        printf("\n\nFunction code: %hhu\n", to_recv[7]);
        printf("Starting Address: %hhu %hhu\n", to_recv[8], to_recv[9]);
        printf("Quantity of Outputs: \n");
        for(i=10;i<(int)k;i++){
                printf("%hhu ", to_recv[i]);
        }
        printf("\n\n");
}

void read_holding_registers(int sock){
        int address;
        int amount;
        uint8_t to_send[12];
        uint8_t to_recv[260];
        int i;

        printf("Enter the Starting Address: ");
        scanf("%d", &address);
        printf("Enter the number of registers to be read: ");
        scanf("%d", &amount);

        to_send[0]=(uint8_t)_msg_id>>8;
        to_send[1]=(uint8_t)(_msg_id & 0x00FF);
        to_send[2]=0;
        to_send[3]=0;
        to_send[4]=0;
        to_send[5]=6;
        to_send[6]=(uint8_t)_slave_id;
        to_send[7]=(uint8_t)0x03;
        to_send[8]=(uint8_t)(address>>8);
        to_send[9]=(uint8_t)(address & 0x00FF);
        to_send[10]=(uint8_t)(amount>>8);
 to_send[11]=(uint8_t)(amount & 0x00FF);

        _msg_id++;
        send(sock, to_send, 12, 0);

        ssize_t k=receive(sock, to_recv);

        printf("\nFunction code: %hhu\n", to_recv[7]);
        printf("Byte Count: %hhu\n", to_recv[8]);
        printf("Register Value\n");
        for(i=9;i<(int)k;i++){
                printf("%hhu ", to_recv[i]);
        }
        printf("\n\n");
}

void write_multiple_registers(int sock){
        int address;
        int amount;
        int i;

        printf("Enter the Starting Address: ");
        scanf("%d", &address);
        printf("Enter the number of registers to be write: ");
        scanf("%d", &amount);

        uint16_t write_regs[amount];

        for(i=0;i<amount;i++){
                printf("[%d]Input register value: ",i);
                scanf("%hd", &write_regs[i]);
        }

        printf("write register value ");
        for(i=0;i<amount;i++){
                printf("[%u] ", write_regs[i]);
        }
        printf("\n\n");

        uint8_t to_send[13+2*amount];
        to_send[0]=(uint8_t)_msg_id>>8;
        to_send[1]=(uint8_t)(_msg_id & 0x00FF);
        to_send[2]=0;
        to_send[3]=0;
        to_send[4]=0;
        to_send[5]=(uint8_t)(5+2*amount);
        to_send[6]=(uint8_t)_slave_id;
        to_send[7]=(uint8_t)0x10;
        to_send[8]=(uint8_t)(address>>8);
        to_send[9]=(uint8_t)(address & 0x00FF);
        to_send[10]=(uint8_t)(amount>>8);
        to_send[11]=(uint8_t)(amount & 0x00FF);
        to_send[12]=(uint8_t)(2*amount);
        for(i=0;i<amount;i++){
 to_send[13+2*i]=(uint8_t)(write_regs[i]>>8);
                to_send[14+2*i]=(uint8_t)(write_regs[i] & 0x00FF);
        }

        _msg_id++;
        send(sock, to_send, (size_t)(13+2*amount), 0);

        uint8_t to_recv[260];

        ssize_t k=receive(sock, to_recv);

        printf("\nFunction code: %hhu\n", to_recv[7]);
        printf("Starting Address: %hhu %hhu\n", to_recv[8], to_recv[9]);
        printf("Quantity of Register\n");
        for(i=10;i<(int)k;i++){
                printf("%hhu ", to_recv[i]);
        }
        printf("\n\n");
}

ssize_t receive(int sock, uint8_t *buf){
        return recv(sock, (char*)buf, 1024, 0);
}


