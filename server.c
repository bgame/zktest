#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>

#include "zkdef.h"

static int is_leader( zhandle_t* zkhandle, char *myid) {
    int ret = 0;
    int flag = 1;

    struct String_vector strings;
    ret = zoo_get_children(zkhandle, root_path, 0, &strings);
    if (ret) {
        fprintf(stderr, "Error %d for %s\n", ret, "get_children");
        exit(EXIT_FAILURE);
    }

    int i;
    for ( i=0; i<strings.count; i++ ) {
        if ( strcmp(myid, strings.data[i])>0 ) { /* 如果我自己不是最小的节点 */
            flag = 0;
            break;
        }
    }

    return flag;
}

void election_watcher(zhandle_t * zh, int type, int state,
        const char* path, void* watcherCtx) {
    watch_func_para_t* para= (watch_func_para_t*)watcherCtx;
    printf("something happened:\n");
    printf("type - %d\n", type);
    printf("state - %d\n", state);
    printf("path - %s\n", path);
    printf("data - %s\n", para->node);

    /* 此处貌似不需要重新监听 */
    /*
    struct String_vector strings;  
    struct Stat stat;
    int ret = zoo_wget_children2(zh, root_path, election_watcher, watcherCtx, &strings, &stat);
    if (ret) {
        fprintf(stderr, "error %d of %s\n", ret, "wget children2");
        exit(EXIT_FAILURE);
    }
    int i;
    for ( i=0; i<strings.count; i++ ) {
        printf("--- %s\n", strings.data[i]);
    }
    deallocate_String_vector(&strings);
    */

    /* 判断主从 */
    if (is_leader(para->zkhandle, para->node)) {
        printf("This is [%s], i am a leader [pid=%d]\n", para->node, pid);
    } else {
        printf("This is [%s], i am a follower [pid=%d]\n", para->node, pid);
    }
}

/*
 * 异步回调
 */
void my_string_completion(int rc, const char * name, const void *data) {
    fprintf(stderr, "[%s]: rc - %d\n", (char*)(data==0?"null":data), rc);
    if ( !rc ) {
        fprintf(stderr, "\tname - %s\n", name);
    }
}

void run() {
    printf("Server is running...\n");
}

int main(int argc, const char * argv[]) {
    int timeout = 5000;
    char path_buffer[PATH_BUFFER_LEN] = {0};
    char node[PATH_BUFFER_LEN] = {0};
    char data[PATH_BUFFER_LEN] = {0};
    pid = getpid();

    zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);
    zhandle_t * zkhandle = zookeeper_init(host, service_watcher_g, timeout, 0, "hello zookeeper.", 0);
    if ( zkhandle==NULL ) {
        fprintf(stderr, "error connect to zookeeper service.\n");
        exit(EXIT_FAILURE);
    }

    /*
    int ret = zoo_acreate(zkhandle, "/app/service/league/master", "alive", 5, 
            &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL, my_string_completion, "zoo_acreate");
            */
    sprintf(data, "%s:%d:%d", inet_ntoa(get_local_ip()), port, pid); 

    int ret = zoo_create(zkhandle, node_path, data, strlen(data), 
            &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL|ZOO_SEQUENCE, path_buffer, PATH_BUFFER_LEN);
    if (ret) {
        fprintf(stderr, "error %d of %s\n", ret, "create node");
        exit(EXIT_FAILURE);
    }
    else {
        get_node_name(path_buffer, node);
        printf("path_buffer:%s, node_name:%s\n", path_buffer, node);
        
        /* get data from znode */
        char buff[PATH_BUFFER_LEN] = {0};
        int buff_len = PATH_BUFFER_LEN;
        ret = zoo_get(zkhandle, path_buffer, 0, buff, &buff_len, NULL);
        printf("node data:%s\n", buff);
    }

    watch_func_para_t para;
    memset(&para, 0, sizeof(para));
    para.zkhandle = zkhandle;
    strcpy(para.node, node);


    /* 判断当前是否是Leader节点 */
    if (is_leader(zkhandle, node)) {
        printf("This is [%s], i am a leader [pid=%d]\n", node, pid);
    } else {
        printf("This is [%s], i am a follower [pid=%d]\n", node, pid);
    }

    /* 监视league下的节点变化事件 */
    struct String_vector strings;  
    struct Stat stat;
    ret = zoo_wget_children2(zkhandle, root_path, election_watcher, &para, &strings, &stat);
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
