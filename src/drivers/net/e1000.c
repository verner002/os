/**
 * @file e1000.c
 * @author verner002
 * @date 24/09/2025
*/

#include "bool.h"
#include "macros.h"
#include "kstdlib/stdio.h"
#include "drivers/ports.h"
#include "drivers/bus/pci.h"
#include "drivers/net/e1000.h"
#include "mm/vmm.h"
#include "drivers/cpu.h"

#define IPV4(a, b, c, d) ((uint32_t)((a << 24) | (b << 16) | (c << 8) | (d << 0)))

#define REG_CTRL                        0x0000
#define REG_STATUS                      0x0008
#define REG_EEPROM                      0x0014
#define REG_CTRL_EXT                    0x0018
#define REG_ICR                         0x00c0
#define REG_IMASK                       0x00d0
#define REG_RCTRL                       0x0100
#define REG_RXDESCLO                    0x2800
#define REG_RXDESCHI                    0x2804
#define REG_RXDESCLEN                   0x2808
#define REG_RXDESCHEAD                  0x2810
#define REG_RXDESCTAIL                  0x2818

#define REG_TCTRL                       0x0400
#define REG_TXDESCLO                    0x3800
#define REG_TXDESCHI                    0x3804
#define REG_TXDESCLEN                   0x3808
#define REG_TXDESCHEAD                  0x3810
#define REG_TXDESCTAIL                  0x3818

#define REG_RDTR                        0x2820      // RX delay timer register
#define REG_RXDCTL                      0x2828      // RX descriptor control
#define REG_RADV                        0x282c      // RX int. absolute delay timer
#define REG_RSRPD                       0x2c00      // RX small packet detect interrupt

#define REG_TIPG                        0x0410      // transmit inter-packet gap
#define ECTRL_SLU                       0x40        // set link up

#define RCTRL_EN                        (1 << 1)    // receiver enable
#define RCTRL_SBP                       (1 << 2)    // store bad packets
#define RCTRL_UPE                       (1 << 3)    // unicast promiscuous enabled
#define RCTRL_MPE                       (1 << 4)    // multicast promiscuous enabled
#define RCTRL_LPE                       (1 << 5)    // long packet reception enable
#define RCTRL_LBM_NONE                  (0 << 6)    // no loopback
#define RCTRL_LBM_PHY                   (3 << 6)    // PHY or external serdesc loopback
#define RTCL_RDMTS_HALF                 (0 << 8)    // free buffer threshold is 1/2 of RDLEN
#define RTCL_RDMTS_QUARTER              (1 << 8)    // free buffer threshold is 1/4 of RDLEN
#define RTCL_RDMTS_EIGHTH               (2 << 8)    // free buffer threshold is 1/8 of RDLEN
#define RCTRL_MO_36                     (0 << 12)   // multicast offset - bits 47:36
#define RCTRL_MO_35                     (1 << 12)   // multicast offset - bits 46:35
#define RCTRL_MO_34                     (2 << 12)   // multicast offset - bits 45:34
#define RCTRL_MO_32                     (3 << 12)   // multicast offset - bits 43:32
#define RCTRL_BAM                       (1 << 15)   // broadcast accept mode
#define RCTRL_VFE                       (1 << 18)   // VLAN filter enable
#define RCTRL_CFIEN                     (1 << 19)   // canonical form indicator enable
#define RCTRL_CFI                       (1 << 20)   // canonical form indicator bit value
#define RCTRL_DPF                       (1 << 22)   // discard pause frames
#define RCTRL_PMCF                      (1 << 23)   // pass MAC control frames
#define RCTRL_SECRC                     (1 << 26)   // strip ethernet CRC

#define RCTRL_BSIZE_256                 (3 << 16)
#define RCTRL_BSIZE_512                 (2 << 16)
#define RCTRL_BSIZE_1024                (1 << 16)
#define RCTRL_BSIZE_2048                (0 << 16)
#define RCTRL_BSIZE_4096                ((3 << 16) | (1 << 25))
#define RCTRL_BSIZE_8192                ((2 << 16) | (1 << 25))
#define RCTRL_BSIZE_16384               ((1 << 16) | (1 << 25))

#define CMD_EOP                         (1 << 0)    // end of packet
#define CMD_IFCS                        (1 << 1)    // insert FCS
#define CMD_IC                          (1 << 2)    // insert checksum
#define CMD_RS                          (1 << 3)    // report status
#define CMD_RPS                         (1 << 4)    // report packet sent
#define CMD_VLE                         (1 << 6)    // VLAN packet enable
#define CMD_IDE                         (1 << 7)    // interrupt delay enable

