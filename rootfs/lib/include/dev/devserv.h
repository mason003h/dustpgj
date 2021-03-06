#ifndef DEVSERV_H
#define DEVSERV_H

#include <types.h>
#include <device.h>
#include <fsinfo.h>
#include <proto.h>

typedef struct {
	int32_t (*mount)(const char* fname, int32_t index);
	int32_t (*unmount)(int32_t pid, const char* fname);
	int32_t (*open)(int32_t pid, int32_t fd, int32_t flags);
	int32_t (*close)(int32_t pid, int32_t fd, fs_info_t* info);
	int32_t (*remove)(fs_info_t* info, const char* fname);
	int32_t (*add)(int32_t pid, const char* fname);
	int32_t (*write)(int32_t pid, int32_t fd, void* buf, uint32_t size, int32_t seek);
	int32_t (*read)(int32_t pid, int32_t fd, void* buf, uint32_t size, int32_t seek);
	int32_t (*dma)(int32_t pid, int32_t fd, uint32_t *size);
	int32_t (*flush)(int32_t pid, int32_t fd);
	int32_t (*ctrl)(int32_t pid, int32_t fd, int32_t cmd, proto_t*input, proto_t* output);
	int32_t (*fctrl)(int32_t pid, const char* fname, int32_t cmd, proto_t*input, proto_t* output);
	void (*step)(void* data);
	int32_t (*ipc_call)(int32_t pid, int32_t call_id, proto_t* input, proto_t* out, void* data);
} device_t;

void dev_run(device_t* dev, int32_t argc, char** argv);

#endif
