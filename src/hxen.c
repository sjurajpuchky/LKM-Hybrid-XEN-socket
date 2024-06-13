//
// Created by jpuchky on 17.5.23.
//
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/socket.h>
#include <linux/net.h>
#include <linux/un.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/types.h>


#include <net/sock.h>
#include <net/tcp_states.h>


#include "xensocket.h"
#include "debug.h"

static char *SOCKET_PATH = "/var/run/hxen.sock";
static int   XEN_SOCKET = 112233;
static char *SOCKET_DIRECTION = "server";

static struct socket *unix_socket = NULL;
struct task_struct *server_thread_task;

static int server_thread(void *arg) {
    struct socket client_socket;
    struct sockaddr_un client_addr;
    int retval;

    allow_signal(SIGKILL); // Allow the thread to be killed

    while (!kthread_should_stop()) {
        // Accept a new client connection
        retval = unix_socket->ops->accept(unix_socket, &client_socket, O_NONBLOCK, true);
        if (retval < 0) {
            pr_err("Failed to accept connection: %d\n", retval);
            continue;
        }

        // Get the client address
        memset(&client_addr, 0, sizeof(struct sockaddr_un));

        pr_info("Accepted connection from %s\n", client_addr.sun_path);

        // Handle client requests
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        struct iov_iter iov;
        struct kvec vec;
        vec.iov_base = buffer;
        vec.iov_len = sizeof(buffer);
        iov_iter_kvec(&iov, READ, &vec, 1, sizeof(buffer));

        retval = kernel_read(client_socket.file, &iov, &vec.iov_len, 0);
        if (retval < 0) {
            pr_err("Failed to read from socket: %d\n", retval);
        } else {
            pr_info("Received data from %s: %s\n", client_addr.sun_path, buffer);
        }

        // Write response to the client
        char response[] = "Hello from the server";
        struct iov_iter iov_out;
        struct kvec vec_out;
        vec_out.iov_base = response;
        vec_out.iov_len = strlen(response);
        iov_iter_kvec(&iov_out, WRITE, &vec_out, 1, strlen(response));

        retval = kernel_write(client_socket.file, &iov_out, &vec_out.iov_len, 0);
        if (retval < 0) {
            pr_err("Failed to write to socket: %d\n", retval);
        } else {
            pr_info("Sent response to %s\n", client_addr.sun_path);
        }

        // Cleanup and release the client socket
        sock_release(&client_socket);
    }

    return 0;
}

static int __init module_init_func(void) {
    TRACE_ENTRY;
    struct sockaddr_un addr;
    int rc = -1;

    rc = proto_register(&xen_proto, 1);
    if (rc != 0) {
        pr_err("%s: Cannot create xen_sock SLAB cache!\n", __FUNCTION__);
        return rc;
    }

    sock_register(&xen_family_ops);



    // Create a Unix socket
    rc = sock_create_kern(NULL, AF_UNIX, SOCK_STREAM, 0, &unix_socket);
    if (rc < 0) {
        pr_err("Failed to create socket: %d\n", rc);
        return rc;
    }

    // Set up the address structure
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // Bind the socket to the address
    rc = unix_socket->ops->bind(unix_socket, (struct sockaddr *) &addr,
                                    sizeof(struct sockaddr_un));
    if (rc < 0) {
        pr_err("Failed to bind socket: %d\n", rc);
        sock_release(unix_socket);
        return rc;
    }

    // Listen for incoming connections

    rc = unix_socket->ops->listen(unix_socket, 10);
    if (rc < 0) {
        pr_err("Failed to listen on socket: %d\n", rc);
        sock_release(unix_socket);
        return rc;
    }

// Create a kernel thread to handle client connections

    server_thread_task = kthread_run(server_thread, NULL, "socket_server_thread");
    if (IS_ERR(server_thread_task)) {
        pr_err("Failed to create server thread\n");
        sock_release(unix_socket);
        return PTR_ERR(server_thread_task);
    }

    pr_info("Unix socket server started\n");



    TRACE_EXIT;
    return rc;
}

static void __exit

module_exit_func(void) {
    TRACE_ENTRY;

    sock_unregister(AF_XEN);
    proto_unregister(&xen_proto);


    // Stop the server thread
    kthread_stop(server_thread_task);

    // Release the socket
    if (unix_socket != NULL) {
        sock_release(unix_socket);
        pr_info("Unix socket released\n");
    }

    TRACE_EXIT;
}

module_init(module_init_func);
module_exit(module_exit_func);

MODULE_LICENSE("GPLv3");
MODULE_AUTHOR("Juraj Puchký <info@baba.bj>");
MODULE_DESCRIPTION("LKM Hybrid XEN socket module");

module_param(SOCKET_PATH, charp, 0000);
MODULE_PARM_DESC(SOCKET_PATH, "Absolute path to unix socket file");

module_param(SOCKET_DIRECTION, charp, 0000);
MODULE_PARM_DESC(SOCKET_DIRECTION, "client/server");

module_param(XEN_SOCKET, int, 0000);
MODULE_PARM_DESC(XEN_SOCKET, "An integer XEN Socket identifier");