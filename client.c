#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>

#include "zkdef.h"

char svr_ip[PATH_BUFFER_LEN] = {0};
unsigned svr_port = 0;
unsigned svr_pid = 0;

static void get_leader(zhandle_t * zkhandle, struct String_vector strings ) {
    int i;
    int myid = 0;
    for ( i=1; i<strings.count; i++ ) {
        if ( strcmp(strings.data[myid], strings.data[i])>0 ) {
            myid = i;
        }
    }
    char leader_path[PATH_BUFFER_LEN] = {0};
    sprintf(leader_path, "%s/%s", root_path, strings.data[myid]);
    /* get data from znode */
    char buff[PATH_BUFFER_LEN] = {0};
    int buff_len = PATH_BUFFER_LEN;
    int ret = zoo_get(zkhandle, leader_path, 0, buff, &buff_len, NULL);
    if (ret) {
        fprintf(stderr, "error %d when get leader from %s\n", ret, strings.data[myid]);
        exit(EXIT_FAILURE);
    }

    printf("master node data:%s\n", buff);
    strcpy(svr_ip, buff);
}

void election_watcher(zhandle_t * zh, int type, int state,
        const char* path, void* watcherCtx) {
    watch_func_para_t* para= (watch_func_para_t*)watcherCtx;
    printf("something happened:\n");
    printf("type - %d\n", type);
    printf("state - %d\n", state);
    printf("path - %s\n", path);
    printf("data - %s\n", para->node);

    /* 此处貌似不需要重新监听*/
    struct String_vector strings;  
    struct Stat stat;
    int ret = zoo_wget_children2(zh, root_path, election_watcher, watcherCtx, &strings, &stat);
    if (ret) {
        fprintf(stderr, "error %d of %s\n", ret, "wget children2");
        exit(EXIT_FAILURE);
    }
    get_leader(zh, strings);

    deallocate_String_vector(&strings);
}

void run() {
    printf("client is running on server[%s]\n", svr_ip);
}

int main(int argc, const char * argv[]) {
    int timeout = 5000;

    zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);
    zhandle_t * zkhandle = zookeeper_init(host, service_watcher_g, timeout, 0, "hello zookeeper from client.", 0);
    if ( zkhandle==NULL ) {
        fprintf(stderr, "error connect to zookeeper service.\n");
        exit(EXIT_FAILURE);
    }

    struct String_vector strings;
    int ret = zoo_get_children(zkhandle, root_path, 0, &strings);
    if (ret) {
        fprintf(stderr, "Error %d for %s\n", ret, "get_children");
        exit(EXIT_FAILURE);
    }

    /* 找到最小的节点,当作leader,并监视之 */
    if ( strings.count<=0 ) {
        fprintf(stderr, "can not found node_path!\n");
        exit(EXIT_FAILURE);
    }

    get_leader(zkhandle, strings);

    /* 监视league下的节点变化事件 */
    struct Stat stat;
    ret = zoo_wget_children2(zkhandle, root_path, election_watcher, "para", &strings, &stat);
    if (ret) {
        fprintf(stderr, "error %d of %s\n", ret, "wget children2");
        exit(EXIT_FAILURE);
    }

    /*
     * main looper
     * */
    int total = 20;
    while (total-- > 0) {
        run();
        sleep(1);
    }

    zookeeper_close(zkhandle);

    return 0;
}
