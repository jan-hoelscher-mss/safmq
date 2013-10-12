#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "uuidgen.h"

struct shmuuid {
	unsigned char	shm_mac[6];
	struct timeval	timestamp;
	unsigned short	clock_seq;
};

static struct sembuf _lockop = { 0, -1, 0};
static struct sembuf _unlockop = { 0, 1, 0};

/* compares two timestamps */
static int tscomp(const struct timeval* lhs, const struct timeval* rhs);
/* retreives the semaphore */
static int getsem(key_t key);
/* locks the semaphore */
static void locksem(int sem);
/* releases the semaphore */
static void releasesem(int sem);
/* retreives the shared memory */
static struct shmuuid* getsharedmem(key_t shmkey);
/* releases the shared memory */
static void releaseshm(struct shmuuid* shared);

static void getmac(unsigned char mac[6]);

static key_t getshmkey();

void uuidgen(struct uuid* puuid)
{
	static key_t	shmkey = -1; 
	//int				shmseg; // TODO: What is this variable for?
	static int		sem = -1;

	if (shmkey == -1)
		shmkey = getshmkey();

	if (sem == -1)
		sem = getsem(shmkey);
	else
		locksem(sem);

	if (sem != -1) {
		static struct shmuuid* shared = (struct shmuuid*)-1;

		if (shared == (struct shmuuid*)-1)
			shared = getsharedmem(shmkey);

		if (shared != (struct shmuuid*)-1) {
			struct timeval	now;
			int				cmp;

			gettimeofday(&now, NULL);
			
			if (tscomp(&(shared->timestamp),&now)) {
				//srand(shared->timestamp.tv_usec);
				srand(shared->clock_seq);
				shared->clock_seq = (unsigned short)rand();
				memcpy(&shared->timestamp,&now,sizeof(now));
			} else
				shared->clock_seq++;

			// takes bottom 12 bits of tv_sec
			puuid->guid.d1 = ((shared->timestamp.tv_sec<<28)&0xF0000000)|((shared->timestamp.tv_usec<<8)&0x0FFFFFFF);
			
			// takes bottom 16 of top 20
			puuid->guid.d2 = (unsigned short)(((shared->timestamp.tv_sec>>4) & 0x00FFFF));
			// takes top 4 of top 16
			puuid->guid.d3 = (unsigned short)(((shared->timestamp.tv_sec>>16) & 0x00FF00) | 0x01);

			memcpy(&(puuid->guid.d4), &(shared->clock_seq), 2);
			puuid->guid.d4[0] = (puuid->guid.d4[0] | 0x80) & 0x9F;
			memcpy((puuid->guid.d4)+2, shared->shm_mac, 6); // copy the mac-address into the guid

			// releaseshm(shared);
		} else
			perror("getting shared memory");
		releasesem(sem);
	} else
		perror("getting semaphore");
}

static key_t getshmkey()
{
	static const char path[] = "/tmp/uuid";
	int fd = open(path, O_CREAT, 0666);
	if (fd > 0) {
		close(fd);
		return ftok(path, 0);
	}
	return ftok("/", 999);
}

/*
Gets and locks the semaphore
*/
static int getsem(key_t key)
{
	int sem = semget(key, 1, IPC_CREAT | IPC_EXCL);
	if (sem == -1 && errno == EEXIST) {
		sem = semget(key, 1, 0);
		if (sem == -1)
			return -1;
		locksem(sem);
		return sem;
	} else if (sem == -1) {
		return -1;
	}
	semctl(sem, 0, SETVAL, (int)0);
	return sem;
}

static void locksem(int sem)
{
	semop(sem, &_lockop, 1);
}

static void releasesem(int sem)
{
	semop(sem, &_unlockop, 1);
}

static struct shmuuid* getsharedmem(key_t shmkey)
{
	int	seg;
	struct shmuuid*	pmem;

	seg = shmget(shmkey, sizeof(struct shmuuid), IPC_CREAT|IPC_EXCL|0666);
	if (seg == -1 && errno == EEXIST) {
		seg = shmget(shmkey, sizeof(struct shmuuid), 0666);
		if (seg == -1) 
			return (struct shmuuid*)-1;
		return (struct shmuuid*)shmat(seg, NULL, 0);
	}
	
	pmem = (struct shmuuid*)shmat(seg, NULL, 0);
	if (pmem != (struct shmuuid*)-1) {
		memset(pmem,0,sizeof(struct shmuuid));
		getmac(pmem->shm_mac);
	}
	return pmem;
}

static void releaseshm(struct shmuuid* shared)
{
	shmdt(shared);
}

#if defined(SAFMQ_POSIX_HWADDR)
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#endif

#ifdef HPUX
#include <netio.h>
#endif

#ifdef AIX
#include <sys/ndd_var.h>
#include <sys/kinfo.h>
#endif

#if defined(FREEBSD_GETIFADDRS)
// Necessary headers for FreeBSD's getifaddrs()
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/ethernet.h>
#endif

