//
// Created by jpuchky on 20.5.23.
//

#ifndef LKM_HYBRID_XEN_SOCKET_DEBUG_H
#define LKM_HYBRID_XEN_SOCKET_DEBUG_H
// #define DEBUG
#ifdef DEBUG
#define TRACE_ENTRY printk(KERN_CRIT "Entering %s\n", __func__)
#define TRACE_EXIT  printk(KERN_CRIT "Exiting %s\n", __func__)
#else
#define TRACE_ENTRY do {} while (0)
#define TRACE_EXIT  do {} while (0)
#endif
#define TRACE_ERROR printk(KERN_CRIT "Exiting (ERROR) %s\n", __func__)

#endif //LKM_HYBRID_XEN_SOCKET_DEBUG_H
