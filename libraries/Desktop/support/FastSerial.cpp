// -*-  tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: t -*-
//
// Interrupt-driven serial transmit/receive library.
//
//      Copyright (c) 2010 Michael Smith. All rights reserved.
//
// Receive and baudrate calculations derived from the Arduino
// HardwareSerial driver:
//
//      Copyright (c) 2006 Nicholas Zambetti.  All right reserved.
//
// Transmit algorithm inspired by work:
//
//      Code Jose Julio and Jordi Munoz. DIYDrones.com
//
//      This library is free software; you can redistribute it and/or
//      modify it under the terms of the GNU Lesser General Public
//      License as published by the Free Software Foundation; either
//      version 2.1 of the License, or (at your option) any later version.
//
//      This library is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//      Lesser General Public License for more details.
//
//      You should have received a copy of the GNU Lesser General Public
//      License along with this library; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//


//#include "../AP_Common/AP_Common.h"
#include "FastSerial.h"
#include "WProgram.h"
#include <unistd.h>
#include <pty.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "desktop.h"

#define LISTEN_BASE_PORT 5760
#define BUFFER_SIZE 128

    
#if   defined(UDR3)
# define FS_MAX_PORTS   4
#elif defined(UDR2)
# define FS_MAX_PORTS   3
#elif defined(UDR1)
# define FS_MAX_PORTS   2
#else
# define FS_MAX_PORTS   1
#endif

static struct tcp_state {
	bool connected; // true if a client has connected
	int listen_fd;  // socket we are listening on
	int fd;         // data socket
	int serial_port;
} tcp_state[FS_MAX_PORTS];



/*
  start a TCP connection for a given serial port. If
  wait_for_connection is true then block until a client connects
 */
