/* This is a small demo of the NetX Secure TLS API running on a 
   high-performance NetX TCP/IP stack.  */
/* This demo works for IPv4 only */

#include "tx_api.h"
#include "nx_api.h"

#ifndef NX_DISABLE_IPV4

#include "nx_secure_tls_api.h"
#include "nx_secure_x509.h"

#define     DEMO_STACK_SIZE         4096

/* Replace the 'ram' driver with your Ethernet driver. */
VOID    _nx_ram_network_driver(struct NX_IP_DRIVER_STRUCT *driver_req);

/* Define packet pool for the demonstration.  */
#define NX_PACKET_SIZE (1536 + sizeof(NX_PACKET))

/* Set up the TLS client global variables. */

static TX_THREAD             client_thread;
static NX_PACKET_POOL        client_pool;
static NX_IP                 client_ip;
static NX_TCP_SOCKET         client_tcp_socket;
static NX_SECURE_TLS_SESSION client_tls_session;
static NX_SECURE_X509_CERT   trusted_certificate;
static UINT                  error_counter;

/* Set up the TLS server global variables */

static TX_THREAD             server_thread;
static NX_PACKET_POOL        server_pool;
static NX_IP                 server_ip;
static NX_TCP_SOCKET         server_tcp_socket;
static NX_SECURE_TLS_SESSION server_tls_session;


static void tls_client_thread_entry(ULONG thread_input);
static void tls_server_thread_entry(ULONG thread_input);


