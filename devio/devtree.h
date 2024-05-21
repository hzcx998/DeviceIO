#ifndef _DEVICE_TREE_H
#define _DEVICE_TREE_H

#include <cJSON.h>
#include <string.h>
#include <stdlib.h>

typedef struct IO_Compatible {
    char *name;
    void *data;
} IO_CompatibleType;

typedef struct IO_DeviceNode {
    cJSON *object;
    char *name;
    int id;
    unsigned long address;
} IO_DeviceNodeType;

static inline char *IO_GetNodeName(IO_DeviceNodeType *node)
{
    return node ? node->name : NULL;
}

static inline int IO_GetNodeID(IO_DeviceNodeType *node)
{
    return node ? node->id : 0;
}

static inline unsigned long IO_GetNodeAddress(IO_DeviceNodeType *node)
{
    return node ? node->address : 0;
}

static inline int IO_GetNodeBool(IO_DeviceNodeType *node, char *name, int defaultValue)
{
    cJSON *object = node->object;
    cJSON *child;
    cJSON_ArrayForEach(child, object) {
        if (!strcmp(child->string, name)) {
            if (child->type == cJSON_False) {
                return 0;
            } else if (child->type == cJSON_True) {
                return 1;
            }
        }
    }
    return defaultValue;
}

static inline int IO_GetNodeInt(IO_DeviceNodeType *node, char *name, int defaultValue)
{
    cJSON *object = node->object;
    cJSON *child;
    cJSON_ArrayForEach(child, object) {
        if (!strcmp(child->string, name)) {
            if (child->type == cJSON_Number) {
                return child->valueint;
            }
        }
    }
    return defaultValue;
}

static inline unsigned long IO_GetNodeHex(IO_DeviceNodeType *node, char *name, unsigned long defaultValue)
{
    cJSON *object = node->object;
    cJSON *child;
    cJSON_ArrayForEach(child, object) {
        if (!strcmp(child->string, name)) {
            if (child->type == cJSON_String) {
                return strtoull(child->valuestring, NULL, 0);
            }
        }
    }
    return defaultValue;
}

static inline double IO_GetNodeDouble(IO_DeviceNodeType *node, char *name, double defaultValue)
{
    cJSON *object = node->object;
    cJSON *child;
    cJSON_ArrayForEach(child, object) {
        if (!strcmp(child->string, name)) {
            if (child->type == cJSON_Number) {
                return child->valuedouble;
            }
        }
    }
    return defaultValue;
}

static inline char *IO_GetNodeString(IO_DeviceNodeType *node, char *name, char *defaultValue)
{
    cJSON *object = node->object;
    cJSON *child;
    cJSON_ArrayForEach(child, object) {
        if (!strcmp(child->string, name)) {
            if (child->type == cJSON_String) {
                return child->valuestring;
            }
        }
    }
    return defaultValue;
}

static inline unsigned char IO_GetNodeU8(IO_DeviceNodeType *node, char *name, unsigned char defaultValue)
{
    cJSON *object = node->object;
    cJSON *child;
    cJSON_ArrayForEach(child, object) {
        if (!strcmp(child->string, name)) {
            if (child->type == cJSON_Number) {
                return (unsigned char)child->valueint;
            }
        }
    }
    return defaultValue;
}

static inline unsigned short IO_GetNodeU16(IO_DeviceNodeType *node, char *name, unsigned short defaultValue)
{
    cJSON *object = node->object;
    cJSON *child;
    cJSON_ArrayForEach(child, object) {
        if (!strcmp(child->string, name)) {
            if (child->type == cJSON_Number) {
                return (unsigned short)child->valueint;
            }
        }
    }
    return defaultValue;
}

static inline unsigned int IO_GetNodeU32(IO_DeviceNodeType *node, char *name, unsigned int defaultValue)
{
    cJSON *object = node->object;
    cJSON *child;
    cJSON_ArrayForEach(child, object) {
        if (!strcmp(child->string, name)) {
            if (child->type == cJSON_Number) {
                return (unsigned int)child->valueint;
            }
        }
    }
    return defaultValue;
}

static inline unsigned long IO_GetNodeU64(IO_DeviceNodeType *node, char *name, unsigned long defaultValue)
{
    cJSON *object = node->object;
    cJSON *child;
    cJSON_ArrayForEach(child, object) {
        if (!strcmp(child->string, name)) {
            if (child->type == cJSON_Number) {
                return (unsigned long)child->valueint;
            }
        }
    }
    return defaultValue;
}

static inline IO_DeviceNodeType *IO_GetNodeObject(IO_DeviceNodeType *node, char *name, IO_DeviceNodeType *defaultValue)
{
    cJSON *object = node->object;
    cJSON *child;

    if (defaultValue) {
        cJSON_ArrayForEach(child, object) {
            if (!strcmp(child->string, name)) {
                if (child->type == cJSON_Object) {
                    defaultValue->name = name;
                    defaultValue->address = 0;
                    defaultValue->id = 0;
                    defaultValue->object = child;
                    return (IO_DeviceNodeType *)defaultValue;
                }
            }
        }
    }
    return NULL;
}

static inline int IO_GetNodeArrayLength(IO_DeviceNodeType *node, char *name)
{
    cJSON *object = node->object;
    cJSON *child;
    cJSON_ArrayForEach(child, object) {
        if (!strcmp(child->string, name)) {
            if (child->type == cJSON_Array) {
                return cJSON_GetArraySize(child);
            }
        }
    }
    return 0;
}

