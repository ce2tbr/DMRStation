
#include "main.h"

#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/context.h>
#include <pulse/glib-mainloop.h>
#include <pulse/stream.h>
#include <pulse/introspect.h>

//#define ADJUST_LATENCY	1

#define PCM_AUDIO_FRAMES	3 //Frames to have to send to ambe
#define RECORD_BUFFER_SIZE	8000*2*5 // Buffer up to 5 seconds of audio
#define RECORD_BUFFER_ATTR_FRAG_SIZE	320*4

#define AUDIO_IN_PREBUFFER	3*320*8

#define AUDIO_TICK_INTERVAL	60	//ms

/* Private typedef -----------------------------------------------------------*/
typedef enum {
  AUDIO_STATE_IDLE = 0,
  AUDIO_STATE_INIT,
  AUDIO_STATE_PLAYING,
}AUDIO_PLAYBACK_StateTypeDef;

extern dmr_control_struct_t dmr_control;

//static const int16_t sine_beep16[] =  {0,101,201,302,402,503,603,704,804,905,1005,1106,1206,1307,1407,1507,1608,1708,1809,1909,2009,2110,2210,2310,2410,2511,2611,2711,2811,2911,3012,3112,3212,3312,3412,3512,3612,3712,3811,3911,4011,4111,4210,4310,4410,4509,4609,4708,4808,4907,5007,5106,5205,5305,5404,5503,5602,5701,5800,5899,5998,6096,6195,6294,6393,6491,6590,6688,6786,6885,6983,7081,7179,7277,7375,7473,7571,7669,7767,7864,7962,8059,8157,8254,8351,8448,8545,8642,8739,8836,8933,9030,9126,9223,9319,9416,9512,9608,9704,9800,9896,9992,10087,10183,10278,10374,10469,10564,10659,10754,10849,10944,11039,11133,11228,11322,11417,11511,11605,11699,11793,11886,11980,12074,12167,12260,12353,12446,12539,12632,12725,12817,12910,13002,13094,13187,13279,13370,13462,13554,13645,13736,13828,13919,14010,14101,14191,14282,14372,14462,14553,14643,14732,14822,14912,15001,15090,15180,15269,15358,15446,15535,15623,15712,15800,15888,15976,16063,16151,16238,16325,16413,16499,16586,16673,16759,16846,16932,17018,17104,17189,17275,17360,17445,17530,17615,17700,17784,17869,17953,18037,18121,18204,18288,18371,18454,18537,18620,18703,18785,18868,18950,19032,19113,19195,19276,19357,19438,19519,19600,19680,19761,19841,19921,20000,20080,20159,20238,20317,20396,20475,20553,20631,20709,20787,20865,20942,21019,21096,21173,21250,21326,21403,21479,21554,21630,21705,21781,21856,21930,22005,22079,22154,22227,22301,22375,22448,22521,22594,22667,22739,22812,22884,22956,23027,23099,23170,23241,23311,23382,23452,23522,23592,23662,23731,23801,23870,23938,24007,24075,24143,24211,24279,24346,24413,24480,24547,24613,24680,24746,24811,24877,24942,25007,25072,25137,25201,25265,25329,25393,25456,25519,25582,25645,25708,25770,25832,25893,25955,26016,26077,26138,26198,26259,26319,26378,26438,26497,26556,26615,26674,26732,26790,26848,26905,26962,27019,27076,27133,27189,27245,27300,27356,27411,27466,27521,27575,27629,27683,27737,27790,27843,27896,27949,28001,28053,28105,28157,28208,28259,28310,28360,28411,28460,28510,28560,28609,28658,28706,28755,28803,28850,28898,28945,28992,29039,29085,29131,29177,29223,29268,29313,29358,29403,29447,29491,29534,29578,29621,29664,29706,29749,29791,29832,29874,29915,29956,29997,30037,30077,30117,30156,30195,30234,30273,30311,30349,30387,30424,30462,30498,30535,30571,30607,30643,30679,30714,30749,30783,30818,30852,30885,30919,30952,30985,31017,31050,31082,31113,31145,31176,31206,31237,31267,31297,31327,31356,31385,31414,31442,31470,31498,31526,31553,31580,31607,31633,31659,31685,31710,31736,31760,31785,31809,31833,31857,31880,31903,31926,31949,31971,31993,32014,32036,32057,32077,32098,32118,32137,32157,32176,32195,32213,32232,32250,32267,32285,32302,32318,32335,32351,32367,32382,32397,32412,32427,32441,32455,32469,32482,32495,32508,32521,32533,32545,32556,32567,32578,32589,32599,32609,32619,32628,32637,32646,32655,32663,32671,32678,32685,32692,32699,32705,32711,32717,32722,32728,32732,32737,32741,32745,32748,32752,32755,32757,32759,32761,32763,32765,32766,32766,32767,32767,32767,32766,32766,32765,32763,32761,32759,32757,32755,32752,32748,32745,32741,32737,32732,32728,32722,32717,32711,32705,32699,32692,32685,32678,32671,32663,32655,32646,32637,32628,32619,32609,32599,32589,32578,32567,32556,32545,32533,32521,32508,32495,32482,32469,32455,32441,32427,32412,32397,32382,32367,32351,32335,32318,32302,32285,32267,32250,32232,32213,32195,32176,32157,32137,32118,32098,32077,32057,32036,32014,31993,31971,31949,31926,31903,31880,31857,31833,31809,31785,31760,31736,31710,31685,31659,31633,31607,31580,31553,31526,31498,31470,31442,31414,31385,31356,31327,31297,31267,31237,31206,31176,31145,31113,31082,31050,31017,30985,30952,30919,30885,30852,30818,30783,30749,30714,30679,30643,30607,30571,30535,30498,30462,30424,30387,30349,30311,30273,30234,30195,30156,30117,30077,30037,29997,29956,29915,29874,29832,29791,29749,29706,29664,29621,29578,29534,29491,29447,29403,29358,29313,29268,29223,29177,29131,29085,29039,28992,28945,28898,28850,28803,28755,28706,28658,28609,28560,28510,28460,28411,28360,28310,28259,28208,28157,28105,28053,28001,27949,27896,27843,27790,27737,27683,27629,27575,27521,27466,27411,27356,27300,27245,27189,27133,27076,27019,26962,26905,26848,26790,26732,26674,26615,26556,26497,26438,26378,26319,26259,26198,26138,26077,26016,25955,25893,25832,25770,25708,25645,25582,25519,25456,25393,25329,25265,25201,25137,25072,25007,24942,24877,24811,24746,24680,24613,24547,24480,24413,24346,24279,24211,24143,24075,24007,23938,23870,23801,23731,23662,23592,23522,23452,23382,23311,23241,23170,23099,23027,22956,22884,22812,22739,22667,22594,22521,22448,22375,22301,22227,22154,22079,22005,21930,21856,21781,21705,21630,21554,21479,21403,21326,21250,21173,21096,21019,20942,20865,20787,20709,20631,20553,20475,20396,20317,20238,20159,20080,20000,19921,19841,19761,19680,19600,19519,19438,19357,19276,19195,19113,19032,18950,18868,18785,18703,18620,18537,18454,18371,18288,18204,18121,18037,17953,17869,17784,17700,17615,17530,17445,17360,17275,17189,17104,17018,16932,16846,16759,16673,16586,16499,16413,16325,16238,16151,16063,15976,15888,15800,15712,15623,15535,15446,15358,15269,15180,15090,15001,14912,14822,14732,14643,14553,14462,14372,14282,14191,14101,14010,13919,13828,13736,13645,13554,13462,13370,13279,13187,13094,13002,12910,12817,12725,12632,12539,12446,12353,12260,12167,12074,11980,11886,11793,11699,11605,11511,11417,11322,11228,11133,11039,10944,10849,10754,10659,10564,10469,10374,10278,10183,10087,9992,9896,9800,9704,9608,9512,9416,9319,9223,9126,9030,8933,8836,8739,8642,8545,8448,8351,8254,8157,8059,7962,7864,7767,7669,7571,7473,7375,7277,7179,7081,6983,6885,6786,6688,6590,6491,6393,6294,6195,6096,5998,5899,5800,5701,5602,5503,5404,5305,5205,5106,5007,4907,4808,4708,4609,4509,4410,4310,4210,4111,4011,3911,3811,3712,3612,3512,3412,3312,3212,3112,3012,2911,2811,2711,2611,2511,2410,2310,2210,2110,2009,1909,1809,1708,1608,1507,1407,1307,1206,1106,1005,905,804,704,603,503,402,302,201,101,0,-101,-201,-302,-402,-503,-603,-704,-804,-905,-1005,-1106,-1206,-1307,-1407,-1507,-1608,-1708,-1809,-1909,-2009,-2110,-2210,-2310,-2410,-2511,-2611,-2711,-2811,-2911,-3012,-3112,-3212,-3312,-3412,-3512,-3612,-3712,-3811,-3911,-4011,-4111,-4210,-4310,-4410,-4509,-4609,-4708,-4808,-4907,-5007,-5106,-5205,-5305,-5404,-5503,-5602,-5701,-5800,-5899,-5998,-6096,-6195,-6294,-6393,-6491,-6590,-6688,-6786,-6885,-6983,-7081,-7179,-7277,-7375,-7473,-7571,-7669,-7767,-7864,-7962,-8059,-8157,-8254,-8351,-8448,-8545,-8642,-8739,-8836,-8933,-9030,-9126,-9223,-9319,-9416,-9512,-9608,-9704,-9800,-9896,-9992,-10087,-10183,-10278,-10374,-10469,-10564,-10659,-10754,-10849,-10944,-11039,-11133,-11228,-11322,-11417,-11511,-11605,-11699,-11793,-11886,-11980,-12074,-12167,-12260,-12353,-12446,-12539,-12632,-12725,-12817,-12910,-13002,-13094,-13187,-13279,-13370,-13462,-13554,-13645,-13736,-13828,-13919,-14010,-14101,-14191,-14282,-14372,-14462,-14553,-14643,-14732,-14822,-14912,-15001,-15090,-15180,-15269,-15358,-15446,-15535,-15623,-15712,-15800,-15888,-15976,-16063,-16151,-16238,-16325,-16413,-16499,-16586,-16673,-16759,-16846,-16932,-17018,-17104,-17189,-17275,-17360,-17445,-17530,-17615,-17700,-17784,-17869,-17953,-18037,-18121,-18204,-18288,-18371,-18454,-18537,-18620,-18703,-18785,-18868,-18950,-19032,-19113,-19195,-19276,-19357,-19438,-19519,-19600,-19680,-19761,-19841,-19921,-20000,-20080,-20159,-20238,-20317,-20396,-20475,-20553,-20631,-20709,-20787,-20865,-20942,-21019,-21096,-21173,-21250,-21326,-21403,-21479,-21554,-21630,-21705,-21781,-21856,-21930,-22005,-22079,-22154,-22227,-22301,-22375,-22448,-22521,-22594,-22667,-22739,-22812,-22884,-22956,-23027,-23099,-23170,-23241,-23311,-23382,-23452,-23522,-23592,-23662,-23731,-23801,-23870,-23938,-24007,-24075,-24143,-24211,-24279,-24346,-24413,-24480,-24547,-24613,-24680,-24746,-24811,-24877,-24942,-25007,-25072,-25137,-25201,-25265,-25329,-25393,-25456,-25519,-25582,-25645,-25708,-25770,-25832,-25893,-25955,-26016,-26077,-26138,-26198,-26259,-26319,-26378,-26438,-26497,-26556,-26615,-26674,-26732,-26790,-26848,-26905,-26962,-27019,-27076,-27133,-27189,-27245,-27300,-27356,-27411,-27466,-27521,-27575,-27629,-27683,-27737,-27790,-27843,-27896,-27949,-28001,-28053,-28105,-28157,-28208,-28259,-28310,-28360,-28411,-28460,-28510,-28560,-28609,-28658,-28706,-28755,-28803,-28850,-28898,-28945,-28992,-29039,-29085,-29131,-29177,-29223,-29268,-29313,-29358,-29403,-29447,-29491,-29534,-29578,-29621,-29664,-29706,-29749,-29791,-29832,-29874,-29915,-29956,-29997,-30037,-30077,-30117,-30156,-30195,-30234,-30273,-30311,-30349,-30387,-30424,-30462,-30498,-30535,-30571,-30607,-30643,-30679,-30714,-30749,-30783,-30818,-30852,-30885,-30919,-30952,-30985,-31017,-31050,-31082,-31113,-31145,-31176,-31206,-31237,-31267,-31297,-31327,-31356,-31385,-31414,-31442,-31470,-31498,-31526,-31553,-31580,-31607,-31633,-31659,-31685,-31710,-31736,-31760,-31785,-31809,-31833,-31857,-31880,-31903,-31926,-31949,-31971,-31993,-32014,-32036,-32057,-32077,-32098,-32118,-32137,-32157,-32176,-32195,-32213,-32232,-32250,-32267,-32285,-32302,-32318,-32335,-32351,-32367,-32382,-32397,-32412,-32427,-32441,-32455,-32469,-32482,-32495,-32508,-32521,-32533,-32545,-32556,-32567,-32578,-32589,-32599,-32609,-32619,-32628,-32637,-32646,-32655,-32663,-32671,-32678,-32685,-32692,-32699,-32705,-32711,-32717,-32722,-32728,-32732,-32737,-32741,-32745,-32748,-32752,-32755,-32757,-32759,-32761,-32763,-32765,-32766,-32766,-32767,-32767,-32767,-32766,-32766,-32765,-32763,-32761,-32759,-32757,-32755,-32752,-32748,-32745,-32741,-32737,-32732,-32728,-32722,-32717,-32711,-32705,-32699,-32692,-32685,-32678,-32671,-32663,-32655,-32646,-32637,-32628,-32619,-32609,-32599,-32589,-32578,-32567,-32556,-32545,-32533,-32521,-32508,-32495,-32482,-32469,-32455,-32441,-32427,-32412,-32397,-32382,-32367,-32351,-32335,-32318,-32302,-32285,-32267,-32250,-32232,-32213,-32195,-32176,-32157,-32137,-32118,-32098,-32077,-32057,-32036,-32014,-31993,-31971,-31949,-31926,-31903,-31880,-31857,-31833,-31809,-31785,-31760,-31736,-31710,-31685,-31659,-31633,-31607,-31580,-31553,-31526,-31498,-31470,-31442,-31414,-31385,-31356,-31327,-31297,-31267,-31237,-31206,-31176,-31145,-31113,-31082,-31050,-31017,-30985,-30952,-30919,-30885,-30852,-30818,-30783,-30749,-30714,-30679,-30643,-30607,-30571,-30535,-30498,-30462,-30424,-30387,-30349,-30311,-30273,-30234,-30195,-30156,-30117,-30077,-30037,-29997,-29956,-29915,-29874,-29832,-29791,-29749,-29706,-29664,-29621,-29578,-29534,-29491,-29447,-29403,-29358,-29313,-29268,-29223,-29177,-29131,-29085,-29039,-28992,-28945,-28898,-28850,-28803,-28755,-28706,-28658,-28609,-28560,-28510,-28460,-28411,-28360,-28310,-28259,-28208,-28157,-28105,-28053,-28001,-27949,-27896,-27843,-27790,-27737,-27683,-27629,-27575,-27521,-27466,-27411,-27356,-27300,-27245,-27189,-27133,-27076,-27019,-26962,-26905,-26848,-26790,-26732,-26674,-26615,-26556,-26497,-26438,-26378,-26319,-26259,-26198,-26138,-26077,-26016,-25955,-25893,-25832,-25770,-25708,-25645,-25582,-25519,-25456,-25393,-25329,-25265,-25201,-25137,-25072,-25007,-24942,-24877,-24811,-24746,-24680,-24613,-24547,-24480,-24413,-24346,-24279,-24211,-24143,-24075,-24007,-23938,-23870,-23801,-23731,-23662,-23592,-23522,-23452,-23382,-23311,-23241,-23170,-23099,-23027,-22956,-22884,-22812,-22739,-22667,-22594,-22521,-22448,-22375,-22301,-22227,-22154,-22079,-22005,-21930,-21856,-21781,-21705,-21630,-21554,-21479,-21403,-21326,-21250,-21173,-21096,-21019,-20942,-20865,-20787,-20709,-20631,-20553,-20475,-20396,-20317,-20238,-20159,-20080,-20000,-19921,-19841,-19761,-19680,-19600,-19519,-19438,-19357,-19276,-19195,-19113,-19032,-18950,-18868,-18785,-18703,-18620,-18537,-18454,-18371,-18288,-18204,-18121,-18037,-17953,-17869,-17784,-17700,-17615,-17530,-17445,-17360,-17275,-17189,-17104,-17018,-16932,-16846,-16759,-16673,-16586,-16499,-16413,-16325,-16238,-16151,-16063,-15976,-15888,-15800,-15712,-15623,-15535,-15446,-15358,-15269,-15180,-15090,-15001,-14912,-14822,-14732,-14643,-14553,-14462,-14372,-14282,-14191,-14101,-14010,-13919,-13828,-13736,-13645,-13554,-13462,-13370,-13279,-13187,-13094,-13002,-12910,-12817,-12725,-12632,-12539,-12446,-12353,-12260,-12167,-12074,-11980,-11886,-11793,-11699,-11605,-11511,-11417,-11322,-11228,-11133,-11039,-10944,-10849,-10754,-10659,-10564,-10469,-10374,-10278,-10183,-10087,-9992,-9896,-9800,-9704,-9608,-9512,-9416,-9319,-9223,-9126,-9030,-8933,-8836,-8739,-8642,-8545,-8448,-8351,-8254,-8157,-8059,-7962,-7864,-7767,-7669,-7571,-7473,-7375,-7277,-7179,-7081,-6983,-6885,-6786,-6688,-6590,-6491,-6393,-6294,-6195,-6096,-5998,-5899,-5800,-5701,-5602,-5503,-5404,-5305,-5205,-5106,-5007,-4907,-4808,-4708,-4609,-4509,-4410,-4310,-4210,-4111,-4011,-3911,-3811,-3712,-3612,-3512,-3412,-3312,-3212,-3112,-3012,-2911,-2811,-2711,-2611,-2511,-2410,-2310,-2210,-2110,-2009,-1909,-1809,-1708,-1608,-1507,-1407,-1307,-1206,-1106,-1005,-905,-804,-704,-603,-503,-402,-302,-201,-101,};

