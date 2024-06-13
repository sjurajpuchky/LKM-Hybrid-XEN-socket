/* xensocket.h
 *
 * Header file for shared-memory sockets transport for communications
 * between two domains on the same machine, under the Xen hypervisor.
 *
 * Authors: Xiaolan (Catherine) Zhang <cxzhang@us.ibm.com>
 *          Suzanne McIntosh <skranjac@us.ibm.com>
 *          John Griffin
 *
 * Modified for Hybrid Socket by Juraj Puchký <sjurajpuchky@seznam.cz>
 *
 * History:
 *          Suzanne McIntosh    13-Aug-07     Initial open source version
 *
 * Copyright (c) 2007, IBM Corporation
 *
 */

#ifndef __XENSOCKET_H__
#define __XENSOCKET_H__

struct sockaddr_xe {
    sa_family_t sxe_family;
    u_int16_t   remote_domid;
    int         shared_page_gref;
};

#define AF_XEN  21
#define PF_XEN  AF_XEN

#define xen_sk(__sk) ((struct xen_sock *)__sk)

#define DPRINTK( x, args... ) printk(KERN_CRIT "%s: line %d: " x, __FUNCTION__ , __LINE__ , ## args );

extern struct descriptor_page;
extern struct xen_sock;

extern static void
initialize_descriptor_page (struct descriptor_page *d);

extern static void
initialize_xen_sock (struct xen_sock *x);

extern static int
xen_create (struct socket *sock, int protocol);

extern static int
xen_bind (struct socket *sock, struct sockaddr *uaddr, int addr_len);

extern static int
server_allocate_descriptor_page (struct xen_sock *x);

extern static int
server_allocate_event_channel (struct xen_sock *x);

extern static int
server_allocate_buffer_pages (struct xen_sock *x);

extern static int
xen_connect (struct socket *sock, struct sockaddr *uaddr, int addr_len, int flags);

extern static int
client_map_descriptor_page (struct xen_sock *x);

extern static int
client_bind_event_channel (struct xen_sock *x);

extern static int
client_map_buffer_pages (struct xen_sock *x);

extern static int
xen_sendmsg (struct kiocb *kiocb, struct socket *sock, struct msghdr *msg, size_t len);

extern static inline int
is_writeable (struct descriptor_page *d);

extern static long
send_data_wait (struct sock *sk, long timeo);

extern static irqreturn_t
client_interrupt (int irq, void *dev_id, struct pt_regs *regs);

extern static int
xen_recvmsg (struct kiocb *iocb, struct socket *sock, struct msghdr *msg, size_t size, int flags);

extern static inline int
is_readable (struct descriptor_page *d);

extern static long
receive_data_wait (struct sock *sk, long timeo);

extern static irqreturn_t
server_interrupt (int irq, void *dev_id, struct pt_regs *regs);

extern static int
local_memcpy_toiovecend (struct iovec *iov, unsigned char *kdata, int offset, int len);

extern static int
xen_release (struct socket *sock);

extern static int
xen_shutdown (struct socket *sock, int how);

extern static void
server_unallocate_buffer_pages (struct xen_sock *x);

extern static void
server_unallocate_descriptor_page (struct xen_sock *x);

extern static void
client_unmap_buffer_pages (struct xen_sock *x);

extern static void
client_unmap_descriptor_page (struct xen_sock *x);

struct descriptor_page {
    uint32_t        server_evtchn_port;
    int             buffer_order; /* num_pages = (1 << buffer_order) */
    int             buffer_first_gref;
    unsigned int    send_offset;
    unsigned int    recv_offset;
    unsigned int    total_bytes_sent;
    unsigned int    total_bytes_received;
    unsigned int    sender_is_blocking;
    atomic_t        avail_bytes;
    atomic_t        sender_has_shutdown;
    atomic_t        force_sender_shutdown;
};


/* struct xen_sock:
 *
 * @sk: this must be the first element in the structure.
 */
struct xen_sock {
    struct sock             sk;
    unsigned char           is_server, is_client;
    domid_t                 otherend_id;
    struct descriptor_page *descriptor_addr;    /* server and client */
    int                     descriptor_gref;    /* server only */
    struct vm_struct       *descriptor_area;    /* client only */
    grant_handle_t          descriptor_handle;  /* client only */
    unsigned int            evtchn_local_port;
    unsigned int            irq;
    unsigned long           buffer_addr;    /* server and client */
    int                    *buffer_grefs;   /* server */
    struct vm_struct       *buffer_area;    /* client */
    grant_handle_t         *buffer_handles; /* client */
    int                     buffer_order;
};

static struct proto xen_proto = {
        .name           = "XEN",
        .owner          = THIS_MODULE,
        .obj_size       = sizeof(struct xen_sock),
};

static const struct proto_ops xen_stream_ops = {
        .family         = AF_XEN,
        .owner          = THIS_MODULE,
        .release        = xen_release,
        .bind           = xen_bind,
        .connect        = xen_connect,
        .socketpair     = sock_no_socketpair,
        .accept         = sock_no_accept,
        .getname        = sock_no_getname,
        .poll           = sock_no_poll,
        .ioctl          = sock_no_ioctl,
        .listen         = sock_no_listen,
        .shutdown       = xen_shutdown,
        .getsockopt     = sock_no_getsockopt,
        .setsockopt     = sock_no_setsockopt,
        .sendmsg        = xen_sendmsg,
        .recvmsg        = xen_recvmsg,
        .mmap           = sock_no_mmap,
        .sendpage       = sock_no_sendpage,
};

static struct net_proto_family xen_family_ops = {
        .family         = AF_XEN,
        .create         = xen_create,
        .owner          = THIS_MODULE,
};

#endif /* __XENSOCKET_H__ */
