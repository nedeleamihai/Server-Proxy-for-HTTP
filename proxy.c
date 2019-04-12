#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>
#include<errno.h>

#define HTTP_PORT 80
#define MAXLEN 500
   
void error(char* msg){
	perror(msg);
	exit(0);
}
  
int main(int argc,char* argv[]){

	struct sockaddr_in cli_addr,serv_addr;
	struct hostent* host;
	int sockfd,newsockfd,clilen;
	pid_t pid;
   
	if(argc<2)
		error("./proxy <port_no>");
        
	bzero((char*)&cli_addr, sizeof(cli_addr));
	bzero((char*)&serv_addr,sizeof(serv_addr));
   
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(atoi(argv[1]));
	serv_addr.sin_addr.s_addr=INADDR_ANY;
   
  
	sockfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(sockfd<0)
		error("Problema in initializarea socketului");
   
	if(bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
		error("Error on binding");
  
  
	listen(sockfd,50);
	clilen=sizeof(cli_addr);
 
	while(1){
 
		newsockfd=accept(sockfd,(struct sockaddr*)&cli_addr,(socklen_t *)&clilen);
   
		if(newsockfd<0)
			error("Problema la conexiune");
  
		pid=fork();
		if(pid==0){ 			
			char buffer[MAXLEN];
			char BUFFERSECOND[MAXLEN];
			char str1[MAXLEN],str2[MAXLEN],str3[MAXLEN];
			char* temp=NULL;
			struct sockaddr_in host_addr;
			int newsockfd1,n,sockfd1,port;
			
			bzero(buffer,MAXLEN);
			recv(newsockfd,buffer,MAXLEN,0);
			bzero(BUFFERSECOND,MAXLEN);  
			strcpy(BUFFERSECOND,buffer);
			sscanf(buffer,"%s %s %s",str1,str2,str3);
   
                        //GET
			if(strncmp(str1,"GET",3)==0 && strncmp(str2,"http://",7)==0 && strncmp(str3,"HTTP/1.0",8)==0){
				strcpy(str1,str2);
   
				temp=strtok(str2,"//");

				port=HTTP_PORT;
				temp=strtok(NULL,"/");
   
				sprintf(str2,"%s",temp);
				printf("host = %s",str2);
				host=gethostbyname(str2);

				strcat(str1,"^]");
				temp=strtok(str1,"//");
				temp=strtok(NULL,"/");
				if(temp!=NULL)
					temp=strtok(NULL,"^]");
					printf("\npath = %s\nPort = %d\n",temp,port);   
   
				bzero((char*)&host_addr,sizeof(host_addr));
				host_addr.sin_port=htons(port);
				host_addr.sin_family=AF_INET;
				bcopy((char*)host->h_addr,(char*)&host_addr.sin_addr.s_addr,host->h_length);
   
				sockfd1=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
				newsockfd1=connect(sockfd1,(struct sockaddr*)&host_addr,sizeof(struct sockaddr));
				sprintf(buffer,"\nConnected to %s  IP - %s\n",str2,inet_ntoa(host_addr.sin_addr));
				if(newsockfd1<0)
					error("Error in connecting to remote server");
   
				bzero((char*)buffer,sizeof(buffer));

				if(temp!=NULL){
					sprintf(buffer,"GET /%s %s\r\nHost: %s\r\nConnection: close\r\n\r\n",temp,str3,str2);
				}else{
					sprintf(buffer,"GET / %s\r\nHost: %s\r\nConnection: close\r\n\r\n",str3,str2);
 				}
 
				n=send(sockfd1,BUFFERSECOND,MAXLEN,0);
				if(n<0){
					error("Error writing to socket");
				}else{
					while(n>0){
						bzero((char*)buffer,MAXLEN);
						n=recv(sockfd1,buffer,MAXLEN,0);
						if(!(n<=0))
							send(newsockfd,buffer,n,0);
					}
				}
			}else{
				//POST
				if(strncmp(str1,"POST",4)==0 && strncmp(str2,"http://",7)==0 && strncmp(str3,"HTTP/1.0",8)==0){
					strcpy(str1,str2);
   
					temp=strtok(str2,"//");

					port=80;
					temp=strtok(NULL,"/");
   	
					sprintf(str2,"%s",temp);
					printf("host = %s",str2);
					host=gethostbyname(str2);

					strcat(str1,"^]");
					temp=strtok(str1,"//");
					temp=strtok(NULL,"/");
					if(temp!=NULL)
						temp=strtok(NULL,"^]");
						printf("\npath = %s\nPort = %d\n",temp,port);

					bzero((char*)&host_addr,sizeof(host_addr));
					host_addr.sin_port=htons(port);
					host_addr.sin_family=AF_INET;
					bcopy((char*)host->h_addr,(char*)&host_addr.sin_addr.s_addr,host->h_length); 

					sockfd1=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
					newsockfd1=connect(sockfd1,(struct sockaddr*)&host_addr,sizeof(struct sockaddr));
					sprintf(buffer,"\nConnected to %s  IP - %s\n",str2,inet_ntoa(host_addr.sin_addr));
					if(newsockfd1<0)
						error("Error in connecting to remote server");
   
					bzero((char*)buffer,sizeof(buffer));

					if(temp!=NULL){
						sprintf(buffer,"GET /%s %s\r\nHost: %s\r\nConnection: close\r\n\r\n",temp,str3,str2);
					}else{
						sprintf(buffer,"GET / %s\r\nHost: %s\r\nConnection: close\r\n\r\n",str3,str2);
 					}

					n=send(sockfd1,BUFFERSECOND,MAXLEN,0);
					if(n<0){
						error("Error writing to socket");
					}else{
						while(n>0){
							bzero((char*)buffer,MAXLEN);
							n=recv(sockfd1,buffer,MAXLEN,0);
							if(!(n<=0))
								send(newsockfd,buffer,n,0);
						}	
					}					
                                }else{
					send(newsockfd,"400 : BAD REQUEST\n",18,0);
				}
			}

			close(sockfd1);
			close(newsockfd);
			close(sockfd);
			exit(0);
		}else{
			close(newsockfd);
		}
	}
	return 0;
}
