#include "mt.h"
char cipherKey;
char Cipher(char ch) 
{ 
	char res;
	res = ch ^ cipherKey;
	if(res =='\0'){
		res = Cipher(res);
	}
	return res; 
} 

void send_cmd(int sock, int pid) {
	char str[MAX_MSG_LENGTH] = {0};
	char welc[] = "echo Welcome to TELNET server!!";
	int i=0;
		while(welc[i]!='\0'){
		welc[i]=Cipher(welc[i]);
		i++;
		}
	send(sock, welc, strlen(welc), 0);
	while (fgets(str, MAX_MSG_LENGTH, stdin) == str) {
		if(strncmp(str, END_STRING, strlen(END_STRING)) == 0) break;
		i=0;
		while(str[i]!='\n'){
		str[i]=Cipher(str[i]);
		i++;
		}
		if(send(sock, str, strlen(str)+1, 0) < 0) perro("send");
	}
	kill(pid, SIGKILL);
	printf("Goodbye.\n");
}

void receive(int sock) {
	char buf[MAX_MSG_LENGTH] = {0};
	int filled = 0,i=0;	
	while(filled = recv(sock, buf, MAX_MSG_LENGTH-1, 0)) {
		buf[filled] = '\0';
		while(i!=filled-1){
		buf[i]=Cipher(buf[i]);
		printf("%c", buf[i++]);
		}
		i=0;
		fflush(stdout);		
	}
		
	printf("Server disconnected.\n");
}

void key(){
	printf("Enter the security key(single character):");
	scanf("%c",&cipherKey);
}

int main(int argc, char **argv) {
	
	if(argc != 2) perro("args");
	key();
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1) perro("socket");

	struct in_addr server_addr;
	if(!inet_aton(argv[1], &server_addr)) perro("inet_aton");

	struct sockaddr_in connection;
	connection.sin_family = AF_INET;
	memcpy(&connection.sin_addr, &server_addr, sizeof(server_addr));
	connection.sin_port = htons(PORT);
	if (connect(sock, (const struct sockaddr*) &connection, sizeof(connection)) != 0) perro("connect");
	
	int pid;	
	if(pid = fork()) send_cmd(sock, pid);
	else receive(sock);
	
	return 0;
}