#define TCTRL_EN                        (1 << 1)    // transmit enable
#define TCTRL_PSP                       (1 << 3)    // pad short packets
#define TCTRL_CT_SHIFT                  4           // collision threshold
#define TCTRL_COLD_SHIFT                12          // collision distance
#define TCTRL_SWXOFF                    (1 << 22)   // software XOFF transmission
#define TCTRL_RTLC                      (1 << 24)   // re-transmit on late collision

#define TSTA_DD                         (1 << 0)    // descriptor done
#define TSTA_EC                         (1 << 1)    // excess collisions
#define TSTA_LC                         (1 << 2)    // late collision
#define LSTA_TU                         (1 << 3)    // transmit underrun

#define E1000_NUM_RX_DESC 32
#define E1000_NUM_TX_DESC 8

static bool init = FALSE;

struct {
    uint8_t irq;
    uint32_t cmt;
    uint32_t base;
    bool eeprom_present;
    uint8_t mac[6];
    uint8_t ipv4[4];
} e1000;

struct __attribute__((__packed__)) __e1000_rx_desc {
    volatile uint32_t addr_lo;
    volatile uint32_t addr_hi;
    volatile uint16_t length;
    volatile uint16_t checksum;
    volatile uint8_t status;
    volatile uint8_t errors;
    volatile uint16_t special;
};

struct __attribute__((__packed__)) __e1000_tx_desc {
    volatile uint32_t addr_lo;
    volatile uint32_t addr_hi;
    volatile uint16_t length;
    volatile uint8_t cso;
    volatile uint8_t cmd;
    volatile uint8_t status;
    volatile uint8_t css;
    volatile uint16_t special;
};

struct __attribute__((__packed__)) __e1000_eth_frame {
    //uint8_t preamble[8];
    uint8_t dst[6]; // destination hardware address
    uint8_t src[6]; // source hardware address
    uint16_t type; // type
};

struct __attribute__((__packed__)) __e1000_arp_packet {
    uint16_t htype; // hardware type
    uint16_t ptype; // protocol type
    uint8_t  hlen; // hardware address length (6 bytes)
    uint8_t  plen; // Protocol address length (4 bytes)
    uint16_t opcode; // ARP Operation Code
    uint8_t  srchw[6]; // source hardware address
    uint8_t  srcpr[4]; // source protocol address
    uint8_t  dsthw[6]; // destination hardware address
    uint8_t  dstpr[4]; // destination protocol address
};

struct __ipv4_header {
    uint8_t vlen; // version & length
    uint8_t service;
    uint16_t dlen; // datagram length
    uint16_t id;
    uint16_t ffragmentoff; // flags & fragment offset
    uint8_t ttl; // time to live
    uint8_t protocol;
    uint16_t checksum;
    uint8_t src[4];
    uint8_t dst[4];
};

struct __attribute__((__packed__)) __icmp_packet {
    uint8_t icmp_type;
    uint8_t icmp_code;
    uint16_t checksum;
};

/**
 * __e1000_write
*/

void __e1000_write(uint16_t address, uint32_t data) {
    switch (e1000.cmt) {
        case 0:
            __outd(e1000.base, address);
            __outd(e1000.base + 4, data);
            break;

        case 1:
            *(uint32_t *)(e1000.base + address) = data;
            break;
    }
}

/**
 * __e1000_read
*/

uint32_t __e1000_read(uint16_t address) {
    switch (e1000.cmt) {
        case 0:
            __outd(e1000.base, address);
            return __ind(e1000.base + 4);

        case 1:
            return *(uint32_t *)(e1000.base + address);
    }

    return 0;
}

/**
 * __e1000_eeprom_read
*/

uint16_t __e1000_eeprom_read(uint8_t address) {
    uint32_t data;
    
    if (e1000.eeprom_present) {
        __e1000_write(REG_EEPROM, ((uint32_t)address << 8) | 1);

        while (!(data = __e1000_read(REG_EEPROM) & (1 << 4)));

        printk("%p: %08x\n", address, data);
    } else {
        __e1000_write(REG_EEPROM, ((uint32_t)address << 2) | 1);

        while (!(data = __e1000_read(REG_EEPROM) & (1 << 1)));
    }

    return (uint16_t)(data >> 16);
}