guint audioTimeout;

uint8_t audioBuffer[RECORD_BUFFER_SIZE] ;
volatile uint32_t audioBufferReadIndex;
volatile uint32_t audioBufferWriteIndex;
volatile uint32_t audioBufferCount;


void audioBufferFlush(void)
{
	audioBufferCount = 0;
	audioBufferReadIndex = 0;
	audioBufferWriteIndex = 0;
}
void audioBufferWrite(const uint8_t *data, uint32_t size)
{
	uint32_t free = RECORD_BUFFER_SIZE - audioBufferCount;

	if (free >= size) {
		if (audioBufferWriteIndex > audioBufferReadIndex)
		{
			int f1 = RECORD_BUFFER_SIZE - audioBufferWriteIndex;
			if (f1 > size)
			{
				memcpy(&audioBuffer[audioBufferWriteIndex], data, size);
			}
			else {
				memcpy(&audioBuffer[audioBufferWriteIndex], data, f1);
				memcpy(&audioBuffer[0], data, size - f1);
			}
		}
		else
		{
			memcpy(&audioBuffer[audioBufferWriteIndex], data, size);
		}
		audioBufferCount += size;
		audioBufferWriteIndex += size;
		audioBufferWriteIndex %= RECORD_BUFFER_SIZE;
		//g_printf("Adding %d total %d\n", size, audioBufferCount);
	}
	else {
		//g_printf("Audio buffer overrun\n");
	}

	//audio_tick();
}

