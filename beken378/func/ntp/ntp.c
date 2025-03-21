/*
 *
 * (C) 2014 David Lettier.
 * (C) 2018 Armink (armink.ztl@gmail.com)
 *
 * http://www.lettier.com/
 *
 * NTP client.
 *
 * Compiled with gcc version 4.7.2 20121109 (Red Hat 4.7.2-8) (GCC).
 *
 * Tested on Linux 3.8.11-200.fc18.x86_64 #1 SMP Wed May 1 19:44:27 UTC 2013 x86_64 x86_64 x86_64 GNU/Linux.
 * Tested on RT-Thread 3.0.0+
 *
 * To compile: $ gcc main.c -o ntpClient.out
 *
 * Usage: $ ./ntpClient.out
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "lwip/sockets.h"
#include <netdb.h>
#include "rtc.h"
#include "rtc_time.h"
#if CFG_USE_NTP
#define NETUTILS_NTP_TIMEZONE	8
#define NETUTILS_NTP_HOSTNAME	"cn.ntp.org.cn"

#ifdef NETUTILS_NTP_TIMEZONE
#define NTP_TIMEZONE                   NETUTILS_NTP_TIMEZONE
#endif

#ifdef NETUTILS_NTP_HOSTNAME
#define NTP_HOSTNAME                   NETUTILS_NTP_HOSTNAME
#endif

#define NTP_TIMESTAMP_DELTA            2208988800ull
#define NTP_GET_TIMEOUT                10

#ifndef NTP_TIMEZONE
#define NTP_TIMEZONE                   8
#endif

#ifndef NTP_HOSTNAME
#define NTP_HOSTNAME                   "cn.pool.ntp.org"
#endif

#define LI(packet)   (uint8_t) ((packet.li_vn_mode & 0xC0) >> 6) // (li   & 11 000 000) >> 6
#define VN(packet)   (uint8_t) ((packet.li_vn_mode & 0x38) >> 3) // (vn   & 00 111 000) >> 3
#define MODE(packet) (uint8_t) ((packet.li_vn_mode & 0x07) >> 0) // (mode & 00 000 111) >> 0

static int g_timezone = NTP_TIMEZONE;
static char g_hostname[64]="cn.pool.ntp.org";

// Structure that defines the 48 byte NTP packet protocol.
typedef struct {
    uint8_t li_vn_mode;      // Eight bits. li, vn, and mode.
                             // li.   Two bits.   Leap indicator.
                             // vn.   Three bits. Version number of the protocol.
                             // mode. Three bits. Client will pick mode 3 for client.

    uint8_t stratum;         // Eight bits. Stratum level of the local clock.
    uint8_t poll;            // Eight bits. Maximum interval between successive messages.
    uint8_t precision;       // Eight bits. Precision of the local clock.

    uint32_t rootDelay;      // 32 bits. Total round trip delay time.
    uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
    uint32_t refId;          // 32 bits. Reference clock identifier.

    uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
    uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.

    uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
    uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.

    uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
    uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.

    uint32_t txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
    uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.

} ntp_packet;              // Total: 384 bits or 48 bytes.

static ntp_packet packet = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void ntp_info_update(int timezone,char* ntphost)
{
    g_timezone = timezone;
    if(ntphost != NULL){
        strcpy(g_hostname,ntphost);
    }
}

static void ntp_error(char* msg)
{
    bk_printf("\033[31;22m[E/NTP]: ERROR %s\033[0m\n", msg); // Print the error message to stderr.
}

/**
 * Get the UTC time from NTP server
 *
 * @note this function is not reentrant
 *
 * @return >0: success, current UTC time
 *         =0: get failed
 */