/* Define some demo certificates. First, the server/device certificate. */
static unsigned char test_device_cert_der[] = {
  0x30, 0x82, 0x03, 0xd2, 0x30, 0x82, 0x02, 0xba, 0xa0, 0x03, 0x02, 0x01,
  0x02, 0x02, 0x01, 0x01, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
  0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05, 0x00, 0x30, 0x7a, 0x31, 0x0b, 0x30,
  0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x0b,
  0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x08, 0x0c, 0x02, 0x43, 0x41, 0x31,
  0x12, 0x30, 0x10, 0x06, 0x03, 0x55, 0x04, 0x07, 0x0c, 0x09, 0x53, 0x61,
  0x6e, 0x20, 0x44, 0x69, 0x65, 0x67, 0x6f, 0x31, 0x16, 0x30, 0x14, 0x06,
  0x03, 0x55, 0x04, 0x0a, 0x0c, 0x0d, 0x45, 0x78, 0x70, 0x72, 0x65, 0x73,
  0x73, 0x20, 0x4c, 0x6f, 0x67, 0x69, 0x63, 0x31, 0x14, 0x30, 0x12, 0x06,
  0x03, 0x55, 0x04, 0x0b, 0x0c, 0x0b, 0x4e, 0x65, 0x74, 0x58, 0x20, 0x53,
  0x65, 0x63, 0x75, 0x72, 0x65, 0x31, 0x1c, 0x30, 0x1a, 0x06, 0x03, 0x55,
  0x04, 0x03, 0x0c, 0x13, 0x4e, 0x65, 0x74, 0x58, 0x20, 0x53, 0x65, 0x63,
  0x75, 0x72, 0x65, 0x20, 0x54, 0x65, 0x73, 0x74, 0x20, 0x43, 0x41, 0x30,
  0x1e, 0x17, 0x0d, 0x31, 0x36, 0x31, 0x31, 0x31, 0x31, 0x31, 0x39, 0x35,
  0x31, 0x30, 0x30, 0x5a, 0x17, 0x0d, 0x32, 0x36, 0x31, 0x31, 0x30, 0x39,
  0x31, 0x39, 0x35, 0x31, 0x30, 0x30, 0x5a, 0x30, 0x62, 0x31, 0x0b, 0x30,
  0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x0b,
  0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x08, 0x0c, 0x02, 0x43, 0x41, 0x31,
  0x16, 0x30, 0x14, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x0d, 0x45, 0x78,
  0x70, 0x72, 0x65, 0x73, 0x73, 0x20, 0x4c, 0x6f, 0x67, 0x69, 0x63, 0x31,
  0x14, 0x30, 0x12, 0x06, 0x03, 0x55, 0x04, 0x0b, 0x0c, 0x0b, 0x4e, 0x65,
  0x74, 0x58, 0x20, 0x53, 0x65, 0x63, 0x75, 0x72, 0x65, 0x31, 0x18, 0x30,
  0x16, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x0f, 0x77, 0x77, 0x77, 0x2e,
  0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x2e, 0x63, 0x6f, 0x6d, 0x30,
  0x82, 0x01, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7,
  0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00, 0x30,
  0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xae, 0x03, 0x2c, 0xec,
  0xa2, 0x79, 0xd1, 0x15, 0x20, 0x88, 0x4d, 0xcd, 0xa2, 0x1b, 0x05, 0xe3,
  0xbd, 0x55, 0xad, 0xc6, 0x1f, 0x64, 0xe8, 0xb5, 0xc5, 0x0d, 0x67, 0xfc,
  0x7e, 0xda, 0xfb, 0x70, 0xf6, 0xc9, 0x47, 0x87, 0x3a, 0xaa, 0x88, 0x00,
  0xf1, 0xa7, 0xf7, 0xe1, 0xf5, 0x2c, 0x54, 0x0e, 0x33, 0xda, 0xbe, 0x9c,
  0x66, 0x30, 0xd9, 0x40, 0xeb, 0x1d, 0xce, 0xe1, 0x55, 0x15, 0x2b, 0x11,
  0x47, 0x6c, 0x7e, 0x88, 0xc6, 0x24, 0xcf, 0x87, 0x1b, 0xb5, 0x1f, 0x47,
  0xb9, 0xef, 0xad, 0x29, 0xd3, 0x2e, 0x43, 0xee, 0x39, 0xdd, 0x09, 0x54,
  0xba, 0xfc, 0xed, 0xbc, 0x2e, 0x0e, 0x53, 0x15, 0x37, 0xcb, 0xc5, 0xf5,
  0xee, 0x70, 0x2a, 0xe8, 0x01, 0x6d, 0xb1, 0x39, 0x94, 0x5a, 0xc2, 0x8a,
  0x00, 0x04, 0xa9, 0xff, 0xea, 0x56, 0xf7, 0xd7, 0xa8, 0x1b, 0xa4, 0x26,
  0xcd, 0x28, 0xaf, 0xfa, 0x52, 0x85, 0x1c, 0x26, 0x3e, 0x5e, 0x01, 0xf7,
  0xe1, 0x66, 0xff, 0xac, 0xad, 0x9c, 0x98, 0x2f, 0xe0, 0x7e, 0x9f, 0xf1,
  0x33, 0x31, 0xc3, 0x7f, 0xe6, 0x58, 0x5d, 0xd8, 0x5f, 0x7d, 0x2b, 0x5a,
  0x55, 0xcf, 0xb1, 0x91, 0x53, 0x41, 0x04, 0xac, 0x86, 0x5e, 0x01, 0x35,
  0x2b, 0x74, 0x8d, 0x46, 0x4d, 0x48, 0xc0, 0x5f, 0x83, 0x67, 0xb5, 0x6d,
  0x52, 0x3f, 0x3e, 0xe6, 0xec, 0xf8, 0x2e, 0x10, 0x28, 0xdb, 0x69, 0xa6,
  0x9d, 0x4b, 0xde, 0x19, 0x2e, 0xd2, 0x5f, 0xc8, 0xa9, 0x3b, 0x52, 0xe9,
  0xb2, 0xcd, 0x6e, 0x19, 0x22, 0xf9, 0x99, 0xa6, 0xcc, 0xf5, 0xd3, 0xec,
  0xff, 0x0c, 0x77, 0x6f, 0x25, 0x92, 0x07, 0x4c, 0x64, 0x7d, 0x34, 0x49,
  0x6f, 0xff, 0x0a, 0xa8, 0x15, 0x64, 0x72, 0x2d, 0x4f, 0x42, 0x05, 0xe8,
  0x2b, 0x01, 0xf1, 0xe3, 0x65, 0x94, 0x23, 0xd9, 0xdf, 0x5e, 0x3b, 0xb5,
  0x02, 0x03, 0x01, 0x00, 0x01, 0xa3, 0x7b, 0x30, 0x79, 0x30, 0x09, 0x06,
  0x03, 0x55, 0x1d, 0x13, 0x04, 0x02, 0x30, 0x00, 0x30, 0x2c, 0x06, 0x09,
  0x60, 0x86, 0x48, 0x01, 0x86, 0xf8, 0x42, 0x01, 0x0d, 0x04, 0x1f, 0x16,
  0x1d, 0x4f, 0x70, 0x65, 0x6e, 0x53, 0x53, 0x4c, 0x20, 0x47, 0x65, 0x6e,
  0x65, 0x72, 0x61, 0x74, 0x65, 0x64, 0x20, 0x43, 0x65, 0x72, 0x74, 0x69,
  0x66, 0x69, 0x63, 0x61, 0x74, 0x65, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d,
  0x0e, 0x04, 0x16, 0x04, 0x14, 0x8d, 0xb0, 0xee, 0x8f, 0x6b, 0x43, 0x52,
  0x29, 0xf4, 0x25, 0xff, 0x3c, 0xda, 0x5f, 0xb3, 0xce, 0x9b, 0x7b, 0x75,
  0xe1, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16,
  0x80, 0x14, 0x1b, 0x8d, 0x06, 0xd9, 0x6b, 0xad, 0xee, 0x82, 0x24, 0x26,
  0x55, 0x9a, 0x1b, 0x03, 0x44, 0x92, 0x0a, 0x06, 0x92, 0x48, 0x30, 0x0d,
  0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05,
  0x00, 0x03, 0x82, 0x01, 0x01, 0x00, 0x75, 0x83, 0x89, 0xab, 0x84, 0x52,
  0x5f, 0xa4, 0x9e, 0x98, 0xca, 0xa3, 0xf9, 0xab, 0xd4, 0x04, 0x32, 0xa4,
  0x8c, 0x96, 0x90, 0x39, 0x88, 0x92, 0xc3, 0xcd, 0x51, 0xc3, 0x01, 0x35,
  0x03, 0x78, 0xfa, 0x0d, 0x1e, 0x7b, 0x79, 0xe9, 0x7d, 0xd8, 0x68, 0x7a,
  0x65, 0xc6, 0x00, 0x7c, 0xa1, 0x7a, 0x52, 0xc9, 0xa3, 0xf4, 0x0b, 0xbd,
  0x76, 0x24, 0xdf, 0xde, 0x22, 0x2d, 0x95, 0xc5, 0xb6, 0x54, 0xb1, 0xac,
  0xb6, 0x9a, 0xe4, 0x68, 0x0f, 0x97, 0x4a, 0x44, 0xa2, 0x87, 0x01, 0x82,
  0xd4, 0x25, 0xbd, 0x01, 0xbc, 0x35, 0x8a, 0x6d, 0xb7, 0x7c, 0x48, 0xaa,
  0x92, 0xd7, 0x57, 0x76, 0x6a, 0xb0, 0xc9, 0x46, 0xa6, 0xbe, 0xbf, 0x0f,
  0xf0, 0xea, 0x62, 0x57, 0x71, 0x42, 0xf6, 0x67, 0xa7, 0xa1, 0x50, 0x87,
  0x14, 0x8e, 0x32, 0xd0, 0x5e, 0xc9, 0x7b, 0x79, 0x7e, 0xfa, 0x17, 0xc7,
  0xad, 0xbd, 0xc3, 0x98, 0x79, 0x45, 0xfb, 0x7f, 0xf7, 0xe6, 0x9f, 0x77,
  0xb3, 0x44, 0xc3, 0xaf, 0x6b, 0x61, 0x6a, 0x04, 0x68, 0x24, 0x2d, 0x31,
  0xf1, 0x28, 0x2c, 0xf4, 0xf0, 0x07, 0xfe, 0xfd, 0x66, 0x98, 0x77, 0x37,
  0x7b, 0x80, 0x1f, 0xb2, 0x49, 0xe4, 0xa6, 0x24, 0x72, 0x42, 0xf4, 0xca,
  0x91, 0x80, 0xa1, 0xb2, 0x0a, 0xc9, 0xc0, 0x93, 0xa7, 0x22, 0x0b, 0x13,
  0x8a, 0xb2, 0x75, 0x4b, 0x66, 0xf9, 0x87, 0x3a, 0x51, 0x97, 0xc7, 0x1e,
  0x2b, 0x61, 0x81, 0x5c, 0xf0, 0xf8, 0x4c, 0xdb, 0x36, 0xc7, 0xba, 0x49,
  0xd9, 0x04, 0x6a, 0x95, 0xb0, 0x7f, 0xfc, 0xce, 0xca, 0x23, 0xad, 0xf9,
  0xaf, 0x8a, 0x72, 0x8e, 0xab, 0xb8, 0x8b, 0x7e, 0xf7, 0x39, 0xa6, 0x22,
  0x56, 0x03, 0x72, 0x06, 0xc3, 0x57, 0x1f, 0x32, 0xaa, 0xb5, 0xa6, 0x00,
  0x67, 0x88, 0x4b, 0x40, 0xe9, 0x5e, 0x4a, 0x6f, 0x76, 0xe8
};
static unsigned int test_device_cert_der_len = 982;

