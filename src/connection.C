/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "remotetest.h"
#include "test_lib.h"
#include <stdio.h>

#include <assert.h>
#include <cstring>
#include <string>

using namespace std;

static Connection *con = NULL;
Connection *getConnection()
{
   return con;
}

void setConnection(Connection *c)
{
   con = c;
}


static void exit_header(MessageBuffer &buffer)
{
   buffer.add("X;", 2);
   buffer.add(EXIT_MSG, strlen(EXIT_MSG));
}

#if defined(os_windows_test)
char *my_strtok(char *str, const char *delim)
{
	return strtok(str, delim);
}
#else
char *my_strtok(char *str, const char *delim)
{
   static char *my_str = NULL;
   static char *save_ptr = NULL;
   
   if (str) {
      char *backup_str = strdup(str);
      if (my_str)
         free(my_str);
      my_str = backup_str;
   }
   else {
      my_str = NULL;
   }
   
   return strtok_r(my_str, delim, &save_ptr);
}
#endif

char *decodeInt(int i, char *buffer)
{
   char *cur = my_strtok(buffer, ":;");
   assert(strcmp(cur, INT_ARG) == 0);
   cur = my_strtok(NULL, ":;");
   sscanf(cur, "%d", (int *) &i);
   return strchr(buffer, ';')+1;
}

char *decodeBool(bool &b, char *buffer)
{
   char *cur = my_strtok(buffer, ":;");
   assert(strcmp(cur, BOOL_ARG) == 0);
   cur = my_strtok(NULL, ":;");
   string str = std::string(cur);
   if (str == "true") {
      b = true;
   }
   else if (str == "false") {
      b = false;
   }
   else {
      assert(0);
   }
   return strchr(buffer, ';')+1;
}

char *decodeString(std::string &str, char *buffer)
{
   assert(strncmp(buffer, STRING_ARG, strlen(STRING_ARG)) == 0);
   char *cur = my_strtok(buffer, ";");
   cur += strlen(STRING_ARG)+1;
   if (strncmp(cur, "<EMPTY>", strlen("<EMPTY>")) == 0)
      str = std::string();
   else
      str = std::string(cur);
   return strchr(buffer, ';')+1;
}

void encodeInt(int i, MessageBuffer &buf)
{
   char s_buffer[64];
   snprintf(s_buffer, 64, "%s:%d;", INT_ARG, i);
   buf.add(s_buffer, strlen(s_buffer));
}

void encodeString(std::string str, MessageBuffer &buf)
{
   buf.add(STRING_ARG, strlen(STRING_ARG));
   buf.add(":", 1);
   if (!str.length())
      buf.add("<EMPTY>", strlen("<EMPTY>"));
   else
      buf.add(str.c_str(), str.length());
   buf.add(";", 1);
}

void encodeBool(bool b, MessageBuffer &buf)
{
   buf.add(BOOL_ARG, strlen(BOOL_ARG));
   buf.add(":", 1);
   string str = b ? "true" : "false";
   buf.add(str.c_str(), str.length());
   buf.add(";", 1);
}

extern void handle_message(char *buffer);
bool Connection::recv_return(char* &buffer) {
   char *msg;
   for (;;) {
      bool result = recv_message(msg);
      if (!result)
         return false;
   
      if (*msg == 'R') {
         buffer = msg+2;
         return true;
      }
      if (*msg == 'M') {
         handle_message(msg+2);
      }
   }
   return false;
}

int Connection::sockfd = -1;
std::string Connection::hostname;
int Connection::port;
bool Connection::has_hostport = false;

Connection::Connection() :
   fd(-1),
   has_error(false)
{
}

Connection::Connection(std::string hostname_, int port_, int fd_exists) :
   fd(-1),
   has_error(false)
{
   hostname = hostname_;
   port = port_;
   has_hostport = true;

   if (fd_exists == -1)
      has_error = !client_connect();
   else
      fd = fd_exists;
}

Connection::~Connection()
{
   MessageBuffer buf;
   exit_header(buf);
   send_message(buf);

#if !defined(os_windows_test)
   if (fd != -1)
      close(fd);
#endif
}

int Connection::getFD()
{
   return fd;
}

bool Connection::hasError()
{
   return has_error;
}

#if !defined(os_windows_test)

#define SOCKTYPE_UNIX

#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>

bool Connection::send_message(MessageBuffer &buffer)
{
   buffer.add("\0", 1);

   uint32_t msg_size_unenc = buffer.get_buffer_size();
   uint32_t msg_size = htonl(msg_size_unenc);

   ssize_t result = send(fd, &msg_size, sizeof(uint32_t), 0);
   if (result == -1) {
      fprintf(stderr,"[%s:%u] - Error sending data count on socket\n", __FILE__, __LINE__);
      return false;
   }
   fprintf(stderr,"[%d] - Send size of %lu, (encoded 0x%lx)\n",
           getpid(), (unsigned long) msg_size_unenc, (unsigned long) msg_size);
   
   result = send(fd, buffer.get_buffer(), msg_size_unenc, 0);
   if (result == -1) {
     fprintf(stderr,"[%s:%u] - Error sending raw data on socket\n", __FILE__, __LINE__);
      return false;
   }
   return true;
}

