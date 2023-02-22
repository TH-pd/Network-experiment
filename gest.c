#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include <sys/socket.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <netdb.h>
#include<stddef.h>
#include <arpa/inet.h>
#include <stdlib.h>

#define BUFF_SIZE   65536
#define PORT        10401

int subst(char *str, char c1, char c2)
{
    int n = 0;
    for( ; *str != '\0'; str++) {
        if(*str == c1) {
            *str = c2;
            n++;
        }
    }
    return n;
}

int get_line(char *line, FILE *fp)
{
    if(fgets(line, BUFF_SIZE, fp) == NULL) {
        return 0;
    }
    subst(line, '\n', '\0');
    return 1;
}

int main(int argc, char *argv[])
{
    int s,recvsize, sendsize, i, eofflg;
    char recv_message[BUFF_SIZE], send_message[BUFF_SIZE];
    struct sockaddr_in sa;
    
    sa.sin_family = AF_INET;
    sa.sin_port = htons(PORT);
    sa.sin_addr.s_addr = inet_addr(argv[1]);

	s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1)
    {
        printf("error in socket\n");
        return -1;
    }
    connect(s, (struct sockaddr*)&sa, sizeof(sa));    
    printf("READY\n");
    while (get_line(send_message, stdin)) {

        sendsize = send(s, send_message, strlen(send_message)/sizeof(char)+1, 0);

        if (sendsize < 0)
        {
            printf("error in send\n");
            return -1;
        }
        if (!strcmp(send_message, "%Q")) {
            break;
        }

        recv_message[0] = '\0';
        eofflg = 1;
        recvsize = recv(s, recv_message, BUFF_SIZE, 0);
		while (recvsize > 0)
		{
//			printf("A:%d:%ld\n", recvsize, strlen(recv_message));
			for(i = 0; i+1 < recvsize;i += strlen(&recv_message[i])+1)
			{
                eofflg = !strcmp(&recv_message[i], "EOF");
				if (eofflg)
				{
					break;
				}
//				printf("%d-%ld\n", i, i+strlen(recv_message));
				printf("%s", &recv_message[i]);
			}
			if (eofflg)
			{
				break;
			}
            recvsize = recv(s, recv_message, BUFF_SIZE, 0);
		}
        if (recvsize < 0)
	    {
    	    printf("error in recv\n");
            return -1;
    	}

    }
    close(s);
}