/* Server private RSA key. */
static unsigned char test_device_cert_key_der[] = {
  0x30, 0x82, 0x04, 0xa4, 0x02, 0x01, 0x00, 0x02, 0x82, 0x01, 0x01, 0x00,
  0xae, 0x03, 0x2c, 0xec, 0xa2, 0x79, 0xd1, 0x15, 0x20, 0x88, 0x4d, 0xcd,
  0xa2, 0x1b, 0x05, 0xe3, 0xbd, 0x55, 0xad, 0xc6, 0x1f, 0x64, 0xe8, 0xb5,
  0xc5, 0x0d, 0x67, 0xfc, 0x7e, 0xda, 0xfb, 0x70, 0xf6, 0xc9, 0x47, 0x87,
  0x3a, 0xaa, 0x88, 0x00, 0xf1, 0xa7, 0xf7, 0xe1, 0xf5, 0x2c, 0x54, 0x0e,
  0x33, 0xda, 0xbe, 0x9c, 0x66, 0x30, 0xd9, 0x40, 0xeb, 0x1d, 0xce, 0xe1,
  0x55, 0x15, 0x2b, 0x11, 0x47, 0x6c, 0x7e, 0x88, 0xc6, 0x24, 0xcf, 0x87,
  0x1b, 0xb5, 0x1f, 0x47, 0xb9, 0xef, 0xad, 0x29, 0xd3, 0x2e, 0x43, 0xee,
  0x39, 0xdd, 0x09, 0x54, 0xba, 0xfc, 0xed, 0xbc, 0x2e, 0x0e, 0x53, 0x15,
  0x37, 0xcb, 0xc5, 0xf5, 0xee, 0x70, 0x2a, 0xe8, 0x01, 0x6d, 0xb1, 0x39,
  0x94, 0x5a, 0xc2, 0x8a, 0x00, 0x04, 0xa9, 0xff, 0xea, 0x56, 0xf7, 0xd7,
  0xa8, 0x1b, 0xa4, 0x26, 0xcd, 0x28, 0xaf, 0xfa, 0x52, 0x85, 0x1c, 0x26,
  0x3e, 0x5e, 0x01, 0xf7, 0xe1, 0x66, 0xff, 0xac, 0xad, 0x9c, 0x98, 0x2f,
  0xe0, 0x7e, 0x9f, 0xf1, 0x33, 0x31, 0xc3, 0x7f, 0xe6, 0x58, 0x5d, 0xd8,
  0x5f, 0x7d, 0x2b, 0x5a, 0x55, 0xcf, 0xb1, 0x91, 0x53, 0x41, 0x04, 0xac,
  0x86, 0x5e, 0x01, 0x35, 0x2b, 0x74, 0x8d, 0x46, 0x4d, 0x48, 0xc0, 0x5f,
  0x83, 0x67, 0xb5, 0x6d, 0x52, 0x3f, 0x3e, 0xe6, 0xec, 0xf8, 0x2e, 0x10,
  0x28, 0xdb, 0x69, 0xa6, 0x9d, 0x4b, 0xde, 0x19, 0x2e, 0xd2, 0x5f, 0xc8,
  0xa9, 0x3b, 0x52, 0xe9, 0xb2, 0xcd, 0x6e, 0x19, 0x22, 0xf9, 0x99, 0xa6,
  0xcc, 0xf5, 0xd3, 0xec, 0xff, 0x0c, 0x77, 0x6f, 0x25, 0x92, 0x07, 0x4c,
  0x64, 0x7d, 0x34, 0x49, 0x6f, 0xff, 0x0a, 0xa8, 0x15, 0x64, 0x72, 0x2d,
  0x4f, 0x42, 0x05, 0xe8, 0x2b, 0x01, 0xf1, 0xe3, 0x65, 0x94, 0x23, 0xd9,
  0xdf, 0x5e, 0x3b, 0xb5, 0x02, 0x03, 0x01, 0x00, 0x01, 0x02, 0x82, 0x01,
  0x01, 0x00, 0xa5, 0x22, 0x2c, 0x52, 0xd0, 0x09, 0x4c, 0x4a, 0x81, 0x59,
  0xf8, 0x83, 0xa9, 0x4f, 0x7d, 0xb2, 0x56, 0xad, 0xe5, 0x3f, 0xfb, 0xf0,
  0xf6, 0x09, 0xf1, 0x5b, 0x3c, 0x90, 0x58, 0x0e, 0x15, 0xc9, 0x68, 0xd9,
  0x30, 0x40, 0xfb, 0x82, 0x73, 0x98, 0x79, 0xbb, 0xcd, 0xb8, 0x27, 0xc3,
  0x8e, 0x6c, 0xff, 0xf6, 0x99, 0x26, 0xb0, 0xaf, 0xb0, 0xac, 0x33, 0xb3,
  0x50, 0xed, 0x73, 0xa1, 0xa8, 0x02, 0x38, 0xc6, 0x93, 0xf9, 0xd6, 0x17,
  0x7e, 0xbd, 0x97, 0xa4, 0xb5, 0x6f, 0x8a, 0xdb, 0x11, 0x78, 0x7c, 0x89,
  0x0e, 0x3c, 0x17, 0xbb, 0x54, 0x2c, 0x8d, 0x5a, 0x93, 0x7d, 0x1e, 0x33,
  0xc7, 0xd2, 0x7d, 0xe5, 0xaa, 0x12, 0x2d, 0xd9, 0x52, 0x4e, 0x63, 0x74,
  0xa6, 0x57, 0x9f, 0x1a, 0xd6, 0x3c, 0xc1, 0xb1, 0xab, 0x66, 0x4a, 0x0b,
  0x88, 0x1d, 0xa6, 0xd1, 0xbc, 0x60, 0x7a, 0x17, 0x1f, 0x8f, 0x9b, 0x35,
  0x57, 0xf8, 0xd0, 0x1c, 0xd3, 0xa6, 0x56, 0xc8, 0x03, 0x9c, 0x08, 0x3b,
  0x1b, 0x5b, 0xc2, 0x03, 0x3b, 0x3a, 0xa4, 0xe8, 0xed, 0x75, 0x66, 0xb0,
  0x85, 0x56, 0x40, 0xfe, 0xae, 0x97, 0x7e, 0xc0, 0x79, 0x49, 0x13, 0x8b,
  0x01, 0x0c, 0xae, 0x4c, 0x3d, 0x54, 0x47, 0xc5, 0x51, 0x40, 0x3d, 0xcc,
  0x4d, 0x17, 0xb3, 0x4e, 0x1d, 0x85, 0x1c, 0x41, 0x07, 0x03, 0x5e, 0xf9,
  0xfa, 0x17, 0x81, 0x24, 0x34, 0xaa, 0xbf, 0x67, 0x73, 0xb6, 0x9c, 0x67,
  0x36, 0xd9, 0xee, 0xf7, 0x86, 0x4c, 0x4d, 0x79, 0xca, 0xd7, 0xfd, 0x72,
  0xf9, 0xb3, 0x73, 0xc3, 0x57, 0xe5, 0x39, 0x72, 0x93, 0x56, 0xc2, 0xec,
  0xf8, 0x25, 0xe4, 0x8f, 0xba, 0xd0, 0x6f, 0x23, 0x8c, 0x39, 0x9e, 0x05,
  0x1a, 0x4e, 0xdc, 0x5e, 0xcd, 0x17, 0x59, 0x94, 0x37, 0x22, 0xb7, 0x39,
  0x50, 0x65, 0xdc, 0x91, 0x3c, 0xe1, 0x02, 0x81, 0x81, 0x00, 0xe4, 0xc6,
  0x42, 0xe5, 0xea, 0xe5, 0x32, 0xf3, 0x51, 0x36, 0x7b, 0x8c, 0x5b, 0x72,
  0x24, 0x1a, 0x4a, 0x44, 0x4f, 0x64, 0xe5, 0xa7, 0x74, 0xd9, 0xb2, 0x29,
  0x8a, 0x08, 0xcf, 0x9b, 0xd2, 0x9d, 0xc4, 0x20, 0x4c, 0xd3, 0x60, 0x4d,
  0xf7, 0xb7, 0xac, 0x92, 0x6b, 0x2b, 0x95, 0x73, 0x6e, 0x57, 0x00, 0x20,
  0x9d, 0xb2, 0xf6, 0xbd, 0x0b, 0xbb, 0xaa, 0x7e, 0x7e, 0x3e, 0x53, 0xfb,
  0x79, 0x7e, 0x45, 0xd5, 0x2e, 0xab, 0x5e, 0xff, 0x5c, 0x0a, 0x45, 0x2d,
  0x27, 0x19, 0xb0, 0x59, 0x0a, 0x39, 0x89, 0xf6, 0xae, 0xc6, 0xe2, 0xd1,
  0x07, 0x58, 0xbe, 0x95, 0x27, 0xaf, 0xf7, 0xa6, 0x2f, 0xaa, 0x37, 0x25,
  0x7c, 0x7b, 0xd3, 0xda, 0x13, 0x76, 0x0a, 0xb6, 0x6c, 0x99, 0x53, 0x5d,
  0xa5, 0x75, 0xfa, 0x10, 0x9b, 0x7f, 0xfe, 0xd7, 0xb4, 0x18, 0x95, 0xa8,
  0x65, 0x85, 0x07, 0xc5, 0xc4, 0xad, 0x02, 0x81, 0x81, 0x00, 0xc2, 0xb8,
  0x8e, 0xed, 0x9d, 0x4a, 0x1f, 0x9c, 0xda, 0x73, 0xf0, 0x2c, 0x35, 0x91,
  0xe4, 0x40, 0x78, 0xe1, 0x12, 0xf3, 0x08, 0xef, 0xdf, 0x97, 0xa0, 0xb0,
  0xdd, 0xea, 0xc2, 0xb9, 0x5b, 0xf8, 0xa1, 0xac, 0x32, 0xfd, 0xb8, 0xe9,
  0x0f, 0xed, 0xfd, 0xe0, 0xdc, 0x38, 0x90, 0x5e, 0xf5, 0x4c, 0x02, 0xc3,
  0x1a, 0x72, 0x18, 0xf7, 0xfe, 0xb7, 0xb8, 0x2a, 0xf8, 0x72, 0xbb, 0x99,
  0x56, 0xec, 0x85, 0x58, 0x31, 0x7e, 0x64, 0xdf, 0x02, 0x05, 0xe3, 0xb2,
  0xbb, 0xe2, 0x1b, 0xd6, 0x43, 0x73, 0xf8, 0x0f, 0xaf, 0x89, 0x57, 0x44,
  0x5f, 0x30, 0x1c, 0xe5, 0x78, 0xbf, 0x0b, 0xe7, 0x4b, 0xbe, 0x80, 0x2f,
  0x3d, 0x35, 0x44, 0xfc, 0x9e, 0x0d, 0x85, 0x5d, 0x94, 0x6e, 0xe9, 0x6a,
  0x72, 0xa7, 0x46, 0xd8, 0x64, 0x6c, 0xe9, 0x61, 0x92, 0xa0, 0xb6, 0xd1,
  0xee, 0xa6, 0xa6, 0xf4, 0x2c, 0x29, 0x02, 0x81, 0x81, 0x00, 0xb4, 0xa7,
  0x7b, 0x1c, 0x64, 0x29, 0x29, 0xda, 0xca, 0x3e, 0xe3, 0xc1, 0x2a, 0x55,
  0x2f, 0xfd, 0x32, 0xb8, 0x4e, 0x99, 0xb6, 0x60, 0x4d, 0xfd, 0xba, 0x9a,
  0xe2, 0xcd, 0xa2, 0x63, 0xc2, 0x25, 0xa3, 0x42, 0x7e, 0x68, 0x4c, 0x9c,
  0x45, 0x09, 0x5d, 0xd5, 0x21, 0x9c, 0x01, 0x20, 0x6d, 0xf9, 0x75, 0xb8,
  0x4b, 0xcf, 0x8e, 0xd8, 0x29, 0xf3, 0xbf, 0xe6, 0xb3, 0x7a, 0x34, 0x87,
  0x58, 0xa1, 0x46, 0x33, 0xd9, 0xee, 0xa9, 0xcd, 0xac, 0xb8, 0xcf, 0x77,
  0xa0, 0x70, 0xc0, 0xb9, 0x0f, 0x41, 0xf0, 0x98, 0x43, 0xdb, 0xfa, 0x30,
  0x66, 0x44, 0xc5, 0xfa, 0xb2, 0xa4, 0x5a, 0x43, 0x79, 0x50, 0x48, 0xcb,
  0xe9, 0x49, 0x3f, 0x39, 0xee, 0x34, 0x40, 0xb1, 0x5d, 0x80, 0x96, 0x3c,
  0x54, 0xf4, 0x9c, 0xcb, 0x90, 0x7f, 0xba, 0x96, 0x4b, 0x39, 0x3e, 0xb5,
  0x03, 0xb5, 0xd1, 0x35, 0x72, 0xe1, 0x02, 0x81, 0x80, 0x60, 0x14, 0xd5,
  0x61, 0xe6, 0x24, 0xf7, 0x28, 0x5c, 0x9a, 0xac, 0xbe, 0x03, 0xc8, 0xf3,
  0x49, 0xe4, 0xdb, 0x9a, 0x90, 0x15, 0xae, 0xd7, 0x33, 0x68, 0x75, 0x1d,
  0x6b, 0x83, 0x9e, 0x17, 0x05, 0xbe, 0x30, 0xcc, 0x10, 0x6a, 0x37, 0x86,
  0x46, 0xb6, 0xe9, 0x47, 0x81, 0x19, 0xab, 0xe1, 0x7a, 0x1a, 0x3a, 0xcf,
  0x47, 0xd1, 0x8e, 0x3d, 0x3f, 0xc6, 0x3e, 0x5d, 0xcd, 0xaf, 0x47, 0xe0,
  0x9e, 0x60, 0xc5, 0xbd, 0xd6, 0x52, 0x4b, 0xc0, 0x21, 0xcb, 0xd3, 0x1b,
  0xe6, 0x5c, 0x3a, 0x03, 0x9a, 0xab, 0xa2, 0x81, 0xc9, 0x51, 0x28, 0x49,
  0x97, 0xe2, 0x0a, 0x50, 0xe4, 0x64, 0x29, 0x43, 0x34, 0xc2, 0xe7, 0x8c,
  0x5a, 0x46, 0xaa, 0x28, 0x0b, 0x1f, 0xed, 0xa7, 0x1a, 0x7b, 0x4e, 0xad,
  0x38, 0x61, 0x3a, 0xd1, 0x82, 0xf4, 0x3d, 0xd3, 0x2e, 0x3e, 0x47, 0xa4,
  0x6c, 0xd3, 0x20, 0xd4, 0xd1, 0x02, 0x81, 0x80, 0x68, 0x1a, 0x8d, 0x3c,
  0x18, 0x3f, 0x42, 0x5e, 0x38, 0x6d, 0x0a, 0x1e, 0x52, 0xd5, 0x8f, 0xd6,
  0x32, 0xff, 0x7c, 0x1c, 0xf3, 0x20, 0x8b, 0x92, 0xa5, 0x44, 0xff, 0x08,
  0x21, 0xa1, 0xce, 0x68, 0x8b, 0x03, 0xe0, 0x90, 0xeb, 0x01, 0x4e, 0x85,
  0xf9, 0xc5, 0xb7, 0x86, 0xee, 0xd0, 0x59, 0x10, 0x73, 0x98, 0x2a, 0xcb,
  0xf6, 0xfe, 0x0d, 0xba, 0x07, 0x91, 0x18, 0xf6, 0xbc, 0x93, 0x8a, 0x91,
  0xdd, 0x80, 0x16, 0x37, 0xdf, 0x75, 0x46, 0x87, 0x68, 0xee, 0xf4, 0x76,
  0x0c, 0xc5, 0x87, 0x38, 0xf5, 0xb6, 0xda, 0x8a, 0xee, 0x62, 0xc8, 0xc0,
  0xa2, 0x8d, 0xbf, 0xd5, 0xf8, 0xba, 0xb5, 0x74, 0xf0, 0x07, 0xa6, 0x1c,
  0xcf, 0x76, 0x61, 0xbe, 0xa4, 0x88, 0x4a, 0x95, 0xb0, 0xa3, 0x70, 0x73,
  0xa1, 0x6f, 0x73, 0xf0, 0xe8, 0x38, 0x8d, 0xe8, 0xd0, 0x7e, 0x2c, 0x0c,
  0xdc, 0x21, 0xfa, 0xc1
};

