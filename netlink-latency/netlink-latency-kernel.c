#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>

#define NETLINK_USER 31
struct sock *nl_sk = NULL;

static void on_receiving_data(struct sk_buff *skb)
{
        struct nlmsghdr *nlh;
        int pid, res;
        struct sk_buff *skb_out = NULL;
        char *msg = "ACK";
        int msg_size = strlen(msg);

        printk(KERN_INFO "Entering: %s\n", __FUNCTION__);
        if (unlikely(!skb)) {
                printk(KERN_INFO "skb NULL pointer");
                return;
        }

        nlh = (struct nlmsghdr*)skb->data;
        printk(KERN_INFO "Netlink received msg payload: %s\n", (char*)nlmsg_data(nlh));
        pid = nlh->nlmsg_pid;   /*pid of sending process */

        if (!(skb_out = nlmsg_new(msg_size, 0))) {
                printk(KERN_INFO "Fail to allocate new skb\n");
                return;
        }

        nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);

        NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
        strncpy(nlmsg_data(nlh), msg, msg_size);

        if ((res = nlmsg_unicast(nl_sk, skb_out, pid)) < 0) {
                printk(KERN_INFO "Error while sending back to user\n");
        }
}

static int __init netlink_latency_init(void)
{
        struct netlink_kernel_cfg cfg = {
                .input = on_receiving_data,
        };

        nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
        if (unlikely(!nl_sk)) {
                printk(KERN_INFO "Error creating socket\n");
                return -10;
        }

        printk(KERN_INFO "Install netlink module\n");
        return 0;
}

static void __exit netlink_latency_exit(void)
{
        printk(KERN_INFO "Exit netlink module\n");
        netlink_kernel_release(nl_sk);
}

module_init(netlink_latency_init);
module_exit(netlink_latency_exit);

MODULE_LICENSE("GPL");
