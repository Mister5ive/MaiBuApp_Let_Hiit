#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "maibu_sdk.h"
#include "maibu_res.h"

/*大圆圆心半径*/
#define HIIT_MAIN_CIRCLE_R 40
#define HIIT_MAIN_CIRCLE_X 64
#define HIIT_MAIN_CIRCLE_Y 64

/*小圆圆心半径*/
#define HIIT_MOVE_CIRCLE_R 3
#define HIIT_MOVE_CIRCLE_X 64
#define HIIT_MOVE_CIRCLE_Y 104


/*主页按键位置*/
#define HIIT_MAIN_BUTTON_ORIGIN_X			115	
#define HIIT_MAIN_BUTTON_ORIGIN_Y			0
#define HIIT_MAIN_BUTTON_SIZE_H				128	
#define HIIT_MAIN_BUTTON_SIZE_W				10	

#define PI 3.1415926

/*文件KEY*/
#define COUNT_HIIT_KEY		1

/*10 分钟*/
#define TIME_HIIT_TOTAL_ONE		10

GPoint pos_table[60] = {
	{ 64,104 },{ 68,104 },{ 72,103 },
	{ 76,102 },{ 80,101 },{ 84,99 },
	{ 88,96 },{ 91,94 },{ 94,91 },
	{ 96,88 },{ 99,84 },{ 101,80 },
	{ 102,76 },{ 103,72 },{ 104,68 },
	{ 104,64 },{ 104,60 },{ 103,56 },
	{ 102,52 },{ 101,48 },{ 99,44 },
	{ 96,41 },{ 94,37 },{ 91,34 },
	{ 88,32 },{ 84,29 },{ 80,27 },
	{ 76,26 },{ 72,25 },{ 68,24 },
	{ 64,24 },{ 60,24 },{ 56,25 },
	{ 52,26 },{ 48,27 },{ 44,29 },
	{ 41,32 },{ 37,34 },{ 34,37 },
	{ 32,40 },{ 29,44 },{ 27,48 },
	{ 26,52 },{ 25,56 },{ 24,60 },
	{ 24,64 },{ 24,68 },{ 25,72 },
	{ 26,76 },{ 27,80 },{ 29,84 },
	{ 32,87 },{ 34,91 },{ 37,94 },
	{ 40,96 },{ 44,99 },{ 48,100 },
	{ 52,102 },{ 56,103 },{ 60,104 }
};

GPoint pos_table_half[30] = {
	{ 64,104 },{ 72,103 },{ 80,100 },
	{ 87,96 } ,{ 93,90 } ,{ 98,84 } ,
	{ 102,76 },{ 103,68 },{ 103,59 },
	{ 102,51 },{ 98,44 } ,{ 93,37 } ,
	{ 87,31 } ,{ 80,27 } ,{ 72,24 } ,
	{ 64,24 } ,{ 55,24 } ,{ 47,27 } ,
	{ 40,31 } ,{ 34,37 } ,{ 29,43 } ,
	{ 25,51 } ,{ 24,59 } ,{ 24,68 } ,
	{ 25,76 } ,{ 29,83 } ,{ 34,90 } ,
	{ 40,96 } ,{ 47,100 },{ 55,103 }
};

/*窗口ID*/
static int32_t g_hiit_window_id = -1;

/*定时器ID*/
static int8_t g_hiit_timer_id = -1;

/*轨迹圆ID*/
static int8_t g_hiit_layer_id_circle = -1;

/*时间倒计时文字ID*/
static int8_t g_hiit_layer_id_time_elapse = -1;

/*圈数文字ID*/
static int8_t g_hiit_layer_id_loop = -1;

/*累计运动时间文字ID*/
static int8_t g_hiit_layer_id_time_sport = -1;

/*强度提示文字ID*/
static int8_t g_hiit_layer_id_station = -1;

/*Button 图层ID*/
static int8_t g_hiit_layer_id_button = -1;

/*倒计时计数*/
static int8_t g_time_elapse_count = -1;

static int8_t g_hiit_start = 0;//pause 0 start 1 

static int8_t g_hiit_station = 0;//low 0 high 1

static int8_t g_hiit_station_change = 0;//change 1 no 0

static int8_t g_hiit_pause = 0;//是否从暂停状态开始

static int8_t g_hiit_last_station = -1;//保存状态



									   /*需要的总圈数*/
static int8_t g_hiit_loop_total = 0;//5 or 10
									/*目前的圈数*/
