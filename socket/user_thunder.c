#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>
#include <string.h> 
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>

#include <linux/netlink.h>
#include <linux/genetlink.h>
#include "../user_thunderfs/user_thunderfs.c"

#define GENLMSG_DATA(glh) ((void *)(NLMSG_DATA(glh) + GENL_HDRLEN))
#define GENLMSG_PAYLOAD(glh) (NLMSG_PAYLOAD(glh, 0) - GENL_HDRLEN)
#define NLA_DATA(na) ((void *)((char*)(na) + NLA_HDRLEN))

int done = 0;
int nl_sd; // The Socket

char buff[4096];
int buff_size;

// Create Netlink Socket
static int create_nl_socket(int protocol, int groups)
{
        socklen_t addr_len;
        int fd;
        struct sockaddr_nl local;
        fd = socket(AF_NETLINK, SOCK_RAW, protocol);
        if(fd < 0){
                perror("socket");
                return -1;
        }
        memset(&local, 0, sizeof(local));
        local.nl_family = AF_NETLINK;
        local.nl_groups = groups;
        if(bind(fd, (struct sockaddr *) &local, sizeof(local)) < 0){
                close(fd);
                return -1;
        }
        return fd;
}

// Send netlink message
int sendto_fd(int s, const char *buf, int bufLen){
        struct sockaddr_nl nladdr;
        int r;
        memset(&nladdr, 0, sizeof(nladdr));
        nladdr.nl_family = AF_NETLINK;
        while(( r = sendto(s, buf, bufLen, 0, (struct sockaddr *) &nladdr, 
                                                sizeof(nladdr))) < bufLen){
                if (r > 0){
                        buf += r;
                        bufLen += r;
                }else if(errno != EAGAIN)
                        return -1;
        }
        return 0;
}

int get_family_id(int sd)
{
        struct {
                struct nlmsghdr n;
                struct genlmsghdr g;
                char buf[256];
        } family_req;

        struct {
                struct nlmsghdr n;
                struct genlmsghdr g;
                char buf[256];
        }ans;

        int id;
        struct nlattr *na;
        int rep_len;

        // Get family name
        family_req.n.nlmsg_type = GENL_ID_CTRL;
        family_req.n.nlmsg_flags = NLM_F_REQUEST;
        family_req.n.nlmsg_seq = 0;
        family_req.n.nlmsg_pid = getpid();
        family_req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
        family_req.g.cmd = CTRL_CMD_GETFAMILY;
        family_req.g.version = 0x1;

        na = (struct nlattr *) GENLMSG_DATA(&family_req);
        na->nla_type = CTRL_ATTR_FAMILY_NAME;
        // Change here
        na->nla_len = strlen("THUNDERFS_LINK") + 1 + NLA_HDRLEN;
        strcpy(NLA_DATA(na), "THUNDERFS_LINK");

        family_req.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);
        if(sendto_fd(sd, (char *) &family_req, family_req.n.nlmsg_len) < 0)
                return -1;

        rep_len = recv(sd, &ans, sizeof(ans), 0);
        if(rep_len < 0){
                perror("recv");
                return -1;
        }

        // Validate response msg
        if(!NLMSG_OK((&ans.n), rep_len)){
                fprintf(stderr, "invalid reply msg\n");
                return -1;
        }

        if(ans.n.nlmsg_type == NLMSG_ERROR){
                fprintf(stderr, "receive error\n");
                return -1;
        }

        na = (struct nlattr *) GENLMSG_DATA(&ans);
        na = (struct nlattr *) ((char *) na + NLA_ALIGN(na->nla_len));
        if(na->nla_type == CTRL_ATTR_FAMILY_ID){
                id = *(__u16 *) NLA_DATA(na);
        }
        return id;
}

struct cmd_type {
        struct nlmsghdr n;
        struct genlmsghdr g;
        char buf[256];
};

// A function to initialize cmd_types
int init_cmd(struct cmd_type *this_cmd, int cmd_opt, int id){
        this_cmd->n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
        this_cmd->n.nlmsg_type = id;
        this_cmd->n.nlmsg_flags = NLM_F_REQUEST;
        this_cmd->n.nlmsg_seq = 60;
        this_cmd->n.nlmsg_pid = getpid();
        this_cmd->g.cmd = cmd_opt; // THUNDER_C_OPEN
}

// A Function to send to the thunderfs sockets in kernel space.
int send_to_thunderfs(struct cmd_type *this_cmd, int cmd_type, 
                                        char *message, int mlength){
        struct nlattr *na;
        struct sockaddr_nl nladdr;
        struct cmd_type ans;
        int r;
        int rep_len;
        na = (struct nlattr*) GENLMSG_DATA(this_cmd);
        na->nla_type = cmd_type;
        na->nla_len = mlength+NLA_HDRLEN;
        memcpy(NLA_DATA(na), message, mlength);
        this_cmd->n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

        memset(&nladdr, 0, sizeof(nladdr));
        nladdr.nl_family = AF_NETLINK;

        r = sendto(nl_sd, (char *)this_cmd, this_cmd->n.nlmsg_len, 0,
                (struct sockaddr *)&nladdr, sizeof(nladdr));
        
        return 0;
}