static unsigned int test_device_cert_key_der_len = 1192;

/* Trusted CA certificate for Client. */
static unsigned char test_ca_cert_der[] = {
  0x30, 0x82, 0x03, 0xc7, 0x30, 0x82, 0x02, 0xaf, 0xa0, 0x03, 0x02, 0x01,
  0x02, 0x02, 0x09, 0x00, 0xa1, 0x79, 0xb0, 0x6a, 0x32, 0xbc, 0x48, 0x67,
  0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01,
  0x0b, 0x05, 0x00, 0x30, 0x7a, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55,
  0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03,
  0x55, 0x04, 0x08, 0x0c, 0x02, 0x43, 0x41, 0x31, 0x12, 0x30, 0x10, 0x06,
  0x03, 0x55, 0x04, 0x07, 0x0c, 0x09, 0x53, 0x61, 0x6e, 0x20, 0x44, 0x69,
  0x65, 0x67, 0x6f, 0x31, 0x16, 0x30, 0x14, 0x06, 0x03, 0x55, 0x04, 0x0a,
  0x0c, 0x0d, 0x45, 0x78, 0x70, 0x72, 0x65, 0x73, 0x73, 0x20, 0x4c, 0x6f,
  0x67, 0x69, 0x63, 0x31, 0x14, 0x30, 0x12, 0x06, 0x03, 0x55, 0x04, 0x0b,
  0x0c, 0x0b, 0x4e, 0x65, 0x74, 0x58, 0x20, 0x53, 0x65, 0x63, 0x75, 0x72,
  0x65, 0x31, 0x1c, 0x30, 0x1a, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x13,
  0x4e, 0x65, 0x74, 0x58, 0x20, 0x53, 0x65, 0x63, 0x75, 0x72, 0x65, 0x20,
  0x54, 0x65, 0x73, 0x74, 0x20, 0x43, 0x41, 0x30, 0x1e, 0x17, 0x0d, 0x31,
  0x36, 0x31, 0x31, 0x31, 0x31, 0x31, 0x39, 0x35, 0x30, 0x30, 0x38, 0x5a,
  0x17, 0x0d, 0x32, 0x36, 0x31, 0x31, 0x30, 0x39, 0x31, 0x39, 0x35, 0x30,
  0x30, 0x38, 0x5a, 0x30, 0x7a, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55,
  0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03,
  0x55, 0x04, 0x08, 0x0c, 0x02, 0x43, 0x41, 0x31, 0x12, 0x30, 0x10, 0x06,
  0x03, 0x55, 0x04, 0x07, 0x0c, 0x09, 0x53, 0x61, 0x6e, 0x20, 0x44, 0x69,
  0x65, 0x67, 0x6f, 0x31, 0x16, 0x30, 0x14, 0x06, 0x03, 0x55, 0x04, 0x0a,
  0x0c, 0x0d, 0x45, 0x78, 0x70, 0x72, 0x65, 0x73, 0x73, 0x20, 0x4c, 0x6f,
  0x67, 0x69, 0x63, 0x31, 0x14, 0x30, 0x12, 0x06, 0x03, 0x55, 0x04, 0x0b,
  0x0c, 0x0b, 0x4e, 0x65, 0x74, 0x58, 0x20, 0x53, 0x65, 0x63, 0x75, 0x72,
  0x65, 0x31, 0x1c, 0x30, 0x1a, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x13,
  0x4e, 0x65, 0x74, 0x58, 0x20, 0x53, 0x65, 0x63, 0x75, 0x72, 0x65, 0x20,
  0x54, 0x65, 0x73, 0x74, 0x20, 0x43, 0x41, 0x30, 0x82, 0x01, 0x22, 0x30,
  0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01,
  0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00, 0x30, 0x82, 0x01, 0x0a, 0x02,
  0x82, 0x01, 0x01, 0x00, 0xd1, 0xdc, 0x3c, 0xe1, 0x1c, 0x7a, 0x3d, 0xb7,
  0x76, 0xcf, 0xab, 0xd7, 0x3c, 0x38, 0xb0, 0x81, 0xb6, 0x37, 0x52, 0xa3,
  0x3d, 0x6f, 0xcd, 0x89, 0xa6, 0xa2, 0xf3, 0xa8, 0xb0, 0x8d, 0xee, 0x0b,
  0x36, 0x94, 0x83, 0x0e, 0x7f, 0x39, 0x87, 0x6e, 0xee, 0x19, 0xe2, 0x1f,
  0x92, 0x3d, 0x01, 0x05, 0x4f, 0x11, 0xcd, 0xcb, 0xa0, 0x79, 0xfc, 0x9d,
  0x6e, 0x93, 0xb1, 0xb7, 0x03, 0xf3, 0xfe, 0xeb, 0x30, 0x67, 0x38, 0x85,
  0x28, 0xdf, 0x93, 0xdb, 0xcb, 0xcb, 0xb1, 0xbe, 0xd3, 0xe1, 0xc2, 0x7d,
  0x8d, 0xbb, 0x70, 0x76, 0x99, 0x08, 0x7c, 0x3f, 0x21, 0x2f, 0x37, 0x97,
  0xf7, 0xe8, 0x6e, 0x8c, 0x7e, 0xbc, 0x30, 0x5f, 0xbf, 0x32, 0x51, 0x1d,
  0x66, 0x76, 0xad, 0x39, 0xfc, 0x94, 0xd4, 0x65, 0xf6, 0xd2, 0x0b, 0x37,
  0xd3, 0x4a, 0xe6, 0xe1, 0xdf, 0x4a, 0x8f, 0x3b, 0x33, 0x16, 0xbe, 0xf7,
  0xd9, 0xbd, 0x73, 0x64, 0xdf, 0x34, 0xa3, 0x55, 0xe7, 0xac, 0xab, 0xa7,
  0xae, 0xc2, 0x20, 0x46, 0xc2, 0xd1, 0xe3, 0x25, 0x3a, 0x47, 0x68, 0x92,
  0xac, 0xd6, 0x12, 0xa4, 0x0a, 0xce, 0xdc, 0xe2, 0x24, 0x12, 0xee, 0xe1,
  0xb2, 0xcd, 0x09, 0xa8, 0xef, 0x36, 0xea, 0x76, 0xf9, 0xb6, 0x63, 0xaa,
  0xac, 0xdd, 0x46, 0x06, 0x6e, 0xd9, 0x1e, 0x08, 0xac, 0x57, 0x12, 0x6c,
  0x21, 0xef, 0x8e, 0xae, 0xf0, 0x27, 0xf1, 0x5c, 0x79, 0xb4, 0xb6, 0x26,
  0x92, 0x11, 0xda, 0xca, 0x80, 0x5e, 0x92, 0x4c, 0xb5, 0xd8, 0xb5, 0x84,
  0x95, 0xe3, 0xef, 0xbc, 0x7e, 0x7d, 0x68, 0x74, 0x4c, 0x34, 0x1a, 0x50,
  0x6d, 0x2d, 0x5f, 0x1b, 0x0e, 0xbe, 0xf5, 0xb4, 0xf1, 0x32, 0x16, 0x44,
  0x24, 0x7a, 0x0e, 0x4b, 0xcd, 0xfa, 0xa5, 0x03, 0x95, 0x2e, 0x44, 0x65,
  0xa8, 0x74, 0xea, 0x17, 0xdd, 0x99, 0xbd, 0xcb, 0x02, 0x03, 0x01, 0x00,
  0x01, 0xa3, 0x50, 0x30, 0x4e, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e,
  0x04, 0x16, 0x04, 0x14, 0x1b, 0x8d, 0x06, 0xd9, 0x6b, 0xad, 0xee, 0x82,
  0x24, 0x26, 0x55, 0x9a, 0x1b, 0x03, 0x44, 0x92, 0x0a, 0x06, 0x92, 0x48,
  0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80,
  0x14, 0x1b, 0x8d, 0x06, 0xd9, 0x6b, 0xad, 0xee, 0x82, 0x24, 0x26, 0x55,
  0x9a, 0x1b, 0x03, 0x44, 0x92, 0x0a, 0x06, 0x92, 0x48, 0x30, 0x0c, 0x06,
  0x03, 0x55, 0x1d, 0x13, 0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xff, 0x30,
  0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b,
  0x05, 0x00, 0x03, 0x82, 0x01, 0x01, 0x00, 0x70, 0xc7, 0x6a, 0x75, 0x27,
  0x14, 0xa0, 0x1c, 0xe0, 0xe0, 0x84, 0x7c, 0x6c, 0x16, 0xa9, 0x0d, 0x4d,
  0xb1, 0xc3, 0x87, 0x37, 0xf6, 0x86, 0x89, 0x6f, 0x73, 0xf0, 0x59, 0x9b,
  0x8c, 0xa4, 0x83, 0x10, 0x2d, 0xb7, 0x8b, 0xd0, 0x9a, 0x81, 0xe0, 0x5c,
  0xd7, 0x20, 0x6f, 0xdc, 0xfc, 0xc8, 0xa0, 0xc2, 0x8e, 0x54, 0xe6, 0xfb,
  0x61, 0x85, 0x37, 0x4b, 0x22, 0x47, 0x09, 0x95, 0x44, 0x12, 0x75, 0xf0,
  0xcf, 0x0b, 0x90, 0x48, 0xb0, 0x02, 0x4c, 0xef, 0x3f, 0xde, 0x6a, 0xfd,
  0xb1, 0x8b, 0x88, 0xd7, 0x84, 0xe5, 0x34, 0x02, 0x96, 0x0a, 0x3f, 0xa8,
  0x8c, 0xbd, 0x1a, 0xd8, 0xf7, 0xf9, 0xe5, 0x49, 0x87, 0xd0, 0x20, 0x4f,
  0xd8, 0xcd, 0xc0, 0xb9, 0x11, 0x2a, 0xd9, 0x0f, 0x75, 0xa6, 0xee, 0x76,
  0x15, 0x9f, 0x12, 0x50, 0x68, 0x4c, 0xc0, 0x05, 0x46, 0x8d, 0xdd, 0x93,
  0x74, 0x31, 0x82, 0x20, 0x37, 0x24, 0x58, 0xb2, 0x88, 0x9b, 0x21, 0xc1,
  0x48, 0xc4, 0x8d, 0x68, 0x3b, 0x91, 0x2c, 0x34, 0xcb, 0x94, 0xd0, 0xbc,
  0xe3, 0x05, 0x24, 0x05, 0xcc, 0xea, 0x05, 0xb1, 0x52, 0x74, 0x4a, 0x23,
  0x65, 0xc4, 0x40, 0x04, 0x86, 0xb1, 0x80, 0x61, 0x97, 0xdc, 0x94, 0x16,
  0x4e, 0x63, 0x31, 0x72, 0x4e, 0x45, 0xe8, 0x3e, 0x3b, 0xb6, 0x99, 0xae,
  0xd8, 0x91, 0x25, 0x3d, 0x62, 0x92, 0x6d, 0x72, 0x01, 0x2c, 0xca, 0x67,
  0x0a, 0xec, 0x00, 0xeb, 0x10, 0xff, 0x6d, 0xac, 0x89, 0x19, 0x2c, 0xb7,
  0xb3, 0xa5, 0xf7, 0xa1, 0x4a, 0xc3, 0xc1, 0xdd, 0xaf, 0xb5, 0x1a, 0x16,
  0x44, 0xdc, 0xa8, 0xb5, 0xca, 0xd0, 0x30, 0xaa, 0x7e, 0x73, 0xd5, 0x2e,
  0x65, 0xd6, 0xf9, 0xbf, 0x5f, 0xda, 0x6f, 0x13, 0xe9, 0xd7, 0x12, 0x6c,
  0x3a, 0x6c, 0x50, 0x26, 0x78, 0x6e, 0xc6, 0xeb, 0x75, 0xe1, 0x3c
};
static unsigned int test_ca_cert_der_len = 971;