/**
 * __e1000_print_mac
*/

void __e1000_print_mac(uint8_t *mac) {
    for (uint32_t i = 0; i < 5; ++i)
        printf("%02x:", mac[i]);

    printf("%02x", mac[5]);
}

/**
 * __e1000_print_ipv4
*/

void __e1000_print_ipv4(uint8_t *ipv4) {
    for (uint32_t i = 0; i < 3; ++i)
        printf("%u.", ipv4[i]);

    printf("%u", ipv4[3]);
}

static struct __e1000_rx_desc *rx_descs;
static struct __e1000_tx_desc *tx_descs;
static uint32_t curr_rx_i = 0;
static uint32_t curr_tx_i = 0;

/**
 * __e1000_handler
*/

__attribute__((interrupt)) void __e1000_handler(INTERRUPT_FRAME *frame) {
    uint32_t status = __e1000_read(REG_ICR);

    if (status & 0x04)
        printk("e1000: link status changed to %s\n", (__e1000_read(REG_STATUS) & (1 << 1)) ? "up" : "down");
    else if (status & 0x10) {
        // good threshold
    } else if (status & 0x80) {
        while (rx_descs[curr_rx_i].status & 0x01) {
            struct __e1000_eth_frame *frame = (struct __e1000_eth_frame *)rx_descs[curr_rx_i].addr_lo;

            uint16_t type_length = swap_bytes16(frame->type);

            if (type_length < 1500) {
                printk("length     : %u byte(s)\n", type_length);
            } else if (type_length >= 1536) {
                switch (type_length) {
                    case 0x0806: {
                        struct __e1000_arp_packet *arp = (struct __e1000_arp_packet *)((void *)frame + sizeof(struct __e1000_eth_frame));
                        uint16_t ptype = swap_bytes16(arp->ptype);
                        
                        uint32_t i = 0;

                        for (; i < 4; ++i)
                            if (arp->dstpr[i] != e1000.ipv4[i])
                                break;

                        if (i < 4)
                            break;

                        struct __e1000_eth_frame *rframe = (struct __e1000_eth_frame *)kmalloc(sizeof(struct __e1000_eth_frame) + sizeof(struct __e1000_arp_packet));

                        if (!rframe) {
                            printk("failed to allocate memory for ethernet reply frame\n");
                            break;
                        }

                        struct __e1000_arp_packet *rarp = (struct __e1000_arp_packet *)((void *)rframe + sizeof(struct __e1000_eth_frame));

                        if (!rarp) {
                            printk("failed to allocate memory for arp reply packet\n");
                            break;
                        }

                        rarp->htype = swap_bytes16(0x0001); // eth type
                        rarp->ptype = swap_bytes16(0x0800); // protocol type
                        rarp->hlen = 6;
                        rarp->plen = 4;
                        rarp->opcode = swap_bytes16(0x0002); // reply opcode
                        memcpy(rarp->dsthw, arp->srchw, 6);
                        memcpy(rarp->srchw, e1000.mac, 6);
                        memcpy(rarp->dstpr, arp->srcpr, 4);
                        memcpy(rarp->srcpr, e1000.ipv4, 4);

                        memcpy(rframe->dst, arp->srchw, 6);
                        memcpy(rframe->src, e1000.mac, 6);
                        rframe->type = swap_bytes16(0x0806); // arp packet

                        __e1000_send(rframe, sizeof(struct __e1000_eth_frame) + sizeof(struct __e1000_arp_packet));
                        kfree(rframe); // packet sent
                        break;
                    }

                    case 0x0800: {
                        struct __ipv4_header *ipv4 = (struct __ipv4_header *)((void *)frame + sizeof(struct __e1000_eth_frame));
                        
                        uint32_t i = 0;

                        for (; i < 4; ++i)
                            if (ipv4->dst[i] != e1000.ipv4[i])
                                break;

                        if (i < 4)
                            break;

                        if (ipv4->protocol == 0x01) {
                            // 0x01 = icmp protocol
                            struct __icmp_packet *icmp = (struct __icmp_packet *)((void *)ipv4 + sizeof(struct __ipv4_header));

                            switch (icmp->icmp_type) {
                                case 0x00: // icmp reply
                                    break;
                                
                                case 0x08: { // icmp request
                                    uint32_t length = rx_descs[curr_rx_i].length;

                                    struct __e1000_eth_frame *reply_frame = (struct __e1000_eth_frame *)kmalloc(length);
                                    memcpy(reply_frame, frame, length);
                                    memcpy(reply_frame->dst, frame->src, 6);
                                    memcpy(reply_frame->src, e1000.mac, 6);
                                    reply_frame->type = frame->type; // icmp packet
                                    struct __ipv4_header *reply_ipv4 = (struct __ipv4_header *)((void *)reply_frame + sizeof(struct __e1000_eth_frame));
                                    memcpy(reply_ipv4->dst, ipv4->src, 4);
                                    memcpy(reply_ipv4->src, e1000.ipv4, 4);
                                    //reply_ipv4->checksum
                                    struct __icmp_packet *reply_icmp = (struct __icmp_packet *)((void *)reply_ipv4 + sizeof(struct __ipv4_header));
                                    reply_icmp->icmp_type = 0x0000;
                                    
                                    reply_icmp->checksum = 0; // zero-out checksum
                                    uint32_t icmp_checksum = 0;

                                    uint32_t icmp_length_bytes = length - sizeof(struct __e1000_eth_frame) - sizeof(struct __ipv4_header);
                                    uint32_t icmp_length_words = (icmp_length_bytes) / 2;
                                    
                                    // binary addition
                                    for (uint32_t i = 0; i < icmp_length_words; ++i)    
                                        icmp_checksum += swap_bytes16(((uint16_t *)reply_icmp)[i]);
                                        
                                    if (icmp_length_bytes & 1)
                                        icmp_checksum += swap_bytes16(*(uint8_t *)((uint16_t *)reply_icmp + icmp_length_bytes));

                                    // end around carry
                                    while (icmp_checksum >> 16)
                                        icmp_checksum = (icmp_checksum & 0xffff) + (icmp_checksum >> 16);

                                    reply_icmp->checksum = swap_bytes16(~(uint16_t)icmp_checksum);
                                    __e1000_send(reply_frame, length);
                                    kfree(reply_frame);
                                    break;
                                }
                            }
                        }
                        break;
                    }

                    case 0x86dd:
                        //printk("ipv6 packet\n");
                        break;
                }
            }

            rx_descs[curr_rx_i].status = 0;
            uint32_t temp = curr_rx_i;
            curr_rx_i = (curr_rx_i + 1) % E1000_NUM_RX_DESC;
            __e1000_write(REG_RXDESCTAIL, temp);
        }
    }

    __send_eoi(e1000.irq);
}

