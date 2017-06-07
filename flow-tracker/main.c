#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <net/tcp.h>
#include <linux/netfilter_ipv4.h>

#include "flow_table.h"

/* param_dev: NIC to operate XPath */
char *param_dev = NULL;
MODULE_PARM_DESC(param_dev, "Interface to operate (NULL=all)");
module_param(param_dev, charp, 0);

int param_port __read_mostly = 5001;
MODULE_PARM_DESC(param_port, "TCP port to match (0=all)");
module_param(param_port, int, 0);

struct flow_table table;

/* hook function for outgoing packets */
static unsigned int hook_func_out(const struct nf_hook_ops *ops,
                                  struct sk_buff *skb,
                                  const struct net_device *in,
                                  const struct net_device *out,
                                  int (*okfn)(struct sk_buff *))
{
        struct iphdr *iph = ip_hdr(skb);
        struct tcphdr *tcph = NULL;
        struct flow_entry f, *entry = NULL;

        if (likely(out) && param_dev && strncmp(out->name, param_dev, IFNAMSIZ))
                return NF_ACCEPT;

        if (unlikely(!iph || iph->protocol != IPPROTO_TCP))
                return NF_ACCEPT;

        tcph = tcp_hdr(skb);
        if (param_port &&
            ntohs(tcph->source) != param_port &&
            ntohs(tcph->dest) != param_port)
                return NF_ACCEPT;

        f.local_ip = iph->saddr;
        f.remote_ip = iph->daddr;
        f.local_port = tcph->source;
        f.remote_port = tcph->dest;
        f.bytes_sent = skb->len;

        if (tcph->syn && insert_table(&table, &f)) {
                printk(KERN_INFO "Insert a flow entry (%pI4:%hu to %pI4:%hu)\n",
                                 &(f.local_ip),
                                 ntohs(f.local_port),
                                 &(f.remote_ip),
                                 ntohs(f.remote_port));
                return NF_ACCEPT;
        }

        if ((tcph->fin || tcph->rst) && delete_table(&table, &f)) {
                printk(KERN_INFO "Delete a flow entry (%pI4:%hu to %pI4:%hu)\n",
                                 &(f.local_ip),
                                 ntohs(f.local_port),
                                 &(f.remote_ip),
                                 ntohs(f.remote_port));
                return NF_ACCEPT;
        }

        if (!(entry = search_table(&table, &f))) {
                printk(KERN_INFO "No flow entry (%pI4:%hu to %pI4:%hu)\n",
                                 &(f.local_ip),
                                 ntohs(f.local_port),
                                 &(f.remote_ip),
                                 ntohs(f.remote_port));
                return NF_ACCEPT;
        }

        /* update per-flow state */
        entry->bytes_sent += skb->len;

        return NF_ACCEPT;
}

/* hook function for incoming packets */
static unsigned int hook_func_in(const struct nf_hook_ops *ops,
                                 struct sk_buff *skb,
                                 const struct net_device *in,
                                 const struct net_device *out,
                                 int (*okfn)(struct sk_buff *))
{
        return NF_ACCEPT;
}

/* Netfilter hook for outgoing packets */
static struct nf_hook_ops nf_hook_out = {
        .hook = hook_func_out,
        .hooknum = NF_INET_POST_ROUTING,
        .pf = PF_INET,
        .priority = NF_IP_PRI_FIRST,
};
/* Netfilter hook for incoming packets */
static struct nf_hook_ops nf_hook_in = {
        .hook = hook_func_in,
        .hooknum = NF_INET_PRE_ROUTING,
        .pf = PF_INET,
        .priority = NF_IP_PRI_FIRST,
};

static int flow_tracker_init(void)
{
        int i = 0;

        /* get interface */
        if (param_dev) {
                /* trim */
                for (i = 0; i < 32 && param_dev[i] != '\0'; i++) {
                        if(param_dev[i] == '\n') {
                                param_dev[i] = '\0';
                                break;
                        }
                }
        }

        if (unlikely(!init_table(&table, 8))) {
                printk(KERN_INFO "FlowTracker: cannot init flow hashtable\n");
                return -1;
        }

        if (unlikely(nf_register_hook(&nf_hook_out))) {
                printk(KERN_INFO "FlowTracker: cannot register Netfilter hook at NF_INET_POST_ROUTING\n");
                return -1;
        }

        if (unlikely(nf_register_hook(&nf_hook_in))) {
                printk(KERN_INFO "FlowTracker: cannot register Netfilter hook at NF_INET_PRE_ROUTING\n");
                return -1;
        }

        printk(KERN_INFO "FlowTracker: Start on %s (TCP port %d)\n",
                         param_dev ? param_dev : "any interface", param_port);

        return 0;
}

static void flow_tracker_exit(void)
{
        nf_unregister_hook(&nf_hook_out);
        nf_unregister_hook(&nf_hook_in);
        free_table(&table);
        printk(KERN_INFO "FlowTracker: Stop on %s (TCP port %d)\n",
                         param_dev ? param_dev : "any interface", param_port);
}

module_init(flow_tracker_init);
module_exit(flow_tracker_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wei Bai baiwei0427@gmail.com");
MODULE_VERSION("0.1");
MODULE_DESCRIPTION("Track Per-Flow State");
