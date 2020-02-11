#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <time.h>
#include <arpa/inet.h>
#include <netdb.h>



int main(int argc,char* argv[])
{
  int listenfd, connfd,port, serconnfd;
  struct sockaddr_in proxyaddr, cliaddr,servaddr;
  socklen_t len;
  int enable =1;
  int n,i=0,k=0;
  char buf[1024];
  char buf2[1024];
  char resp[1024];
  char t1[500],t2[500],t3[500];
  char **pp;
  char *cache,*temp,*temp2,*temp3,*temp4,*temp5;
  int flag =0;
  int linelen;
  int a;
  char zzz[]="PST";
  struct timeval timeout;
  timeout.tv_sec = 3;
  timeout.tv_usec = 0;
  FILE* f;

  int month,wday, year, date,hour,min,sec;
  char *smon;
  char log[500];
  time_t timer;
  struct tm *t;
  char sad[100];
  
 
  char req[1024]= "GET /rfc2616.html HTTP/1.1\r\nUser-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:59.0) Gecko/20100101 Firefox/59.0\r\n Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*";
  strcat(req,"/");
  strcat(req,"*;q=0.8\r\n Accept-Language: en-US,en;q=0.5\r\nAccept-Encoding: gzip, deflate\r\nCookie: _ga=GA1.3.1086174080.1523035245\r\nUpgrade-Insecure-Requests: 1\r\nHost: ee323.kaist.ac.kr\r\nVia: 1.1 ubuntu (ee323_proxy/1.0.0)\r\nX-Forwarded-For: 127.0.0.1\r\nCache-Control: max-age=259200\r\nConnection: keep-alive\r\n\r\n");

  char *add1 = "Via: 1.1 ubuntu (ee323_proxy/1.0.0)\r\n"; 

  //printf("%s",req);

  struct in_addr addr;
  struct hostent *hostp;
  
  port = atoi(argv[1]);
  /*opening listening fd*/    
  listenfd = socket(AF_INET, SOCK_STREAM,0);
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
  bzero(&proxyaddr,sizeof(proxyaddr));

  proxyaddr.sin_family = AF_INET;
  proxyaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  proxyaddr.sin_port = htons(port);

  bind(listenfd, &proxyaddr, sizeof(proxyaddr));
  listen(listenfd,8);
  /*loop for accept, for proxy to connect with client*/
  while(1)
    {
      bzero((char*)buf,1024);
      bzero((char*)buf2,1024);

      bzero((char*)t1,1024);
      bzero((char*)t2,1024);
      bzero((char*)t3,1024);      
      connfd = accept(listenfd,&cliaddr,&len);
      n=read(connfd,buf,1024);
    
      sscanf(buf,"%s %s %s",t1,t2,t3);


      /*check if request is GET, than modify the request */
      if(t1==NULL) {printf("continue\n");continue;}
      if(strncmp(t1,"GET",3)==0)
	{ 	  
	  char *ptr;
	  int i=0;
	  bzero((char*)sad,100);
	  strcpy(sad,t2);
	  ptr = strtok(t2,"/");
	  ptr=strtok(NULL,"/");
	  hostp=gethostbyname(ptr);
	  ptr=strtok(NULL,"/");
	  	  
	  if(hostp==NULL)
	    {
	      printf("INVALID DOMAIN\n");
	      continue;
	    }
	  if(flag==0)//first time, no cache
	    {
	      printf("flag=0, no cache\n");
	      cache = (char*)calloc(1000000,sizeof(char));
	      temp = cache;
	      serconnfd = socket(AF_INET, SOCK_STREAM, 0);
	  
	      bzero(&servaddr, sizeof(servaddr));
	      servaddr.sin_family = AF_INET;
	      servaddr.sin_port = htons(80);
	      bcopy((char *)(hostp->h_addr_list[0]),(char*)&servaddr.sin_addr.s_addr,hostp->h_length);
	      connect(serconnfd,&servaddr,sizeof(servaddr));
	      write(serconnfd,req,sizeof(req));
	      k=0;
	      bzero((char*)resp,1024);
	      n=read(serconnfd,resp,500);
	      temp3 = resp;
	      temp4 = buf2;
	      
	      /*modifying the response*/
	      while(*(temp3)!='\n')
		{
		  temp3+=1;
		}
	      temp3+=1;	      
	      a=temp3-resp;
	      a=500-a;
	      for(i=0;i<a;i++)
		{
		  *temp4=*temp3;
		  temp4+=1;
		  temp3+=1;
		}
	      temp5=add1;
	      	      
	      temp4 = buf2;
	      temp3=resp;
	      while(*(temp3)!='\n')
		{
		  temp3+=1;
		}
	      temp3+=1;
	      
	      while(*temp5!='\0')
		{
		  *temp3=*temp5;
		  temp3+=1;
		  temp5+=1;
		}
	      for(i=0;i<a;i++)
		{
		  *temp3=*temp4;
		  temp3+=1;
		  temp4+=1;
		}
	      n=temp3-resp;
	      
	      // for caching
	      for(i=0;i<n;i++)
		{
		  *(temp+i)=*(resp+i);
		}
	      
	      write(connfd,resp,n);		     
	      temp +=n;
	      bzero((char*)resp,1024);

	      while(n=read(serconnfd,resp,1024))
		{		  
		  if(n>0)
		    {
		      //for caching
		      for(i=0;i<n;i++)
			{
			  *(temp+i)=*(resp+i);
			}		      
		      write(connfd,resp,n);		     
		      temp +=n;
		      k+=1;
		      
		      //if it is end of response, exit while loop
		      if((*(temp-4)==13) && (*(temp-3)==10) && (*(temp-2)==13) && (*(temp-1)==10))
			break;
		      bzero((char*)resp,1024);

		    }		  
		}
	      //make proxy log
	      unsigned short clientport = cliaddr.sin_port;
	      struct in_addr ip = cliaddr.sin_addr;
	      char *sip=inet_ntoa(ip);

	      timer = time(NULL);
	      t=localtime(&timer);
	      month = t->tm_mon+1;
	      year = t->tm_year+1900;
	      date = t->tm_mday;
	      hour = t->tm_hour;
	      min = t->tm_min;
	      sec = t->tm_sec;
	      switch(month)
		{	      
		case 1 : smon = "JAN";break;
		case 2 : smon = "FEB";break;
		case 3 : smon = "MAR";break;
		case 4 : smon = "APR";break;
		case 5 : smon = "MAY";break;
		case 6 : smon = "JUN";break;
		case 7 : smon = "JUL";break;
		case 8 : smon = "AUG";break;
		case 9 : smon = "SEP";break;
		case 10 : smon = "OCT";break;
		case 11 : smon = "NOV";break;
		case 12 : smon = "DEC";break;
		}

	      if(flag==0)
		sip="127.0.0.1";
	      sprintf(log,"%d %s %d %d:%d:%d %s %s %s %s %s\n",date,smon,year,hour,min,sec,zzz,sip,t1,sad,t3);

	      f=fopen("proxy.log","a");
	      fprintf(f,log);
	      fclose(f);
	      
	      if(strcmp(t2,"http:\000/ee323.kaist.ac.kr\000rfc2616.html")==0)
		{
		  flag = 1;
		  printf("flag changed to 1\n");
		}
	      else
		{printf("flag not changed\n");}
	      close(serconnfd);
	    }//end of if flag==0
	  
	  else//cache, flag==1
	    {
	      //make proxy log
	      unsigned short clientport = cliaddr.sin_port;
	      struct in_addr ip = cliaddr.sin_addr;
	      char *sip=inet_ntoa(ip);

	      timer = time(NULL);
	      t=localtime(&timer);
	      month = t->tm_mon+1;
	      year = t->tm_year+1900;
	      date = t->tm_mday;
	      hour = t->tm_hour;
	      min = t->tm_min;
	      sec = t->tm_sec;
	      switch(month)
		{	      
		case 1 : smon = "JAN";break;
		case 2 : smon = "FEB";break;
		case 3 : smon = "MAR";break;
		case 4 : smon = "APR";break;
		case 5 : smon = "MAY";break;
		case 6 : smon = "JUN";break;
		case 7 : smon = "JUL";break;
		case 8 : smon = "AUG";break;
		case 9 : smon = "SEP";break;
		case 10 : smon = "OCT";break;
		case 11 : smon = "NOV";break;
		case 12 : smon = "DEC";break;
		}

	      if(flag==0)
		sip="127.0.0.1";
	      sprintf(log,"%d %s %d %d:%d:%d %s %s %s %s %s\n",date,smon,year,hour,min,sec,zzz,sip,t1,sad,t3);

	      f=fopen("proxy.log","a");
	      fprintf(f,log);
	      fclose(f);
	      printf("flag=1, caching\n");
	      //caching
    	      write(connfd,cache,temp-cache);
	    }	    
	}//finish IF GET
      	
      close(connfd);
      
    }//finish while
  close(listenfd);
  free(cache);
  return 0;
}
