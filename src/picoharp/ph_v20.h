#ifndef PH_V20_H_
#define PH_V20_H_

#include <stdio.h>

#include "../picoquant.h"
#include "../types.h"

#define PH_V20_BASE_RESOLUTION 4e-12

#pragma pack(2)
typedef struct {
	int32 MapTo;
	int32 Show;
} ph_v20_display_curve_t;

typedef struct {
	float32 Start;
	float32 Step;
	float32 Stop;
} ph_v20_param_t;

typedef struct {
	int32 InputType;
	int32 InputLevel;
	int32 InputEdge;
	int32 CFDPresent;
	int32 CFDLevel;
	int32 CFDZCross;
} ph_v20_router_channel_t;

typedef struct {
	int32 CurveIndex;
	time32 TimeOfRecording;
	char HardwareIdent[16];
	char HardwareVersion[8];
	int32 HardwareSerial;
	int32 SyncDivider;
	int32 CFDZeroCross0;
	int32 CFDLevel0;
	int32 CFDZeroCross1;
	int32 CFDLevel1;
	int32 Offset;
	int32 RoutingChannel;
	int32 ExtDevices;
	int32 MeasMode;
	int32 SubMode;
	float32 P1;
	float32 P2;
	float32 P3;
	int32 RangeNo;
	float32 Resolution;
	int32 Channels;
	int32 AcquisitionTime;
	int32 StopAfter;
	int32 StopReason;
	int32 InpRate0;
	int32 InpRate1;
	int32 HistCountRate;
	int64 IntegralCount;
	int32 Reserved;
	int32 DataOffset;
	int32 RouterModelCode;
	int32 RouterEnabled;
	int32 RtCh_InputType;
	int32 RtCh_InputLevel;
	int32 RtCh_InputEdge;
	int32 RtCh_CFDPresent;
	int32 RtCh_CFDLevel;
	int32 RtCh_CFDZeroCross;
} ph_v20_curve_t;

typedef struct {
	char HardwareIdent[16];
	char HardwareVersion[8];
	int32 HardwareSerial;
	int32 SyncDivider;
	int32 CFDZeroCross0;
	int32 CFDLevel0;
	int32 CFDZeroCross1;
	int32 CFDLevel1;
	float32 Resolution;
	int32 RouterModelCode;
	int32 RouterEnabled;
	ph_v20_router_channel_t *RtCh;
} ph_v20_board_t;

typedef struct {
	char CreatorName[18];
	char CreatorVersion[12];
	char FileTime[18];
	char CRLF[2];
	char Comment[256];
	int32 NumberOfCurves;

	/* Note that Records is the only difference between interactive and 
	 * tttr main headers. Interactive calls this BitsPerHistogBin.
	 */
	int32 BitsPerRecord;

	int32 RoutingChannels;
	int32 NumberOfBoards;
	int32 ActiveCurve;
	int32 MeasurementMode;
	int32 SubMode;
	int32 RangeNo;
	int32 Offset;
	int32 AcquisitionTime;
	int32 StopAt;
	int32 StopOnOvfl;
	int32 Restart;
	int32 DisplayLinLog;
	int32 DisplayTimeAxisFrom;
	int32 DisplayTimeAxisTo;
	int32 DisplayCountAxisFrom;
	int32 DisplayCountAxisTo;
	ph_v20_display_curve_t DisplayCurve[8];
	ph_v20_param_t Param[3];
	int32 RepeatMode;
	int32 RepeatsPerCurve;
	int32 RepeatTime;
	int32 RepeatWaitTime;
	char ScriptName[20];
	ph_v20_board_t *Brd;
} ph_v20_header_t;

typedef struct {
	ph_v20_curve_t *Curve;
	uint32 **Counts;
} ph_v20_interactive_t;

typedef struct {
	int32 ExtDevices;
	int32 Reserved[2];
	int32 InpRate0;
	int32 InpRate1;
	int32 StopAfter;
	int32 StopReason;
	int32 NumRecords;
	int32 ImgHdrSize;
	uint32 *ImgHdr;
} ph_v20_tttr_header_t;

typedef struct {
	/* Note that this order is opposite that of the manual (see pt2demo.c)*/
	uint32 time: 28;
	uint32 channel: 4;
} ph_v20_t2_record_t;

typedef struct {
	uint32 nsync: 16;
	uint32 dtime: 12;
	uint32 channel: 4;
} ph_v20_t3_record_t;

/* After defining the structures, there a few well-characterized things we want
 * to do to them, such as reading and freeing.
 */

int ph_v20_dispatch(FILE *in_stream, FILE *out_stream, pq_header_t *pq_header,
		pq_options_t *options);

int ph_v20_header_read(FILE *in_stream, ph_v20_header_t *ph_header,
		pq_options_t *options);
void ph_v20_header_free(ph_v20_header_t *ph_header);
void ph_v20_header_print(FILE *out_stream, ph_v20_header_t *ph_header);

int ph_v20_interactive_header_read(FILE *in_stream, 
		ph_v20_header_t *ph_header,
		ph_v20_interactive_t *interactive, pq_options_t *options);
int ph_v20_interactive_data_read(FILE *in_stream,
		ph_v20_header_t *ph_header,
		ph_v20_interactive_t *interactive, pq_options_t *options);
void ph_v20_interactive_header_free(ph_v20_interactive_t *interactive);
void ph_v20_interactive_data_free(ph_v20_header_t *ph_header,
		ph_v20_interactive_t *interactive);
void ph_v20_interactive_header_print(FILE *out_stream, 
		ph_v20_header_t *ph_header,
		ph_v20_interactive_t *interactive);
void ph_v20_interactive_data_print(FILE *out_stream,
		ph_v20_header_t *ph_header,
		ph_v20_interactive_t *interactive,
		pq_options_t *options);
int ph_v20_interactive_stream(FILE *in_stream, FILE *out_stream,
		pq_header_t *pq_header, ph_v20_header_t *ph_header, 
		pq_options_t *options);

int ph_v20_tttr_header_read(FILE *in_stream, ph_v20_tttr_header_t *tttr_header,
		pq_options_t *options);
void ph_v20_tttr_header_print(FILE *out_stream, 
		ph_v20_tttr_header_t *tttr_header);
void ph_v20_tttr_header_free(ph_v20_tttr_header_t *tttr_header);

int ph_v20_t2_record_stream(FILE *in_stream, FILE *out_stream, 
		ph_v20_header_t *ph_header,
		ph_v20_tttr_header_t *tttr_header, pq_options_t *options);
int ph_v20_t3_record_stream(FILE *in_stream, FILE *out_stream,
		ph_v20_header_t *ph_header,
		ph_v20_tttr_header_t *tttr_header, pq_options_t *options);
int ph_v20_tttr_stream(FILE *in_stream, FILE *out_stream, 
		pq_header_t *pq_header, ph_v20_header_t *ph_header, 
		pq_options_t *options);

#endif
