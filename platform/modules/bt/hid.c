#include "include.h"
#include "api.h"

void bsp_bt_hid_keyboard(u16 keycode);
void bsp_bt_hid_mouse(bool is_press, s16 x, s16 y);

uint8_t cfg_hid_type = BT_HID_TYPE;

#if BT_HID_MENU_EN
void bsp_bt_hid_tog_conn(void)
{
    if (xcfg_cb.bt_hid_menu_en) {
        func_bt_chkclr_warning(BT_WARN_HID_CON|BT_WARN_HID_DISCON);
        f_bt.hid_menu_flag = 2;     //按键连接/断开HID，置上标志，用于播放提示音
        if (bt_hid_is_connected()) {
            bt_hid_disconnect();
        } else {
            bt_hid_connect();
        }
    }
}
#endif


bool bsp_bt_hid_vol_change(u16 keycode)
{
#if BT_A2DP_VOL_HID_CTRL_EN
    if ((xcfg_cb.bt_hid_en) && (bt_hid_is_connected())) {
        printf("hid vol consumer : %X\n",keycode);
        bt_hid_consumer(keycode);               //consumer key vol_up or vol_down, ios
        return true;
    }
#endif
    return false;
}

void bsp_bt_hid_photo(void)
{
#if BT_HID_TYPE==HID_TYPE_DOUYIN
    bsp_bt_hid_keyboard(HID_VOL_DOWN);
    delay_5ms(20);
    bsp_bt_hid_keyboard(HID_KBD_RELEASE);
#else
    bt_hid_key(HID_KEY_ENTER);              //enter key, android 4.0以上
    delay_5ms(10);
    bt_hid_consumer(HID_KEY_VOL_UP);        //consumer key vol_up, ios
#endif // BT_HID_TYPE
}

//点击左边屏幕中心位置
void bsp_bt_hid_screen_left(void)
{
    bt_hid_touch_screen(0);
    delay_5ms(10);
    bt_hid_touch_screen(1);
}

//点击右边屏幕中心位置
void bsp_bt_hid_screen_right(void)
{
    bt_hid_touch_screen(0x11);
    delay_5ms(10);
    bt_hid_touch_screen(0x10);
}

#if BT_TOUCH_SCREEN_EN
struct ts_ctl_t ts_ctl_temp;
void hid_data_send(void)
{
    struct ts_ctl_t *p = &ts_ctl_temp;
    p->act = 1;  //触摸按下是1，松开是0
    p->x = 0x6400; //改变坐标位置
    p->y = 0x7902;
    bt_hid_touch_screen_set_key(p);
    delay_5ms(10);
    p->act = 1;  //触摸按下是1，松开是0
    p->x = 0x6400;
    p->y = 0x7e02;
    bt_hid_touch_screen_set_key(p);
    delay_5ms(10);
    p->act = 1;  //触摸按下是1，松开是0
    p->x = 0x5f00;
    p->y = 0x7e02;
    bt_hid_touch_screen_set_key(p);
    delay_5ms(10);
    p->act = 0;  //松开0
    p->x = 0x5f00;
    p->y = 0x7e02;
    bt_hid_touch_screen_set_key(p);
}

//bt_hid_consumer(0x01);  //按键用这个函数发HID信息
#endif


#if BT_HID_EN && (BT_HID_TYPE == HID_TYPE_DOUYIN)

/**************************触摸屏*******************************************/

static u8 finger_buff[9];
/**
 * @brief 模拟触点函数。x，y是绝对位置，10,10是在手机10,10的位置上;
 * @param is_press	1按下，0抬起
 * @param x	模拟触点横坐标
 * @param y	模拟触点纵坐标
 **/