/* Define an request to be sent to the TLS server. */
UCHAR http_request[] = { "GET /example.html HTTP/1.1" };

/* Define some HTML data (web page) with an HTTPS header to serve to connecting
   clients. */
UCHAR html_data[] = { "HTTP/1.1 200 OK\r\n" \
        "Date: Tue, 19 May 2020 23:59:59 GMT\r\n" \
        "Content-Type: text/html\r\n" \
        "Content-Length: 200\r\n\r\n" \
        "<html>\r\n"\
        "<body>\r\n"\
        "<b>Hello NetX Secure User!</b>\r\n"\
        "This is a simple webpage\r\n"\
        "served up using NetX Secure!\r\n"\
        "</body>\r\n"\
        "</html>\r\n" };

/* Define the metadata area for TLS cryptography. The actual size needed can be
   Ascertained by calling nx_secure_tls_metadata_size_calculate.
*/
static CHAR crypto_metadata_server[18000];
static CHAR crypto_metadata_client[18000];

/* TLS buffers and certificate containers. */
static UCHAR tls_packet_buffer_server[40000];
static UCHAR tls_packet_buffer_client[40000];
static NX_SECURE_X509_CERT certificate;
static NX_SECURE_X509_CERT remote_certificate, remote_issuer;
static UCHAR remote_cert_buffer[2000];
static UCHAR remote_issuer_buffer[2000];

