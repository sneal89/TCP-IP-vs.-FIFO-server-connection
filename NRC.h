
#ifndef _NRC_H_
#define _NRC_H_

#include "common.h"

class NRC
{
public:
	
private:
	int sockfd;
	
public:
	NRC(const string host_name, const string port); //client 
	
	NRC(const string port,void (*handle_process_loop)(NRC*)); //server

	NRC(int fd);

	~NRC();
	/* Destructor of the local copy of the bus. By default, the Server Side deletes any IPC 
	 mechanisms associated with the channel. */

	int open_pipe(string _pipe_name, int mode);

	char* cread(int *len=NULL);
	/* Blocking read of data from the channel. Returns a string of characters
	 read from the channel. Returns NULL if read failed. */

	int cwrite(char *msg, int msglen);
	/* Write the data to the channel. The function returns the number of characters written
	 to the channel. */
	 
};

#endif
