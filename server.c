#include "mt.h"

#define D(...) fprintf(new_stream, __VA_ARGS__)

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

int main() {
	int sock;
	struct sockaddr_in name;
	char buf[MAX_MSG_LENGTH] = {0};

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) perro("opening socket");

	int optval = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

	name.sin_family = AF_INET;
	name.sin_addr.s_addr = INADDR_ANY;
	name.sin_port = htons(PORT);
	if(bind(sock, (void*) &name, sizeof(name))) perro("binding tcp socket");
	if(listen(sock, 1) == -1) perro("listen");
	
	struct sockaddr cli_addr;
	int cli_len = sizeof(cli_addr);
	int new_socket, new_fd, pid;
	FILE* new_stream;

	
	if(new_fd = dup(STDERR_FILENO) == -1) perro("dup");
	new_stream = fdopen(new_fd, "w");
	
	D("Initializing server...\n");
	printf("Enter the security key(single character):");
	scanf("%c",&cipherKey);
	while(new_socket = accept(sock, &cli_addr, &cli_len)) {
		int keep=0;
		
		printf("do you want to keep this client? -->");
		scanf("%d",&keep);
		
	
		D("Client connected.\nForking... ");
		if(pid = fork()) D("child pid = %d.\n", pid);
		else {
			pid = getpid();
			if(new_socket < 0) perro("accept");
			if(dup2(new_socket, STDOUT_FILENO) == -1) perro("dup2");
			//if(dup2(new_socket, STDERR_FILENO) == -1) perro("dup2");
			while(1) {
				int readc = 0, filled = 0;
				if(keep){
				while(1) {
					readc = recv(new_socket, buf+filled, MAX_MSG_LENGTH-filled-1, 0);
					if(!readc) break;
					filled += readc;
					if(buf[filled-1] == '\0') break;
					
				}
				}
				if(!readc) {
					D("\t[%d] Client disconnected.\n", pid);
					break;
				}
				D("\t[%d] Encrypted Command received: %s", pid, buf);
				int i;
				int x=0;
				while(buf[x]!='\n'){
				buf[x]=Cipher(buf[x]);
				x++;
				}
				buf[strcspn(buf, "\n")] = 0;
				char str[]=" > tmp.txt";
				strcat(buf,str);
				D("%s",buf);
				system(buf);
				char ch,ech;
				FILE *fp,*fp2;
				fp = fopen("tmp.txt","r");
				fp2 = fopen("enc.txt","w");
				ch = fgetc(fp);
				ech = Cipher(ch);
				
				while(ch!=EOF){
					fputc(ech,fp2);
					ch = fgetc(fp);
					ech = Cipher(ch);
				}
				fclose(fp);
				fclose(fp2);
				system("cat enc.txt");
				system("rm tmp.txt");
				system("rm enc.txt");
				D("\t[%d] Finished executing command.\n", pid);
				D("===================================================\n");
				char sym[]="\n> ";
				int j=0;
				while(sym[j]!='\0'){
					sym[j]=Cipher(sym[j]);
					j++;
				}
				send(new_socket, sym, 4, MSG_NOSIGNAL);
			}
			close(new_socket);
			D("\t[%d] Dying.", pid);
			exit(0);
		}
		
	}
	fclose(new_stream);
	close(sock);
	return 0;
}
