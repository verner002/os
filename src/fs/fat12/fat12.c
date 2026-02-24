/**
 * File Allocation Table 12
 * 
 * Author: verner002
*/

/**
 * Includes
*/

#include "macros.h"
#include "fs/fat12/fat12.h"
#include "hal/driver.h"
#include "fs/fat12/super.h"
#include "fs/file.h"

#define __is_8dot3_char(x) (((x) >= '0' && (x) <= '9') || ((x) >= 'A' && (x) <= 'Z') || (x) == ' ' || (x) == '!' || (x) == '#' || (x) == '$' || (x) == '%' || (x) == '&' || (x) == '\'' || (x) == '(' || (x) == ')' || (x) == '-' || (x) == '@' || (x) == '^' || (x) == '_' || (x) == '`' || (x) == '{' || (x) == '}' || (x) == '~')

/**
 * Static Global Variables
*/

static uint8_t *fat = NULL;
static bool update_root_dir = true;
static FAT12_RECORD *root_dir = NULL;

/**
 * __fat12_read_fat
*/

int __fat12_read_fat(__kdev_t kdev) {
    if (!fat) {
        fat = (uint8_t *)kmalloc(9*512);

        if (!fat)
            return -1;
    }

    struct super_block super;

    int result = get_super(kdev, &super);

    if (result)
        return result;

    struct fat12_info *info = (struct fat12_info *)super.fs_data;

    struct __block_dev_driver *driver = (struct __block_dev_driver *)driver_lookup(MAJOR(kdev));

    if (!driver)
        return -1; // device driver not present

    //return __fdc_read_sectors(1, 9, (uint32_t)fat);
    return driver->read(MINOR(kdev), info->reserved_sectors, info->sectors_per_fat, (char *)fat);
}

/**
 * __fat12_read_root_dir
*/

int __fat12_read_root_dir(__kdev_t kdev) {
    struct super_block super = {
        .lock = false
    };

    int result = get_super(kdev, &super);

    if (result)
        return result;

    struct fat12_info *info = (struct fat12_info *)super.fs_data;

    if (!root_dir) {
        root_dir = (FAT12_RECORD *)kmalloc(info->number_of_entries * 32);

        if (!root_dir)
            return -1;
    }

    if (!update_root_dir)
        return 0;

    struct __block_dev_driver *driver = (struct __block_dev_driver *)driver_lookup(MAJOR(kdev));

    if (!driver)
        return -1; // device driver not present

    int error = driver->read(MINOR(kdev), info->sectors_per_fat * info->number_of_fats + info->reserved_sectors, (info->number_of_entries * 32) / super.block_size, (char *)root_dir);
    //error = __fdc_read_sectors(9 * 2 + 1, (224 * 32) / 512, (uint32_t)root_dir);

    if (!error)
        update_root_dir = false;

    return error;
}

/**
 * __fat12_file_exists
*/

bool __fat12_file_exists(__kdev_t kdev, char const *filename) {
    if (!root_dir)
        if (__fat12_read_root_dir(kdev))
            return false;

    for (uint32_t i = 0; i < 224; ++i) {
        FAT12_RECORD *record = &root_dir[i];

        if (!strncmp(record->filename, filename, 11))
            return true;
    }

    return false;
}

/**
 * __fat12_load_file
*/

int32_t __fat12_load_file(__kdev_t kdev, char const *filename, uint32_t buffer) {
    struct __block_dev_driver *driver = (struct __block_dev_driver *)driver_lookup(MAJOR(kdev));

    if (!driver)
        return -1; // device driver not present

    uint8_t minor = MINOR(kdev);
    int (* read)(uint8_t minor, uint32_t offset, uint32_t count, char *buffer) = driver->read;

    for (uint32_t i = 0; i < 224; ++i) {
        FAT12_RECORD *record = &root_dir[i];

        if (!strncmp(record->filename, filename, 11)) {
            uint32_t cluster = record->first_cluster;
            uint32_t last_opcode;

            do {
                uint32_t lba = ((cluster - 2) * 1) + 1 + (2 * 9) + (224 * 32 / 512);
                last_opcode = read(minor, lba, 1, (uint8_t *)buffer);
                //last_opcode = __fdc_read_sectors(lba, 1, buffer);
                uint32_t next_cluster = *(uint16_t *)(fat + cluster + cluster / 2);

                if (cluster & 1) next_cluster >>= 4;

                next_cluster &= 0x00000fff;
                cluster = next_cluster;
                buffer += 512;
            } while (!last_opcode && cluster < 0x00000ff8);

            return last_opcode;
        }
    }

    return -1;
}