void bsp_bt_hid_finger(bool is_press, s16 x, s16 y)
{
	if(!bt_hid_is_connected()) {
		return;
	}
    memset(finger_buff, 0, 9);

    douyin_hid_code = finger_buff;

    finger_buff[0] = RPT_ID_DOUYIN_FINGER;
    finger_buff[1] = is_press?0x07:0x00;
    finger_buff[2] = 0x05;
    finger_buff[3] = (u8)((u16)x >> 0);
    finger_buff[4] = (u8)((u16)x >> 8);
    finger_buff[5] = (u8)((u16)y >> 0);
    finger_buff[6] = (u8)((u16)y >> 8);
    finger_buff[7] = is_press;

    bt_hid_send_key(HID_DOUYIN, 0);
    delay_5ms(16);

}
/**
 *    向下滑
 */
void user_finger_down(void)
{
    bsp_bt_hid_finger(1, 2000, 1000);
    bsp_bt_hid_finger(1, 2000, 1100);
    bsp_bt_hid_finger(1, 2000, 1400);
    bsp_bt_hid_finger(1, 2000, 2000);
    bsp_bt_hid_finger(1, 2000, 2800);
    bsp_bt_hid_finger(0, 2000, 3000);

}


/**
 *    向上滑
 */
void user_finger_up(void)
{
   bsp_bt_hid_finger(1, 2000, 3000);
   bsp_bt_hid_finger(1, 2000, 2900);
   bsp_bt_hid_finger(1, 2000, 2700);
   bsp_bt_hid_finger(1, 2000, 2300);
   bsp_bt_hid_finger(1, 2000, 1000);
   bsp_bt_hid_finger(0, 2000, 1000);
}

/**
 *    滑动返回
 */
void user_finger_r2l(void)
{
    bsp_bt_hid_finger(1, 00, 1700);
    bsp_bt_hid_finger(1, 100, 1700);
    bsp_bt_hid_finger(1, 200, 1700);
    bsp_bt_hid_finger(1, 300, 1700);
    bsp_bt_hid_finger(1, 400, 1700);
    bsp_bt_hid_finger(0, 400, 1700);

}

/**
 *    单击
 */
void user_finger_p(void)
{
    bsp_bt_hid_finger(1, 2000, 2000);
    bsp_bt_hid_finger(0, 2000, 2000);
}


/**
 *    双击
 */
void user_finger_pp(void)
{

    bsp_bt_hid_finger(1, 2000, 2000);
    bsp_bt_hid_finger(0, 2000, 2000);
    bsp_bt_hid_finger(1, 2000, 2000);
    bsp_bt_hid_finger(0, 2000, 2000);

}

/**************************鼠标键盘*******************************************/

static u8 point_buff[9];
/**
 * @brief 鼠标滑轮函数

 * @param wheel	纵向滑轮
 * @param acpan	水平滑轮
 **/
AT(.text.hid.douyin)
void bsp_bt_hid_pully(int8_t wheel,int8_t acpan)
{
    if(!bt_hid_is_connected()) {
		return;
	}

    if(wheel < -127){
        wheel = -127;
    }
    if(acpan < -127){
        acpan = -127;
    }

    if(wheel > 127){
        wheel = 127;
    }
    if(acpan > 127){
        acpan = 127;
    }

    memset(point_buff, 0, 9);
    douyin_hid_code = point_buff;
    point_buff[0] = RPT_ID_DOUYIN_POINTER;
    point_buff[1] = 0;
    point_buff[2] = wheel;
    point_buff[3] = acpan;
//    print_r(point_buff, 9);
    bt_hid_send_key(HID_DOUYIN, 0);
    delay_5ms(1);
}


/**
 * @brief 鼠标按键函数
 * @param button	按键，一个位代表一个按键
 **/
AT(.text.hid.douyin)
void hid_mouse_press(u8 button){
	if(!bt_hid_is_connected()) {
		return;
	}
    memset(point_buff, 0, 9);

    douyin_hid_code = point_buff;

    point_buff[0] = RPT_ID_DOUYIN_POINTER;
    point_buff[1] = button;
//    print_r(point_buff, 9);
    bt_hid_send_key(HID_DOUYIN, 0);
    delay_5ms(1);

}