static inline int IO_GetNodeArrayBool(IO_DeviceNodeType *node, char *name, int idx, int defaultValue)
{
    cJSON *object = node->object;
    cJSON *child, *array;
    cJSON_ArrayForEach(child, object) {
        if (!strcmp(child->string, name)) {
            if (child->type == cJSON_Array) {
                array = cJSON_GetArrayItem(child, idx);
                if (array) {
                    if (array->type == cJSON_False) {
                        return 0;
                    } else if (array->type == cJSON_True) {
                        return 1;
                    }
                }
            }
        }
    }
    return defaultValue;
}

static inline int IO_GetNodeArrayInt(IO_DeviceNodeType *node, char *name, int idx, int defaultValue)
{
    cJSON *object = node->object;
    cJSON *child, *array;
    cJSON_ArrayForEach(child, object) {
        if (!strcmp(child->string, name)) {
            if (child->type == cJSON_Array) {
                array = cJSON_GetArrayItem(child, idx);
                if (array) {
                    if (array->type == cJSON_Number) {
                        return array->valueint;
                    }
                }
            }
        }
    }
    return defaultValue;
}

static inline unsigned long IO_GetNodeArraHex(IO_DeviceNodeType *node, char *name, int idx, unsigned long defaultValue)
{
    cJSON *object = node->object;
    cJSON *child, *array;
    cJSON_ArrayForEach(child, object) {
        if (!strcmp(child->string, name)) {
            if (child->type == cJSON_Array) {
                array = cJSON_GetArrayItem(child, idx);
                if (array) {
                    if (array->type == cJSON_String) {
                        return strtoull(array->valuestring, NULL, 0);
                    }
                }
            }
        }
    }
    return defaultValue;
}

static inline double IO_GetNodeArrayDouble(IO_DeviceNodeType *node, char *name, int idx, double defaultValue)
{
    cJSON *object = node->object;
    cJSON *child, *array;
    cJSON_ArrayForEach(child, object) {
        if (!strcmp(child->string, name)) {
            if (child->type == cJSON_Array) {
                array = cJSON_GetArrayItem(child, idx);
                if (array) {
                    if (array->type == cJSON_Number) {
                        return array->valuedouble;
                    }
                }
            }
        }
    }
    return defaultValue;
}

static inline char *IO_GetNodeArraString(IO_DeviceNodeType *node, char *name, int idx, char *defaultValue)
{
    cJSON *object = node->object;
    cJSON *child, *array;
    cJSON_ArrayForEach(child, object) {
        if (!strcmp(child->string, name)) {
            if (child->type == cJSON_Array) {
                array = cJSON_GetArrayItem(child, idx);
                if (array) {
                    if (array->type == cJSON_String) {
                        return array->valuestring;
                    }
                }
            }
        }
    }
    return defaultValue;
}

static inline unsigned char IO_GetNodeArrayU8(IO_DeviceNodeType *node, char *name, int idx, unsigned char defaultValue)
{
    cJSON *object = node->object;
    cJSON *child, *array;
    cJSON_ArrayForEach(child, object) {
        if (!strcmp(child->string, name)) {
            if (child->type == cJSON_Array) {
                array = cJSON_GetArrayItem(child, idx);
                if (array) {
                    if (array->type == cJSON_Number) {
                        return (unsigned char)array->valueint;
                    }
                }
            }
        }
    }
    return defaultValue;
}

static inline unsigned short IO_GetNodeArrayU16(IO_DeviceNodeType *node, char *name, int idx, unsigned short defaultValue)
{
    cJSON *object = node->object;
    cJSON *child, *array;
    cJSON_ArrayForEach(child, object) {
        if (!strcmp(child->string, name)) {
            if (child->type == cJSON_Array) {
                array = cJSON_GetArrayItem(child, idx);
                if (array) {
                    if (array->type == cJSON_Number) {
                        return (unsigned short)array->valueint;
                    }
                }
            }
        }
    }
    return defaultValue;
}

static inline unsigned int IO_GetNodeArrayU32(IO_DeviceNodeType *node, char *name, int idx, unsigned int defaultValue)
{
    cJSON *object = node->object;
    cJSON *child, *array;
    cJSON_ArrayForEach(child, object) {
        if (!strcmp(child->string, name)) {
            if (child->type == cJSON_Array) {
                array = cJSON_GetArrayItem(child, idx);
                if (array) {
                    if (array->type == cJSON_Number) {
                        return (unsigned int)array->valueint;
                    }
                }
            }
        }
    }
    return defaultValue;
}

static inline unsigned long IO_GetNodeArrayU64(IO_DeviceNodeType *node, char *name, int idx, unsigned long defaultValue)
{
    cJSON *object = node->object;
    cJSON *child, *array;
    cJSON_ArrayForEach(child, object) {
        if (!strcmp(child->string, name)) {
            if (child->type == cJSON_Array) {
                array = cJSON_GetArrayItem(child, idx);
                if (array) {
                    if (array->type == cJSON_Number) {
                        return (unsigned long)array->valueint;
                    }
                }
            }
        }
    }
    return defaultValue;
}

void IO_InitDeviceTree(void);
int IO_ParseDeviceTree(char *buf);
/**
 * 在支持文件的系统上加载设备树
 */
char *IO_LoadDeviceTree(char *filename);

#endif