static int8_t g_hiit_loop_cur = 0;
/*运动时间分钟*/
static int8_t g_hiit_time_min = 0;
/*运动时间秒*/
static int8_t g_hiit_time_sec = 0;
/*一组high + low 的时间*/
//static int8_t g_hiit_per_time = 0;//2 or 1
/*一圈时间*/
static int8_t g_hiit_per_loop_time = 60;//30 or 60
										/*模式选择*/
static int8_t g_hiit_mode = 0;//0-60 ;1-30




static void hiit_window_update(int8_t elapse, int8_t station, int8_t loop_change);
static void init_hiit(int8_t loop_total, int8_t per_loop_time, int8_t mode);

/*背景圆图层创建*/
static P_Layer hiit_circle_track_create() {

	Geometry *geometry[1];

	LayerGeometry layer_geometry;
	memset(geometry, 0, sizeof(geometry));


	Circle c = { { HIIT_MAIN_CIRCLE_X,HIIT_MAIN_CIRCLE_Y }, HIIT_MAIN_CIRCLE_R };

	Geometry cg = { GeometryTypeCircle, FillOutline, GColorBlack, (void*)&c };
	geometry[0] = &cg;

	layer_geometry.num = 1;
	layer_geometry.p_g = geometry;

	P_Layer layer = app_layer_create_geometry(&layer_geometry);

	return layer;
}

/*移动小圆图层创建*/
static P_Layer hiit_circle_trackpoint_create(uint8_t elapse) {

	uint8_t loc_x, loc_y;

	GPoint *p_Pos = NULL;
	if (g_hiit_mode == 0)
		p_Pos = pos_table;
	else
		p_Pos = pos_table_half;


	loc_x = p_Pos[g_hiit_per_loop_time - elapse].x;
	loc_y = p_Pos[g_hiit_per_loop_time - elapse].y;

	Geometry *geometry[1];

	LayerGeometry layer_geometry;
	memset(geometry, 0, sizeof(geometry));

	Circle c = { { loc_x, loc_y }, HIIT_MOVE_CIRCLE_R };

	Geometry cg = { GeometryTypeCircle, FillArea, GColorBlack, (void*)&c };
	geometry[0] = &cg;

	layer_geometry.num = 1;
	layer_geometry.p_g = geometry;

	P_Layer layer = app_layer_create_geometry(&layer_geometry);

	return layer;
}

/*强度文本图层创建*/
static P_Layer hiit_station_text_create(uint8_t station) {

	GRect front = { { 40, 38 },{ 12, 50 } };
	char buf[7] = "";


	if (station == 0)
		sprintf(buf, "  low ");
	else if (station == 1)
		sprintf(buf, " high ");
	else if (station == 2)
		sprintf(buf, "finish");
	else if (station == 3)
		sprintf(buf, " stop ");
	else if (station == 4)
		sprintf(buf, "pause ");


	/*生成文本结构体, 依次为文本内容、文本显示框架、对齐方式、字体字号*/
	LayerText text = { buf, front, GAlignCenter, U_GBK_SIMSUNBD_12 };

	/*创建文本图层*/
	P_Layer layer = app_layer_create_text(&text);

	return layer;

}

/*倒计时文本图层图层创建*/
static P_Layer hiit_time_elpase_text_create(uint8_t elpase) {

	GRect front = { { 56, 56 },{ 15, 20 } };
	char buf[2] = "";
	sprintf(buf, "%02d", elpase);

	/*生成文本结构体, 依次为文本内容、文本显示框架、对齐方式、字体字号*/
	LayerText text = { buf, front, GAlignCenter, U_GBK_SIMSUNBD_16 };

	/*创建文本图层*/
	P_Layer layer = app_layer_create_text(&text);

	return layer;
}

/*圈数文本图层创建*/
static P_Layer hiit_loop_text_create(uint8_t cur, uint8_t total) {

	GRect front = { { 44, 80 },{ 15, 40 } };
	char buf[6] = "";
	sprintf(buf, "%02d/%02d", cur, total);
	/*生成文本结构体, 依次为文本内容、文本显示框架、对齐方式、字体字号*/
	LayerText text = { buf, front, GAlignCenter, U_GBK_SIMSUN_16 };

	/*创建文本图层*/
	P_Layer layer = app_layer_create_text(&text);

	return layer;
}

