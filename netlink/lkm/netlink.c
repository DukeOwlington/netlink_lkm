#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>

#define NETLINK_USER 31

MODULE_LICENSE("GPL");                          /* The license type -- this affects available functionality */
MODULE_AUTHOR("MadMax");           			    /* The author -- visible when you use modinfo */
MODULE_DESCRIPTION("A simple netlink lkm"); 	/* The description -- see modinfo */
MODULE_VERSION("0.1");             			    /* A version number to inform users */

struct sock *nl_sk = NULL;
static void recv_msg(struct sk_buff*);

static struct netlink_kernel_cfg cfg = {
      .input = recv_msg,
};
static void recv_msg(struct sk_buff *skb) {
  struct nlmsghdr *nlh;
  int pid;
  struct sk_buff *skb_out;
  int msg_size;
  char msg[128];
  int res;

  printk(KERN_INFO "Entering: %s\n", __FUNCTION__);

  nlh = (struct nlmsghdr *)skb->data;
  printk(KERN_INFO "Netlink received msg payload:%s\n",
         (char *)nlmsg_data(nlh));

  sprintf(msg, "Netlink module received: %s", (char *)nlmsg_data(nlh));
  msg_size = strlen(msg);

  pid = nlh->nlmsg_pid; /*pid of sending process */
  skb_out = nlmsg_new(msg_size, 0);

  if (!skb_out) {
    printk(KERN_ERR "Failed to allocate new skb\n");
    return;
  }

  nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
  NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
  strncpy(nlmsg_data(nlh), msg, msg_size); /* put the message */

  res = nlmsg_unicast(nl_sk, skb_out, pid);

  if (res < 0) printk(KERN_INFO "Error while sending bak to user\n");
}

static int __init mnetlink_init(void) {
  printk("Entering: %s\n", __FUNCTION__);

  nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);

  if (!nl_sk) {
    printk(KERN_ALERT "Error creating socket.\n");
    return -1;
  }

  return 0;
}

static void __exit mnetlink_exit(void) {
  printk(KERN_INFO "exiting hello module\n");
  netlink_kernel_release(nl_sk);
}

module_init(mnetlink_init);
module_exit(mnetlink_exit);