/**
 * @brief 鼠标滑动函数。x,y是相对位置，比如10,10是相对当前位置移动10,10;
 * @param is_press	1按下，0抬起
 * @param x	模拟触点横坐标
 * @param y	模拟触点纵坐标
 **/
AT(.text.hid.douyin)
void hid_mouse_slide(s16 x, s16 y){
	if(!bt_hid_is_connected()) {
		return;
	}
    memset(point_buff, 0, 9);

    douyin_hid_code = point_buff;

    if(x < -2047){
        x = -2047;
    }
    if(y < -2047){
        y = -2047;
    }

    if(x > 2048){
        x = 2048;
    }
    if(y > 2048){
        y = 2048;
    }

    point_buff[0] = RPT_ID_DOUYIN_MOUSE;
    point_buff[1] = (u8)((u16)x >> 0);
    point_buff[2] = (u8)((((u16)x >> 8) & 0x0f) | (((u16)y & 0x0f) << 4));
    point_buff[3] = (u8)((u16)y >> 4);

//    print_r(point_buff, 9);
    bt_hid_send_key(HID_DOUYIN, 0);
    delay_5ms(5);

}

AT(.text.hid.douyin)
void bsp_bt_hid_mouse(bool is_press, s16 x, s16 y)
{
	if(!bt_hid_is_connected()) {

		return;
	}
    memset(point_buff, 0, 9);

    douyin_hid_code = point_buff;

    static bool is_press_bk = 0;

    if(x < -2047){
        x = -2047;
    }
    if(y < -2047){
        y = -2047;
    }

    if(x > 2048){
        x = 2048;
    }
    if(y > 2048){
        y = 2048;
    }
    if(is_press_bk != is_press){
        is_press_bk = is_press;
        point_buff[0] = RPT_ID_DOUYIN_POINTER;
        point_buff[1] = is_press;
//        print_r(point_buff, 9);
        bt_hid_send_key(HID_DOUYIN, 0);
        delay_5ms(16);
    }
    if(x || y){
        point_buff[0] = RPT_ID_DOUYIN_MOUSE;
        point_buff[1] = (u8)((u16)x >> 0);
        point_buff[2] = (u8)((((u16)x >> 8) & 0x0f) | (((u16)y & 0x0f) << 4));
        point_buff[3] = (u8)((u16)y >> 4);

//        print_r(point_buff, 9);
        bt_hid_send_key(HID_DOUYIN, 0);
        delay_5ms(16);
    }

}

//键盘
static u8 hid_dy_key_buff[3];
AT(.text.hid.douyin)
void bsp_bt_hid_keyboard(u16 keycode)
{
    memset(hid_dy_key_buff, 0, 3);
    douyin_hid_code = hid_dy_key_buff;
    hid_dy_key_buff[0] = RPT_ID_DOUYIN_KBD;
    hid_dy_key_buff[1] = (u8)(keycode & 0xff);
    hid_dy_key_buff[2] = (u8)(keycode >> 8);
    bt_hid_send_key(HID_DOUYIN, 0);

    delay_5ms(16);
}

/**
 *    向下滑
 */
void user_mouse_down(void)
{
    if(bt_is_ios_device()){
        bsp_bt_hid_mouse(0, -2047, -2047);      //这两步是把指针回到左上角去
        bsp_bt_hid_mouse(0, 60, 60);       	    //移动到上中间
        bsp_bt_hid_mouse(1, 0, 10);       	    //按住按键，移动往下移动
        bsp_bt_hid_mouse(1, 0, 40);       	    //按住按键，移动往下移动
        bsp_bt_hid_mouse(1, 0, 100);       	    //按住按键，移动往下移动
        bsp_bt_hid_mouse(0, 0, 0);       	    //松开按键
        bsp_bt_hid_mouse(0, -2047, -2047);      //这两步是把指针回到左上角去
    }else{
        bsp_bt_hid_mouse(0, -2047, -2047);      //这两步是把指针回到左上角去
        bsp_bt_hid_mouse(0, 400, 300);       	//移动到上中间
        for(u8 i=1;i<5;i++){
            bsp_bt_hid_mouse(1, 0, 100+i*100);  //按住按键，移动往下移动
        }
        bsp_bt_hid_mouse(0, 0, 0);       	    //松开按键
        bsp_bt_hid_mouse(0, -2047, -2047);      //这两步是把指针回到左上角去
    }
}

