#pragma once

#if defined(_WIN32)
# pragma comment(lib, "Ws2_32.lib")
# include <WinSock2.h>

//ASSUMPTION:!! On an intel little endian machine

# define htobe16(x) htons(x)
# define htole16(x) (x)
# define be16toh(x) ntohs(x)
# define le16toh(x) (x)

# define htobe32(x) htonl(x)
# define htole32(x) (x)
# define be32toh(x) ntohl(x)
# define le32toh(x) (x)

# define htobe64(x) htonll(x)
# define htole64(x) (x)
# define be64toh(x) ntohll(x)
# define le64toh(x) (x)
#else
# include <endian.h>
#endif