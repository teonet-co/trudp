#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define	SA	struct sockaddr
#define	MAXLINE		4096

void Sendto(int fd, const void *ptr, size_t nbytes, int flags,
	   const struct sockaddr *sa, socklen_t salen) {
	if (sendto(fd, ptr, nbytes, flags, sa, salen) != (ssize_t)nbytes) {
		printf("sendto error\n");
        exit(1);
    }
}


char *sock_ntop(const struct sockaddr *sa, socklen_t salen) {
    char portstr[8];
    static char str[128];

	switch (sa->sa_family) {
	case AF_INET: {
		struct sockaddr_in	*sin = (struct sockaddr_in *) sa;

		if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
			return(NULL);
		if (ntohs(sin->sin_port) != 0) {
			snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin->sin_port));
			strcat(str, portstr);
		}
		return(str);
	}

	case AF_INET6: {
		struct sockaddr_in6	*sin6 = (struct sockaddr_in6 *) sa;

		str[0] = '[';
		if (inet_ntop(AF_INET6, &sin6->sin6_addr, str + 1, sizeof(str) - 1) == NULL)
			return(NULL);
		if (ntohs(sin6->sin6_port) != 0) {
			snprintf(portstr, sizeof(portstr), "]:%d", ntohs(sin6->sin6_port));
			strcat(str, portstr);
			return(str);
		}
		return (str + 1);
	}

	default:
		snprintf(str, sizeof(str), "sock_ntop: unknown AF_xxx: %d, len %d",
				 sa->sa_family, salen);
		return(str);
	}
    return (NULL);
}

char *Sock_ntop(const struct sockaddr *sa, socklen_t salen) {
	char	*ptr;

	if ( (ptr = sock_ntop(sa, salen)) == NULL) {
		printf("sock_ntop error\n");
        exit(1);
    }
	return(ptr);
}

ssize_t Recvfrom(int fd, void *ptr, size_t nbytes, int flags,
		 struct sockaddr *sa, socklen_t *salenptr) {
	ssize_t		n;

	if ( (n = recvfrom(fd, ptr, nbytes, flags, sa, salenptr)) < 0) {
		printf("recvfrom error\n");
        exit(1);
    }
	return(n);
}

int udp_server(const char *host, const char *serv, socklen_t *addrlenp) {
	int	sockfd, n;
	struct addrinfo	hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC; // AF_INET6  /* FOR IpV6 SERVER, that can serve two types 4 and 6 */
	hints.ai_socktype = SOCK_DGRAM;

	if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0) {
		printf("udp_server error for %s, %s: %s\n",
				 host, serv, gai_strerror(n));
        exit(1);
    }
	ressave = res;

	do {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sockfd < 0)	continue;

        /* FOR IpV6 SERVER, that can serve two types 4 and 6 */
        // int off = 0;
        // setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&off, sizeof(off)); 

		if (bind(sockfd, res->ai_addr, res->ai_addrlen) == 0) {
			break;
        }

		close(sockfd);
	} while ( (res = res->ai_next) != NULL);

	if (res == NULL) {
		printf("udp_server error for %s, %s\n", host, serv);
        exit(1);
    }

	if (addrlenp) *addrlenp = res->ai_addrlen;

	freeaddrinfo(ressave);

	return(sockfd);
}

int main(int argc, char **argv) {
	int	sockfd;
	ssize_t	n;
	char buff[MAXLINE];
	time_t ticks;
	socklen_t len;
	struct sockaddr_storage	cliaddr;

	if (argc == 2) {
        sockfd = udp_server(NULL, argv[1], NULL);
    } else if (argc == 3) {
    	sockfd = udp_server(argv[1], argv[2], NULL);
    } else {
		printf("usage: sudp [ <host> ] <port>\n");
        exit(1);
    }

	for ( ; ; ) {
		len = sizeof(cliaddr);
		n = Recvfrom(sockfd, buff, MAXLINE, 0, (SA *)&cliaddr, &len);
		printf("datagram from %s\n", Sock_ntop((SA *)&cliaddr, len));

		ticks = time(NULL);
		snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
		Sendto(sockfd, buff, strlen(buff), 0, (SA *)&cliaddr, len);
	}
}
