// Copyright (c) 2015 Electric Power Research Institute, Inc.
// author: Mark Slicker <mark.slicker@gmail.com>

#include <arpa/inet.h>

typedef struct _UdpPort {
  PollEvent pe;
  Address source;
  int size;
  char buffer[];
} UdpPort;

char *net_receive (UdpPort *p, int *length) {
  // printf("IN udp.c: net_receive()\n"); // ADDED!!!
  p->source.length = sizeof (Address);
  // printf("...socket fd: %i, buffer %s\n", p->pe.socket, p->buffer); // ADDED!!!

  // ADDED!!! PRINT IPv4 ADDRESS OF SOURCE
  // https://stackoverflow.com/questions/1276294/getting-ipv4-address-from-a-sockaddr-structure
  struct sockaddr_in addr_in1 = p->source.in;
  struct sockaddr_in *addr_in = &addr_in1;
  char *s = inet_ntoa(addr_in->sin_addr);
  // printf("...IPv4 address of source: %s\n", s); // ADDED!!!

  // ADDED!!! PRINT IPv6 ADDRESS OF SOURCE
  // https://stackoverflow.com/questions/3727421/expand-an-ipv6-address-so-i-can-print-it-to-stdout
  char str[40];
  struct in6_addr addr1= p->source.in6.sin6_addr;
  struct in6_addr *addr = &addr1;
  sprintf(str, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                (int)addr->s6_addr[0], (int)addr->s6_addr[1],
                (int)addr->s6_addr[2], (int)addr->s6_addr[3],
                (int)addr->s6_addr[4], (int)addr->s6_addr[5],
                (int)addr->s6_addr[6], (int)addr->s6_addr[7],
                (int)addr->s6_addr[8], (int)addr->s6_addr[9],
                (int)addr->s6_addr[10], (int)addr->s6_addr[11],
                (int)addr->s6_addr[12], (int)addr->s6_addr[13],
                (int)addr->s6_addr[14], (int)addr->s6_addr[15]);
  // printf("...IPv6 address of source: %s\n", str); // ADDED!!!

  *length = recvfrom (p->pe.socket, p->buffer, p->size, 0,
		      (struct sockaddr *)&p->source, &p->source.length);
  
  // if (*length < 0) { perror("...Error: ");} // ADDED!!! // socket is nonblocking, so when no bytes read
                                            // returns -1 and error EAGAIN (Resource temporarily unavilable)

  p->pe.end = *length < 0;
  // printf ("...udp_read: %d\n", *length); 
  return *length < 0? NULL : p->buffer;
}

int net_send (UdpPort *p, char *buffer, int length, Address *addr) {
  // printf ("udp_write %d\n", length); fflush (stdout);  
  return sendto (p->pe.socket, buffer, length, 0,
		 (struct sockaddr *)(addr), addr->length);
}

int net_reply (UdpPort *p, char *buffer, int length) {
  return sendto (p->pe.socket, buffer, length, 0,
 		 (struct sockaddr *)(&p->source), p->source.length);
}

UdpPort *new_udp_port (int size) {
  UdpPort *p = calloc (1, sizeof (UdpPort) + size);
  p->pe.type = UDP_PORT; p->size = size; return p;
}

void net_open (UdpPort *p, Address *address) {
  if ((p->pe.socket = socket (address->family, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    print_error ("udp_open, socket");
  non_block_enable (p->pe.socket);
  reuse_address (p->pe.socket);
  event_add (p->pe.socket, p);
  if (bind (p->pe.socket, (struct sockaddr *)address, address->length) < 0)
    print_error ("upd_open, bind");
}

void net_join_group (UdpPort *p, const char *addr, int loop) {
  multicast_join (p->pe.socket, addr, loop);
}