/**
 * __e1000_link_up
*/

void __e1000_link_up(void) {
    // disable auto-negotiation
    /*uint32_t txcwr = __e1000_read(0x0178);
    __e1000_write(0x0178, txcwr & ~(1 << 31));*/

    uint32_t ctrl = __e1000_read(REG_CTRL);
    ctrl |= (1 << 5); // ASDE (auto speed detection)
    ctrl |= (1 << 6); // SLU (set link up)
    __e1000_write(REG_CTRL, ctrl);

    // RCTRL - receive enable
    uint32_t rctrl = __e1000_read(REG_RCTRL);
    rctrl |= (1 << 1);
    __e1000_write(REG_RCTRL, rctrl); // receiver enable

    // TCTRL - transmit enable
    uint32_t tctrl = __e1000_read(REG_TCTRL);
    tctrl |= (1 << 1);
    __e1000_write(REG_TCTRL, tctrl); // transmit enable
}

/**
 * __e1000_init_rx
*/

void __e1000_init_rx(void) {
    rx_descs = (struct __e1000_rx_desc *)kzalloc(sizeof(struct __e1000_rx_desc) * E1000_NUM_RX_DESC, 16);

    if (!rx_descs) {
        printk("e1000: failed to allocate memory for rx descriptors\n");
        return;
    }

    //__map_page((uint32_t)rx_descs, (uint32_t)rx_descs, PAGE_READ_WRITE | PAGE_CACHE_DISABLED | PAGE_WRITE_THROUGH);

    for (uint32_t i = 0; i < E1000_NUM_RX_DESC; ++i) {
        uint32_t addr = (uint32_t)pgalloc();

        //printk("%u: %p\n", i, addr);

        if (!addr) {
            printk("allocation for desc_%u failed\n", i);
            return;
        }

        //__map_page(addr, addr, PAGE_READ_WRITE | PAGE_CACHE_DISABLED | PAGE_WRITE_THROUGH);

        rx_descs[i].addr_lo = addr;
        rx_descs[i].addr_hi = 0;
        rx_descs[i].status = 0;
    }

    pgreserve((void *)(e1000.base + 0x2800));
    __map_page(e1000.base + 0x2800, e1000.base + 0x2800, PAGE_READ_WRITE);

    __e1000_write(REG_RXDESCLO, (uint32_t)rx_descs);
    __e1000_write(REG_RXDESCHI, 0);

    __e1000_write(REG_RXDESCLEN, sizeof(struct __e1000_rx_desc) * E1000_NUM_RX_DESC);

    __e1000_write(REG_RXDESCHEAD, 0);
    __e1000_write(REG_RXDESCTAIL, E1000_NUM_RX_DESC - 1);
    __e1000_write(REG_RDTR, 0);
    __e1000_write(REG_RADV, 0);
    //__e1000_write(REG_RXDCTL, 0x02008000); // enable + threshold
    __e1000_write(REG_RCTRL, RCTRL_EN | RCTRL_SBP | RCTRL_UPE | RCTRL_MPE | RCTRL_LBM_NONE | RTCL_RDMTS_HALF | RCTRL_BAM | RCTRL_SECRC  | RCTRL_BSIZE_4096);
}

