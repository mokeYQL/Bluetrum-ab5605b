#ifndef _BSP_DAC_H
#define _BSP_DAC_H

//数字音量
#define MAX_DIG_VAL         32767

#define DIG_P18DB           (MAX_DIG_VAL / 0.125893)
#define DIG_P17DB           (MAX_DIG_VAL / 0.141254)
#define DIG_P16DB           (MAX_DIG_VAL / 0.158489)
#define DIG_P15DB           (MAX_DIG_VAL / 0.177828)
#define DIG_P14DB           (MAX_DIG_VAL / 0.199526)
#define DIG_P13DB           (MAX_DIG_VAL / 0.223872)
#define DIG_P12DB           (MAX_DIG_VAL / 0.251189)
#define DIG_P11DB           (MAX_DIG_VAL / 0.281838)
#define DIG_P10DB           (MAX_DIG_VAL / 0.316228)
#define DIG_P9DB            (MAX_DIG_VAL / 0.354813)
#define DIG_P8DB            (MAX_DIG_VAL / 0.398107)
#define DIG_P7DB            (MAX_DIG_VAL / 0.446684)
#define DIG_P6DB            (MAX_DIG_VAL / 0.501187)
#define DIG_P5DB            (MAX_DIG_VAL / 0.562341)
#define DIG_P4DB            (MAX_DIG_VAL / 0.630957)
#define DIG_P3DB            (MAX_DIG_VAL / 0.707946)
#define DIG_P2DB            (MAX_DIG_VAL / 0.794328)
#define DIG_P1DB            (MAX_DIG_VAL / 0.891251)
#define DIG_N0DB            (MAX_DIG_VAL / 1.000000)
#define DIG_N1DB            (MAX_DIG_VAL / 1.122018)
#define DIG_N2DB            (MAX_DIG_VAL / 1.258925)
#define DIG_N3DB            (MAX_DIG_VAL / 1.412538)
#define DIG_N4DB            (MAX_DIG_VAL / 1.584893)
#define DIG_N5DB            (MAX_DIG_VAL / 1.778279)
#define DIG_N6DB            (MAX_DIG_VAL / 1.995262)
#define DIG_N7DB            (MAX_DIG_VAL / 2.238721)
#define DIG_N8DB            (MAX_DIG_VAL / 2.511886)
#define DIG_N9DB            (MAX_DIG_VAL / 2.818383)
#define DIG_N10DB           (MAX_DIG_VAL / 3.162278)
#define DIG_N11DB           (MAX_DIG_VAL / 3.548134)
#define DIG_N12DB           (MAX_DIG_VAL / 3.981072)
#define DIG_N13DB           (MAX_DIG_VAL / 4.466836)
#define DIG_N14DB           (MAX_DIG_VAL / 5.011872)
#define DIG_N15DB           (MAX_DIG_VAL / 5.623413)
#define DIG_N16DB           (MAX_DIG_VAL / 6.309573)
#define DIG_N17DB           (MAX_DIG_VAL / 7.079458)
#define DIG_N18DB           (MAX_DIG_VAL / 7.943282)
#define DIG_N19DB           (MAX_DIG_VAL / 8.912509)
#define DIG_N20DB           (MAX_DIG_VAL / 10.000000)
#define DIG_N21DB           (MAX_DIG_VAL / 11.220185)
#define DIG_N22DB           (MAX_DIG_VAL / 12.589254)
#define DIG_N23DB           (MAX_DIG_VAL / 14.125375)
#define DIG_N24DB           (MAX_DIG_VAL / 15.848932)
#define DIG_N25DB           (MAX_DIG_VAL / 17.782794)
#define DIG_N26DB           (MAX_DIG_VAL / 19.952623)
#define DIG_N27DB           (MAX_DIG_VAL / 22.387211)
#define DIG_N28DB           (MAX_DIG_VAL / 25.118864)
#define DIG_N29DB           (MAX_DIG_VAL / 28.183829)
#define DIG_N30DB           (MAX_DIG_VAL / 31.622777)
#define DIG_N31DB           (MAX_DIG_VAL / 35.481339)
#define DIG_N32DB           (MAX_DIG_VAL / 39.810717)
#define DIG_N33DB           (MAX_DIG_VAL / 44.668359)
#define DIG_N34DB           (MAX_DIG_VAL / 50.118723)
#define DIG_N35DB           (MAX_DIG_VAL / 56.234133)
#define DIG_N36DB           (MAX_DIG_VAL / 63.095734)
#define DIG_N37DB           (MAX_DIG_VAL / 70.794578)
#define DIG_N38DB           (MAX_DIG_VAL / 79.432823)
#define DIG_N39DB           (MAX_DIG_VAL / 89.125094)
#define DIG_N40DB           (MAX_DIG_VAL / 100.000000)
#define DIG_N41DB           (MAX_DIG_VAL / 112.201845)
#define DIG_N42DB           (MAX_DIG_VAL / 125.892541)
#define DIG_N43DB           (MAX_DIG_VAL / 141.253754)
#define DIG_N44DB           (MAX_DIG_VAL / 158.489319)
#define DIG_N45DB           (MAX_DIG_VAL / 177.827941)
#define DIG_N46DB           (MAX_DIG_VAL / 199.526231)
#define DIG_N47DB           (MAX_DIG_VAL / 223.872114)
#define DIG_N48DB           (MAX_DIG_VAL / 251.188643)
#define DIG_N49DB           (MAX_DIG_VAL / 281.838293)
#define DIG_N50DB           (MAX_DIG_VAL / 316.227766)
#define DIG_N51DB           (MAX_DIG_VAL / 354.813389)
#define DIG_N52DB           (MAX_DIG_VAL / 398.107171)
#define DIG_N53DB           (MAX_DIG_VAL / 446.683592)
#define DIG_N54DB           (MAX_DIG_VAL / 501.187234)
#define DIG_N55DB           (MAX_DIG_VAL / 562.341325)
#define DIG_N56DB           (MAX_DIG_VAL / 630.957344)
#define DIG_N57DB           (MAX_DIG_VAL / 707.945784)
#define DIG_N58DB           (MAX_DIG_VAL / 794.328235)
#define DIG_N59DB           (MAX_DIG_VAL / 891.250938)
#define DIG_N60DB           0

extern const uint16_t tbl_sample_rate[10];

u8 bsp_volume_inc(u8 vol);
u8 bsp_volume_dec(u8 vol);
void bsp_change_volume(u8 vol);
void a2dp_set_vol(u8 a2dp_vol);
bool bsp_set_volume(u8 vol);
void dac_init(void);
void dac_set_anl_offset(u8 bt_call_flag);
void dac_dnr_init(u8 voice_cnt, u16 voice_pow, u8 silence_cnt, u16 silence_pow);
void dac_dnr_set_sta(u8 sta);
u8 dac_dnr_get_sta(void);

//VCMBUF_OUT_EN(bit)  bit取值为0或1,为1时打开VCMBUF电源(约1.35V),为0时关闭VCMBUF电源(此时该引脚为高阻状态,内部没有上/下拉)
#define VCMBUF_OUT_EN(bit)     AUANGCON0 = (AUANGCON0 & ~((1<<14)|(1<<26))) | ((bit<<14)|(bit<<26))
#endif // _BSP_DAC_H