uint32_t audioReadBuffer(uint8_t *data, uint32_t size)
{
	if (audioBufferCount > size)
	{
		if (audioBufferReadIndex + size <= RECORD_BUFFER_SIZE)
		{
			memcpy(data, &audioBuffer[audioBufferReadIndex], size);
		}
		else
		{
			int f1 = RECORD_BUFFER_SIZE - audioBufferReadIndex;
			memcpy(data, &audioBuffer[audioBufferReadIndex], f1);
			memcpy(data, &audioBuffer[0], size - f1);
		}

		audioBufferReadIndex += size;
		audioBufferReadIndex %= RECORD_BUFFER_SIZE;
		audioBufferCount -= size;
		//g_printf("Reading %d total %d\n", size, audioBufferCount);
	}

	//TODO: read available instead of 0
	return 0;
}

gboolean audio_tick(void)
{
	uint8_t data[320];

	if (audioBufferCount >= 320 * PCM_AUDIO_FRAMES)
	{
		for (int i = 0; i<PCM_AUDIO_FRAMES; i++)
		{
			audioReadBuffer(data, 320);
			ambe_send(data, 320);
		}
	}

	tick_DMRQueue();

	return true;
}

volatile static AUDIO_PLAYBACK_StateTypeDef  audio_state;

int beep_idx;

