#include "flow_table.h"
#include "crc_table.h"

/* Lookup table based solution */
static inline u16 crc16(u8 *data, unsigned int len)
{
        u16 crc = 0;
        unsigned int i;
        u8 pos;

        if (unlikely(!data))
                goto out;

        for (i = 0; i < len; i++) {
                /* XOR-in next input byte into MSB of crc, that's our new intermediate divident */
                pos = (u8)(crc >> 8) ^ data[i];
                /* Shift out the MSB used for division per lookup table and XOR with the remainder */
                crc = (crc << 8) ^ CRC_HASH_TABLE[(int)pos];
        }

out:
        return crc;
}

static inline u16 flow_hash(struct flow_entry *flow)
{
        u16 tuples[6] = {flow->local_ip >> 16, flow->local_ip & 0xffff,
                         flow->remote_ip >> 16, flow->remote_ip & 0xffff,
                         flow->local_port, flow->remote_port};

        return crc16((u8*)tuples, 12);
}

bool init_table(struct flow_table *table, u8 bits)
{
        unsigned int i, num_lists;

        if (unlikely(!table))
                return false;

        if (bits > 16) {
                printk(KERN_INFO "bits should be NO LARGER THAN 16\n");
                return false;
        }

        table->lists = vmalloc(sizeof(struct hlist_head) << bits);
        if (unlikely(!(table->lists))) {
                printk(KERN_INFO "Vmalloc Error\n");
                return false;
        }


        table->bits = bits;
        atomic_set(&(table->num_flows), 0);
        spin_lock_init(&(table->lock));
        num_lists = 1 << bits;

        for (i = 0; i < num_lists; i++)
                INIT_HLIST_HEAD(&(table->lists[i]));

        return true;
}

void free_table(struct flow_table *table)
{
        struct flow_entry *obj = NULL;
        struct hlist_node *tmp = NULL;
        unsigned int i, num_lists = 1 << table->bits;

        for (i = 0; i < num_lists; i++) {
                hlist_for_each_entry_safe(obj, tmp, &(table->lists[i]), hlist) {
                        hlist_del(&(obj->hlist));
                        kfree(obj);
                }
        }

        vfree(table->lists);
}

bool insert_table(struct flow_table *table, struct flow_entry *flow)
{
        u16 index = flow_hash(flow) >> (16 - table->bits);
        struct flow_entry *obj = NULL;
        struct hlist_node *tmp = NULL;
        unsigned long flags;

        if (unlikely(!table || !flow || !(table->lists)))
                return false;

        spin_lock_irqsave(&(table->lock), flags);
        hlist_for_each_entry_safe(obj, tmp, &(table->lists[index]), hlist) {
                /* if the flow entry already exists */
                if (obj->local_ip == flow->local_ip &&
                    obj->remote_ip == flow->remote_ip &&
                    obj->local_port == flow->local_port &&
                    obj->remote_port == flow->remote_port) {
                            printk(KERN_INFO "The flow entry already exists\n");
                            spin_unlock_irqrestore(&(table->lock), flags);
                            return false;
                }
        }
        spin_unlock_irqrestore(&(table->lock), flags);

        if (unlikely(!(obj = kmalloc(sizeof(struct flow_entry), GFP_ATOMIC))))
                return false;

        *obj = *flow;
        INIT_HLIST_NODE(&(obj->hlist));
        spin_lock_irqsave(&(table->lock), flags);
        hlist_add_head(&(obj->hlist), &(table->lists[index]));
        spin_unlock_irqrestore(&(table->lock), flags);
        atomic_inc(&(table->num_flows));

        return true;
}

struct flow_entry *search_table(struct flow_table *table, struct flow_entry *flow)
{
        u16 index = flow_hash(flow) >> (16 - table->bits);
        struct flow_entry *obj = NULL;
        struct hlist_node *tmp = NULL;

        if (unlikely(!table || !flow || !(table->lists)))
                return NULL;

        hlist_for_each_entry_safe(obj, tmp, &(table->lists[index]), hlist) {
                /* if the flow entry already exists */
                if (obj->local_ip == flow->local_ip &&
                    obj->remote_ip == flow->remote_ip &&
                    obj->local_port == flow->local_port &&
                    obj->remote_port == flow->remote_port) {
                            return obj;
                }
        }

        return NULL;
}

bool delete_table(struct flow_table *table, struct flow_entry *flow)
{
        u16 index = flow_hash(flow) >> (16 - table->bits);
        struct flow_entry *obj = NULL;
        struct hlist_node *tmp = NULL;
        unsigned long flags;

        if (unlikely(!table || !flow || !(table->lists)))
                return false;

        spin_lock_irqsave(&(table->lock), flags);
        hlist_for_each_entry_safe(obj, tmp, &(table->lists[index]), hlist) {
                /* if the flow entry already exists */
                if (obj->local_ip == flow->local_ip &&
                    obj->remote_ip == flow->remote_ip &&
                    obj->local_port == flow->local_port &&
                    obj->remote_port == flow->remote_port) {
                            hlist_del(&(obj->hlist));
                            kfree(obj);
                            spin_unlock_irqrestore(&(table->lock), flags);
                            atomic_dec(&(table->num_flows));
                            return true;
                }
        }
        spin_unlock_irqrestore(&(table->lock), flags);
        return false;
}