bool Connection::recv_message(char* &buffer)
{
   static char *cur_buffer = NULL;
   static int cur_buffer_size = 0;
   bool sock_error;

   if (!waitForAvailData(fd, recv_timeout, sock_error))
      return false;
   if (sock_error) {
     fprintf(stderr,"[%d] - Recv sock exception\n", getpid());
   }
   
   uint32_t msg_size = 0, enc_msg_size = 0;
   ssize_t result;
   result = recv(fd, &enc_msg_size, sizeof(uint32_t), MSG_WAITALL);
   if (result == -1) {
      int errornum = errno;
      fprintf(stderr,"Error receiving data size on socket: %s\n", strerror(errornum));
      return false;
   }
   if (result == 0) {
      fprintf(stderr,"[%d] - Recv zero, other side shutdown.\n", getpid());
      return false;
   }
   msg_size = ntohl(enc_msg_size);
   fprintf(stderr,"[%d] - Recv size of %lu, (encoded 0x%x, result = %d)\n",
                getpid(), (unsigned long) msg_size, enc_msg_size, (int) result);

   assert(msg_size < (1024*1024)); //No message over 1MB--should be plenty
   
   if (msg_size == 0) {
      //Other side hung up.
      return false;
   }
   if (msg_size > (uint32_t)cur_buffer_size) {
      if (cur_buffer)
         free(cur_buffer);
      cur_buffer = NULL;
   }
   if (cur_buffer == NULL) {
      cur_buffer_size = msg_size+1;
      cur_buffer = (char *) malloc(cur_buffer_size);
   }
   memset(cur_buffer, 0, cur_buffer_size);

   result = recv(fd, cur_buffer, msg_size, MSG_WAITALL);
   if (result == -1) {
      fprintf(stderr,"[%s:%u] - Error receiving data on socket\n", __FILE__, __LINE__);
      return false;
   }

   buffer = cur_buffer;
   return true;
}

bool Connection::waitForAvailData(int sock, int timeout_s, bool &sock_error)
{
   fd_set readfds;
   fd_set exceptfds;
   fd_set writefds;
   FD_ZERO(&readfds);
   FD_ZERO(&exceptfds);
   FD_ZERO(&writefds);
   FD_SET(sock, &readfds);
   FD_SET(sock, &exceptfds);

   struct timeval timeout;
   timeout.tv_sec = timeout_s;
   timeout.tv_usec = 0;
   sock_error = false;

   for (;;) {
      fprintf(stderr,"Waiting for available data on fd %d\n", sock);
      int result = select(sock+1, &readfds, &writefds, &exceptfds, &timeout);
      if (result == -1 && errno == EINTR) 
         continue;
      else if (result == -1) {
         fprintf(stderr,"[%s:%u] - Error selecting to accept connections\n", __FILE__, __LINE__);
         return false;
      }
      else if (result == 0) {
         fprintf(stderr,"[%s:%u] - Timeout accepting connections\n", __FILE__, __LINE__);
         return false;
      }
      else if (result >= 1) {
         if (FD_ISSET(sock, &readfds) && FD_ISSET(sock, &exceptfds)) {
            sock_error = true;
            return true;
         }
         if (FD_ISSET(sock, &readfds)) {
            return true;
         }
         if (FD_ISSET(sock, &exceptfds)) {
            sock_error = true;
            return false;
         }
         assert(0);
      }      
      else {
         assert(0);
      }
   }
   return false;
}

bool Connection::server_accept()
{
   struct sockaddr_in addr;
   socklen_t socklen = sizeof(struct sockaddr_in);
   bool sock_error;

   fprintf(stderr,"[%s:%u] - server_accept waiting for connection\n", __FILE__, __LINE__);
   if (!waitForAvailData(sockfd, accept_timeout, sock_error))
      return false;
       
   assert(fd == -1); //One connection at a time.

   fd = accept(sockfd, (sockaddr *) &addr, &socklen);
   if (fd == -1) {
     fprintf(stderr,"[%s:%u] - Error accepting connection\n", __FILE__, __LINE__);
      return false;
   }
   fprintf(stderr,"server_accept recieved new connection on fd %d\n", fd);
   
   return true;
}

