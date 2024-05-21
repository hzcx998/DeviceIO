#include <devtree.h>
#include <devio.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

static cJSON_Hooks IO_cJSON_Hooks = {.malloc_fn = IO_Malloc, .free_fn = IO_Free};

static cJSON* RootNode = NULL;

void ParseNode(cJSON* node);
void ParseChildNode(cJSON* childNode, int ignoreObject);

char *IO_LoadDeviceTree(char *filename)
{
    FILE *file;
    char *buf = NULL;
    long fileSize;

    file = fopen(filename, "r");
    if (!file) {
        printf("open device tree file %s failed!\n", filename);
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        printf("fseek failed: %s\n", strerror(errno));
        goto Closefile;
    }

    fileSize = ftell(file);
    if (fileSize == -1) {
        printf("ftell failed :%s\n", strerror(errno));
        goto Closefile;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        printf("fseek failed: %s\n", strerror(errno));
        goto Closefile;
    }

    printf("Device file size: %d\n", fileSize);

    buf = IO_Malloc(fileSize + 1);
    if (!buf) {
        printf("no enough memory: %s\n", strerror(errno));
        goto Closefile;
    }

    fileSize = fread(buf, fileSize, 1, file);
    if (fileSize != 1) {
        printf("read file failed: %s\n", strerror(errno));
        goto Freebuf;
    }
    
    printf("Load device file %s success.\n", filename);

    goto Closefile;
Freebuf:
    IO_Free(buf);
Closefile:
    fclose(file);
    return buf;
}

void ParseChildNode(cJSON* childNode, int ignoreObject)
{
    printf("    %s: ", childNode->string);

    if(childNode->type == cJSON_String)
    {
        printf("(string) %s\n", childNode->valuestring);
    }
    else if(childNode->type == cJSON_Number)
    {
        printf("(number) %d, %f\n", childNode->valueint, childNode->valuedouble);
    }
    else if(childNode->type == cJSON_False)
    {
        printf("False\n");
    }
    else if(childNode->type == cJSON_True)
    {
        printf("True\n");
    }
    else if(childNode->type == cJSON_NULL)
    {
        printf("NULL\n");
    }
    else if(childNode->type == cJSON_Array)
    {
        printf("    [\n");

        cJSON *tmp;
        int Size = cJSON_GetArraySize(childNode);
        int j;
        for(j = 0; j < Size; j++)
        {
            tmp = cJSON_GetArrayItem(childNode, j);
            if(tmp == NULL)
                continue;
            printf("        %s\n", tmp->valuestring);
        }
        printf("    ]\n");
    }
    else if (childNode->type == cJSON_Object)
    {
        if (ignoreObject != 0) {
            return;
        }
        printf("(node) %p\n", childNode);
        ParseNode(childNode);
    }
}

static void MakeNode(IO_DeviceNodeType *node, cJSON *json)
{
    char *p = json->string; /* name */
    char *name = p;
    char *id = NULL;
    char *addr = NULL;
    while(*p)
    {
        if(*p == ':')
        {
            *p = '\0';
            id = p + 1;
            if((*id == ':') || (*id == '@'))
                id = NULL;
        }
        else if(*p == '@')
        {
            *p = '\0';
            addr = p + 1;
            if((*addr == ':') || (*addr == '@'))
                addr = NULL;
        }
        p++;
    }
    node->object = json;
    node->name = (name && (*name != '\0')) ? name : "";
    node->id = (id && (*id != '\0')) ? strtol(id, NULL, 0) : 0;
    node->address = (addr && (*addr != '\0')) ? strtoull((const char *)addr, NULL, 0) : 0;

    printf("---> Node: name:%s id:%d addr:%p object:%p\n", node->name, node->id, node->address, node->object);
}

static void ProbeDriverCompatible(char *name, cJSON *node, cJSON *parentNode)
{
    IO_DriverType *driver;
    IO_DeviceType *device;
    IO_DeviceNodeType deviceNode;

    if (!IO_GetDriverByCompitable(name, &driver)) {
        if (driver->probeDevice != NULL) {
            /**
             * 转换成设备节点
             */
            MakeNode(&deviceNode, parentNode);
            if(strcmp(IO_GetNodeString(&deviceNode, "status", "okay"), "disabled") != 0)
            {
                device = driver->probeDevice(driver, &deviceNode);
                if (device != NULL) {
                    printf("[INFO] Probe device node %s driver %s device %s .\n", name, driver->name, device->name);
                }
            }
        }
    }
}

static void MatchDriverCompatible(cJSON *childNode, cJSON *parentNode)
{
    /**
     * 查找兼容属性
     */
    if (!strcmp(childNode->string, "compatible")) {
        /**
         * 如果是数组，就获取数组，不然就直接获取值。
         */
        if (childNode->type == cJSON_Array) {
            cJSON *tmp;
            int Size = cJSON_GetArraySize(childNode);
            int j;
            for(j = 0; j < Size; j++)
            {
                tmp = cJSON_GetArrayItem(childNode, j);
                if(tmp == NULL)
                    continue;
                ProbeDriverCompatible(tmp->valuestring, tmp, parentNode);
            }
        } else if (childNode->type == cJSON_String) {
            ProbeDriverCompatible(childNode->valuestring, childNode, parentNode);
        } else {
            printf("\n Bad compatible on %p\n", childNode);
        }
    }
}

void ParseNode(cJSON* node)
{
    printf("\nNode %s\n", node->string);
    cJSON *childNode;
    cJSON_ArrayForEach(childNode, node) {
        MatchDriverCompatible(childNode, node);
        /**
         * 递归遍历子节点。
         */
        ParseChildNode(childNode, 0);
    }
}

int IO_ParseDeviceTree(char *buf)
{
    if (!buf) {
        return -1;
    }

    /* 解析整段JSO数据 */
    RootNode = cJSON_Parse(buf);
    if(RootNode == NULL)
    {
        printf("Parse root node failed!\n");
        return -1;
    }

    /**
     * 根节点使用root临时名
     */
    RootNode->string = "root";

    ParseNode(RootNode);
    RootNode->string = NULL;

    cJSON_Delete(RootNode);
    
    return 0;
}

void IO_InitDeviceTree(void)
{
    cJSON_InitHooks(&IO_cJSON_Hooks);
}