/**
 *    向上滑
 */
void user_mouse_up(void)
{
    if(bt_is_ios_device()){
        bsp_bt_hid_mouse(0, -2047, 2047);  	    //这两步是把指针回到左下角去
        bsp_bt_hid_mouse(0, 60, -60);           //移动到下中间
        bsp_bt_hid_mouse(1, 0, -10);       	    //按住按键，移动往下移动
        bsp_bt_hid_mouse(1, 0, -40);       	    //按住按键，移动往下移动
        bsp_bt_hid_mouse(1, 0, -100);           //按住按键，移动往下移动
        bsp_bt_hid_mouse(0, 0, 0);       	    //松开按键
        bsp_bt_hid_mouse(0, -2047, 2047);  	    //这两步是把指针回到左下角去
    }else{
        bsp_bt_hid_mouse(0, -2047, 2047);  	    //这两步是把指针回到左下角去
        bsp_bt_hid_mouse(0, 400, -300);         //移动到下中间
        for(u8 i=1;i<5;i++){
            bsp_bt_hid_mouse(1, 0, -100-i*100); //按住按键，移动往下移动
        }
        bsp_bt_hid_mouse(0, 0, 0);       	    //松开按键
        bsp_bt_hid_mouse(0, -2047, 2047);  	    //这两步是把指针回到左下角去
    }
}

/**
 *    单击
 */
void user_mouse_p(void)
{
    if(bt_is_ios_device()){
        bsp_bt_hid_mouse(0, -2047, -2047);      //这两步是把指针回到左上角去
        bsp_bt_hid_mouse(0, 60, 100);           //移动到上中间
        bsp_bt_hid_mouse(1, 0, 0);       	    //按住按键，移动往下移动
        bsp_bt_hid_mouse(0, 0, 0);       	    //松开按键
        bsp_bt_hid_mouse(0, -2047, -2047);      //这两步是把指针回到左上角去
    }else{
        bsp_bt_hid_mouse(0, -2047, -2047);      //这两步是把指针回到左上角去
        bsp_bt_hid_mouse(0, 400, 400);          //移动到上中间
        bsp_bt_hid_mouse(1, 0, 0);       	    //按住按键，移动往下移动
        bsp_bt_hid_mouse(0, 0, 0);       	    //松开按键
        bsp_bt_hid_mouse(0, -2047, -2047);      //这两步是把指针回到左上角去
    }
}


/**
 *    双击
 */
void user_mouse_pp(void)
{
    if(bt_is_ios_device()){
        bsp_bt_hid_mouse(0, -2047, -2047);      //这两步是把指针回到左上角去
        bsp_bt_hid_mouse(0, 60, 100);           //移动到上中间
        bsp_bt_hid_mouse(1, 0, 0);       	    //按住按键，移动往下移动
        bsp_bt_hid_mouse(0, 0, 0);       	    //按住按键，移动往下移动
        bsp_bt_hid_mouse(1, 0, 0);       	    //按住按键，移动往下移动
        bsp_bt_hid_mouse(0, 0, 0);       	    //松开按键
        bsp_bt_hid_mouse(0, -2047, -2047);      //这两步是把指针回到左上角去
    }else{
        bsp_bt_hid_mouse(0, -2047, -2047);      //这两步是把指针回到左上角去
        bsp_bt_hid_mouse(0, 400, 400);          //移动到上中间
        bsp_bt_hid_mouse(1, 0, 0);       	    //按住按键，移动往下移动
        bsp_bt_hid_mouse(0, 0, 0);       	    //松开按键
        bsp_bt_hid_mouse(1, 0, 0);       	    //按住按键，移动往下移动
        bsp_bt_hid_mouse(0, 0, 0);       	    //松开按键
        bsp_bt_hid_mouse(0, -2047, -2047);      //这两步是把指针回到左上角去
    }
}