bool Connection::client_connect()
{
   fprintf(stderr,"Trying client_connect to %s:%d\n", hostname.c_str(), port);

   assert(has_hostport);
   fd = socket(PF_INET, SOCK_STREAM, 0);
   if (fd == -1) {
      fprintf(stderr,"Unable to create client socket: %s\n", strerror(errno));
      return false;
   }

   fprintf(stderr,"Trying to get hostname for %s\n", hostname.c_str());
   struct addrinfo *ai, *p;
   struct addrinfo hints;
 
   memset(&hints, 0x00, sizeof(hints));
   hints.ai_family   = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_protocol = 0;
   hints.ai_flags    = AI_NUMERICSERV;
 
   string portstr = std::to_string(port);
   if (getaddrinfo(hostname.c_str(), portstr.c_str(), &hints, &ai)) {
      fprintf(stderr,"Error looking up hostname %s\n", hostname.c_str());
      return false;
   }

   fprintf(stderr,"Got a size %d address for hostname %s\n", ai->ai_addrlen, hostname.c_str());
   if (ai == NULL) {
      fprintf(stderr,"No addresses with hostname %s\n", hostname.c_str());
      return false;
   } 

   int result = 0;
   for (p = ai;p != NULL; p = p->ai_next) {
     char ipstr[INET6_ADDRSTRLEN];
     void *addr;
     // p->ai_addr->sin_addr->s_addr is htonl(inet_network(HOSTNAME)
     if (p->ai_family == AF_INET) {
       struct sockaddr_in *sockaddr = (struct sockaddr_in *)p->ai_addr;
       addr = &(sockaddr->sin_addr);
       /* convert the IP to a string and print it: */
       inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);             
       result = connect(fd, (struct sockaddr *) &sockaddr, sizeof(sockaddr));
     } else if (p->ai_family == AF_INET6) {
       struct sockaddr_in6 *sockaddr = (struct sockaddr_in6 *)p->ai_addr;
       addr = &(sockaddr->sin6_addr);
       /* convert the IP to a string and print it: */
       inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);             
       result = connect(fd, (struct sockaddr *) &sockaddr, sizeof(sockaddr));
     }
     fprintf(stderr,"Connecting to %s:%u\n", ipstr, port);
   }

   if (result == -1) {
      fprintf(stderr,"[%s:%u] - Error connecting to server\n", __FILE__, __LINE__);
      return false;
   }

   fprintf(stderr,"Successfully connected to %s\n", hostname.c_str());
   return true;
}

bool Connection::server_setup(string &hostname_, int &port_)
{
   if (has_hostport) {
      fprintf(stderr,"server_setup returning existing hostname/port %s/%d",
                   hostname.c_str(), port);
      hostname_ = hostname;
      port_ = port;
      assert(sockfd != -1);
      return true;
   }

   sockfd = socket(PF_INET, SOCK_STREAM, 0);
   if (sockfd == -1) {
      fprintf(stderr,"Unable to create socket: %s\n", strerror(errno));
      return false;
   }
   fprintf(stderr,"server_setup socket call returned %d\n", sockfd);
   
   struct sockaddr_in addr;
   socklen_t socklen = sizeof(struct sockaddr_in);
   
   memset(&addr, 0, socklen);
   addr.sin_family = AF_INET;
   addr.sin_port = 0;
   addr.sin_addr.s_addr = INADDR_ANY;
   
   int result = ::bind(sockfd, (struct sockaddr *) &addr, socklen);
   if (result != 0){
      fprintf(stderr,"Unable to bind socket: %s\n", strerror(errno));
      return false;
   }
   fprintf(stderr,"[%s:%u] - server_setup successful bind on socket\n", __FILE__, __LINE__);

   result = listen(sockfd, 16);
   if (result == -1) {
     fprintf(stderr,"Unable to listen on socket: %s\n", strerror(errno));
      return false;
   }
   fprintf(stderr,"[%s:%u] - server_setup successful listen on socket\n", __FILE__, __LINE__);

   result = getsockname(sockfd, (sockaddr *) &addr, &socklen);
   if (result != 0) {
      fprintf(stderr,"[%s:%u] - Unable to getsockname on socket\n", __FILE__, __LINE__);
      return false;
   }

   char *override_name = getenv("DYNINST_TESTSERVER_HOST");
   if (override_name) {
      hostname = override_name;
   }
   else {
      char name_buffer[1024];
      result = gethostname(name_buffer, 1024);
      if (result != 0) {
         fprintf(stderr,"[%s:%u] - Unable to get hostname\n", __FILE__, __LINE__);
         return false;
      }
      hostname = name_buffer;
   }

   port = (int) addr.sin_port;

   hostname_ = hostname;
   port_ = port;
   has_hostport = true;

   fprintf(stderr,"[%s:%u] - server_setup returning new hostname/port %s/%d\n",
                __FILE__, __LINE__, hostname.c_str(), port);
   return true;
}

#else

bool Connection::send_message(MessageBuffer &)
{
	assert(0);
   return true;
}

bool Connection::recv_message(char* &)
{
	assert(0);
	return true;
}

bool Connection::waitForAvailData(int sock, int timeout_s, bool &sock_error)
{
	assert(0);
	return true;
}

bool Connection::server_accept()
{
	assert(0);
	return true;
}

bool Connection::client_connect()
{
	assert(0);
	return true;
}

bool Connection::server_setup(string &hostname_, int &port_)
{
	assert(0);
	return true;
}

#endif