// A Function to send to the thunderfs sockets in kernel space.
ssize_t get_from_thunderfs(struct cmd_type *this_cmd, 
                                        int cmd_type, char *ret_message) {
        struct nlattr *na;
        struct cmd_type ans;
        struct sockaddr_nl nladdr;
        ssize_t rep_len;
        na = (struct nlattr*) GENLMSG_DATA(this_cmd);
        na->nla_type = cmd_type;
        this_cmd->n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

        memset(&nladdr, 0, sizeof(nladdr));
        nladdr.nl_family = AF_NETLINK;

        rep_len = recv(nl_sd, &ans, sizeof(ans), 0);
        if(ans.n.nlmsg_type == NLMSG_ERROR ) {
                printf("Error Received NACK - leaving \n");
                return -1;
        } if ( rep_len < 0 ){
                printf("error received reply msg \n");
                return -1;

        } if ( !NLMSG_OK((&ans.n), rep_len )) {
                printf("invalid reply message received\n");
                return -1;
        }

        rep_len = GENLMSG_PAYLOAD(&ans.n);
        na = (struct nlattr *) GENLMSG_DATA(&ans);
        char * rv = (char *) NLA_DATA(na);
        printf("Kernel Says: %i\t%i\n", (int) rv[0], (int) rv[1]);
        return snprintf(ret_message, rep_len, "%s", rv);
}

#define EQUAL_STRS 0
#define OPEN_CMD 1
#define READ_CMD 2
#define WRITE_CMD 3
#define STATE_CMD 4
int main(){
        nl_sd = create_nl_socket(NETLINK_GENERIC, 0);
        printf("nl_sd: %i\n", nl_sd);
        if(nl_sd < 0){
                printf("Create Failure\n");
                return 0;
        }
        int id = get_family_id(nl_sd);
        struct cmd_type open_cmd;
        struct cmd_type read_cmd;
        struct cmd_type write_cmd;
        struct cmd_type state_cmd;

        struct nlattr *na;

        init_cmd(&open_cmd, OPEN_CMD, id);
        init_cmd(&read_cmd, READ_CMD, id);
        init_cmd(&write_cmd, WRITE_CMD, id);
        init_cmd(&state_cmd, STATE_CMD, id);
        
        char *init = "Initialize";
        int init_len = 12;
        char rv[200];
        char ret_mes[200];
        char filedata[4096];
        unsigned long ret_len;
        ssize_t len;
        char *init_file = "This is My File\nI like this file because it is mine\n";
        int init_fsize = sizeof("This is My File\nI like this file because it is mine\n");
        struct file_system thunder_system;

        char kern_cmd;
        char *endptr;
        memset(&filedata, 0, 4096); // Wipe that shit

        init_filesystem( &thunder_system );
        open_file(&thunder_system, 234680);
        write_file(&thunder_system, init_file, init_fsize, 234680);

        len = get_file(&thunder_system, filedata, 234680);
        printf("File:\n%s", filedata);
        memset(&filedata, 0, len+5);


        // Initialize the State.
        send_to_thunderfs(&state_cmd, STATE_CMD, init, init_len);
        do{
                len = get_from_thunderfs(&state_cmd, STATE_CMD, rv);
                kern_cmd = rv[0];
                printf("Got Message len: %i\n", (int) len);

                if ( (int) kern_cmd == OPEN_CMD){
                        unsigned long file_id;
                        unsigned long file_size;
                        char rv_buff[10];
                        int fsize;

                        file_id = strtoul( &rv[1], &endptr, 16);
                        file_size = open_file(&thunder_system, file_id);

                        printf("File ID: %lu\n", file_id);
                        fsize = sprintf(rv_buff, "%lx\n", file_size); 
                        printf("RV FSize: %lu\n", file_size);
                        
                        send_to_thunderfs(&open_cmd, OPEN_CMD, rv_buff, fsize+2);
                        printf("Confirmed OPEN\n\n");

                } else if( (int) kern_cmd == READ_CMD){
                        unsigned long file_id;
                        int file_len;
                        file_id = strtoul( &rv[1], &endptr, 16);
                        printf("File ID: %lu\n", file_id);

                        file_len = (int) get_file(&thunder_system, filedata, file_id);
                        printf("data:\n%s-\n", filedata);
                        //sleep(5);

                        send_to_thunderfs(&read_cmd, READ_CMD, filedata, file_len+2);
                        memset(&filedata, 0, file_len+5);
                        printf("Confirmed READ\n\n");

                } else if( (int) kern_cmd == WRITE_CMD ){
                        unsigned long file_id;
                        unsigned long file_size;
                        int fsize;
                        ssize_t file_len;
                        char rv_buff[10];

                        file_id = strtoul( &rv[1], &endptr, 16);
                        printf("File ID: %lu\n", file_id);

                        ret_len = sprintf(ret_mes, "%s", endptr);
                        printf("%lu\n", ret_len);
                        file_size = write_file(&thunder_system, 
                                                ret_mes, ret_len, file_id);

                        fsize = sprintf(rv_buff, "%lx", file_size);
                        printf("RV FSize: %lu\n", file_size);
                        
                        send_to_thunderfs(&write_cmd, WRITE_CMD, rv_buff, fsize+2);
                        printf("Confirmed WRITE\n\n");
                } else {
                        printf("Not Confirmed\n\n");
                }
        } while(1 == 1);

        printf("Thats all folks\n");
        return 0;


}
        //printf("value: 0x%lx\n", value);
        //sprintf(test, "%lx",value);
        //new_value = strtoul(test, &endptr, 16);
        //printf("new_value_16: 0x%lx\n", new_value);
        //printf("&test: %lx\n", (unsigned long) test);
        //printf("&test: %lx\n", (unsigned long) &test[4]);
        //printf("*endptr: %lx\n", (unsigned long) endptr);

