#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>

#define	SA	struct sockaddr
#define	MAXLINE		4096

void Sendto(int fd, const void *ptr, size_t nbytes, int flags,
	   const struct sockaddr *sa, socklen_t salen) {
	if (sendto(fd, ptr, nbytes, flags, sa, salen) != (ssize_t)nbytes) {
		printf("sendto error\n");
        exit(1);
    }
}

ssize_t Recvfrom(int fd, void *ptr, size_t nbytes, int flags,
		 struct sockaddr *sa, socklen_t *salenptr) {
	ssize_t	n;

	if ( (n = recvfrom(fd, ptr, nbytes, flags, sa, salenptr)) < 0) {
		printf("recvfrom error\n");
        exit(1);
    }
	return(n);
}


char *sock_ntop_host(const struct sockaddr *sa, socklen_t salen) {
    static char str[128];

	switch (sa->sa_family) {
	case AF_INET: {
		struct sockaddr_in	*sin = (struct sockaddr_in *) sa;

		if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
			return(NULL);
		return(str);
	}

	case AF_INET6: {
		struct sockaddr_in6	*sin6 = (struct sockaddr_in6 *) sa;

		if (inet_ntop(AF_INET6, &sin6->sin6_addr, str, sizeof(str)) == NULL)
			return(NULL);
		return(str);
	}

	default:
		snprintf(str, sizeof(str), "sock_ntop_host: unknown AF_xxx: %d, len %d",
				 sa->sa_family, salen);
		return(str);
	}
    return (NULL);
}

char *Sock_ntop_host(const struct sockaddr *sa, socklen_t salen) {
	char *ptr;

	if ( (ptr = sock_ntop_host(sa, salen)) == NULL) {
		printf("sock_ntop_host error\n");
        exit(1);
    }
	return(ptr);
}

int udp_client(const char *host, const char *serv, void **saptr, socklen_t *lenp) {
	int	sockfd, n;
	struct addrinfo	hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0) {
		printf("udp_client error for %s, %s: %s\n",
				 host, serv, gai_strerror(n));
        exit(1);
    }
	ressave = res;

	do {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sockfd >= 0) break;
	} while ( (res = res->ai_next) != NULL);

	if (res == NULL) {
		printf("udp_client error for %s, %s\n", host, serv);
        exit(1);
    }

	*saptr = malloc(res->ai_addrlen);
	memcpy(*saptr, res->ai_addr, res->ai_addrlen);
	*lenp = res->ai_addrlen;

	freeaddrinfo(ressave);

	return(sockfd);
}

int main(int argc, char **argv) {
	int	sockfd, n;
	char recvline[MAXLINE + 1];
	socklen_t salen;
	struct sockaddr	*sa;

	if (argc != 3) {
		printf("usage: cudp <hostname/IPaddress> <service/port>\n");
        exit(1);
    }

	sockfd = udp_client(argv[1], argv[2], (void **) &sa, &salen);

	printf("sending to %s\n", Sock_ntop_host(sa, salen));

	Sendto(sockfd, "", 1, 0, sa, salen);
	n = Recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
	recvline[n] = '\0';
    printf("RECV = %s\n", recvline);
    while(1);
	exit(0);
}