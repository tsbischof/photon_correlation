#ifndef TH_V20_H_
#define TH_V20_H_

/* For defintions, see the manuals for the PicoHarp. Naming convention follows
 * that defined there for the structure elements, but the names of the 
 * structures are adapted to my own naming convention.
 */

/* Base resolution of the timing hardware. Interactive mode has its own 
 * specification of the resolution.
 */  

#include <stdio.h>
#include "../picoquant.h"

#pragma pack(2)
typedef struct {
	int32 MapTo;
	int32 Show;
} th_v20_display_curve_t;

typedef struct {
	float32 Start;
	float32 Step;
	float32 Stop;
} th_v20_param_t;

typedef struct {
	int32 BoardSerial;
	int32 CFDZeroCross;
	int32 CFDDiscriminatorMin;
	int32 SYNCLevel;
	int32 CurveOffset;
	float32 Resolution;
} th_v20_board_t;

typedef struct {
	int32 CurveIndex;
	time32 TimeOfRecording; /* time_t TimeOfRecording */
	int32 BoardSerial;
	int32 CFDZeroCross;
	int32 CFDDiscrMin;
	int32 SyncLevel;
	int32 CurveOffset;
	int32 RoutingChannel;
	int32 SubMode;
	int32 MeasMode;
	float32 P1;
	float32 P2;
	float32 P3;
	int32 RangeNo;
	int32 Offset;
	int32 AcquisitionTime;
	int32 StopAfter;
	int32 StopReason;
	int32 SyncRate;
	int32 CFDCountRate;
	int32 TDCCountRate;
	int32 IntegralCount;
	float32 Resolution;
	int32 reserve1;
	int32 reserve2;
	uint32 *Counts;
} th_v20_interactive_t;

typedef struct {
	char HardwareVersion[6];
	char FileTime[18];
	char CRLF[2];
	char Comment[256];
	int32 NumberOfChannels;
	int32 NumberOfCurves;
	int32 BitsPerChannel;
	int32 RoutingChannels;
	int32 NumberOfBoards;
	int32 ActiveCurve;
	int32 MeasurementMode;
	int32 HistogrammingMode;
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
	th_v20_display_curve_t DisplayCurve[8];
	th_v20_param_t Param[3];
	int32 RepeatMode;
	int32 RepeatsPerCurve;
	int32 RepeatTime;
	int32 RepeatWaitTime;
	char ScriptName[20];
	th_v20_board_t *Brd;
} th_v20_header_t;

typedef struct {
	int32 TTTRGlobClock;
	int32 Reserved[6];
	int32 SyncRate;
	int32 AverageCFDRate;
	int32 StopAfter;
	int32 StopReason;
	int32 NumberOfRecords;
} th_v20_tttr_header_t;

typedef struct {
	uint32 TimeTag: 16;
	uint32 Channel: 12;
	uint32 Route: 2;
	uint32 Valid: 1;
	uint32 Reserved: 1;
} th_v20_tttr_record_t;

int th_v20_dispatch(FILE *in_stream, FILE *out_stream, pq_header_t *pq_header,
		options_t *options);

int th_v20_header_read(FILE *in_stream, th_v20_header_t *th_header,
		options_t *options);
void th_v20_header_free(th_v20_header_t *th_header);
void th_v20_header_print(FILE *out_stream, th_v20_header_t *th_header);

int th_v20_interactive_read(FILE *in_stream, th_v20_header_t *th_header,
		th_v20_interactive_t **interactive, options_t *options);
void th_v20_interactive_free(th_v20_interactive_t **interactive,
		th_v20_header_t *th_header);
void th_v20_interactive_header_print(FILE *out_stream, 
		th_v20_header_t *th_header, th_v20_interactive_t **interactive);
void th_v20_interactive_data_print(FILE *out_stream, th_v20_header_t *th_header,
		th_v20_interactive_t **interactive, options_t *options);
int th_v20_interactive_stream(FILE *in_stream, FILE *out_stream,
		pq_header_t *pq_header, th_v20_header_t *th_header, 
		options_t *options);

int th_v20_tttr_header_read(FILE *in_stream, th_v20_header_t *th_header,
		th_v20_tttr_header_t *tttr_header, options_t *options);
void th_v20_tttr_header_print(FILE *out_stream, 
		th_v20_tttr_header_t *tttr_header);

int th_v20_tttr_record_stream(FILE *in_stream, FILE *out_stream,
		th_v20_header_t *th_header, th_v20_tttr_header_t *tttr_header,
		options_t *options);

int th_v20_tttr_stream(FILE *in_stream, FILE *out_stream, 
		pq_header_t *pq_header, th_v20_header_t *th_header, 
		options_t *options);


#endif