time_t_at ntp_get_time(void)
{
    int sockfd=-1, n; // Socket file descriptor and the n return result from writing/reading from the socket.
    int portno = 123; // NTP UDP port number.
    char* host_name = g_hostname;   //NTP_HOSTNAME; // NTP server host-name.
    time_t_at new_time = 0;
    fd_set readset;
    struct timeval timeout;

    // Create and zero out the packet. All 48 bytes worth.
    os_memset(&packet, 0, sizeof(ntp_packet));

    // Set the first byte's bits to 00,011,011 for li = 0, vn = 3, and mode = 3. The rest will be left set to zero.
    *((char *) &packet + 0) = 0x1b; // Represents 27 in base 10 or 00011011 in base 2.

    // Create a UDP socket, convert the host-name to an IP address, set the port number,
    // connect to the server, send the packet, and then read in the return packet.
    struct sockaddr_in serv_addr; // Server address data structure.
    struct hostent *server;      // Server data structure.

    os_memset((char *) &serv_addr, 0, sizeof(serv_addr));

    if(inet_aton(host_name, &serv_addr.sin_addr.s_addr) == 1) {
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(portno);
    } else {
        server = gethostbyname(host_name); // Convert URL to IP.
        if (server == NULL) {
            ntp_error("no such host");
            goto __exit;
        }

        // Zero out the server address structure.
        //  os_memset((char *) &serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;

        // Copy the server's IP address to the server address structure.
        os_memcpy((char *) &serv_addr.sin_addr.s_addr, (char *) server->h_addr, server->h_length);

        // Convert the port number integer to network big-endian style and save it to the server address structure.
        serv_addr.sin_port = htons(portno);
    }

    // Call up the server using its IP address and port number.
    sockfd = lwip_socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP); // Create a UDP socket.
    if (sockfd < 0) {
        ntp_error("opening socket");
        goto __exit;
    }

    if (lwip_connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        ntp_error("connecting");
        goto __exit;
    }

    // Send it the NTP packet it wants. If n == -1, it failed.
    n = lwip_write(sockfd, (char*) &packet, sizeof(ntp_packet));
    if (n < 0) {
        ntp_error("writing to socket");
        goto __exit;
    }

    timeout.tv_sec = NTP_GET_TIMEOUT;
    timeout.tv_usec = 0;

    FD_ZERO(&readset);
    FD_SET(sockfd, &readset);

    if (lwip_select(sockfd + 1, &readset, NULL, NULL, &timeout) <= 0) {
        ntp_error("select the socket timeout(10s)");
        goto __exit;
    }

    // Wait and receive the packet back from the server. If n == -1, it failed.
    n = lwip_read(sockfd, (char*) &packet, sizeof(ntp_packet));
    if (n < 0) {
        ntp_error("reading from socket");
        goto __exit;
    }

    // These two fields contain the time-stamp seconds as the packet left the NTP server.
    // The number of seconds correspond to the seconds passed since 1900.
    // ntohl() converts the bit/byte order from the network's to host's "endianness".
    packet.txTm_s = ntohl(packet.txTm_s); // Time-stamp seconds.
    packet.txTm_f = ntohl(packet.txTm_f); // Time-stamp fraction of a second.

    // Extract the 32 bits that represent the time-stamp seconds (since NTP epoch) from when the packet left the server.
    // Subtract 70 years worth of seconds from the seconds since 1900.
    // This leaves the seconds since the UNIX epoch of 1970.
    // (1900)------------------(1970)**************************************(Time Packet Left the Server)
    new_time = (time_t_at) (packet.txTm_s - NTP_TIMESTAMP_DELTA);

__exit:
    close(sockfd);
    return new_time;
}

/**
 * Get the local time from NTP server
 *
 * @return >0: success, current local time, offset timezone by NTP_TIMEZONE
 *         =0: get failed
 */
time_t_at ntp_get_local_time(void)
{
    time_t_at cur_time = ntp_get_time();

    if (cur_time)
    {
        /* add the timezone offset for set_time/set_date */
       // cur_time += NTP_TIMEZONE * 3600;
        cur_time += g_timezone * 3600;
    }
    bk_printf("get local time:%d,g_timezone:%d,g_hostname:%s\r\n",cur_time,g_timezone,g_hostname);
    return cur_time;
}

/**
 * Sync current local time to RTC by NTP
 *
 * @return >0: success, current local time, offset timezone by NTP_TIMEZONE
 *         =0: sync failed
 */
time_t_at ntp_sync_to_rtc(void)
{
    struct tm *cur_tm;
    time_t_at cur_time = ntp_get_local_time();

    if (cur_time)
    {
        cur_tm = localtime((time_t *)&cur_time);
        set_time(cur_tm->tm_hour, cur_tm->tm_min, cur_tm->tm_sec);
        cur_tm = localtime((time_t *)&cur_time);
        set_date(cur_tm->tm_year + 1900, cur_tm->tm_mon + 1 , cur_tm->tm_mday);
    }

    return cur_time;
}
#endif