static void tcp_start_connection(unsigned int serial_port, bool wait_for_connection)
{
	struct tcp_state *s = &tcp_state[serial_port];
	int one=1;
	struct sockaddr_in sockaddr;
	int ret;

	s->serial_port = serial_port;

	/*  Create the listening socket  */
	s->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (s->listen_fd == -1) {
        fprintf(stderr, "ECHOSERV: Error creating listening socket - %s\n", strerror(errno));
        exit(1);
	}

	memset(&sockaddr,0,sizeof(sockaddr));

#ifdef HAVE_SOCK_SIN_LEN
	sockaddr.sin_len = sizeof(sockaddr);
#endif
	sockaddr.sin_port = htons(LISTEN_BASE_PORT+serial_port);
	sockaddr.sin_family = AF_INET;

	s->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (s->listen_fd == -1) {
		fprintf(stderr, "socket failed - %s\n", strerror(errno));
		exit(1);
	}

	/* we want to be able to re-use ports quickly */
	setsockopt(s->listen_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

	ret = bind(s->listen_fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
	if (ret == -1) {
		fprintf(stderr, "bind failed on port %u - %s\n",
				LISTEN_BASE_PORT+serial_port,
				strerror(errno));
		exit(1);
	}

	ret = listen(s->listen_fd, 5);
	if (ret == -1) {
        fprintf(stderr, "listen failed - %s\n", strerror(errno));
        exit(1);
	}

	printf("Serial port %u on TCP port %u\n", serial_port, LISTEN_BASE_PORT+serial_port);
	fflush(stdout);

	if (wait_for_connection) {
		printf("Waiting for connection ....\n");
		s->fd = accept(s->listen_fd, NULL, NULL);
		if (s->fd == -1) {
            fprintf(stderr, "accept() error - %s", strerror(errno));
            exit(1);
        }
		s->connected = true;
    }
}


/*
  use select() to see if something is pending
 */
static bool select_check(int fd)
{
	fd_set fds;
	struct timeval tv;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	// zero time means immediate return from select()
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	if (select(fd+1, &fds, NULL, NULL, &tv) == 1) {
		return true;
	}
	return false;
}


/*
  see if a new connection is coming in
 */
static void check_connection(struct tcp_state *s)
{
	if (s->connected) {
		// we only want 1 connection at a time
		return;
	}
	if (select_check(s->listen_fd)) {
		s->fd = accept(s->listen_fd, NULL, NULL);
		if (s->fd != -1) {
			s->connected = true;
			printf("New connection on serial port %u\n", s->serial_port);
		}
	}
}


FastSerial::Buffer __FastSerial__rxBuffer[FS_MAX_PORTS];
FastSerial::Buffer __FastSerial__txBuffer[FS_MAX_PORTS];

// Constructor /////////////////////////////////////////////////////////////////

FastSerial::FastSerial(const uint8_t portNumber, volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
					   volatile uint8_t *ucsra, volatile uint8_t *ucsrb, const uint8_t u2x,
					   const uint8_t portEnableBits, const uint8_t portTxBits) :
					   _ubrrh(ubrrh),
					   _ubrrl(ubrrl),
					   _ucsra(ucsra),
					   _ucsrb(ucsrb),
					   _u2x(portNumber),
					   _portEnableBits(portEnableBits),
					   _portTxBits(portTxBits),
					   _rxBuffer(&__FastSerial__rxBuffer[portNumber]),
					   _txBuffer(&__FastSerial__txBuffer[portNumber])
{
}

// Public Methods //////////////////////////////////////////////////////////////

void FastSerial::begin(long baud)
{
	tcp_start_connection(_u2x, _u2x == 0?true:false);
}

void FastSerial::begin(long baud, unsigned int rxSpace, unsigned int txSpace)
{
	begin(baud);
}

void FastSerial::end()
{
}

int FastSerial::available(void)
{
	struct tcp_state *s = &tcp_state[_u2x];

	check_connection(s);

	if (!s->connected) {
		return 0;
	}

	if (select_check(s->fd)) {
#ifdef FIONREAD
		// use FIONREAD to get exact value if possible
		int num_ready;
		if (ioctl(s->fd, FIONREAD, &num_ready) == 0) {
			if (num_ready > BUFFER_SIZE) {
				return BUFFER_SIZE;
			}
			if (num_ready == 0) {
				// EOF is reached
				fprintf(stdout, "Closed connection on serial port %u\n", s->serial_port);
				close(s->fd);
				s->connected = false;
				return 0;
			}
			return num_ready;
		}
#endif
		return 1; // best we can do is say 1 byte available
	}
	return 0;
}

int FastSerial::txspace(void)
{
	// always claim there is space available
	return BUFFER_SIZE;
}

int FastSerial::read(void)
{
	struct tcp_state *s = &tcp_state[_u2x];
	char c;

	if (available() <= 0) {
		return -1;
	}

    int n = recv(s->fd, &c, 1, MSG_DONTWAIT | MSG_NOSIGNAL);
	if (n <= 0) {
		// the socket has reached EOF
		close(s->fd);
		s->connected = false;
		fprintf(stdout, "Closed connection on serial port %u\n", s->serial_port);
		fflush(stdout);
		return -1;
	}
    if (n == 1) {
		return (int)c;
	}
	return -1;
}

int FastSerial::peek(void)
{
	return -1;
}

void FastSerial::flush(void)
{
}

void FastSerial::write(uint8_t c)
{
	struct tcp_state *s = &tcp_state[_u2x];
	check_connection(s);
	if (!s->connected) {
		return;
	}
	send(s->fd, &c, 1, MSG_DONTWAIT | MSG_NOSIGNAL);
}

// Buffer management ///////////////////////////////////////////////////////////

bool FastSerial::_allocBuffer(Buffer *buffer, unsigned int size)
{
	return false;
}

void FastSerial::_freeBuffer(Buffer *buffer)
{
}

/*
  return true if any bytes are pending
 */
void desktop_serial_select_setup(fd_set *fds, int *fd_high)
{
	int i;

	for (i=0; i<FS_MAX_PORTS; i++) {
		if (tcp_state[i].connected) {
			FD_SET(tcp_state[i].fd, fds);
			if (tcp_state[i].fd > *fd_high) {
				*fd_high = tcp_state[i].fd;
			}
		}
	}
}
