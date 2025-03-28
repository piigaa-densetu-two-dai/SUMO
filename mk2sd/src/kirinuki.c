/*
 * 切り抜きソース
 *
 * mk2sdファームウェアのソースコードの切り抜きです(一部要約)
 * PC-6001mk2_SDのPC-6001mk2_SD.inoに相当する部分です
 * PC-6001mk2_SD.inoを参考にしました
 */

#include <ctype.h>
#include <stdint.h>
#include <string.h>

#include "ff.h" /* ファイル操作にFatFs(http://elm-chan.org/fsw/ff/)を使用しています */

extern volatile uint8_t PORTA = 0;
extern volatile uint8_t PORTB = 0;
extern volatile uint8_t PORTC = 0;

#define PC0		0 /* p6_recv() -> p6_send()の間隔が短いと誤動作するのでこれで対策(PC6001がPORTCを読むとPC0が立つようにi8255の動作を勝手拡張) */
#define PC2		2
#define PC7		7

#define FNAME_MAX	33

static FATFS fatfs;
static FIL fil;

/* 送信 */
static void p6_send(const uint8_t data)
{
	PORTB = data;
	PORTC |= (1 << PC7);
	while (!(PORTC & (1 << PC2))); /* HIGHになるまでループ */
	PORTC &= ~(1 << PC7);
	while (PORTC & (1 << PC2)); /* LOWになるまでループ */

	return;
}

/* 受信 */
static uint8_t p6_recv(void)
{
	uint8_t data;

	while (!(PORTC & (1 << PC2))); /* HIGHになるまでループ */
	data = PORTA << 4; /* 上位4ビット受信 */
	PORTC |= (1 << PC7); /* FLGをセット */
	while (PORTC & (1 << PC2)); /* LOWになるまでループ */
	PORTC &= ~(1 << PC7); /* FLGをリセット */

	while (!(PORTC & (1 << PC2))); /* HIGHになるまでループ */
	data |= PORTA & 0b00001111; /* 下位4ビット受信 */
	PORTC |= (1 << PC7); /* FLGをセット */
	while (PORTC & (1 << PC2)); /* LOWになるまでループ */
	PORTC &= ~((1 << PC7) | (1 << PC0)); /* FLGをリセット */

	while (!(PORTC & (1 << PC0))); /* PC6001がPORTCを読むのを待つ ここオリジナルにない処理 */

	return data;
}

/* 文字列受信 */
static void recv_name(char *name)
{
	uint8_t data;

	for (uint8_t i = 0, j = 0 ; i < FNAME_MAX ; i++) {
		if ((data = p6_recv()) != '"') { /* '"'は無視 */
			name[j++] = toupper(data); /* 大文字にしてみる */
		}
	}

	return;
}

/* 拡張子がp6tでなければp6tを付加 */
static void add_p6t(char *name)
{
	char *ptr;

	if (!(ptr = strrchr(name, '.'))) {
		strcat(name, ".P6T");
		return;
	}
	if (!strcasecmp(ptr, ".P6T")) {
		return;
	}
	strcat(name, ".P6T");

	return;
}

/* 拡張子がp6/casでなければcasを付加 */
static void add_cas(char *name)
{
	char *ptr;

	if (!(ptr = strrchr(name, '.'))) {
		strcat(name, ".CAS");
		return;
	}
	if (!strcasecmp(ptr, ".P6")) {
		return;
	}
	if (!strcasecmp(ptr, ".CAS")) {
		return;
	}
	strcat(name, ".CAS");

	return;
}

/* p6t_v1ヘッダ */
struct __attribute__ ((__packed__)) p6t_v1 {
	uint8_t magic[2];
	uint8_t version;
	uint8_t start;
	uint8_t mode;
	uint8_t page;
	uint16_t cmdlen;
	uint8_t cmd[255];
};

/* p6t_v2ヘッダ */
struct __attribute__ ((__packed__)) p6t_v2 {
	uint8_t magic[2];
	uint8_t version;
	uint8_t block;
	uint8_t start;
	uint8_t mode;
	uint8_t page;
	uint16_t cmdlen;
	uint8_t cmd[255];
};