uint32_t audioStart;
uint16_t sine_beep_freq = 440;

/* The Sample format to use */
static const pa_sample_spec ss = {
	.format = PA_SAMPLE_S16LE,
	.rate = 8000,
	.channels = 1
};

pa_cvolume mainVolume = {
		.channels = 1,
		.values = {PA_VOLUME_NORM}
};

pa_context *ctxt;
pa_glib_mainloop *pa_ml;

pa_buffer_attr buffer_attr;
pa_buffer_attr buffer_record_attr;

pa_stream * audio_stream = NULL;
pa_stream * audio_record_stream = NULL;


void test_audio(void)
{
	//uint16_t sample;

	//uint16_t *buffer = audio_get_buffer();

	/*

	int step = (int)(sine_beep_freq/3.915f);

	 for (uint16_t j = 0; j < AUDIO_BUFFER_SIZE/2; j++)
	{
		beep_idx = beep_idx + step;

		if (beep_idx >= 0x0800)
		{
			beep_idx=beep_idx-0x0800;
		}
		sample = abs(sine_beep16[beep_idx]);

		buffer[2*j+1] = sample;
		buffer[2*j] = sample;


	}

	 sine_beep_freq += 4;
	if (sine_beep_freq> 2000)
	{
		sine_beep_freq = 440;
	}

	audio_advance_buffer();

	*/

	/*
	for ( int i = 0 ; i<AUDIO_BUFFER_SIZE/2; i++) {
		//sample = pcm1_raw[audioStart+1] <<8 | pcm1_raw[audioStart];
		//sample = abs(sample) >>3;

		buffer[i*2+1] = sample;
		buffer[i*2] = sample;

		audioStart+=2;
		if (audioStart > pcm1_raw_len){
			audioStart = 0;
		}
	}

	audio_advance_buffer();
	*/
}