/* Pointer to the TLS ciphersuite table that is included in the platform-specific
   cryptography subdirectory. The table maps the cryptographic routines for the
   platform to function pointers usable by the TLS library.
   
   For TLS Web servers, define NX_SECURE_ENABLE_AEAD_CIPHER in NetX Crypto to 
   allow web browsers to connect using AES_128_GCM cipher suites.
*/
extern const NX_SECURE_TLS_CRYPTO nx_crypto_tls_ciphers_ecc;
extern const USHORT nx_crypto_ecc_supported_groups[];
extern const NX_CRYPTO_METHOD* nx_crypto_ecc_curves[];
extern const UINT nx_crypto_ecc_supported_groups_size;

/* Local IP address. */
#define TLS_SERVER_ADDRESS  IP_ADDRESS(192, 168, 1, 160)
#define TLS_CLIENT_ADDRESS  IP_ADDRESS(192, 168, 1, 167)

/* Define the server port.*/
#define SERVER_PORT         443

int main()
{

    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();
}


/* Define what the initial system looks like.  */
void    tx_application_define(void *first_unused_memory)
{
CHAR   *pointer;
UINT    status;


    error_counter = 0;

    /* Setup the working pointer.  */
    pointer =  (CHAR *) first_unused_memory;

    /* Create a helper thread for the server. */
    tx_thread_create(&server_thread, "TLS Server thread", tls_server_thread_entry, 0,  
                     pointer, DEMO_STACK_SIZE, 
                     4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);

    pointer =  pointer + DEMO_STACK_SIZE;

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create the server packet pool.  */
    status =  nx_packet_pool_create(&server_pool, "TLS Server Packet Pool", NX_PACKET_SIZE,
                                    pointer, NX_PACKET_SIZE * 16);
    pointer = pointer + NX_PACKET_SIZE * 16;
    if (status)
        error_counter++;

    /* Create an IP instance.  */
    status = nx_ip_create(&server_ip, "TLS Server IP", TLS_SERVER_ADDRESS, 
                          0xFFFFFF00UL, &server_pool, _nx_ram_network_driver,
                          pointer, 4096, 1);
    pointer =  pointer + 4096;
    if (status)
        error_counter++;

    /* Enable ARP and supply ARP cache memory for the server IP instance.  */
    status = nx_arp_enable(&server_ip, (void *) pointer, 1024);
    pointer = pointer + 1024;
    if (status)
        error_counter++;

     /* Enable TCP traffic.  */
    status = nx_tcp_enable(&server_ip);
    if (status)
        error_counter++;

    /* Create the TLS Client thread. */
    status = tx_thread_create(&client_thread, "TLS Client", tls_client_thread_entry, 0,
                              pointer, DEMO_STACK_SIZE, 
                              6, 6, TX_NO_TIME_SLICE, TX_AUTO_START);
    pointer =  pointer + DEMO_STACK_SIZE;
    if (status)
        error_counter++;

    /* Create the Client packet pool.  */
    status =  nx_packet_pool_create(&client_pool, "TLS Client Packet Pool", NX_PACKET_SIZE,
                                    pointer, NX_PACKET_SIZE * 16);
    pointer = pointer + NX_PACKET_SIZE * 16;
    if (status)
        error_counter++;

    /* Create an IP instance.  */
    status = nx_ip_create(&client_ip, "TLS Client IP", TLS_CLIENT_ADDRESS, 
                          0xFFFFFF00UL, &client_pool, _nx_ram_network_driver,
                          pointer, 2048, 1);
    pointer =  pointer + 2048;
    if (status)
        error_counter++;

    status  = nx_arp_enable(&client_ip, (void *) pointer, 1024);
    pointer =  pointer + 2048;
    if (status)
        error_counter++;

     /* Enable TCP traffic.  */
    status = nx_tcp_enable(&client_ip);
    if (status)
        error_counter++;
}

