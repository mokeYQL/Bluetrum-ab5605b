#ifndef _FUNC_TOM_CAT_H
#define _FUNC_TOM_CAT_H

typedef struct {
    u8 rec_en      :   1,
       rec_type    :   1;   //0:按键录音  1:声控录音
    u16 rec_time_ms;
}func_tom_t;
extern func_tom_t f_tom;

void func_tom_cat_message(u16 msg);
void func_tom_cat_stop(void);
void func_tom_cat_start(void);

u8 tom_dnr_process(void);

void os_spiflash_read_loop(void *buf, u32 addr, uint len);
void os_spiflash_program_loop(void *buf, u32 addr, uint len);
void os_spiflash_erase_loop(u32 addr);
void os_spiflash_loop_init(u32 start_addr, u32 end_addr, u32 cur_start_addr);

void tom_cat_switch_cycfs_init(u8 rec_type);
void cycrec_fs_init(u32 begin_addr, u32 end_addr, int(*read)(void *buf, u32 addr, u32 len),void(*write)(void *buf, u32 addr, u32 len),void (*erase)(u32 addr) );


#if (GUI_SELECT != GUI_NO)
void func_tom_cat_display(void);
#else
#define func_tom_cat_display()
#endif

#endif // _FUNC_TOM_CAT_H