void audio_start(void)
{
	audio_state = AUDIO_STATE_PLAYING;

}

void audio_stop(void)
{
	audio_state = AUDIO_STATE_IDLE;

}




void audio_pause(void)
{
	audio_state = AUDIO_STATE_IDLE;
}






void audio_write_buffer(const uint8_t *data, uint16_t size)
{
	pa_stream_write	(	audio_stream,
			data,
			(size_t) size,
			NULL,
			0,
			PA_SEEK_RELATIVE
			);
}


void pa_sink_info_cb_func
(pa_context *c, const pa_sink_info *i, int eol, void *userdata)
{
	if (i)
	{
		g_print("Sink %d: %s\n%s\n",i->index,  i->name, i->description);
	}
}

void pa_source_info_cb_func
(pa_context *c, const pa_source_info *i, int eol, void *userdata)
{
	if (i)
	{
		g_print("Source %d: %s\n%s\n",i->index,  i->name, i->description);
	}
}

void audio_init(void)
{
	int r;
	int pa_ready = 0;

	pa_ml = pa_glib_mainloop_new(NULL);
	g_assert(pa_ml);

	pa_mainloop_api *pa_mlapi = pa_glib_mainloop_get_api(pa_ml);

	ctxt = pa_context_new(pa_mlapi, "OpenDMR");
	g_assert(ctxt);

	pa_context_set_state_callback(ctxt, context_state_callback, &pa_ready);

	r = pa_context_connect(ctxt, NULL, PA_CONTEXT_NOAUTOSPAWN|PA_CONTEXT_NOFAIL, NULL);
	g_assert(r == 0);


	audioTimeout = g_timeout_add(AUDIO_TICK_INTERVAL, (GSourceFunc) audio_tick, NULL);
}