/* Thread entry for the TLS Client demo. */
VOID tls_client_thread_entry(ULONG thread_input)
{
UINT       status;
ULONG      actual_status;
NX_PACKET *send_packet;
NX_PACKET *receive_packet;
UCHAR      receive_buffer[200];
ULONG      bytes;
ULONG      server_ipv4_address;

    /* We are not using the thread input parameter so suppress compiler warning. */
    NX_PARAMETER_NOT_USED(thread_input);

    /* Ensure the IP instance has been initialized.  */
    status = nx_ip_status_check(&client_ip, NX_IP_INITIALIZE_DONE, &actual_status,
                                NX_IP_PERIODIC_RATE);
    if (status)
        error_counter++;

    /* Create a TCP socket to use for our TLS session.  */
    status = nx_tcp_socket_create(&client_ip, &client_tcp_socket, "TLS Client Socket",
                                   NX_IP_NORMAL, NX_FRAGMENT_OKAY,
                                   NX_IP_TIME_TO_LIVE, 8192, NX_NULL, NX_NULL);
    if (status)
        error_counter++;

    /* Create a TLS session for our socket. This sets up the TLS session object for
       later use */
    status = nx_secure_tls_session_create(&client_tls_session,
                                          &nx_crypto_tls_ciphers_ecc,
                                          crypto_metadata_client,
                                          sizeof(crypto_metadata_client));
    if (status)
        error_counter++;

    /* Initialize ECC parameters for this session. */
    status = nx_secure_tls_ecc_initialize(&client_tls_session,
                                          nx_crypto_ecc_supported_groups,
                                          nx_crypto_ecc_supported_groups_size,
                                          nx_crypto_ecc_curves);
    if (status)
        error_counter++;

    /* Set the packet reassembly buffer for this TLS session. */
    status = nx_secure_tls_session_packet_buffer_set(&client_tls_session, tls_packet_buffer_client,
                                                     sizeof(tls_packet_buffer_client));
    if (status)
        error_counter++;

    /* Initialize an X.509 certificate with our CA root certificate data. */
    status = nx_secure_x509_certificate_initialize(&trusted_certificate, test_ca_cert_der,
                                                   test_ca_cert_der_len, NX_NULL, 0, NX_NULL, 0,
                                                   NX_SECURE_X509_KEY_TYPE_NONE);
    if (status)
        error_counter++;

    /* Add the initialized certificate as a trusted root certificate. */
    status = nx_secure_tls_trusted_certificate_add(&client_tls_session, &trusted_certificate);
    if (status)
        error_counter++;

    /* Bind the socket.  */
    status = nx_tcp_client_socket_bind(&client_tcp_socket, 0, NX_WAIT_FOREVER);
    if (status)
        error_counter++;

    /* Setup this thread to open a connection on the TCP socket to a remote server.
       The IP address can be used directly or it can be obtained via DNS or other
       means.*/
    server_ipv4_address = TLS_SERVER_ADDRESS;
    status = nx_tcp_client_socket_connect(&client_tcp_socket, server_ipv4_address,
                                          SERVER_PORT, NX_WAIT_FOREVER);
    if (status)
    {
        error_counter++;
        return;
    }

    /* Start the TLS Session using the connected TCP socket. This function will
       ascertain from the TCP socket state that this is a TLS Client session. */
    status = nx_secure_tls_session_start(&client_tls_session, &client_tcp_socket,
                                         NX_WAIT_FOREVER);
    if (status)
    {
        error_counter++;
        return;
    }

    /* Allocate a TLS packet to send an HTTP request over TLS (HTTPS). */
    status = nx_secure_tls_packet_allocate(&client_tls_session, &client_pool, &send_packet,
                                           NX_WAIT_FOREVER);
    if (status)
        error_counter++;

    /* Populate the packet with our HTTP request. */
    status = nx_packet_data_append(send_packet, http_request, sizeof(http_request), &client_pool,
                                   NX_WAIT_FOREVER);
    if (status)
        error_counter++;

    /* Send the HTTP request over the TLS Session, turning it into HTTPS. */
    status = nx_secure_tls_session_send(&client_tls_session, send_packet, NX_WAIT_FOREVER);

    /* If the send fails, you must release the packet.  */
    if (status != NX_SUCCESS)
    {

        /* Release the packet since the packet was not sent.  */
        nx_packet_release(send_packet);
    }

    /* Receive the HTTP response and any data from the server. */
    status = nx_secure_tls_session_receive(&client_tls_session, &receive_packet,
                                           NX_WAIT_FOREVER);
    if (status == NX_SUCCESS)
    {

        /* Extract the data we received from the remote server. */
        status = nx_packet_data_extract_offset(receive_packet, 0, receive_buffer,
                                               sizeof(receive_buffer) - 1,  &bytes);

        /* Display the response data. */
        receive_buffer[bytes] = 0;
        printf("Received data: %s\n", receive_buffer);

        /* Release the packet when done with it. */
        nx_packet_release(receive_packet);
    }

    /* End the TLS session now that we have received our HTTPS/HTML response. */
    status = nx_secure_tls_session_end(&client_tls_session, NX_WAIT_FOREVER);
    if (status)
        error_counter++;

    /* Check for errors to make sure the session ended cleanly. */

    /* Disconnect the TCP socket. */
    status = nx_tcp_socket_disconnect(&client_tcp_socket, NX_WAIT_FOREVER);

}