/**
 * __8dot3_to_filename
*/

char *__8dot3_to_filename(FAT12_RECORD *record) {
    uint8_t record_attrs = record->attributes;

    if (
        record_attrs & FAT12_ATTRIBUTE_VOLUME ||
        record_attrs & FAT12_ATTRIBUTE_HIDDEN
    )
        return NULL; // not implemented

    char const *record_ext = record->extension;

    uint32_t fname_length;
    uint32_t ext_length = FAT12_EXT_LENGTH;

    while (ext_length > 0) {
        if (record_ext[ext_length - 1] != ' ')
            break;

        --ext_length;
    }

    bool is_dir = record_attrs & FAT12_ATTRIBUTE_SUBDIR;

    if (is_dir && ext_length)
        return NULL; // subdir shall not have extension

    char first = *record->filename;
    char record_fname[FAT12_NAME_LENGTH];
    *record_fname = (first == (char)0x05 ? (char)0xe5 : first);
    strncpy(record_fname + 1, record->filename + 1, FAT12_NAME_LENGTH - 1);

    if (is_dir && record_fname[0] == '.') {
        fname_length = 1;

        if (record_fname[1] == '.')
            ++fname_length;

        for (uint32_t i = fname_length; i < FAT12_NAME_LENGTH; ++i)
            if (record_fname[i] != ' ')
                return NULL;
    } else {
        for (fname_length = FAT12_NAME_LENGTH; fname_length > 0; --fname_length)
            if (record_fname[fname_length - 1] != ' ')
                break;
    }

    if (!fname_length)
        return NULL;

    uint32_t extra = 0;

    if (ext_length)
        ++extra; // add dot

    char *filename = (char *)kmalloc((fname_length + extra + ext_length) * sizeof(char));

    if (!filename)
        return NULL; // allocation failed

    for (uint32_t i = 0; i < fname_length; ++i) {
        char ch = record_fname[i];

        if (!__is_8dot3_char(ch))
            return NULL;

        filename[i] = ch;
    }

    if (ext_length) {
        filename[fname_length] = '.';

        for (uint32_t i = 0; i < ext_length; ++i) {
            char ch = record_ext[i];
    
            if (!__is_8dot3_char(ch))
                return NULL;
    
            filename[fname_length + extra + i] = ch;
        }
    }

    filename[fname_length + extra + ext_length] = '\0';
    return filename;
}

/**
 * __vfat_to_filename
*/

char *__vfat_to_filename() {
    return NULL;
}

/**
 * __fat12_list_dir
*/

/*int32_t __fat12_list_dir(VFS_DIR_NODE *node) {
    if (1 || !node->h.parent) {
        // root node
        __fat12_read_root_dir();

        uint32_t subnodes_count = 0;
        uint32_t subnodes_capacity = 16;
        VFS_NODE **subnodes = (VFS_NODE **)kmalloc(sizeof(VFS_NODE *) * subnodes_capacity);

        for (uint32_t i = 0; i < 224; ++i) {
            FAT12_RECORD *record = &((FAT12_RECORD *)root_dir)[i];
            char const *filename = __8dot3_to_filename(record);

            VFS_NODE *subnode;

            if (record->attributes & FAT12_ATTRIBUTE_SUBDIR) {
                // dir node
                VFS_DIR_NODE *dir = __new_vfs_dir_node(filename);

                subnode = (VFS_NODE *)dir;
            } else {
                // file node
                VFS_FILE_NODE *file = __new_vfs_file_node(filename);

                subnode = (VFS_NODE *)file;
            }

            if (subnodes_count >= subnodes_capacity) {
                subnodes_capacity *= 2;

                subnodes = krealloc(subnodes, subnodes_capacity);
            }

            subnodes[subnodes_count++] = subnode;
        }

        node->nodes = subnodes;
        node->nodes_count = subnodes_count;
        node->nodes_capacity = subnodes_capacity;
    }
}*/

