#include "include.h"


#if BT_PBAP_EN

//获取IOS的电话本，需要在bt_get_class_of_device()中把COD改为0x240480

struct pbap_buf_t {
    char name[160];
    char anum[20];
    char bnum[20];
};

//type:本地号码[0], 来电号码[1], 去电号码[2], 未接号码[3]
void bt_pbap_data_callback(u8 type, void *item)
{
    //注意函数内不要进行耗时大的操作，会影响电话本获取的速度
    struct pbap_buf_t *p = (struct pbap_buf_t *)item;
    printf("[%d] [name:%s]  ", type, p->name);
    printf("[tele:%s]  ", p->anum);
    if (type) {
        printf("[date:%s]\n", p->bnum);
    } else {
        if (p->bnum[0]) {
            printf("[%s]\n", p->bnum);
        } else {
            printf("\n");
        }
    }
}
#endif