void audio_deinit(void)
{
	pa_stream_disconnect (audio_stream);
	pa_stream_disconnect (audio_record_stream);

	pa_context_unref(ctxt);
	pa_glib_mainloop_free(pa_ml);
}



#ifdef ADJUST_LATENCY

static int latency = 20000; // start latency in micro seconds
static int underflows = 0;

static void stream_underflow_cb(pa_stream *s, void *userdata) {
  // We increase the latency by 50% if we get 6 underflows and latency is under 2s
  // This is very useful for over the network playback that can't handle low latencies
  g_printf("underflow\n");
  underflows++;
  if (underflows >= 6 && latency < 2000000) {
    latency = (latency*3)/2;
    buffer_attr.maxlength = pa_usec_to_bytes(latency,&ss);
    buffer_attr.tlength = pa_usec_to_bytes(latency,&ss);
    pa_stream_set_buffer_attr(s, &buffer_attr, NULL, NULL);
    underflows = 0;
    g_printf("latency increased to %d\n", latency);
  }
}
#endif

void pa_stream_notify_cb_func(pa_stream *s, void *userdata)
{

	//asert(s);
	pa_stream_state_t state = pa_stream_get_state(s);

	switch (state) {
		case PA_STREAM_CREATING:
			break;
		case PA_STREAM_READY:
			g_printf("Record stream ready\n");
			break;

		default:
			break;
	}

	g_printf("State %d\n", state);
}

