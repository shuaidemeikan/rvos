#include "virtio.h"

virtq_desc* virtio_blk_desc;
virtq_avail* virtio_blk_avail;
virtq_used* virtio_blk_used;

static inline uint64_t read_device_features()
{
    uint64_t fea;
    virtio_write_reg(VIRTIO0, DEVICE_FEATURE_SEL, 0);
    fea = virtio_read_reg(VIRTIO0, DEVICE_FEATURES);
    virtio_write_reg(VIRTIO0, DEVICE_FEATURE_SEL, 1);
    fea += (virtio_read_reg(VIRTIO0, DEVICE_FEATURES) << 32);
    return fea;
}

static inline void write_device_features(uint64_t fea)
{
    uint32_t fea32;
    virtio_write_reg(VIRTIO0, DRIVER_FEATURE_SEL, 0);
    fea32 = (uint32_t)fea;
    virtio_write_reg(VIRTIO0, DRIVER_FEATURES, fea32);
    virtio_write_reg(VIRTIO0, DRIVER_FEATURE_SEL, 1);
    fea32 = (uint32_t)(fea >> 32);
    virtio_write_reg(VIRTIO0, DRIVER_FEATURES, fea32);
}

void virtio_init()
{
    // 当前只有一个virtio设备，是一个磁盘
    virtio_write_reg(VIRTIO0, DEVICES_STATUS, ACKNOWLEDGE);
    virtio_write_reg(VIRTIO0, DEVICES_STATUS, DRIVER);
    uint64_t fea = read_device_features();
    write_device_features(fea);
    virtio_write_reg(VIRTIO0, DEVICES_STATUS, FEATURES_OK);
    // 大致初始化完成，读一下磁盘数据
    virtio_blk_config* blk_config = (virtio_blk_config*)(VIRTIO0 + 0x100);
    // 初始化virtio queue_ring
    virtio_blk_desc = (virtq_desc*)page_alloc(1);
    virtio_blk_avail = (virtq_avail*)page_alloc(1);
    virtio_blk_used = (virtq_used*)page_alloc(1);
    memcpy((uint8_t*)(VIRTIO0 + QUEUE_DESC_LOW), &virtio_blk_desc, 4);
    memcpy((uint8_t*)(VIRTIO0 + QUEUE_AVAIL_LOW), &virtio_blk_avail, 4);
    memcpy((uint8_t*)(VIRTIO0 + QUEUE_USED_LOW), &virtio_blk_used, 4);
    virtio_write_reg(VIRTIO0, QUEUE_NUM, VIRIOBLK_NUM);                 // 写描述符表数量寄存器
    int num_max = virtio_read_reg(VIRTIO0, QUEUE_NUM_MAX);
    // 结束，将状态寄存器置为ok
    virtio_write_reg(VIRTIO0, DEVICES_STATUS, DRIVER_OK);
}