#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/init.h>
#include <linux/types.h>
#include <linux/netfilter.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/netdevice.h>
#include <linux/netfilter_ipv4.h>

//Outgoing packets POSTROUTING
static struct nf_hook_ops nfho_outgoing;
//Incoming packets PREROUTING
static struct nf_hook_ops nfho_incoming;

//POSTROUTING for outgoing packets
static unsigned int hook_func_out(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff *))
{
	struct iphdr  *old_iph = ip_hdr(skb);
	struct iphdr  *iph;	//our new IP header 
	unsigned int max_headroom;	//the extra header space needed 
	unsigned int len_to_expand;	
	unsigned short original_tot_len;	//total length of original IP packet

	//We don't listen on eth0
	if (strncmp(out->name, "eth0", IFNAMSIZ)  !=  0)
	{
		//We only deal with ICMP packets
		if (old_iph && old_iph->protocol == IPPROTO_ICMP)
		{
			original_tot_len = ntohs(old_iph->tot_len);
			max_headroom = sizeof(struct iphdr) + LL_RESERVED_SPACE(out);

			if (skb_headroom(skb) < max_headroom)
			{
				len_to_expand = max_headroom - skb_headroom(skb);
				//Expand reallocate headroom for sk_buff
				if (pskb_expand_head(skb, len_to_expand,  0,  GFP_ATOMIC))
				{
					printk(KERN_INFO "Unable to expand sk_buff\n");
					return NF_DROP;
				}
				printk(KERN_INFO "Expand a packet by %u bytes\n", len_to_expand);
			}

			/*
			 *	Push down and install the IPIP header.
			 */		
			skb_push(skb, sizeof(struct iphdr));
			skb_reset_network_header(skb);
				
			//I am not sure whether skb_make_writable is necessary 
			if (!skb_make_writable(skb, sizeof(struct iphdr)))
			{
				printk(KERN_INFO "Not writable\n");
				return NF_DROP;
			}

			iph = (struct iphdr *)skb_network_header(skb);
			iph->version = 4;
			iph->ihl = sizeof(struct iphdr) >> 2;
			iph->tot_len =  htons(original_tot_len + sizeof(struct iphdr));
			iph->id = old_iph->id;
			iph->frag_off = old_iph->frag_off;
			iph->protocol = IPPROTO_IPIP;
			iph->tos = old_iph->tos;
			iph->daddr = old_iph->daddr;
			iph->saddr = old_iph->saddr;
			iph->ttl = old_iph->ttl;
			iph->check = 0;
			iph->check = ip_fast_csum(iph, iph->ihl);
			//printk(KERN_INFO "Encap a packet\n");
		}
	}

	return NF_ACCEPT;
}

//PREROUTING for incoming packets
static unsigned int hook_func_in(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff *))
{
	struct iphdr  *iph = ip_hdr(skb);
	
	//We don't listen on eth0
	if (strncmp(in->name, "eth0", IFNAMSIZ)  !=  0)
	{
		if (iph && iph->protocol == IPPROTO_IPIP)
		{
			skb_pull(skb, iph->ihl*4);
			skb_reset_network_header(skb);
			skb->transport_header = skb->network_header + iph->ihl*4;
			//printk(KERN_INFO "Decap a packet\n");
		}
	}
	
	return NF_ACCEPT;
}

int ipip_init(void)
{
	//POSTROUTING
	nfho_outgoing.hook = hook_func_out;	//function to call when conditions below met
	nfho_outgoing.hooknum = NF_INET_POST_ROUTING;	//called in post_routing
	nfho_outgoing.pf = PF_INET;	//IPV4 packets
	nfho_outgoing.priority = NF_IP_PRI_FIRST;	//set to highest priority over all other hook functions
	nf_register_hook(&nfho_outgoing);	//register hook
	
	//PREROUTING
	nfho_incoming.hook = hook_func_in;                    //function to call when conditions below met
	nfho_incoming.hooknum = NF_INET_PRE_ROUTING;          //called in pre_routing
	nfho_incoming.pf = PF_INET;                           //IPV4 packets
	nfho_incoming.priority = NF_IP_PRI_FIRST;             //set to highest priority over all other hook functions
	nf_register_hook(&nfho_incoming);                     //register hook
	
	printk(KERN_INFO "Register Netfilter hooks\n");
	return 0;
}

void ipip_exit(void)
{
	nf_unregister_hook(&nfho_outgoing); 
	nf_unregister_hook(&nfho_incoming); 

	printk(KERN_INFO "Unregister Netfilter hooks\n");
}

module_init(ipip_init);
module_exit(ipip_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("BAI Wei baiwei0427@gmail.com");
MODULE_VERSION("0.1");
MODULE_DESCRIPTION("Kernel module of IP in IP");