void stream_cork_cb(pa_stream *s, int success, void *userdata)
{
	//assert(success);

	/*if (success)
	{
		g_print("cork\n");
	}
	else
	{
		g_print("cork error\n");
	}*/

	if(pa_stream_is_corked(s)) {

		if (dmr_control.dmr_status == DMR_STATUS_TX)
		{
			g_printf("TX Ending\n");
			pa_stream_flush(s, NULL, NULL);
			pa_stream_drop(s);
			dmr_stop_tx();
		}
	}

}

void pa_record_stream_request_function(pa_stream *p, size_t nbytes, void *userdata)
{
	const void *data;
	size_t len;


	//pa_threaded_mainloop_lock(pa_ml);
	if (nbytes >0 ){

		if (pa_stream_peek(audio_record_stream, &data, &len) < 0)
		{
			g_print("Get PCM data error");
		}
		else
		{
			audioBufferWrite( data, (uint32_t) len);
			pa_stream_drop(audio_record_stream);

			/*
			g_printf("len %d\n", len);
			for (int i = 0; i < PCM_AUDIO_FRAMES; i++)
			{
				ambe_send(data+320*i, 320);
			}

			//audio_write_buffer(data, len);
			*/

		}
	}

	//pa_threaded_mainloop_unlock(pa_ml);
}



