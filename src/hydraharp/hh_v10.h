#ifndef HH_V10_H_
#define HH_V10_H_

#include <stdio.h>

#include "../picoquant_util.h"
#include "../types.h"

#pragma pack(2)
typedef struct {
	int32 MapTo;
	int32 Show;
} hh_v10_display_curve_t;

typedef struct {
	float32 Start;
	float32 Step;
	float32 Stop;
} hh_v10_param_t;

typedef struct {
	int32 Model;
	int32 Version;
} hh_v10_module_t;

typedef struct {
	int32 ModuleIdx;
	int32 CFDLevel;
	int32 CFDZeroCross;
	int32 Offset;
} hh_v10_input_channel_t;

typedef struct {
	int32 CurveIndex;
	time32 TimeOfRecording;
	char HardwareIdent[16];
	char HardwareVersion[8];
	int32 HardwareSerial;
	int32 NoOfModules;
	hh_v10_module_t Module[10];
	float64 BaseResolution;
	int64 InputsEnabled;
	int32 InpChanPresent;
	int32 RefClockSource;
	int32 ExtDevices;
	int32 MarkerSettings;
	int32 SyncDivider;
	int32 SyncCFDLevel;
	int32 SyncCFDZero;
	int32 SyncOffset;
	int32 InpModuleIdx;
	int32 InpCFDLevel;
	int32 InpCFDZeroCross;
	int32 InpOffset;
	int32 InpChannel;
	int32 MeasMode;
	int32 SubMode;
	int32 Binning;
	float64 Resolution;
	int32 Offset;
	int32 AcquisitionTime;
	int32 StopAfter;
	int32 StopReason;
	float32 P1;
	float32 P2;
	float32 P3;
	int32 SyncRate;
	int32 InputRate;
	int32 HistCountRate;
	int64 IntegralCount;
	int32 HistogramBins;
	int32 DataOffset;
} hh_v10_curve_t;

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

	int32 ActiveCurve;
	int32 MeasurementMode;
	int32 SubMode;
	int32 Binning;
	float64 Resolution;
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
	hh_v10_display_curve_t DisplayCurve[8];
	hh_v10_param_t Param[3];
	int32 RepeatMode;
	int32 RepeatsPerCurve;
	int32 RepeatTime;
	int32 RepeatWaitTime;
	char ScriptName[20];
	char HardwareIdent[16];
	char HardwarePartNo[8];
	int32 HardwareSerial;
	int32 NumberOfModules;
	hh_v10_module_t ModuleInfo[10];
	float64 BaseResolution;
	int64 InputsEnabled;
	int32 InputChannelsPresent;
	int32 RefClockSource;
	int32 ExtDevices;
	int32 MarkerSettings;
	int32 SyncDivider;
	int32 SyncCFDLevel;
	int32 SyncCFDZeroCross;
	int32 SyncOffset;
	hh_v10_input_channel_t *InpChan;
	int32 *InputRate;
} hh_v10_header_t;

typedef struct {
	hh_v10_curve_t *Curve;
	uint32 **Counts;
} hh_v10_interactive_t;

typedef struct {
	int32 SyncRate;
	int32 StopAfter;
	int32 StopReason;
	int32 ImgHdrSize;
	int64 NumRecords;
	uint32 *ImgHdr;
} hh_v10_tttr_header_t;

typedef struct {
	/* Note that this order is opposite that of the manual (see ht2demo.c)*/
	uint32 time: 25;
	uint32 channel: 6;
	uint32 special: 1;
} hh_v10_t2_record_t;

typedef struct {
	uint32 nsync: 10;
	uint32 dtime: 15;
	uint32 channel: 6;
	uint32 special: 1;
} hh_v10_t3_record_t;

/* After defining the structures, there a few well-characterized things we want
 * to do to them, such as reading and freeing.
 */
int hh_v10_dispatch(FILE *in_stream, FILE *out_stream, pq_header_t *pq_header,
		options_t *options);

int hh_v10_header_read(FILE *in_stream, hh_v10_header_t *hh_header,
		options_t *options);
void hh_v10_header_free(hh_v10_header_t *hh_header);
void hh_v10_header_print(FILE *out_stream,
	       hh_v10_header_t *hh_header);

int hh_v10_interactive_header_read(FILE *in_stream, 
		hh_v10_header_t *hh_header,
		hh_v10_interactive_t *interactive, options_t *options);
int hh_v10_interactive_data_read(FILE *in_stream,
		hh_v10_header_t *hh_header,
		hh_v10_interactive_t *interactive, options_t *options);
void hh_v10_interactive_header_free(hh_v10_interactive_t *interactive);
void hh_v10_interactive_data_free(hh_v10_header_t *hh_header,
		hh_v10_interactive_t *interactive);
void hh_v10_interactive_header_print(FILE *out_stream, 
		hh_v10_header_t *hh_header,
		hh_v10_interactive_t *interactive);
void hh_v10_interactive_data_print(FILE *out_stream,
		hh_v10_header_t *hh_header,
		hh_v10_interactive_t *interactive,
		options_t *options);
int hh_v10_interactive_stream(FILE *in_stream, FILE *out_stream,
		pq_header_t *pq_header, hh_v10_header_t *hh_header, 
		options_t *options);

int hh_v10_tttr_header_read(FILE *in_stream, hh_v10_tttr_header_t *tttr_header,
		options_t *options);
void hh_v10_tttr_header_print(FILE *out_stream, 
		hh_v10_tttr_header_t *tttr_header);
void hh_v10_tttr_header_free(hh_v10_tttr_header_t *tttr_header);

int hh_v10_t2_record_stream(FILE *in_stream, FILE *out_stream, 
		hh_v10_header_t *hh_header,
		hh_v10_tttr_header_t *tttr_header, options_t *options);
int hh_v10_t3_record_stream(FILE *in_stream, FILE *out_stream,
		hh_v10_header_t *hh_header,
		hh_v10_tttr_header_t *tttr_header, options_t *options);
int hh_v10_tttr_stream(FILE *in_stream, FILE *out_stream, 
		pq_header_t *pq_header, hh_v10_header_t *hh_header, 
		options_t *options);

#endif
