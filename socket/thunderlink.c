#include <net/genetlink.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>

// Annoying depends
int thunder_open_socket(struct sk_buff *skb_2, struct genl_info *info);
int thunder_write_socket(struct sk_buff *skb_2, struct genl_info *info);
int thunder_read_socket(struct sk_buff *skb_2, struct genl_info *info);
int thunder_state_cmd(struct sk_buff *skb_2, struct genl_info *info);

struct genl_info open_info;
struct genl_info write_info;
struct genl_info read_info;
struct genl_info state_info;



// attributes (variables): 
enum {
        THUNDER_A_UNSPEC,
        THUNDER_A_OPEN,
        THUNDER_A_READ,
        THUNDER_A_WRITE,
        THUNDER_A_MAIN,
                __THUNDER_A_MAX,
};
#define THUNDER_A_MAX (__THUNDER_A_MAX - 1) 

// Attribute policy.
static struct nla_policy thunder_genl_policy[THUNDER_A_MAX + 1] = {
        [THUNDER_A_OPEN] = {.type = NLA_NUL_STRING},
        [THUNDER_A_READ] = {.type = NLA_NUL_STRING},
        [THUNDER_A_WRITE] = {.type = NLA_NUL_STRING},
        [THUNDER_A_MAIN] = {.type = NLA_NUL_STRING},
};

// Commands: used by userspace applications
enum {
        THUNDER_C_UNSPEC,
        THUNDER_C_OPEN,
        THUNDER_C_READ,
        THUNDER_C_WRITE,
        THUNDER_C_MAIN,
                __THUNDER_C_MAX,
};
#define THUNDER_C_MAX (__THUNDER_C_MAX - 1)

struct genl_ops thunder_gnl_ops[] = {
        {       .cmd = THUNDER_C_OPEN,
                .flags = 0,
                .policy = thunder_genl_policy,
                .doit = thunder_open_socket,
                .dumpit = NULL,
        }, {
                .cmd = THUNDER_C_READ, 
                .flags = 0, 
                .policy = thunder_genl_policy, 
                .doit = thunder_read_socket, 
                .dumpit = NULL, 
        }, {
                .cmd = THUNDER_C_WRITE, 
                .flags = 0, 
                .policy = thunder_genl_policy, 
                .doit = thunder_write_socket, 
                .dumpit = NULL, 
        }, {
                .cmd = THUNDER_C_MAIN, 
                .flags = 0, 
                .policy = thunder_genl_policy, 
                .doit = thunder_state_cmd, 
                .dumpit = NULL, 
        }
};

#define VERSION_NR 1
// Family Definition
static struct genl_family thunder_gnl_family = {
        .id = GENL_ID_GENERATE,         // Let netlink generate the ID
        .hdrsize = 0,
        .name = "THUNDERFS_LINK",       // name this family
        .version = VERSION_NR,          // Version #
        .maxattr = THUNDER_A_MAX,       // 
};


void check_received(struct nlattr * na, int cmd){
        char *mydata;
        if(na){
                mydata = (char *)nla_data(na);
                if(mydata == NULL){
                        printk(KERN_INFO "Error whiel receiving data\n");

                }else{
                        printk(KERN_INFO "Received: %s\n", mydata);
                }
        }else{
                printk(KERN_INFO "Error no info->attrs %i\n", cmd);
        }
}



int thunder_send_to_user(struct genl_info *cmd_info, int A_CMD, int C_CMD, const char *str){
        struct sk_buff *skb;
        void *msg_head;
        int rc;

        skb = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
        if(skb == NULL){
                return -1;
        }
        msg_head = genlmsg_put(skb, 0, cmd_info->snd_seq+1,
                        &thunder_gnl_family, 0, (u8) C_CMD);

        //msg_head = genlmsg_put(skb, 0, open_info.snd_seq+1,
        //                &thunder_gnl_family, 0, THUNDER_C_OPEN);

        if(msg_head == NULL){
                rc = -ENOMEM;
                printk(KERN_INFO "Error Occured after genlmsg_put\n");
                return -1;
        }

        //rc = nla_put_string(skb, THUNDER_A_OPEN, "Hello From Kernel");
        //rc = nla_put_string(skb, A_CMD, "Hello From Kernel");
        rc = nla_put_string(skb, A_CMD, str);

        if(rc != 0){
                printk(KERN_INFO "Error Occured after nla_put_string\n");
                return -1;
        }
        genlmsg_end(skb, msg_head);


        rc = genlmsg_unicast(genl_info_net(cmd_info), skb, cmd_info->snd_portid);
        //rc = genlmsg_unicast(genl_info_net(&open_info), skb, open_info.snd_portid);


        if( rc != 0){
                printk(KERN_INFO "Error Occured after genlmsg_unicast\n");
                return -1;
        }
        printk(KERN_INFO "Done Sending to user space\n");
        return 0;
}

int thunder_open_socket(struct sk_buff *skb_2, struct genl_info *info){
        struct nlattr *na;
        //struct sk_buff *skb;
        int open_cmd = THUNDER_A_OPEN;
        if (info == NULL){
                printk(KERN_INFO "Got a Null gen_info *info\n");
                return 0;
        }
        open_info.snd_seq = info->snd_seq;
        open_info.snd_portid = info->snd_portid;
        open_info.nlhdr = info->nlhdr;
        open_info.genlhdr = info->genlhdr;
        open_info._net = info->_net;

        if (info == NULL){
                printk(KERN_INFO "Got a Null gen_info *info\n");
                return 0;
        }
        na = info->attrs[open_cmd];

        // Test Function
        check_received(na, open_cmd);
        //thunder_send_to_user(&open_info, THUNDER_A_OPEN, THUNDER_C_OPEN, "Hello From Kernel Open");
        printk(KERN_INFO "Thunder_Open Kernal To User\n");
        return 0;
}