void context_state_callback(pa_context *c, void *userdata) {
	  pa_context_state_t state;
	  int *pa_ready = userdata;
	  state = pa_context_get_state(c);
	  switch  (state) {
			// These are just here for reference
		  case PA_CONTEXT_UNCONNECTED:
		  case PA_CONTEXT_CONNECTING:
		  case PA_CONTEXT_AUTHORIZING:
		  case PA_CONTEXT_SETTING_NAME:
		  default:
			break;
		  case PA_CONTEXT_FAILED:
		  case PA_CONTEXT_TERMINATED:
			*pa_ready = 2;
			break;
		  case PA_CONTEXT_READY:
			*pa_ready = 1;

			pa_context_get_sink_info_list(ctxt,
						pa_sink_info_cb_func,
						NULL
						);

			pa_context_get_source_info_list(ctxt,
									pa_source_info_cb_func,
									NULL
									);

			audio_stream = pa_stream_new (ctxt, "DMROut", &ss, NULL);

#ifdef ADJUST_LATENCY
			pa_stream_set_underflow_callback(audio_stream, stream_underflow_cb, NULL);
#endif
			buffer_attr.fragsize = -1;
			buffer_attr.maxlength = -1;
			buffer_attr.minreq = 3*320 * 2; //; //320
			buffer_attr.tlength = -1;
			buffer_attr.prebuf = AUDIO_IN_PREBUFFER; //-1

			pa_stream_connect_playback	(audio_stream,
				NULL, //"alsa_output.usb-Logitech_Logitech_USB_Headset-00.analog-stereo",
				&buffer_attr,
				0,
				NULL, //voulme
				NULL
				);

			buffer_record_attr.fragsize = RECORD_BUFFER_ATTR_FRAG_SIZE; //320 * PCM_AUDIO_FRAMES; //320 * PCM_AUDIO_FRAMES;
			buffer_record_attr.maxlength = -1;
			buffer_record_attr.minreq = -1;
			buffer_record_attr.tlength = -1;
			buffer_record_attr.prebuf = -1;

			audio_record_stream = pa_stream_new (ctxt, "DMRIn", &ss, NULL);

			pa_stream_set_read_callback	(audio_record_stream,
										pa_record_stream_request_function,
										NULL
										);

			pa_stream_set_state_callback(audio_record_stream,
									pa_stream_notify_cb_func ,
									NULL
									);

			pa_stream_connect_record  (audio_record_stream,
							NULL, //"alsa_input.usb-Logitech_Logitech_USB_Headset-00.analog-mono",
							&buffer_record_attr,
							PA_STREAM_START_CORKED
							);



			break;
	  }
}

/**
 * Change output stream volume
 */
void setVolume(uint32_t volume)
{
	pa_cvolume_set	(&mainVolume,
			1,
			(pa_volume_t) 	volume
		);

	/*
	pa_context_set_sink_volume_by_index (ctxt,
		0,
		&mainVolume,
		NULL,
		NULL
		);*/

	//TODO: Use preferred audio device

	if (audio_stream)
	{
	pa_context_set_sink_volume_by_index (ctxt,
			pa_stream_get_device_index(audio_stream),
			&mainVolume,
			NULL,
			NULL
			);
	}
	/*
	pa_context_set_sink_volume (ctxt,
				"alsa_output.platform-sound.analog-stereo",
				&mainVolume,
				NULL,
				NULL
				);
	*/
}



/**
 * Start audio recording
 */
void audio_record_start(void)
{
	if (audio_record_stream)
	{
		audioBufferFlush();
		pa_stream_cork(audio_record_stream,
					0,
					stream_cork_cb,
					NULL
					);
	}
}

/**
 * Sop audio recording
 */
void audio_record_stop(void)
{
	if (audio_record_stream)
	{
		audioBufferFlush();
		pa_stream_cork(audio_record_stream,
						1,
						stream_cork_cb,
						NULL
						);
	}
}