/* p6t_v1ヘッダのモード、ページ、コマンド文字列を送信 */
static void p6t_info_v1(void)
{
	struct p6t_v1 p6t_v1;
	UINT ret;
	uint8_t data;

	f_rewind(&fil);
	if (f_read(&fil, &p6t_v1, sizeof(p6t_v1), &ret) != FR_OK) {
		p6_send(0xf0);
		return;
	}
	if (ret < offsetof(struct p6t_v1, cmd)) {
		p6_send(0xf0);
		return;
	}
	if ((p6t_v1.magic[0] != 'P') || (p6t_v1.magic[1] != '6') || (p6t_v1.version != 1)) {
		p6_send(0xf0);
		return;
	}
	if ((0 == p6t_v1.mode) || (6 <= p6t_v1.mode)) {
		p6_send(0xf0);
		return;
	}
	while (1) {
		if (f_read(&fil, &data, sizeof(data), &ret) != FR_OK) {
			p6_send(0xf0);
			return;
		}
		if (ret != sizeof(data)) {
			p6_send(0xf0);
			return;
		}
		if (data == 0xd3) {
			f_lseek(&fil, f_tell(&fil) - 1);
			break;
		}
	}

	p6_send(0x00);
	p6_send(p6t_v1.mode); /* モード */
	p6_send(p6t_v1.page); /* ページ */
	p6_send(p6t_v1.cmdlen); /* コマンド文字数(8bitに切り詰め) */
	for (uint8_t i = 0 ; i < (uint8_t)p6t_v1.cmdlen ; i++) { /* コマンド文字列 */
		p6_send(p6t_v1.cmd[i]);
	}

	return;
}

/* p6t_v2ヘッダのモード、ページ、コマンド文字列を送信 */
static void p6t_info_v2(void)
{
	uint32_t size;
	UINT ret;
	struct p6t_v2 p6t_v2;

	if (f_lseek(&fil, f_size(&fil) - sizeof(size)) != FR_OK) {
		p6_send(0xf0);
		return;
	}
	if (f_read(&fil, &size, sizeof(size), &ret) != FR_OK) {
		p6_send(0xf0);
		return;
	}
	if (ret != sizeof(size)) {
		p6_send(0xf0);
		return;
	}
	if (f_lseek(&fil, size) != FR_OK) {
		p6_send(0xf0);
		return;
	}
	if (f_read(&fil, &p6t_v2, sizeof(p6t_v2), &ret) != FR_OK) {
		p6_send(0xf0);
		return;
	}
	if (ret < offsetof(struct p6t_v2, cmd)) {
		p6_send(0xf0);
		return;
	}
	if ((p6t_v2.magic[0] != 'P') || (p6t_v2.magic[1] != '6') || (p6t_v2.version != 2)) {
		p6_send(0xf0);
		return;
	}
	if ((0 == p6t_v2.mode) || (6 <= p6t_v2.mode)) {
		p6_send(0xf0);
		return;
	}
	f_rewind(&fil);

	p6_send(0x00); 
	p6_send(p6t_v2.mode); /* モード */
	p6_send(p6t_v2.page); /* ページ */
	p6_send(p6t_v2.cmdlen); /* コマンド文字数(8bitに切り詰め) */
	for (uint8_t i = 0 ; i < (uint8_t)p6t_v2.cmdlen ; i++) { /* コマンド文字列 */
		p6_send(p6t_v2.cmd[i]);
	}

	return;
}

/* p6tヘッダのモード、ページ、コマンド文字列を送信 */
static void p6t_info(void)
{
	uint8_t data[2] = { 0 };
	UINT ret;

	f_read(&fil, data, sizeof(data), &ret);
	if ((data[0] == 'P') && (data[1] == '6')) {
		p6t_info_v1();
	} else {
		p6t_info_v2();
	}

	return;
}

/* p6tファイル読み込みオープン */
static void p6t_openr(void)
{
	char name[FNAME_MAX + 4]; /* FNAME_MAX + ".p6t" */

	recv_name(name);
	add_p6t(name);

	if (fil.obj.fs) {
		f_close(&fil);
	}
	if (f_open(&fil, name, FA_OPEN_EXISTING | FA_READ) != FR_OK) {
		p6_send(0xf1);
		return;
	}
	p6_send(0x00);

	p6t_info();

	return;
}

/* p6/casファイル読み込みオープン */
static void p6_openr(void)
{
	char name[FNAME_MAX + 4]; /* FNAME_MAX + ".cas" */

	recv_name(name);
	add_cas(name);

	if (fil.obj.fs) {
		f_close(&fil);
	}
	if (f_open(&fil, name, FA_OPEN_EXISTING | FA_READ) != FR_OK) {
		p6_send(0xf1);
		return;
	}
	p6_send(0x00);

	return;
}