int thunder_write_socket(struct sk_buff *skb_2, struct genl_info *info){
        struct nlattr *na;
        int open_cmd = THUNDER_A_WRITE;
        if (info == NULL){
                printk(KERN_INFO "Got a Null gen_info *info\n");
                return 0;
        }
        write_info.snd_seq = info->snd_seq;
        write_info.snd_portid = info->snd_portid;
        write_info.nlhdr = info->nlhdr;
        write_info.genlhdr = info->genlhdr;
        write_info._net = info->_net;

        na = info->attrs[open_cmd];

        // Test Function
        check_received(na, open_cmd);

        printk(KERN_INFO "Thunder_Write Kernal To User\n");
        return 0;
}

int thunder_read_socket(struct sk_buff *skb_2, struct genl_info *info){
        struct nlattr *na;
        int open_cmd = THUNDER_A_READ;
        if (info == NULL){
                printk(KERN_INFO "Got a Null gen_info *info\n");
                return 0;
        }
        read_info.snd_seq = info->snd_seq;
        read_info.snd_portid = info->snd_portid;
        read_info.nlhdr = info->nlhdr;
        read_info.genlhdr = info->genlhdr;
        read_info._net = info->_net;

        na = info->attrs[open_cmd];

        // Test Function
        check_received(na, open_cmd);

        printk(KERN_INFO "Thunder_Read Kernal To User\n");
        return 0;
}

int thunder_state_cmd(struct sk_buff *skb_2, struct genl_info *info){
        struct nlattr *na;
        int open_cmd = THUNDER_A_MAIN;
        if (info == NULL){
                printk(KERN_INFO "Got a Null gen_info *info\n");
                return 0;
        }
        state_info.snd_seq = info->snd_seq;
        state_info.snd_portid = info->snd_portid;
        state_info.nlhdr = info->nlhdr;
        state_info.genlhdr = info->genlhdr;
        state_info._net = info->_net;

        na = info->attrs[open_cmd];

        check_received(na, open_cmd);

        //thunder_send_to_user(&state_info, THUNDER_A_MAIN, THUNDER_C_MAIN, "Hello From Kernel State Cmd");
        printk(KERN_INFO "Thunder State Cmd\n");
        return 0;
}

/*
struct genl_info open_info;
struct genl_info write_info;
struct genl_info read_info;
struct genl_info state_info; */

#define DISPATCH_OPEN 1
#define DISPATCH_READ 2
#define DISPATCH_WRITE 3

void thunder_cmd_dispatch(int cmd, unsigned long operation){
        int CMD_A = THUNDER_A_MAIN;
        int CMD_C = THUNDER_C_MAIN;
        char user_cmds[2];
        printk(KERN_INFO "CMD: %i\n", cmd);
        if (cmd == DISPATCH_OPEN){
                printk(KERN_INFO "Open Case\n");
                user_cmds[0] = (char) DISPATCH_OPEN;
                user_cmds[1] = (char) operation;
                thunder_send_to_user(&state_info, CMD_A, CMD_C, user_cmds); 
        } else if(cmd == DISPATCH_READ) { 
                printk(KERN_INFO "Read Case\n");
                user_cmds[0] = (char) DISPATCH_READ;
                user_cmds[1] = (char) operation;
                thunder_send_to_user(&state_info, CMD_A, CMD_C, user_cmds);
        } else if(cmd == DISPATCH_WRITE) {
                printk(KERN_INFO "Write Case\n");
                user_cmds[0] = (char) DISPATCH_WRITE;
                user_cmds[1] = (char) operation;
                thunder_send_to_user(&state_info, CMD_A, CMD_C, user_cmds);
        } else {
                //thunder_send_to_user();
                printk(KERN_INFO "Default Choice\n");
        }

}



//int __init init_mod(void) // Required to insmod
//{
//        int rc;
//        struct genl_family *init = &thunder_gnl_family;
//        struct genl_ops *init_ops = thunder_gnl_ops;
//        init->ops = init_ops;
//        init->n_ops = 4;
//        rc = genl_register_family(init);
//
//
//        //genl_register_family_with_ops_grups(&thunder_gnl_family,
//                                //&thunder_gnl_ops, NULL);
//
//        if(rc != 0){
//                printk(KERN_INFO "Register Ops: %i\n", rc);
//                genl_unregister_family(init);
//                printk(KERN_INFO "An Error Occured with insmod\n");
//                return -1;
//        }
//        printk(KERN_INFO "INIT GENERIC NETLINK MODULE\n");
//        return 0;
//}
//
//void clean_mod(void)
//{
//        int ret;
//        ret = genl_unregister_family(&thunder_gnl_family);
//        if(ret != 0){
//                printk(KERN_INFO "Unregister Family: %i\n", ret);
//
//        }
//        printk(KERN_INFO "Removing NETLINK Module\n");
//}
//
//module_init(init_mod);
//module_exit(clean_mod);
//MODULE_LICENSE("GPL");
