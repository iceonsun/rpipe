//
// Created on 11/13/17.
//

#ifndef RPIPE_RCOMMON_H
#define RPIPE_RCOMMON_H


#include <uv.h>
#include <types.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct rbuf_t {
    char *base;
    int len;
    void *data;
} rbuf_t;

void free_rbuf(rbuf_t *buf);

typedef struct rwrite_req_t {
    uv_write_t write;
    uv_buf_t buf;
} rwrite_req_t;

void alloc_buf(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
void free_rwrite_req(rwrite_req_t *req);

typedef struct rudp_send_t {
    uv_udp_send_t udp_send;
    uv_buf_t buf;
} rudp_send_t;

void free_rudp_send(rudp_send_t *send);

///* get system time */
//static inline void itimeofday(long *sec, long *usec)
//{
//#if defined(__unix)
//    struct timeval time;
//	gettimeofday(&time, NULL);
//	if (sec) *sec = time.tv_sec;
//	if (usec) *usec = time.tv_usec;
//#else
//    static long mode = 0, addsec = 0;
//    BOOL retval;
//    static IINT64 freq = 1;
//    IINT64 qpc;
//    if (mode == 0) {
//        retval = QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
//        freq = (freq == 0)? 1 : freq;
//        retval = QueryPerformanceCounter((LARGE_INTEGER*)&qpc);
//        addsec = (long)time(NULL);
//        addsec = addsec - (long)((qpc / freq) & 0x7fffffff);
//        mode = 1;
//    }
//    retval = QueryPerformanceCounter((LARGE_INTEGER*)&qpc);
//    retval = retval * 2;
//    if (sec) *sec = (long)(qpc / freq) + addsec;
//    if (usec) *usec = (long)((qpc % freq) * 1000000 / freq);
//#endif
//}
///* get clock in millisecond 64 */
//static inline IINT64 iclock64(void)
//{
//    long s, u;
//    IINT64 value;
//    itimeofday(&s, &u);
//    value = ((IINT64)s) * 1000 + (u / 1000);
//    return value;
//}
//
//static inline IUINT32 iclock()
//{
//    return (IUINT32)(iclock64() & 0xfffffffful);
//}

#ifdef __cplusplus
}
#endif
#endif //RPIPE_RCOMMON_H
