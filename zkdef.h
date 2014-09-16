/*
 * Filename: zkdef.h
 * Description: 
 * Author: - 
 * Created: 2014-09-17 00:30:56
 * Revision: $Id$
 */
 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <zookeeper/zookeeper.h>
#include <zookeeper/zookeeper_log.h>

#define PATH_BUFFER_LEN 64
const char * host = "172.17.139.228:2181,172.17.139.228:2182,172.17.139.228:2183";
const char * root_path = "/app/service/league";
const char * node_path = "/app/service/league/svr";
const unsigned port = 7689;
int pid = 0;

typedef struct _watch_func_para_t {
    zhandle_t *zkhandle;
    char node[PATH_BUFFER_LEN];
} watch_func_para_t;

static struct in_addr get_local_ip() {
    int fd; 
    struct ifreq ifreq;
    struct sockaddr_in* sin;

    fd = socket(PF_INET, SOCK_DGRAM, 0); 
    memset(&ifreq, 0x00, sizeof(ifreq));
    strncpy(ifreq.ifr_name, "eth1", sizeof(ifreq.ifr_name));
    ioctl(fd, SIOCGIFADDR, &ifreq);
    close(fd);
    sin = (struct sockaddr_in* )&ifreq.ifr_addr;

    return sin->sin_addr; /* .s_addr; */
}

/*
 * 提取path中的node name
 * /aaa/bb/ccc -> ccc
 */
static void get_node_name(const char *buf, char *node) {
    const char *p = buf;
    int i;
    for ( i=strlen(buf); i>=0; i-- ) {
        if (*(p + i) == '/') {
            break;
        }
    }

    strcpy(node, p + i + 1);
    return;
}

void service_watcher_g(zhandle_t * zh, int type, int state,
        const char* path, void* watcherCtx) {
    printf("global watcher - type:%d,state:%d\n", type, state);
    if ( type==ZOO_SESSION_EVENT ) {
        if ( state==ZOO_CONNECTED_STATE ) {
            printf("connected to zookeeper service successfully!\n");
            printf("timeout:%d\n", zoo_recv_timeout(zh));
        }
        else if ( state==ZOO_EXPIRED_SESSION_STATE ) {
            printf("zookeeper session expired!\n");
        }
    }
    else {
        printf("other type:%d\n", type);
    }
}
