#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/netfilter.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/ip_tunnels.h>
#include <linux/netdevice.h>
#include <linux/netfilter_ipv4.h>

#define DEBUG 0

//Print sk_buff information
static void print_skb(struct sk_buff *skb);
//Reduce TCP MSS
static void reduce_tcp_mss(struct sk_buff *skb, unsigned short int reduce_size);
//Outgoing packets POSTROUTING
static struct nf_hook_ops nfho_outgoing;
//Incoming packets PREROUTING
static struct nf_hook_ops nfho_incoming;

//Print sk_buff information
static void print_skb(struct sk_buff *skb)
{
	if (unlikely(!skb))
		return;

	printk(KERN_INFO "skb->len: %u skb_is_gso(skb): %d\n", skb->len, skb_is_gso(skb));
}

//Reduce TCP MSS
static void reduce_tcp_mss(struct sk_buff *skb, unsigned short int reduce_size)
{
	struct iphdr *iph;
	struct tcphdr *tcph;
	unsigned int tcp_len;	//TCP packet length
	unsigned int tcph_len;	//TCP header length
	unsigned char *ptr = NULL;
	unsigned short int mss;
	unsigned int offset;

	//printk(KERN_INFO "reduce_tcp_mss\n");
	if (unlikely(!skb))
		return;

	iph = ip_hdr(skb);
	tcph = tcp_hdr(skb);
	if (unlikely(!iph || !tcph))
		return;

	//Not a TCP SYN packet
	if (unlikely(!(tcph->syn)))
		return;

	//If we can not modify this packet
	if (skb_linearize(skb)!= 0)
		return;

	tcph_len = (unsigned int)(tcph->doff<<2);
	ptr = (unsigned char*)tcph + sizeof(struct tcphdr);

	while (1)
	{
		//TCP option kind: MSS (2)
		if (*ptr == 2)
		{
			mss = ntohs(*((unsigned short int*)(ptr + 2)));
			mss = mss - reduce_size;
			*(unsigned short int*)(ptr + 2) = htons(mss);
			//printk("MSS: %u\n", mss);
			break;
		}

		//TCP option kind: No-Operation (1)
		if (*ptr == 1)
			offset = 1;
		//Other TCP options
		else
			//Get length of this TCP option
			offset = (unsigned int)*(ptr + 1);

		if (ptr - (unsigned char *)tcph + offset >= tcph_len)
			break;
		else
			ptr += offset;
	}

	tcp_len = skb->len - (iph->ihl<<2);
	tcph->check=0;
	tcph->check = csum_tcpudp_magic(iph->saddr, iph->daddr,
							  tcp_len, iph->protocol,
							  csum_partial((char *)tcph, tcp_len, 0));

	skb->ip_summed = CHECKSUM_UNNECESSARY;
}
//POSTROUTING for outgoing packets
static unsigned int hook_func_out(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff *))
{
	struct iphdr  *old_iph = ip_hdr(skb);
	struct iphdr  *iph;	//our new IP header
	struct tcphdr *tcph;
	unsigned int max_headroom;	//the extra header space needed
	unsigned int len_to_expand;
	unsigned short original_tot_len;	//total length of original IP packet

	//We only listen on eth1
	if (strncmp(out->name, "eth1", IFNAMSIZ)  ==  0)
	{
		//We only deal with TCP packets
		if (old_iph && old_iph->protocol == IPPROTO_TCP)
		{
			tcph = tcp_hdr(skb);
			//We only filter TCP port 5001
			if (ntohs(tcph->source) != 5001 && ntohs(tcph->dest) != 5001)
				return NF_ACCEPT;

			//Modify TCP MSS
			if (tcph->syn)
				reduce_tcp_mss(skb, sizeof(struct iphdr));

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
				//if (DEBUG)
					//printk(KERN_INFO "Expand a packet by %u bytes\n", len_to_expand);
			}

			skb = iptunnel_handle_offloads(skb, false, SKB_GSO_IPIP);
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

			if (DEBUG)
				print_skb(skb);
		}
	}

	return NF_ACCEPT;
}

//PREROUTING for incoming packets
static unsigned int hook_func_in(unsigned int hooknum, struct sk_buff *skb, const struct net_device *in, const struct net_device *out, int (*okfn)(struct sk_buff *))
{
	struct iphdr  *iph = ip_hdr(skb);

	//We only listen on eth1
	if (strncmp(in->name, "eth1", IFNAMSIZ)  ==  0)
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
