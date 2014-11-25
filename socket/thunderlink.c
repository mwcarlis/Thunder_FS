#include <net/genetlink.h>
#include <linux/kernel.h>
#include <linux/module.h>

// Annoying depends
int thunder_open(struct sk_buff *skb_2, struct genl_info *info);

// attributes (variables): 
enum {
        THUNDER_A_UNSPEC,
        THUNDER_A_OPEN,
                __THUNDER_A_MAX,
};
#define THUNDER_A_MAX (__THUNDER_A_MAX - 1) 

// Attribute policy.
static struct nla_policy thunder_genl_policy[THUNDER_A_MAX + 1] = {
        [THUNDER_A_OPEN] = {.type = NLA_NUL_STRING},
};

// Commands: used by userspace applications
enum {
        THUNDER_C_UNSPEC,
        THUNDER_C_OPEN,
                __THUNDER_C_MAX,
};
#define THUNDER_C_MAX (__THUNDER_C_MAX - 1)

struct genl_ops thunder_gnl_ops_open = {
        .cmd = THUNDER_C_OPEN,
        .flags = 0,
        .policy = thunder_genl_policy,
        .doit = thunder_open,
        .dumpit = NULL,
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

int thunder_open(struct sk_buff *skb_2, struct genl_info *info){
        //struct nlattr *na;
        //struct sk_buff *skb;
        //int rc;
        struct nlattr *na;
        char *mydata;
        if (info == NULL){
                printk(KERN_INFO "Got a Null gen_info *info\n");
                return 0;
        }

        na = info->attrs[THUNDER_A_OPEN];
        if(na){
                mydata = (char *)nla_data(na);
                if(mydata == NULL){
                        printk(KERN_INFO "Error whiel receiving data\n");

                }else{
                        printk(KERN_INFO "Received: %s\n", mydata);
                }
        }else{
                printk(KERN_INFO "Error no info->attrs %i\n", THUNDER_A_OPEN);
        }
        printk(KERN_INFO "Thunder_Open Kernal To User\n");
        return 0;
}



int __init init_mod(void) // Required to insmod
{
        int rc;
        struct genl_family *init = &thunder_gnl_family;
        struct genl_ops *init_ops = &thunder_gnl_ops_open;
        init->ops = init_ops;
        init->n_ops = 1;

        rc = genl_register_family(init);
        if(rc != 0){
                printk(KERN_INFO "Register Ops: %i\n", rc);
                genl_unregister_family(init);
                printk(KERN_INFO "An Error Occured with insmod\n");
                return -1;
        }
        printk(KERN_INFO "INIT GENERIC NETLINK MODULE\n");
        return 0;
}

void clean_mod(void)
{
        int ret;
        ret = genl_unregister_family(&thunder_gnl_family);
        if(ret != 0){
                printk(KERN_INFO "Unregister Family: %i\n", ret);

        }
        printk(KERN_INFO "Removing NETLINK Module\n");
}

module_init(init_mod);
module_exit(clean_mod);
MODULE_LICENSE("GPL");