static int mac_addr_sys ( u_char *addr)
{
/* implementation for Linux */
#if defined(SAFMQ_POSIX_HWADDR)
    struct ifreq ifr;
    struct ifreq *IFR;
    struct ifconf ifc;
    char buf[1024];
    int s, i;
    int ok = 0;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s==-1) {
        return -1;
    }

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    ioctl(s, SIOCGIFCONF, &ifc);
 
    IFR = ifc.ifc_req;

    for (i = ifc.ifc_len / sizeof(struct ifreq); --i >= 0; IFR++) {
        strcpy(ifr.ifr_name, IFR->ifr_name);
        if (ioctl(s, SIOCGIFFLAGS, &ifr) == 0) {
            if (! (ifr.ifr_flags & IFF_LOOPBACK)) {
                if (ioctl(s, SIOCGIFHWADDR, &ifr) == 0) {
                    ok = 1;
                    break;
                }
            }
        }
    }

    close(s);
    if (ok) {
        bcopy( ifr.ifr_hwaddr.sa_data, addr, 6);
    }
    else {
        return -1;
    }
    return 0;

#elif defined(HPUX)
/* implementation for HP-UX */
#define LAN_DEV0 "/dev/lan0"
    int		fd;
    struct fis	iocnt_block;
    int		i;
    char	net_buf[sizeof(LAN_DEV0)+1];
    char	*p;

    (void)sprintf(net_buf, "%s", LAN_DEV0);
    p = net_buf + strlen(net_buf) - 1;

    /* 
     * Get 802.3 address from card by opening the driver and interrogating it.
     */
    for (i = 0; i < 10; i++, (*p)++) {
        if ((fd = open (net_buf, O_RDONLY)) != -1) {
			iocnt_block.reqtype = LOCAL_ADDRESS;
			ioctl (fd, NETSTAT, &iocnt_block);
			close (fd);

            if (iocnt_block.vtype == 6)
                break;
        }
    }

    if (fd == -1 || iocnt_block.vtype != 6) {
        return -1;
    }

	bcopy( &iocnt_block.value.s[0], addr, 6);
	return 0;

#elif defined(AIX)
/* implementation for AIX */
    int size;
    struct kinfo_ndd *nddp;

    size = getkerninfo(KINFO_NDD, 0, 0, 0);
    if (size <= 0) {
        return -1;
    }
    nddp = (struct kinfo_ndd *)malloc(size);
          
    if (!nddp) {
        return -1;
    }
    if (getkerninfo(KINFO_NDD, nddp, &size, 0) < 0) {
        free(nddp);
        return -1;
    }
    bcopy(nddp->ndd_addr, addr, 6);
    free(nddp);
    return 0;
#elif defined(FREEBSD_GETIFADDRS)
	struct ifaddrs* addrs;
    int err;

    err = getifaddrs(&addrs);
    if (err == 0) {
        struct ifaddrs* cur = addrs;
		err = -1;
        while (cur) {
			if (cur->ifa_addr->sa_family == AF_LINK) {
				struct sockaddr_dl* sdl = (struct sockaddr_dl*)cur->ifa_addr;
				if (sdl->sdl_alen == ETHER_ADDR_LEN &&
					(sdl->sdl_type == IFT_ETHER || sdl->sdl_type == IFT_L2VLAN) ) {
					struct ether_addr* ea = (struct ether_addr*)(sdl->sdl_data + sdl->sdl_nlen);
					bcopy(ea->octet, addr, 6);
					err = 0;
					break;
				}
			}
		}
		freeifaddrs(addrs);
	}
	return err;
#else
#error No Mac Address Locator defined
#endif

/* Not implemented platforms */
	return -1;
}

static void getmac(unsigned char mac[6])
{
	// TODO: Get the mac address
	int iret = mac_addr_sys(mac);
	if (iret)
		memset(mac,1,sizeof(mac));
}

static int tscomp(const struct timeval* lhs, const struct timeval* rhs)
{
	int ret;
	ret = lhs->tv_sec - rhs->tv_sec;
	if (!ret)
		ret = lhs->tv_usec - rhs->tv_usec;
	return ret;
}

static void rmshm(int shmid)
{
	printf("removeing shared memory for shmid:%ld\n", shmid);
	if (shmid != -1) {
		if (shmctl(shmid, IPC_RMID, 0) == -1)
			perror("removing shared memory");
	}
}

#if 0
int main(int argc, char* argv[])
{
	struct uuid Uuid = {0};
	int			x;

	if (argc > 1 && strcmp(argv[1], "clear")==0)
		rmshm(shmget(getshmkey(), sizeof(struct shmuuid), 0));

	while (98751) {
		uuidgen(&Uuid);
		printf("%08lx-%04lx-%04lx-%04lx-%02lx%02lx%02lx%02lx%02lx%02lx\n",
			Uuid.guid.d1,Uuid.guid.d2,Uuid.guid.d3,
			*(unsigned short*)Uuid.guid.d4,Uuid.guid.d4[2],Uuid.guid.d4[3],
			Uuid.guid.d4[4],Uuid.guid.d4[5],Uuid.guid.d4[6],Uuid.guid.d4[7]);
	}

	return 0;
}
#endif