/**
 * __e1000_init_tx
*/

void __e1000_init_tx(void) {
    tx_descs = (struct __e1000_tx_desc *)kzalloc(sizeof(struct __e1000_tx_desc) * E1000_NUM_TX_DESC, 16);

    if (!tx_descs) {
        printk("e1000: failed to allocate memory for tx descriptors\n");
        return;
    }

    //__map_page((uint32_t)tx_descs, (uint32_t)tx_descs, PAGE_READ_WRITE | PAGE_CACHE_DISABLED | PAGE_WRITE_THROUGH);

    for (uint32_t i = 0; i < E1000_NUM_TX_DESC; ++i) {
        uint32_t addr = (uint32_t)pgalloc();

        //printk("%u: %p\n", i, addr);

        if (!addr) {
            printk("allocation for desc_%u failed\n", i);
            return;
        }

        //__map_page(addr, addr, PAGE_READ_WRITE | PAGE_CACHE_DISABLED | PAGE_WRITE_THROUGH);

        tx_descs[i].addr_lo = addr;
        tx_descs[i].addr_hi = 0;
        tx_descs[i].status = TSTA_DD;
    }

    pgreserve((void *)(e1000.base + 0x3800));
    __map_page(e1000.base + 0x3800, e1000.base + 0x3800, PAGE_READ_WRITE);

    __e1000_write(REG_TXDESCLO, (uint32_t)tx_descs);
    __e1000_write(REG_TXDESCHI, 0);

    __e1000_write(REG_TXDESCLEN, sizeof(struct __e1000_rx_desc) * E1000_NUM_TX_DESC);

    __e1000_write(REG_TXDESCHEAD, 0);
    __e1000_write(REG_TXDESCTAIL, E1000_NUM_TX_DESC - 1);
    //__e1000_write(REG_TXDCTL, 0x02008000); // enable + threshold (from rx)
    __e1000_write(REG_TCTRL,  TCTRL_EN | TCTRL_PSP | (15 << TCTRL_CT_SHIFT) | (64 << TCTRL_COLD_SHIFT) | TCTRL_RTLC);
    __e1000_write(REG_TCTRL,  0b0110000000000111111000011111010);
    __e1000_write(REG_TIPG,  0x0060200a);
}

/**
 * __e1000_send
*/

int32_t __e1000_send(void *data, uint16_t length) {
    tx_descs[curr_tx_i].addr_lo = (uint32_t)data;
    rx_descs[curr_tx_i].addr_hi = 0;
    tx_descs[curr_tx_i].length = length;
    tx_descs[curr_tx_i].cmd = CMD_EOP | CMD_IFCS | CMD_RS;
    tx_descs[curr_tx_i].status = 0;

    uint8_t old_cur = curr_tx_i;
    curr_tx_i = (curr_tx_i + 1) % E1000_NUM_TX_DESC;
    __e1000_write(REG_TXDESCTAIL, curr_tx_i);

    while(!(tx_descs[old_cur].status & 0xff));
    return 0;
}

/**
 * __e1000_set_ip
*/

void __e1000_set_ip(uint32_t ip) {
    e1000.ipv4[0] = (uint8_t)(ip >> 24);
    e1000.ipv4[1] = (uint8_t)(ip >> 16);
    e1000.ipv4[2] = (uint8_t)(ip >> 8);
    e1000.ipv4[3] = (uint8_t)(ip >> 0);
}

