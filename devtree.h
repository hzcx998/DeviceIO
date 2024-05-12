#ifndef _DEVICE_TREE_H
#define _DEVICE_TREE_H

#include <cJSON.h>

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

#endif