uint16_t __lfn_get_char(VFAT_LFN_ENTRY *entry, uint32_t i) {
    // roll-up into a cycle?
    if (i < VFAT_LFN_CHARS1)
        return entry->chars1[i];

    i -= VFAT_LFN_CHARS1;

    if (i < VFAT_LFN_CHARS2)
        return entry->chars2[i];

    i -= VFAT_LFN_CHARS2;

    if (i < VFAT_LFN_CHARS3)
        return entry->chars3[i];

    // RFN: print warning?
    return (uint16_t)0;
}

uint32_t __ucs2_strlen(uint16_t const *str) {
    uint32_t l = 0;
    
    while (*str++)
        ++l;

    return l;
}

/**
 * __lfn_to_filename
*/

uint16_t *__lfn_to_filename(VFAT_LFN_ENTRY *entry, uint32_t *ucs2_filename_length, uint32_t *entries_count) {
    if (!entry)
        return NULL;

    uint32_t ordinal = entry->ordinal;

    if (!(ordinal & VFAT_LAST_LFN_ENTRY))
        printk("vfat: warning: last lfn entry without VFAT_LAST_LFN_ENTRY bit set\n");

    uint32_t entries_total = ordinal & VFAT_ORDINAL;

    uint32_t chars_last;

    for (chars_last = 0; chars_last < VFAT_LFN_CHARS; ++chars_last)
        if (!__lfn_get_char(entry, chars_last))
            break;

    uint32_t chars_total = chars_last + (entries_total - 1) * VFAT_LFN_CHARS;

    //printk("vfat: info: allocating %u byte(s) for ucs2 file name\n", chars_total + 1);

    uint16_t *ucs2_filename = (uint16_t *)kmalloc(sizeof(uint16_t) * (chars_total + 1));

    if (!ucs2_filename) {
        printk("vfat: error: allocation failed\n");
        return NULL;
    }

    uint32_t ucs2_fname_i = 0;

    for (uint32_t i = entries_total - 1; i > 0; --i) {
        VFAT_LFN_ENTRY *lfn_entry = &entry[i];

        for (uint32_t j = 0; j < VFAT_LFN_CHARS; ++j)
            ucs2_filename[ucs2_fname_i++] = __lfn_get_char(lfn_entry, j);
    }

    for (uint32_t i = 0; i < chars_last; ++i)
        ucs2_filename[ucs2_fname_i++] = __lfn_get_char(entry, i);

    if (ucs2_filename_length)
        *ucs2_filename_length = chars_total;

    if (entries_count)
        *entries_count = entries_total;

    ucs2_filename[ucs2_fname_i] = (uint16_t)0;
    return ucs2_filename;
}

/**
 * __fat12_list_rootdir
*/

int32_t __fat12_list_rootdir(__kdev_t kdev) {
    int result = __fat12_read_root_dir(kdev);

    if (result)
        return result;

    for (uint32_t i = 0; i < 224; ++i) {
        FAT12_RECORD *record = &((FAT12_RECORD *)root_dir)[i];
        uint8_t *filename = record->filename;

        uint8_t ch = *filename;

        if (ch == 0x00)
            break; // last record
        else if (ch == 0xe5)
            continue; // deleted record

        char *fname = NULL;

        if (
            record->attributes & FAT12_ATTRIBUTE_VOLUME &&
            record->attributes & FAT12_ATTRIBUTE_SYSTEM &&
            record->attributes & FAT12_ATTRIBUTE_HIDDEN &&
            record->attributes & FAT12_ATTRIBUTE_READONLY
        ) {
            VFAT_LFN_ENTRY *lfn = (VFAT_LFN_ENTRY *)record;

            if (lfn->ordinal & VFAT_DELETED_LFN_ENTRY) {
                printk("vfat: info: found deleted lfn entry\n");
                // skip entry (all entries based on ordinal?)
            }

            uint32_t ucs2_filename_length;
            uint32_t entries_count;
            uint16_t *ucs2_filename = __lfn_to_filename(lfn, &ucs2_filename_length, &entries_count);

            if (!ucs2_filename)
                continue; // record corrupted

            uint32_t fname_index = 0;
            fname = (char *)kmalloc(sizeof(char) * (ucs2_filename_length + 1));

            if (fname) {
                uint16_t *ptr = ucs2_filename;
                uint16_t ch;

                while ((ch = *ptr++))
                    fname[fname_index++] = ch & 0xff00 ? '?' : ch;

                fname[fname_index] = '\0';
            }

            //printk("entry with name length %u byte(s)\n", chars_total);
            kfree(ucs2_filename);

            i += entries_count;

            if (i >= 224) {
                printk("fat12: warning: possibly corrupted data\n");
                continue;
            }

            record = &((FAT12_RECORD *)root_dir)[i];
        }

        if (!fname)
            fname = __8dot3_to_filename(record);

        if (!fname) {
            printk("fat12: error: failed to retrieve file name\n");
            continue;
        }

        static char const *months[] = {
            "Jan",
            "Feb",
            "Mar",
            "Apr",
            "May",
            "Jun",
            "Jul",
            "Aug",
            "Sep",
            "Oct",
            "Nov",
            "Dec"
        };

        uint16_t date = record->last_write_date;
        uint16_t time = record->last_write_time;

        uint32_t minutes = (time >> 5) & 0x3f;
        uint32_t hours = (time >> 11) & 0x1f;

        uint32_t day = date & 0x1f;
        uint32_t month = (date >> 5) & 0x0f;

        printf("%crwxr-xr-x root root %s %u ",
            (record->attributes & FAT12_ATTRIBUTE_SUBDIR) ? 'd' : '-',
            months[month - 1],
            day
        );

        if (hours < 10)
            putchar('0');

        printf("%u:", hours);

        if (minutes < 10)
            putchar('0');

        printf("%u %s\n", minutes, fname);
        kfree(fname);
    }

    return 0;
}