/*累计运动时间文本图层创建*/
static P_Layer hiit_time_sport_text_create() {

	GRect front = { { 18, 110 },{ 12, 90 } };
	char buf[17] = "";
	uint8_t sec = 0;
	if (g_hiit_mode == 0)
		sec = g_hiit_per_loop_time - g_time_elapse_count;
	else
		sec = g_hiit_per_loop_time - g_time_elapse_count + g_hiit_station * g_hiit_per_loop_time;

	sprintf(buf, "total time %02d:%02d", g_hiit_time_min, sec);
	/*生成文本结构体, 依次为文本内容、文本显示框架、对齐方式、字体字号*/
	LayerText text = { buf, front, GAlignCenter, U_GBK_SIMSUNBD_12 };

	/*创建文本图层*/
	P_Layer layer = app_layer_create_text(&text);

	return layer;
}


/*按钮图层创建*/
static P_Layer hiit_button_layer_create(uint8_t mode) {

	GRect frame_bg_bmp = { { HIIT_MAIN_BUTTON_ORIGIN_X, HIIT_MAIN_BUTTON_ORIGIN_Y },
	{ HIIT_MAIN_BUTTON_SIZE_H,  HIIT_MAIN_BUTTON_SIZE_W } };

	GBitmap bg_bitmap;
	if (mode == 0)
		res_get_user_bitmap(RES_BITMAP_HIIT_MAIN_BUTTON_PROCESS_BMP, &bg_bitmap);
	else
		res_get_user_bitmap(RES_BITMAP_HIIT_MAIN_BUTTON_STOP_BMP, &bg_bitmap);


	LayerBitmap lb = { bg_bitmap, frame_bg_bmp, GAlignCenter };
	P_Layer	 layer_bg_bmp = app_layer_create_bitmap(&lb);

	return layer_bg_bmp;
}

/*按钮图层更新*/
static P_Layer hiit_button_layer_update(uint8_t mode) {

	P_Layer p_old_layer = NULL, p_new_layer = NULL;

	/*根据窗口ID获取窗口句柄*/
	P_Window p_window = (P_Window)app_window_stack_get_window_by_id(g_hiit_window_id);
	if (p_window == NULL)
	{
		return;
	}
	/*获取窗口中小圆图层句柄*/
	p_old_layer = app_window_get_layer_by_id(p_window, g_hiit_layer_id_button);
	if (p_old_layer != NULL)
	{
		/*更新小圆位置*/
		p_new_layer = hiit_button_layer_create(mode);
		app_window_replace_layer(p_window, p_old_layer, p_new_layer);
	}
	/*窗口显示*/
	app_window_update(p_window);
}

static void hiit_finish(uint8_t m_auto)
{

	g_hiit_start = 0;
	//g_hiit_time_min = 0;
	g_time_elapse_count = g_hiit_per_loop_time;
	g_hiit_station = 0;
	//g_hiit_station = g_hiit_last_station;
	g_hiit_station_change = 1;
	g_hiit_loop_cur = g_hiit_loop_total - 1;// 0;
	g_hiit_pause = 0;
	if (m_auto == 0)//停止按键
	{	
		hiit_window_update(g_hiit_per_loop_time, 3, 1);
	}
	else {//计时结束
		
		hiit_window_update(g_hiit_per_loop_time, 2, 1);
		hiit_button_layer_update(1);
	}

}

/*定义后退按键事件*/
static void hiit_button_back(void *context)
{
	P_Window p_window = (P_Window)context;
	if (NULL != p_window)
	{
		app_window_stack_pop(p_window);
	}
	/*取消定时器*/
	app_service_timer_unsubscribe(g_hiit_timer_id);
	/*保存数据*/
	//app_persist_write_data_extend(COUNT_HIIT_KEY, (unsigned char *)&g_score, sizeof(Score));
}
/*定义向上按键事件*/
static void hiit_button_up(void *context)
{
	if (g_hiit_start == 0)
	{
		//station 复位low
		g_hiit_station_change = 1;
		if (g_hiit_pause == 1)
		{
			g_hiit_pause = 0;
			hiit_window_update(-1, g_hiit_last_station, 0);


		}
		else
		{
			g_hiit_loop_cur = 0;
			hiit_window_update(g_hiit_per_loop_time, 0, 1);
			hiit_button_layer_update(0);

		}

		g_hiit_start = 1;
	}
	else
	{
		g_hiit_station_change = 1;
		g_hiit_pause = 1;
		hiit_window_update(-1, 4, 0);
		g_hiit_start = 0;
	}
	maibu_service_vibes_pulse(VibesPulseTypeMiddle, 1);
}

