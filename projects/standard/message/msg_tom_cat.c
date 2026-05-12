#include "include.h"
#include "func.h"
#include "bsp_spiflash1.h"
bool spifalsh_rec_play_last_file(void);

#if FUNC_TOM_CAT_EN
AT(.text.func.tom_cat.msg)
void func_tom_cat_message(u16 msg)
{
    switch (msg) {

        case KU_REC:
            printf("KU_REC\n");
            func_mp3_res_play(RES_BUF_REC_START_MP3, RES_LEN_REC_START_MP3);
            sfunc_record();
            if (rec_cb.flag_play) {
                rec_cb.flag_play = 0;
                func_tom_cat_stop();
                spifalsh_rec_play_last_file();
                func_tom_cat_start();
            }
            f_tom.rec_type = 0;
            break;

        case KU_PLAY:   //按键录音文件播放
            func_tom_cat_stop();
            tom_cat_switch_cycfs_init(0);
            spifalsh_rec_play_last_file();
            func_tom_cat_start();
            break;

        case KL_PLAY:   //声控录音文件播放.
            func_tom_cat_stop();
            tom_cat_switch_cycfs_init(1);
            spifalsh_rec_play_last_file();
            func_tom_cat_start();
            break;

        default:
            func_message(msg);
            break;
    }
}
#endif // FUNC_TOM_CAT_EN