/************* TLS Server *************************/

/* Define the TLS Server thread.  */
void    tls_server_thread_entry(ULONG thread_input)
{
UINT       status;
ULONG      actual_status;
NX_PACKET *send_packet;
NX_PACKET *receive_packet;
UCHAR      receive_buffer[100];
ULONG      bytes;

    NX_PARAMETER_NOT_USED(thread_input);

    /* Ensure the IP instance has been initialized.  */
    status = nx_ip_status_check(&server_ip, NX_IP_INITIALIZE_DONE, &actual_status,
                                NX_IP_PERIODIC_RATE);
    if (status)
        error_counter++;

    /* Create a TCP socket to use for our TLS session.  */
    status = nx_tcp_socket_create(&server_ip, &server_tcp_socket, "TLS Server Socket",
                                  NX_IP_NORMAL, NX_FRAGMENT_OKAY,
                                  NX_IP_TIME_TO_LIVE, 8192, NX_NULL, NX_NULL);
    if (status)
        error_counter++;

    /* Create a TLS session for our socket.  */
    status = nx_secure_tls_session_create(&server_tls_session,
                                          &nx_crypto_tls_ciphers_ecc,
                                          crypto_metadata_server,
                                          sizeof(crypto_metadata_server));
    if (status)
        error_counter++;

    status = nx_secure_tls_ecc_initialize(&server_tls_session,
                                          nx_crypto_ecc_supported_groups,
                                          nx_crypto_ecc_supported_groups_size,
                                          nx_crypto_ecc_curves);
    if (status)
        error_counter++;

    /* Set the packet reassembly buffer for this TLS session. */
    status = nx_secure_tls_session_packet_buffer_set(&server_tls_session, tls_packet_buffer_server,
                                                     sizeof(tls_packet_buffer_server));
    if (status)
        error_counter++;

    /* Initialize an X.509 certificate and private ECC key for our TLS Session. */
    status = nx_secure_x509_certificate_initialize(&certificate, test_device_cert_der, test_device_cert_der_len, NX_NULL, 0,
                                                   test_device_cert_key_der, test_device_cert_key_der_len,
                                                   NX_SECURE_X509_KEY_TYPE_RSA_PKCS1_DER);
    if (status)
        error_counter++;

    /* Add the initialized certificate as a local identity certificate. */
    status = nx_secure_tls_local_certificate_add(&server_tls_session, &certificate);
    if (status)
        error_counter++;

    /* Setup this thread to listen on the TCP socket. */
    status = nx_tcp_server_socket_listen(&server_ip, SERVER_PORT, &server_tcp_socket, 5, NX_NULL);
    if (status)
        error_counter++;

    while(1)
    {

        /* Accept a client TCP socket connection.  */
        status = nx_tcp_server_socket_accept(&server_tcp_socket, NX_WAIT_FOREVER);
        if (status)
        {
            printf("Socket accept failure: %x\n", status);
            error_counter++;

            /* Unaccept the server socket.  */
            nx_tcp_server_socket_unaccept(&server_tcp_socket);

            /* Setup server socket for listening again.  */
            nx_tcp_server_socket_relisten(&server_ip, SERVER_PORT, &server_tcp_socket);

            continue;
        }

        /* Start the TLS Session using the connected TCP socket. */
        status = nx_secure_tls_session_start(&server_tls_session, &server_tcp_socket,
                                             NX_WAIT_FOREVER);

        if (status == NX_SUCCESS)
        {
            /* Receive the HTTPS request. */
            status = nx_secure_tls_session_receive(&server_tls_session, &receive_packet,
                                                   NX_WAIT_FOREVER);

            if (status == NX_SUCCESS)
            {
                /* Extract the HTTP request information from the HTTPS request. */
                status = nx_packet_data_extract_offset(receive_packet, 0, receive_buffer,
                                                       sizeof(receive_buffer) - 1, &bytes);
                if (status)
                    error_counter++;

                /* Display the HTTP request data. */
                receive_buffer[bytes] = 0;
                printf("Received data: %s\n", receive_buffer);

                /* Release the packet when done with it */
                nx_packet_release(receive_packet);
            }

            /* Allocate a TLS packet to send HTML data back to client. */
            status = nx_secure_tls_packet_allocate(&server_tls_session, &server_pool, &send_packet,
                                                   NX_WAIT_FOREVER);
            if (status)
                error_counter++;

            /* Populate the packet with our HTTP response and HTML web page data. */
            status = nx_packet_data_append(send_packet, html_data, sizeof(html_data), &server_pool,
                                           NX_WAIT_FOREVER);
            if (status)
                error_counter++;

            /* Send the HTTP response over the TLS Session, turning it into HTTPS. */
            status = nx_secure_tls_session_send(&server_tls_session, send_packet,
                                                NX_WAIT_FOREVER);

            /* If the send fails, you must release the packet.  */
            if (status != NX_SUCCESS)
            {
                /* Release the packet since it was not sent.  */
                nx_packet_release(send_packet);
                error_counter++;
            }

        }

        /* End the TLS session now that we have sent our HTTPS/HTML response. */
        status = nx_secure_tls_session_end(&server_tls_session, NX_WAIT_FOREVER);

        /* Check for errors to make sure the session ended cleanly! */
        if (status)
            error_counter++;

        /* Disconnect the TCP socket so we can be ready for the next request. */
        status = nx_tcp_socket_disconnect(&server_tcp_socket, NX_WAIT_FOREVER);
        if (status)
            error_counter++;

        /* Unaccept the server socket.  */
        status = nx_tcp_server_socket_unaccept(&server_tcp_socket);
        if (status)
            error_counter++;

        /* Setup server socket for listening again.  */
        status = nx_tcp_server_socket_relisten(&server_ip, SERVER_PORT, &server_tcp_socket);
        if (status)
            error_counter++;

    }

}

#endif /* NX_DISABLE_IPV4  */