/**
 * __init_e1000
*/

int32_t __init_e1000(struct __bus *b, struct __pci_header *h) {
    printf("        Initializing...\n");

    if (init) {
        printf("        Already initialized (only one card supported)\n");
        return 1;
    }

    struct __pci_h_device *d = (struct __pci_h_device *)h;

    printf("        VENDOR_ID=%04x DEVICE_ID=%04x\n", d->h.h_vendor, d->h.h_id);

    e1000.irq = d->d_int_line;
    printf("        IRQ line                : %u\n", e1000.irq);

    if (d->h.h_vendor != 0x8086 || (d->h.h_id != 0x100e && d->h.h_id != 0x153a && d->h.h_id != 0x10ea))
        printf("        Probably not an Intel E1000 network card, may not work correctly\n");

    printf("        Communication mechanism : ");

    if (d->d_bar0 & 0x00000001) {
        // ports
        printf("IO\n");
        printf("        IO base                 : %08x\n", d->d_bar0 & 0xfffffffc);

        e1000.cmt = 0;
        e1000.base = d->d_bar0 & 0xfffffffc;
    } else {
        // mmio
        if ((d->d_bar0 & 0x00000006) != 0x00000000) {
            printf("Unsupported\n");
            return 1;
        }

        printf("MMIO\n");
        printf("        MMIO base               : %08x\n", d->d_bar0 & 0xfffffff0);

        e1000.cmt = 1;
        e1000.base = d->d_bar0 & 0xfffffff0;

        pgreserve((void *)(e1000.base & 0xfffff000));

        if (__map_page(e1000.base & 0xfffff000, e1000.base & 0xfffff000, PAGE_READ_WRITE)) {
            printk("mapping failed\n");
            return 2;
        }
    }

    __e1000_write(REG_EEPROM, 0x00000010);

    bool eeprom_present = FALSE;

    for (uint32_t i = 0; i < 1000; ++i) {
        if (__e1000_read(REG_EEPROM) & (1 << 8)) {
            eeprom_present = TRUE;
            break;
        }
    }

    e1000.eeprom_present = eeprom_present;

    printf("        EEPROM                  : %s\n", eeprom_present ? "present" : "not present");

    if (eeprom_present) {
        for (uint32_t i = 0 ; i < 3; ++i)
            ((uint16_t *)e1000.mac)[i] = __e1000_eeprom_read(i);
        
    } else {
        pgreserve((void *)(e1000.base + 0x00005400));

        if (__map_page(e1000.base + 0x00005400, e1000.base + 0x00005400, PAGE_READ_WRITE))
            printk("mapping failed\n");

        // access to the table must be 32-bit
        uint32_t *mmio_mac = (uint32_t *)(e1000.base + 0x00005400);

        if (!*mmio_mac) {
            printk("e1000: failed to retrieve mac\n");
            return 2;
        }

        ((uint32_t *)e1000.mac)[0] = mmio_mac[0];

        uint32_t temp = mmio_mac[1];

        ((uint16_t *)e1000.mac)[2] = (uint16_t)temp;
    }

    printf("        MAC                     : ");

    __e1000_print_mac(e1000.mac);
    putchar('\n');

    __e1000_set_ip(IPV4(192, 168, 0, 2));

    printf("        IPv4                    : ");
    __e1000_print_ipv4(e1000.ipv4);
    putchar('\n');

    __e1000_init_rx();
    __e1000_init_tx();

    if (e1000.irq != 0xff) {
        __disable_interrupts();

        uint8_t interrupt = e1000.irq <= 7 ? (0x20 + e1000.irq) : (0x70 + e1000.irq - 8);

        __set_handler(interrupt, 0x0008, INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_32BIT_INTERRUPT_GATE, &__e1000_handler);
        
        __enable_interrupts();
        __enable_irq(e1000.irq);

        // enable interrupts
        __e1000_write(REG_IMASK, 0x182d4/*0x1f6dc*/);
        __e1000_read(REG_ICR);
    }

    __e1000_link_up();

    printf("        Link state              : %s\n", (__e1000_read(REG_STATUS) & (1 << 1)) ? "UP" : "DOWN");

    __e1000_write(REG_CTRL, __e1000_read(REG_CTRL) | (1 << 10));
    init = TRUE; // TODO: we should use mutex
    return 0;
}