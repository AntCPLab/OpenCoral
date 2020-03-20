
#include "sockets.h"
#include "Exceptions/Exceptions.h"
#include "Tools/time-func.h"

#include <iostream>
#include <fcntl.h>
using namespace std;

void error(const char *str)
{
  char err[1000];
  gethostname(err,1000);
  strcat(err," : ");
  strcat(err,str);
  perror(err);
  throw bad_value();
}

void error(const char *str1,const char *str2)
{
  char err[1000];
  gethostname(err,1000);
  strcat(err," : ");
  strcat(err,str1);
  strcat(err,str2);
  perror(err);
  throw bad_value();
}

void set_up_server_socket(sockaddr_in& dest,int& consocket,int& main_socket,int Portnum)
{

  struct sockaddr_in serv; /* socket info about our server */
  int socksize = sizeof(struct sockaddr_in);

  memset(&dest, 0, sizeof(dest));    /* zero the struct before filling the fields */
  memset(&serv, 0, sizeof(serv));    /* zero the struct before filling the fields */
  serv.sin_family = AF_INET;         /* set the type of connection to TCP/IP */
  serv.sin_addr.s_addr = INADDR_ANY; /* set our address to any interface */
  serv.sin_port = htons(Portnum);    /* set the server port number */

  main_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (main_socket<0) { error("set_up_socket:socket"); }

  int one=1;
  int fl=setsockopt(main_socket,SOL_SOCKET,SO_REUSEADDR,(char*)&one,sizeof(int));
  if (fl<0) { error("set_up_socket:setsockopt"); }

  /* disable Nagle's algorithm */
  fl= setsockopt(main_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&one,sizeof(int));
  if (fl<0) { error("set_up_socket:setsockopt");  }

  octet my_name[512];
  memset(my_name,0,512*sizeof(octet));
  gethostname((char*)my_name,512);

  /* bind serv information to mysocket
   *   - Just assume it will eventually wake up
   */
  fl=1;
  while (fl!=0)
    { fl=::bind(main_socket, (struct sockaddr *)&serv, sizeof(struct sockaddr));
      if (fl != 0)
        { cerr << "Binding to socket on " << my_name << ":" << Portnum << " failed, trying again in a second ..." << endl;
          sleep(1);
        }
#ifdef DEBUG_NETWORKING
      else
        { cerr << "Bound on port " << Portnum << endl; }
#endif
    }
  if (fl<0) { error("set_up_socket:bind");  }

  /* start listening, allowing a queue of up to 1 pending connection */
  fl=listen(main_socket, 1);
  if (fl<0) { error("set_up_socket:listen");  }

  consocket = accept(main_socket, (struct sockaddr *)&dest, (socklen_t*) &socksize);

  if (consocket<0) { error("set_up_socket:accept"); }

}

void close_server_socket(int consocket,int main_socket)
{
  if (close(consocket)) { error("close(socket)"); }
  if (close(main_socket)) { error("close(main_socket"); };
}

void set_up_client_socket(int& mysocket,const char* hostname,int Portnum)
{
   mysocket = socket(AF_INET, SOCK_STREAM, 0);
   if (mysocket<0) { error("set_up_socket:socket");  }

  /* disable Nagle's algorithm */
  int one=1;
  int fl= setsockopt(mysocket, IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof(int));
  if (fl<0) { error("set_up_socket:setsockopt");  }

  fl=setsockopt(mysocket, SOL_SOCKET, SO_REUSEADDR, (char*)&one, sizeof(int));
  if (fl<0) { error("set_up_socket:setsockopt"); }

   struct addrinfo hints, *ai=NULL,*rp;
   memset (&hints, 0, sizeof(hints));
   hints.ai_family = AF_INET;
   hints.ai_flags = AI_CANONNAME;

   octet my_name[512];
   memset(my_name,0,512*sizeof(octet));
   gethostname((char*)my_name,512);

   int erp;
   for (int i = 0; i < 60; i++)
     { erp=getaddrinfo (hostname, NULL, &hints, &ai);
       if (erp == 0)
         { break; }
       else
         { cerr << "getaddrinfo on " << my_name << " has returned '" << gai_strerror(erp) <<
           "' for " << hostname << ", trying again in a second ..." << endl;
           if (ai)
             freeaddrinfo(ai);
           sleep(1);
         }
     }
   if (erp!=0)
     { error("set_up_socket:getaddrinfo");  }

   bool success = false;
   socklen_t len = 0;
   const struct sockaddr* addr = 0;
   for (rp=ai; rp!=NULL; rp=rp->ai_next)
      { addr = ai->ai_addr;

        if (ai->ai_family == AF_INET)
           {
             len = ai->ai_addrlen;
             success = true;
             continue;
           }
      }

   if (not success)
     {
       for (rp = ai; rp != NULL; rp = rp->ai_next)
         cerr << "Family on offer: " << ai->ai_family << endl;
       runtime_error(string("No AF_INET for ") + (char*)hostname + " on " + (char*)my_name);
     }


   Timer timer;
   timer.start();
   struct sockaddr_in* addr4 = (sockaddr_in*) addr;
   addr4->sin_port = htons(Portnum);      // set destination port number
#ifdef DEBUG_IPV4
   cout << "connect to ip " << hex << addr4->sin_addr.s_addr << " port " << addr4->sin_port << dec << endl;
#endif

   int attempts = 0;
   long wait = 1;
   do
   {  fl=1;
      while (fl==1 || errno==EINPROGRESS)
        {
          fl=connect(mysocket, addr, len);
          attempts++;
          if (fl != 0)
            usleep(wait *= 2);
        }
   }
   while (fl == -1 && (errno == ECONNREFUSED || errno == ETIMEDOUT)
            && timer.elapsed() < 60);

   if (fl < 0)
     {
       cout << attempts << " attempts" << endl;
       error("set_up_socket:connect:", hostname);
     }

   freeaddrinfo(ai);

#ifdef __APPLE__
  int flags = fcntl(mysocket, F_GETFL, 0);
  fl = fcntl(mysocket, F_SETFL, O_NONBLOCK |  flags);
  if (fl < 0)
    error("set non-blocking");
#endif
}

void close_client_socket(int socket)
{
  if (close(socket))
    {
      char tmp[1000];
      sprintf(tmp, "close(%d)", socket);
      error(tmp);
    }
}

unsigned long long sent_amount = 0, sent_counter = 0;


template<>
void send(int socket,int a)
{
  unsigned char msg[1];
  msg[0]=a&255;
  if (send(socket,msg,1,0)!=1)
    { error("Send error - 2 ");  }
}


template<>
void receive(int socket,int& a)
{
  unsigned char msg[1];
  receive(socket, msg, 1);
  a=msg[0];
}



void send_ack(int socket)
{
  char msg[]="OK";
  if (send(socket,msg,2,0)!=2)
        { error("Send Ack");  }
}


int get_ack(int socket)
{
  char msg[]="OK";
  char msg_r[2];
  int i=0,j;
  while (2-i>0)
    { j=recv(socket,msg_r+i,2-i,0);
      i=i+j;
    }

  if (msg_r[0]!=msg[0] || msg_r[1]!=msg[1]) { return 1; }
  return 0;
}