/**
 * __fat12_list_root
*/

int __fat12_list_root(struct dentry *mountpoint, __kdev_t kdev) {
    int result = __fat12_read_root_dir(kdev);

    if (result)
        return result;

    // TODO: read number_of_entries from super block
    for (uint32_t i = 0; i < 224; ++i) {
        FAT12_RECORD *record = &((FAT12_RECORD *)root_dir)[i];
        uint8_t *filename = record->filename;

        uint8_t ch = *filename;

        if (ch == 0x00)
            break; // last record
        else if (ch == 0xe5)
            continue; // deleted record

        char *fname = NULL;

        if (
            record->attributes & FAT12_ATTRIBUTE_VOLUME &&
            record->attributes & FAT12_ATTRIBUTE_SYSTEM &&
            record->attributes & FAT12_ATTRIBUTE_HIDDEN &&
            record->attributes & FAT12_ATTRIBUTE_READONLY
        ) {
            VFAT_LFN_ENTRY *lfn = (VFAT_LFN_ENTRY *)record;

            if (lfn->ordinal & VFAT_DELETED_LFN_ENTRY) {
                printk("vfat: info: found deleted lfn entry\n");
                // skip entry (all entries based on ordinal?)
            }

            uint32_t ucs2_filename_length;
            uint32_t entries_count;
            uint16_t *ucs2_filename = __lfn_to_filename(lfn, &ucs2_filename_length, &entries_count);

            if (!ucs2_filename)
                continue; // record corrupted

            uint32_t fname_index = 0;
            fname = (char *)kmalloc(sizeof(char) * (ucs2_filename_length + 1));

            if (fname) {
                uint16_t *ptr = ucs2_filename;
                uint16_t ch;

                while ((ch = *ptr++))
                    fname[fname_index++] = ch & 0xff00 ? '?' : ch;

                fname[fname_index] = '\0';
            }

            //printk("entry with name length %u byte(s)\n", chars_total);
            kfree(ucs2_filename);

            i += entries_count;

            if (i >= 224) {
                printk("fat12: warning: possibly corrupted data\n");
                continue;
            }

            record = &((FAT12_RECORD *)root_dir)[i];
        }

        if (!fname)
            fname = __8dot3_to_filename(record);

        if (!fname) {
            printk("fat12: error: failed to retrieve file name\n");
            continue;
        }

        static char const *months[] = {
            "Jan",
            "Feb",
            "Mar",
            "Apr",
            "May",
            "Jun",
            "Jul",
            "Aug",
            "Sep",
            "Oct",
            "Nov",
            "Dec"
        };

        uint16_t date = record->last_write_date;
        uint16_t time = record->last_write_time;

        uint32_t minutes = (time >> 5) & 0x3f;
        uint32_t hours = (time >> 11) & 0x1f;

        uint32_t day = date & 0x1f;
        uint32_t month = (date >> 5) & 0x0f;

        create_file(mountpoint, fname, 0, 0, (record->attributes & FAT12_ATTRIBUTE_SUBDIR) ? 0x80000000 : 0x00000000);
        kfree(fname);
    }

    return 0;
}