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

#define GENLMSG_DATA(glh) ((void *)(NLMSG_DATA(glh) + GENL_HDRLEN))
#define GENLMSG_PAYLOAD(glh) (NLMSG_PAYLOAD(glh, 0) - GENL_HDRLEN)
#define NLA_DATA(na) ((void *)((char*)(na) + NLA_HDRLEN))

int done = 0;
int nl_sd; // The Socket

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
       // return get_from_thunderfs(this_cmd, cmd_type);
}

// A Function to send to the thunderfs sockets in kernel space.
int get_from_thunderfs(struct cmd_type *this_cmd, int cmd_type, char *ret_message /*, 
                                        char *message, int mlength*/){
        struct nlattr *na;
        struct cmd_type ans;
        struct sockaddr_nl nladdr;
        int r;
        int rep_len;
        na = (struct nlattr*) GENLMSG_DATA(this_cmd);
        na->nla_type = cmd_type;
        //na->nla_len = mlength+NLA_HDRLEN;
        //memcpy(NLA_DATA(na), message, mlength);
        this_cmd->n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

        memset(&nladdr, 0, sizeof(nladdr));
        nladdr.nl_family = AF_NETLINK;

        r = recv(nl_sd, &ans, sizeof(ans), 0);
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
        char * result = (char *) NLA_DATA(na);
        printf("Kernel Says: %s\n", result);
        //*ret_message = result;
        return 0;
}

//void user_state_machine(void){
//}

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
        
        //----------------------------------- STATE Command
        char *mes = "Initialize";
        char *rv;
        int meslength = 12;
        printf("New Cmd:\n");
        send_to_thunderfs(&state_cmd, STATE_CMD, mes, meslength);
        get_from_thunderfs(&state_cmd, STATE_CMD, rv);
        //if (rv == "YAYMAN"){
        //        printf( "Confirmed\n");
        //}
        printf("\n\n");

        //----------------------------------- Open Command
        char * message = "Hello World!";
        int mlength = 14;
        printf("Open Cmd:\n");
        send_to_thunderfs(&open_cmd, OPEN_CMD, message, mlength);
        get_from_thunderfs(&open_cmd, OPEN_CMD, rv);
        
        //----------------------------------- Read Command
        //char * jmessage = "Jello World!";
        //int jlength = 14;
        //send_to_thunderfs(&read_cmd, READ_CMD, jmessage, jlength);

        //----------------------------------- Write Command
        //char * pmessage = "Mello Herld!";
        //int plength = 14;
        //send_to_thunderfs(&write_cmd, WRITE_CMD, pmessage, plength);

        printf("Thats all folks\n");
        return 0;


}