/* p6/casファイル書込みオープン (BASIC用) */
static void p6_openw(void)
{
	char name[FNAME_MAX + 4]; /* FNAME_MAX + ".cas" */

	recv_name(name);
	if (name[0] == 0x00) {
		p6_send(0xf1);
		return;
	}
	add_cas(name);

	if (fil.obj.fs) {
		f_close(&fil);
	}
	if (f_open(&fil, name, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {
		p6_send(0xf0);
		return;
	}

	p6_send(0x00); /* OK */

	return;
}

/* ファイル読み込み、送信 */
static void p6_read(void)
{
	uint8_t data;
	UINT ret;

	f_read(&fil, &data, sizeof(data), &ret);
	p6_send(data);

	return;
}

/* 受信、ファイル書込み (BASIC用) */
static void p6_write(void)
{
	uint8_t data;
	static uint8_t header;
	UINT ret;

	data = p6_recv();
	if (f_tell(&fil) == 0) {
		header = data;
	}
	/* 2バイト目から10バイト目が0x00になるので0xd3に置換え */
	if ((f_tell(&fil) < 10) && (header == 0xd3) && (data == 0x00)) {
		data = 0xd3;
	}
	f_write(&fil, &data, sizeof(data), &ret);

	return;
}

/* ファイルクローズ (BASIC用) */
static void p6_close(void)
{
	if (fil.flag & FA_WRITE) { /* 書込みモードの時のみクローズ */
		f_close(&fil);
	}

	return;
}

/* ルートディレクトリ検索 */
static void dirlist(void)
{
	char pattern[FNAME_MAX];
	FRESULT fr;
	DIR dir;
	FILINFO fno;
	uint16_t cnt = 0;

	recv_name(pattern);
	if (pattern[0] == 0) { /* パターン未指定の場合 */
		strcpy(pattern, "*");
	}

	p6_send(0x00); /* OK */

	fr = f_findfirst(&dir, &fno, "", pattern);

	while ((fr == FR_OK) && fno.fname[0]) {
		if (fno.fattrib & AM_DIR) { /* ディレクトリならスキップ */
			fr = f_findnext(&dir, &fno);
			continue;
		}

		for (uint8_t i = 0 ; i <= 36 ; i++) {
			if (fno.fname[i] == 0) {
				break;
			}
			p6_send(toupper(fno.fname[i]));
		}
		p6_send('\r');
		p6_send(0x00);

		if ((++cnt % 10) == 0) { /* 10ファイル毎に中断 */
			p6_send(0xfe); /* 問い合わせ */

			uint8_t data = p6_recv();
			if (data == 'B') { /* 前ページへ */
				cnt -= MIN(cnt, 20);
				f_closedir(&dir);
				fr = f_findfirst(&dir, &fno, "", pattern);
				for (uint16_t i = 0 ; i < cnt ; ) { 
					if (!(fno.fattrib & AM_DIR)) {
						i++;
					}
					fr = f_findnext(&dir, &fno);
				}
				continue;
			} else if (data == 0xff) { /* 終了 */
				break;
			}
		}

		fr = f_findnext(&dir, &fno);
	}

	f_closedir(&dir);

	/* 終了 */
	p6_send(0xff);
	p6_send(0x00);

	return;
}

void p6_loop(void)
{
	while (1) {
		switch (p6_recv()) { /* コマンド取得待ち */
			case 0x61: /* ファイルリスト出力 */
				p6_send(0x00); /* OK */
				dirlist();
				break;
			case 0x62: /* PC-6001 p6tファイル LOAD */
				p6_send(0x00); /* OK */
				p6t_openr();
				break;
			case 0x63: /* PC-6001 p6ファイル LOAD */
				p6_send(0x00); /* OK */
				p6_openr();
				break;
			case 0x64: /* PC-6001 LOAD ONE BYTE FROM CMT */
				p6_send(0x00); /* OK */
				p6_read();
				break;
			case 0x65: /* PC-6001 LOAD ONE BYTE FROM CMT(MODE5) */
				p6_send(0x00); /* OK */
				p6_read();
				break;
			case 0x66: /* PC-6001 BASIC 1Byte 受信(MODE5) */
				p6_send(0x00); /* OK */
				p6_write();
				break;
			case 0x67: /* PC-6001 BASIC SAVE開始 */
				p6_send(0x00); /* OK */
				p6_openw();
				break;
			case 0x68: /* PC-6001 BASIC 1Byte 受信 */
				p6_send(0x00); /* OK */
				p6_write();
				break;
			case 0x69: /* PC-6001 BASIC SAVE終了 */
				p6_send(0x00); /* OK */
				p6_close();
				break;
			default:
				p6_send(0xf4); /* CMD ERROR */
				break;
		}
	}
}