// reference scheme
AT(.text.hid.douyin)
bool hid_mouse_handler(u8 opcode)
{
    static u8 cur_opcode  = HID_MOUSE_MAX;
    if(!bt_hid_is_connected()|| opcode > HID_MOUSE_MAX) {
		return false;
	}
	/*状态切换时 复位鼠标位置*/
	if(cur_opcode != opcode ){
        cur_opcode = opcode;
        if(opcode < HID_MOUSE_WHEEL_UP){
            hid_mouse_slide(-2047, 2047);//回到左下角
            hid_mouse_slide(160, -381);
            hid_mouse_slide(0, 0);
            hid_mouse_slide(0, 0);
            delay_5ms(5);
        }else if(opcode > HID_MOUSE_DOWN_SLIDE){
            hid_mouse_slide(-2047, -2047);//回到左上角
            hid_mouse_slide(-2047, -2047);//回到左上角
            hid_mouse_slide(0, -2047);
            hid_mouse_slide(60, 120);
        }
	}

	switch(opcode){

    case HID_MOUSE_UP_SLIDE:
        hid_mouse_slide(-2047, 2047);//回到左下角
        hid_mouse_slide(160, -381);
        hid_mouse_slide(0, -82);
        hid_mouse_press(1);          //按下
        hid_mouse_slide(0, 80);
        hid_mouse_slide(0, 80);
        hid_mouse_slide(0, 80);
        hid_mouse_press(0);          //松开
        hid_mouse_slide(-2047, 2047);
        break;
    case HID_MOUSE_DOWN_SLIDE:
        hid_mouse_slide(-2047, 2047);//回到左下角
        hid_mouse_slide(160, -381);
        hid_mouse_slide(0, 80);

        hid_mouse_press(1);

        hid_mouse_slide(0, -82);
        hid_mouse_slide(0, -82);
        hid_mouse_slide(0, -82);

        hid_mouse_press(0);

        hid_mouse_slide(-2047, 2047);
        break;
    case HID_MOUSE_BUTTON_1:
        hid_mouse_press(1);
        delay_5ms(5);
        hid_mouse_press(0);
        break;
    case HID_MOUSE_WHEEL_UP:
        bsp_bt_hid_pully(-46,0);
        delay_5ms(5);
        bsp_bt_hid_pully(-46,0);
        delay_5ms(5);
        bsp_bt_hid_pully(-6,0);
        delay_5ms(5);
        break;
    case HID_MOUSE_WHEEL_DOWN:
        bsp_bt_hid_pully(50,0);
        delay_5ms(5);
        bsp_bt_hid_pully(50,0);
        delay_5ms(5);
        bsp_bt_hid_pully(40,0);
        delay_5ms(5);
        break;
    case HID_MOUSE_ACPAN_UP:
        bsp_bt_hid_pully(0,-80);
        delay_5ms(5);
        bsp_bt_hid_pully(0,-80);
        break;
    case HID_MOUSE_ACPAN_DOWN:
        bsp_bt_hid_pully(0,80);
        delay_5ms(5);
        bsp_bt_hid_pully(0,80);
        break;
	default:
        break;

	}
	return true;
}
#else
AT(.text.hid.douyin)
void bsp_bt_hid_keyboard(u16 keycode){}
bool hid_mouse_handler(u8 opcode){ return false;}
#endif // BT_HID_EN && (BT_HID_TYPE == HID_TYPE_DOUYIN)