/*定义向下按键事件*/
static void hiit_button_down(void *context)
{
	//if (g_hiit_start == 1 || (g_hiit_start == 0 && g_hiit_pause == 0))
	{
		g_hiit_time_min = 0;
		hiit_finish(0);
		hiit_button_layer_update(1);
		maibu_service_vibes_pulse(VibesPulseTypeMiddle, 1);
	}
	
}

/*定义设置按键事件*/
static void hiit_button_mode(void *context)
{
	/*只有停止的时候才能设置*/
	if (g_hiit_start == 0 && g_hiit_pause == 0)
	{
		if (g_hiit_mode == 0)
		{
			g_hiit_mode = 1;
			init_hiit(10, 30, 1);
			hiit_window_update(g_hiit_per_loop_time, -1, 1);

		}
		else
		{
			g_hiit_mode = 0;
			init_hiit(5, 60, 0);
			hiit_window_update(g_hiit_per_loop_time, -1, 1);

		}
	}
}

static void hiit_window_update(int8_t elapse, int8_t station, int8_t loop_change) {


	P_Layer p_old_circle = NULL, p_new_circle = NULL;
	P_Layer p_old_time_elapse = NULL, p_new_time_elapse = NULL;
	P_Layer p_old_station = NULL, p_new_station = NULL;
	P_Layer p_old_loop = NULL, p_new_loop = NULL;
	P_Layer p_old_time_sport = NULL, p_new_time_sport = NULL;

	/*根据窗口ID获取窗口句柄*/
	P_Window p_window = (P_Window)app_window_stack_get_window_by_id(g_hiit_window_id);
	if (p_window == NULL)
	{
		return;
	}

	if (elapse >= 0)//暂停不更新
	{
		/*获取窗口中小圆图层句柄*/
		p_old_circle = app_window_get_layer_by_id(p_window, g_hiit_layer_id_circle);
		if (p_old_circle != NULL)
		{
			/*更新小圆位置*/
			p_new_circle = hiit_circle_trackpoint_create(elapse);
			app_window_replace_layer(p_window, p_old_circle, p_new_circle);
		}
		/*更新倒计时*/
		p_old_time_elapse = app_window_get_layer_by_id(p_window, g_hiit_layer_id_time_elapse);
		if (p_old_time_elapse != NULL)
		{
			p_new_time_elapse = hiit_time_elpase_text_create(elapse);
			app_window_replace_layer(p_window, p_old_time_elapse, p_new_time_elapse);
		}
		/*更新计时*/
		p_old_time_sport = app_window_get_layer_by_id(p_window, g_hiit_layer_id_time_sport);
		if (p_old_time_elapse != NULL)
		{
			p_new_time_sport = hiit_time_sport_text_create();
			app_window_replace_layer(p_window, p_old_time_sport, p_new_time_sport);
		}
	}

	/*station 图层*/
	if (g_hiit_station_change == 1 && station >= 0)
	{
		g_hiit_station_change = 0;

		p_old_station = app_window_get_layer_by_id(p_window, g_hiit_layer_id_station);
		if (p_old_station != NULL)
		{
			/*更新station*/
			if (station == 0 || station == 1)//保存low or high 状态
				g_hiit_last_station = station;

			p_new_station = hiit_station_text_create(station);
			app_window_replace_layer(p_window, p_old_station, p_new_station);
		}
	}
	/*loop 圈数变化*/
	if (loop_change == 1)
	{
		p_old_loop = app_window_get_layer_by_id(p_window, g_hiit_layer_id_loop);
		if (p_old_loop != NULL)
		{
			/*更新station*/
			g_hiit_loop_cur++;
			p_new_loop = hiit_loop_text_create(g_hiit_loop_cur, g_hiit_loop_total);
			app_window_replace_layer(p_window, p_old_loop, p_new_loop);
		}

	}

	/*窗口显示*/
	app_window_update(p_window);
}


