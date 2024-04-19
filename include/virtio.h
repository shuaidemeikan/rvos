#ifndef _VIRTIO_H
#define _VIRTIO_H

#include "types.h"
#include "platform.h"
#include "memtools.h"
#include "page.h"

#define VIRTIO0 0x10008000L

#define VIRTIO_REG(virtioaddr, reg) ((volatile uint64_t *)(virtioaddr + reg))

#define virtio_read_reg(virtioaddr, reg) (*(VIRTIO_REG(virtioaddr, reg)))
#define virtio_write_reg(virtioaddr, reg, v) (*(VIRTIO_REG(virtioaddr, reg)) = (v))

// virtio over mmio寄存器偏移
#define VERSION                 0x04
#define DEVICEID                0x08
#define VENDORID                0x0c
#define DEVICE_FEATURES         0x10
#define DEVICE_FEATURE_SEL      0x14
#define DRIVER_FEATURES         0x20
#define DRIVER_FEATURE_SEL      0x24
#define QUEUE_SEL               0x30
#define QUEUE_NUM_MAX           0x34
#define QUEUE_NUM               0x38
#define QUEUE_READY             0x3c
#define QUEUE_NOTIFY            0x50
#define INTERRUPT_STATUS        0x60
#define INTERRUPT_ACK           0x64
#define DEVICES_STATUS          0x70
#define QUEUE_DESC_LOW          0x80
#define QUEUE_DESC_HIGH         0x84
#define QUEUE_AVAIL_LOW         0x90
#define QUEUE_AVAIL_HIGH        0x94
#define QUEUE_USED_LOW          0xA0
#define QUEUE_USED_HIGH         0xA4
#define CONFIG_GENERATION       0xFC

// virtio状态码
#define ACKNOWLEDGE     1
#define DRIVER          2
#define FAILED          128
#define FEATURES_OK     8
#define DRIVER_OK       4
#define DEVICE_NEEDS_RESET     64

#define VIRIOBLK_NUM    256         // 描述符表项数

// desc的flags
#define VIRTQ_DESC_F_NEXT   1                   // 后面还有描述符表
#define VIRTQ_DESC_F_WRITE     2                // 表示这块内存对于设备的权限是write-only
#define VIRTQ_DESC_F_INDIRECT   4               // 没搞懂是干什么的

// virtio操作码
#define VIRTIO_BLK_T_IN                 0               // 读
#define VIRTIO_BLK_T_OUT                1               // 写
#define VIRTIO_BLK_T_FLUSH              4 
#define VIRTIO_BLK_T_GET_ID             8 
#define VIRTIO_BLK_T_GET_LIFETIME       10 
#define VIRTIO_BLK_T_DISCARD            11 
#define VIRTIO_BLK_T_WRITE_ZEROES       13 
#define VIRTIO_BLK_T_SECURE_ERASE       14

#pragma pack(1)
typedef struct _virtio_blk_config 
{
    uint64_t capacity;
    uint32_t size_max;
    uint32_t seg_max;
    struct 
    {
        uint16_t cylinders;
        uint8_t heads;
        uint8_t sectors;
    } geometry;
    uint32_t blk_size;
    struct 
    {
        uint8_t physical_block_exp;
        uint8_t alignment_offset;
        uint16_t min_io_size;
        uint32_t opt_io_size;
    } topology;
    uint8_t writeback;
}virtio_blk_config;

typedef struct _virtq_desc { 
        uint64_t addr;
        uint32_t len;
        uint16_t flags;
        uint16_t next;
}virtq_desc;

typedef struct _virtq_avail {
        uint16_t flags;
        uint16_t idx;
        uint16_t ring[ /* Queue Size */ ];
}virtq_avail;

typedef struct _virtq_used_elem {
        uint32_t id;
        uint32_t len;
}virtq_used_elem;

typedef struct _virtq_used {
        uint16_t flags;
        uint16_t idx;
        struct _virtq_used_elem ring[ /* Queue Size */];
}virtq_used;

typedef struct _virtio_blk_req {
    uint32_t type; // 操作类型
    uint32_t reserved;
    uint64_t sector; // 读取的起始扇区号
}virtio_blk_req;

#pragma pack()

void virtio_init();

#endif // !_VIRTIO_H