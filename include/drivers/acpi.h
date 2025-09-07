/**
 * Advanced Configuration & Power Interface
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "null.h"
#include "types.h"
#include "kernel/e820.h"
#include "kernel/vmm.h"
#include "kstdlib/stdio.h"

/**
 * Types Definitions
*/

typedef struct __rsdp RSDP;
typedef struct __xsdp XSDP;
typedef struct __sdt_header SDT_HEADER;
typedef struct __sdt SDT;
typedef struct __apic APIC;
typedef struct __apic_header APIC_HEADER;
typedef struct __apic_cpu_lapic APIC_CPU_LAPIC;
typedef struct __apic_io_apic APIC_IO_APIC;
typedef struct __generic_addr GENERIC_ADDRESS;
typedef struct __fadt FADT;

/**
 * Constants
*/

#define APIC_TYPE_CPU_LAPIC 0 // processor local apic
#define APIC_TYPE_IO_APIC 1 // i/o apic
#define APIC_TYPE_IO_APIC_ISO 2 // i/o apic interrupt service override
#define APIC_TYPE_IO_NMIS 3 // i/o non-maskable interrupt source
#define APIC_TYPE_LAPIC_NMI 4 // local apic non-maskable interrupts
#define APIC_TYPE_LAPIC_AO 5 // local apic address override
#define APIC_TYPE_CPU_DLAPIC 9 // processor dual local apic

/**
 * Structures
*/

struct __attribute__((__packed__)) __rsdp {
    uint8_t signature[8]; // 'RSD PTR '
    uint8_t checksum;
    uint8_t oem_id[6];
    uint8_t revision;
    uint32_t rsdt_base;
};

struct __attribute__((__packed__)) __xsdp {
    uint8_t signature[8]; // 'RSD PTR '
    uint8_t checksum;
    uint8_t oem_id[6];
    uint8_t revision;
    uint32_t rsdt_base;
    uint32_t length;
    uint64_t xsdt_base;
    uint8_t echecksum;
    uint8_t reserved[3];
};

struct __attribute__((__packed__)) __sdt_header {
    uint8_t signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    uint8_t oem_id[6];
    uint8_t oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
};

struct __attribute__((__packed__)) __sdt {
    SDT_HEADER header;
    uint32_t sdts[];
};

struct __attribute__((__packed__)) __apic {
    SDT_HEADER h;
    uint32_t local_apic_addr;
    uint32_t flags;
};

struct __attribute__((__packed__)) __apic_header {
    uint8_t type;
    uint8_t length;
};

struct __attribute__((__packed__)) __apic_cpu_lapic {
    APIC_HEADER h;
    uint8_t acpi_cpu_id;
    uint8_t apic_id;
    uint32_t flags;
};

struct __attribute__((__packed__)) __apic_io_apic {
    APIC_HEADER h;
    uint8_t io_apic_id;
    uint8_t __reserved0;
    uint32_t io_apic_addr;
    uint32_t global_sys_int_base;
};

struct __generic_addr {
    uint8_t addr_space_type;
    uint8_t bit_width;
    uint8_t bit_offset;
    uint8_t access_size;
    uint64_t address;
};

struct __fadt {
    SDT_HEADER h;
    uint32_t firmware_ctrl;
    uint32_t dsdt;
    uint8_t __reserved1; // int model (used in ACPI 1.0, for compatibility only)
    uint8_t  preferred_power_management_profile;
    uint16_t sci_interrupt;
    uint32_t smi_command_port;
    uint8_t acpi_enabled;
    uint8_t acpi_disabled;
    uint8_t s4bios_req;
    uint8_t pstate_control;
    uint32_t pm1a_event_block;
    uint32_t pm1b_event_block;
    uint32_t pm1a_control_block;
    uint32_t pm1b_control_block;
    uint32_t pm2_control_block;
    uint32_t pm_timer_block;
    uint32_t gpe0_block;
    uint32_t gpe1_block;
    uint8_t pm1_event_length;
    uint8_t pm1_control_length;
    uint8_t pm2_control_length;
    uint8_t pm_timer_length;
    uint8_t gpe0_length;
    uint8_t gpe1_length;
    uint8_t gpe1_base;
    uint8_t c_state_control;
    uint16_t worst_c2_latency;
    uint16_t worst_c3_latency;
    uint16_t flush_size;
    uint16_t flush_stride;
    uint8_t duty_offset;
    uint8_t duty_width;
    uint8_t day_alarm;
    uint8_t month_alarm;
    uint8_t century;
    uint16_t ia_boot_arch_flags; // reserved in ACPI 1.0, used since ACPI 2.0+
    uint8_t __reserved2;
    uint32_t flags;
    GENERIC_ADDRESS reset_reg;
    uint8_t reset_value;
    uint8_t __reserved3[3];
    // available on ACPI 2.0+
    uint64_t x_firmware_ctrl;
    uint64_t x_dsdt;
    GENERIC_ADDRESS x_pm1a_event_block;
    GENERIC_ADDRESS x_pm1b_event_block;
    GENERIC_ADDRESS x_pm1a_control_block;
    GENERIC_ADDRESS x_pm1b_control_block;
    GENERIC_ADDRESS x_pm2_control_block;
    GENERIC_ADDRESS x_pm_timer_block;
    GENERIC_ADDRESS x_gpe0_block;
    GENERIC_ADDRESS x_gpe1_Bbock;
};

/**
 * Declarations
*/

int32_t __init_acpi(void);