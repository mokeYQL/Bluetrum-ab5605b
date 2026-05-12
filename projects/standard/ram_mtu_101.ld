#include "config.h"

ENTRY(_start)

/* Define the flash max size */
__max_flash_size = FLASH_CODE_SIZE;

__base = 0x10000000;

/* Define the area */
__aram_vma = 0x20000;
__bram_vma = 0x18000;
__dram_vma = 0x58000;
__eram_vma = 0x24000;

__comm_vma = 0x14000;

MEMORY
{
    init            : org = __base,             len = 512
    flash(rx)       : org = __base + 512,       len = __max_flash_size
    comm(rx)        : org = __comm_vma,         len = 16k

    stack           : org = 0x10800,            len = 1k
    data            : org = 0x10C00,            len = 13k
    heap            : org = 0x1d400,            len = 11k
    aram            : org = __aram_vma,         len = 16k
    bram            : org = __bram_vma,         len = 21k
    dram            : org = __dram_vma,         len = 12k
    eram            : org = __eram_vma,         len = 4k
}

SECTIONS
{
    .init : {
        *(.reset)
    } > init

    .comm : {
        __comm_start = .;
        *(.vector)
        *(.plt)
        *(.com_text*)
        *(.com_text.bb.leisr)
        *(.com_text.stack.handler)
        *(.com_rodata*)
        *(.srodata*)
        *debug.o(.rodata*)
        *(.data*)
        *(.sdata*)
        *\ext_scan.o(.rodata*)
        *\ble_evt.o(.rodata*)
        *\hci_transport_h3.o(.rodata*)
        *(.load_text)
        *(.load_rodata)
        *(.btmem.bb.buf)
        __comm_end = .;
        . = ALIGN(512);
    } > comm AT > flash

    .flash : {
        __code_start_stream = .;
        *(.com_stream.text*)
        . = ALIGN(512);
        __code_end_stream = .;

        __code_start_record = .;
        *(.com_rec*)
        . = ALIGN(512);
        __code_end_record = .;

        __code_start_bt_comm = .;
        *(.bcom_text*)
        *(.bcom_rodata*)
        . = ALIGN(512);
        __code_end_bt_comm = .;

        __code_start_karaok_dednr = .;
        *(.com_mic_dnr*)
        . = ALIGN(512);
        __code_end_karaok_dednr = .;

        __code_start_mp3dec = .;
        *(.mp3dec*)
        . = ALIGN(512);
        __code_end_mp3dec = .;

        __code_start_sbcdec = .;
        *(.sbcdec*)
        *(.sbc_com*)
        . = ALIGN(512);
        __code_end_sbcdec = .;

        __code_start_sbcenc = .;
        *(.rodata.sbcenc*)
        *(.text.sbcenc*)
        . = ALIGN(512);
        __code_end_sbcenc = .;

        __code_start_mp3enc = .;
        *(.mpaenc*)
        . = ALIGN(512);
        __code_end_mp3enc = .;

        __code_start_wavdec = .;
        *(.wavdec*)
        . = ALIGN(512);
        __code_end_wavdec = .;

        __code_start_wmadec = .;
        *(.wmadec.bitstream)
        *(.wmadec.windows)
        *(.wmadec*)
        . = ALIGN(512);
        __code_end_wmadec = .;

        __code_start_apedec = .;
        *(.apedec*)
        . = ALIGN(512);
        __code_end_apedec = .;

        __code_start_flacdec = .;
        *(.flacdec*)
        . = ALIGN(512);
        __code_end_flacdec = .;

        __code_start_usbdev = .;
        *(.usbdev*)
        . = ALIGN(512);
        __code_end_usbdev = .;

        __code_start_wma_rlc44q = .;
        *(.wma_huff_rlc44q_table)
        . = ALIGN(512);
        __code_end_wma_rlc44q = .;

        __code_start_wma_rlc44o = .;
        *(.wma_huff_rlc44o_table)
        . = ALIGN(512);
        __code_end_wma_rlc44o = .;

        __code_start_wma_rlc16 = .;
        *(.wma_huff_rlc16_table)
        . = ALIGN(512);
        __code_end_wma_rlc16 = .;

        __code_start_sleep = .;
        *(.saradc_text*)
        *(.sleep_text*)
        *(.text.charge_com*)
        . = ALIGN(512);
        __code_end_sleep = .;

        __code_start_bb_test = .;
        *(.bb_test*)
        *(.bb_test_rodata*)
        . = ALIGN(512);
        __code_end_bb_test = .;

        __code_start_bt_voice = .;
        *(.bt_voice*)
        . = ALIGN(512);
        __code_end_bt_voice = .;

        __code_start_sco_nr = .;
        *(.bt_sco.alc)
        *(.text.alc.table)
        *(.text.aec.asm)
        *(.text.aec.util)
        *(.text.fft)
        *(.text.aec.ns)
        *(.text.aec.trum)
        *(.text.aec.core)
        *(.rodata.fft)
        *(.rodata.aec*)
        . = ALIGN(512);
        __code_end_sco_nr = .;

        __code_start_wsola = .;
        /*(.mav_com)    //SYS_MAGIC_VOICE_EN*/
        /*(.rodata.mav) //SYS_MAGIC_VOICE_EN*/
        . = ALIGN(512);
        __code_end_wsola = .;

         __code_start_piano = .;
        *(.piano_com*)
        . = ALIGN(512);
        __code_end_piano = .;

        __code_start_update = .;
        *(.text.update*)
        . = ALIGN(512);
        __code_end_update = .;

        __code_start_eq = .;
        *(.com_eq.text*)
        . = ALIGN(512);
        __code_end_eq = .;

        __code_start_spdif = .;
        *(.spdif_text*)
        . = ALIGN(512);
        __code_end_spdif = .;

        __code_start_rf = .;
        *(.text.rf_init*)
        . = ALIGN(512);
        __code_end_rf = .;

        __code_start_auxdnr = .;
        *(.text.aux_dnr*)
        *(.text.aux_sdadc*)
        . = ALIGN(512);
        __code_end_auxdnr = .;

        *(.irq_init.aligned)
        *(.text*)
        *(.rodata*)
        *(.rela*)
        LONG(0)
        . = ALIGN(512);

    } > flash

    .stack (NOLOAD) : {
        __irq_stack_start = .;
        . = 0x400;
        __irq_stack = .;
    } > stack
    __irq_stack_size = __irq_stack - __irq_stack_start;

    .bss (NOLOAD) : {
        __bss_start = .;
        *(COMMON)
        *(.bss*)
        *(.sbss*)
        *(.buf*)
    #if SYS_KARAOK_EN
        *(.sdadc_buf)
    #endif
        *(.dac_obuf)
        *(.btmem.stack.sdp)
        *(.gpdma_buf)
    #if BT_PBAP_EN||BT_MAP_EN
        *(.btmem_pbap.ram)
    #endif
    #if KARAOK_REC_EN
        *(.kara_rec_cache)
    #endif // KARAOK_REC_EN
        *(.btstack_hci.buf)
        __bss_end = .;
    } > data
    __bss_size = __bss_end - __bss_start;

    .heap (NOLOAD) : {
        *(.mem_heap)
    } > heap

    .eram __eram_vma (NOLOAD) : {
        *(.btmem.bb.le*)
        *(.btmem.ble.adv)
        *(.btmem.stack.hid)
        *(.btmem.stack.memory)
        *(.btmem.stack.st)
        *(.btmem.tws)
    } > eram

    .bram __bram_vma (NOLOAD) : {
        *(.btmem.bthw)
        *(.btmem*)
    } > bram

    .wma_btmem __bram_vma (NOLOAD) : {
        *(.wma_coefs_buf)
    } > bram

    .mp3enc_btmem __bram_vma (NOLOAD) : {
        *(.l2enc_sb_buf)
    } > bram

    .echo_btmem __bram_vma (NOLOAD) : {
        *(.echo_buf)
    } > bram


	.mpeg_btmem __bram_vma (NOLOAD) : {
		*(.mpeg_buf)
		*(.dual_buf)
		*(.speed_buf1)
	} > bram

    .i2s_btmem __bram_vma (NOLOAD) : {
        *(.i2s_cache*)
    } > bram

    .ble_buf __aram_vma (NOLOAD) : {
        . = 0x1000;
        *(.ble_buf*)
        *(.ble_cache*)
    } > aram

    .aec_ram __aram_vma (NOLOAD) : {
        *(.aec_ram.buf)
        *(.aec_buf*)
        *(.nr_cache*)
        *(.sco_eq_buf)
        *(.far_data.buf)
#if AEC_DUMP_FILE
        *(.dum_file.buf)
#endif
        . = 0x4000;
    } > aram

    .alc_ram __aram_vma (NOLOAD) : {
        . = 0x1000;
        *(.alc_ram.buf)
        *(.nr_buf*)
        *(.wavenc.aec)
#if (BT_HFP_CALL_KARAOK_EN && SYS_MAGIC_VOICE_EN)
        *(.mav_cache_flash)
#endif
        . = 0x3800;
        *(.echo_cache)
        . = 0x4000;
    } > aram

    .fcc_buf __aram_vma (NOLOAD) : {
        . = 0x1000;
        *(.fcc_buf.huart)
        *(.fcc_buf*)
    } > aram

    .sbc_cache __aram_vma (NOLOAD) : {
        . = 0x670;
        *(.a2dp_buf*)
        *(.mav_buf)
        . = 0x1000;
        *(.sbc_cache_buf)
        . = 0x3a2c;                                 //不能与avio_buf空间重叠，avio_buf播放提示音会使用

        . = 0x4000;
    } > aram

    .aram_music __aram_vma (NOLOAD) : {
        *(.mp3buf.dec)
        . = 0x1000;
        *(.pff_buf*)
        . = 0x1200;
        *(.udev_buf*)
        *(.usb_buf*)
        *(.mp3_id3_buf)
        *(.lrc_buf*)
        *(.pff.scan.tempbuf)
        . = 0x3800;
        *(.avio_buf)
        *(.diskio_buf)
        *(.pff.buf)
        *(.fname.buf)
        . = 0x4000;
    } > aram

    .aram_upd __aram_vma (NOLOAD) : {
        . = 0x3200;
        *(.upd_buf*)
        . = 0x3800;
    } > aram

    .aram_wma __aram_vma (NOLOAD) : {
        . = 0x1200;
        *(.wma_coefs_buf0)
        *(.wma_ctx_buf)
        *(.wma_wincb_buf)
        *(.wma_id3_buf)
        . = 0x3800;
    } > aram

    .aram_flac __aram_vma (NOLOAD) : {
        . = 0x1200;
        *(.flac_decoded_buf)
        . = 0x3800;
    } > aram

    .aram_fnav __aram_vma (NOLOAD) : {
        . = 0x1000;
        *(.fnav_buf)
        . = 0x2800;
    } > aram

    .aram_fmrx __aram_vma (NOLOAD) : {
        . = 0x1000;
        *(.fmrx_cb_buf)
    } > aram

    .aram_sbc __aram_vma (NOLOAD) : {
        *(.sbcbuf.dec)
        . = 0x670;
        *(.l2enc.buf*)
        . = 0x1800;
        *(.sco_mav_buf*)
        . = 0x3000;
    } > aram

    .sbcenc __aram_vma (NOLOAD) : {
        . = 0x670;
        *(.sbcenc.buf*)
        . = 0x1000;
    } > aram

    .aram_msc_stm __aram_vma (NOLOAD) : {
        *(.stm_buf)
        . = 0x1000;
    } > aram

    .dram_upd __dram_vma (NOLOAD) : {
        *(.ota_buf*)
    } > dram

    .dram_mp3 __dram_vma (NOLOAD) : {
        *(.mp3_decram_buf)
        . = 0x3000;
    } > dram

    .dram_ape __dram_vma (NOLOAD) : {
        *(.apebuf.cb)
        *(.apebuf.filter)
        *(.apebuf.decode)
        *(.apebuf.seektbl)
        . = 0x3000;
    } > dram

    .dram_ble __dram_vma (NOLOAD) : {
        *(.ble_rx_buf)
        . = 0x3000;
    } > dram

    .dram_wma __dram_vma (NOLOAD) : {
        *(.wmabuf*)
        . = 0x3000;
    } > dram

    .dram_flac __dram_vma (NOLOAD) : {
        *(.flacbuf*)
        *(.flac_decoded_buf1)
        *(.wavbuf.dec)
        . = 0x3000;
    } > dram

    .dram_sbc __dram_vma (NOLOAD) : {
        *(.sbc_decram_buf)
        . = 0xD38;
        *(.a2dp_play)
        *(.mav_cache)
        . = 0x3000;
     }  > dram



    .dram_sco __dram_vma (NOLOAD) : {
        *(.wavenc.sco)
        . = 0xD38;                              //0xd38之前为msbc解码空间
        *(.sco_cache)
        *(.sco_data)
        *(.sco_hci)
        *(.plc_buf)
        *(.plc_data)
        . = 0x3000;
    } > dram

    .dram_rec __dram_vma (NOLOAD) : {
        . = 0xD38;
        *(.rec.wave.header)
        *(.recenc.obuf)
        *(.rec.obuf)
        *(.l2enc_bit_buf)
        . = 0x3000;
    } > dram

    .dram_usb __dram_vma (NOLOAD) : {
        *(.usb.device.desc.buf)
        *(.ude.aubuf)
        . = 0x3000;
    } > dram

    .dram_spf __dram_vma (NOLOAD) : {
        *(.spf_buf)
        . = 0x3000;
    } > dram
}

/* Calc the lma */
__bank_size = SIZEOF(.flash);
__comm_lma = LOADADDR(.comm);
__comm_size = SIZEOF(.comm);