static void hiit_timer_callback(date_time_t tick_time, uint32_t millis, void *context) {


	if (g_hiit_start == 1)
	{
		int8_t l_loop_change = 0;

		g_time_elapse_count--;
		if (g_time_elapse_count <= 0)
		{

			if (g_hiit_mode == 0 || (g_hiit_mode == 1 && g_hiit_station == 1))
				g_hiit_time_min++;


			if (g_hiit_time_min != 0 && g_hiit_time_min % TIME_HIIT_TOTAL_ONE == 0 && g_time_elapse_count == 0 && g_hiit_station == 1)
			{

				if (g_hiit_time_min >= 100)
					g_hiit_time_min = 0;
				/*change*/
				g_hiit_station_change = 1;

				hiit_finish(1);
				maibu_service_vibes_pulse(VibesPulseTypeMiddle, 3);

				return;
			}

			if (g_hiit_station == 1)
			{
				g_hiit_station = 0;
				l_loop_change = 1;
			}
			else
				g_hiit_station = 1;

			g_time_elapse_count = g_hiit_per_loop_time;

			/*change*/
			g_hiit_station_change = 1;

			/*震动*/
			if (l_loop_change == 1)
				maibu_service_vibes_pulse(VibesPulseTypeMiddle, 2);
			else
				maibu_service_vibes_pulse(VibesPulseTypeMiddle, 1);
		}

		hiit_window_update(g_time_elapse_count, g_hiit_station, l_loop_change);
	}

}


static P_Window hiit_window_create() {

	P_Window p_window = app_window_create();
	if (NULL == p_window)
	{
		return NULL;
	}
	/*大圆*/
	P_Layer layer_big = hiit_circle_track_create();
	app_window_add_layer(p_window, layer_big);

	/*小圆*/
	P_Layer layer_small = hiit_circle_trackpoint_create(g_hiit_per_loop_time);
	g_hiit_layer_id_circle = app_window_add_layer(p_window, layer_small);

	/*计时字体*/
	P_Layer layer_elapse = hiit_time_elpase_text_create(g_hiit_per_loop_time);//app_layer_create_text(&text);

																			  /*添加文本图层到窗口中*/
	g_hiit_layer_id_time_elapse = app_window_add_layer(p_window, layer_elapse);

	//station
	P_Layer layer_station = hiit_station_text_create(0);//app_layer_create_text(&text2);

	g_hiit_layer_id_station = app_window_add_layer(p_window, layer_station);

	//loop
	P_Layer layer_loop = hiit_loop_text_create(g_hiit_loop_cur, g_hiit_loop_total);

	g_hiit_layer_id_loop = app_window_add_layer(p_window, layer_loop);

	//运动累计时间
	P_Layer layer_time = hiit_time_sport_text_create();

	g_hiit_layer_id_time_sport = app_window_add_layer(p_window, layer_time);

	/*后退按键回调*/
	app_window_click_subscribe(p_window, ButtonIdBack, hiit_button_back);
	app_window_click_subscribe(p_window, ButtonIdUp, hiit_button_up);
	app_window_click_subscribe(p_window, ButtonIdDown, hiit_button_down);
	app_window_click_subscribe(p_window, ButtonIdSelect, hiit_button_mode);

	return p_window;
}

/********************************
@func:			初始化
@loop_total:	需要的总圈数,5 or 10
@per_loop_time:一圈的时间,30 or 60
@per_time:		一组高强度+低强度时间，2 or 1
@mode:			模式

********************************/
static void init_hiit(int8_t loop_total, int8_t per_loop_time, int8_t mode) {

	g_hiit_start = 0;

	g_hiit_loop_total = loop_total;

	g_hiit_per_loop_time = per_loop_time;
	g_time_elapse_count = g_hiit_per_loop_time;

	g_hiit_loop_cur = 0;

	g_hiit_station = 0;//low 0 high 1

	g_hiit_station_change = 0;//change 1 no 0

	g_hiit_pause = 0;//是否从暂停状态开始

	g_hiit_last_station = -1;//保存状态

	//g_hiit_time_min = 0;

	g_hiit_time_sec = 0;

	//g_hiit_per_time = per_time;//2 or 1

	g_hiit_mode = mode;//0-60 ;1-30
}

static P_Window create_init_main_window() {

	P_Window p_window = NULL;


	init_hiit(5, 60, 0);

	p_window = hiit_window_create();

	P_Layer	 layer_bg_bmp = hiit_button_layer_create(1);
	g_hiit_layer_id_button = app_window_add_layer(p_window, layer_bg_bmp);

	g_hiit_timer_id = app_service_timer_subscribe(1000, hiit_timer_callback, (void *)p_window);

	/*添加状态栏, 显示时间*/
	app_plug_status_bar_create(p_window, NULL, NULL, NULL);
	app_plug_status_bar_add_time(p_window);
	app_plug_status_bar_add_battery(p_window);

	return p_window;
}

int main()
{
	//simulator_init();

	P_Window p_window = create_init_main_window();

	g_hiit_window_id = app_window_stack_push(p_window);

	//simulator_wait();

	return 0;
}

