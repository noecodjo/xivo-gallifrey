/*
 * Asterisk -- A telephony toolkit for Linux.
 *
 * Simple fax applications
 * 
 * 2007-2008, Dmitry Andrianov <asterisk@dima.spb.ru>
 *
 * Code based on original implementation by Steve Underwood <steveu@coppice.org>
 *
 * Source: http://users.netplex.net/~andrew/asterisk/
 *
 * PATCHED BY (C) 2009-2010 Proformatique <dev@proformatique.com>
 * - added PhaseEStatus, PhaseEString in FaxReceived and FaxSent events
 * - sent FaxReceived and FaxSent events even if fax didn't successfully receive or send
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License
 *
 */

/*** MODULEINFO
	<depend>spandsp</depend>
	<defaultenabled>yes</defaultenabled>
***/
 
#include "asterisk.h"

ASTERISK_FILE_VERSION(__FILE__, "$Revision$")

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <pthread.h>
#include <errno.h>
#include <tiffio.h>

#define SPANDSP_EXPOSE_INTERNAL_STRUCTURES
#include <spandsp.h>
#include <spandsp/version.h>

#include "asterisk/lock.h"
#include "asterisk/file.h"
#include "asterisk/logger.h"
#include "asterisk/channel.h"
#include "asterisk/pbx.h"
#include "asterisk/app.h"
#include "asterisk/dsp.h"
#include "asterisk/module.h"
#include "asterisk/manager.h"
extern int option_debug;

/* Compatibility with Asterisk 1.6 */
/* from logger.h: */
#define ast_debug(level, ...) do {       \
	if (option_debug >= (level) ) \
		ast_log(LOG_DEBUG, __VA_ARGS__); \
} while (0)

/* from time.h: */
inline static int ast_tvdiff_sec(struct timeval end, struct timeval start)
{
	int result = end.tv_sec - start.tv_sec;
	if (result > 0 && end.tv_usec < start.tv_usec)
		result--;
	else if (result < 0 && end.tv_usec > start.tv_usec)
		result++;

	return result;
}

#ifdef AST_HAS_T38_SUPPORT
int64_t static ast_tvdiff_us(struct timeval end, struct timeval start)
{
	return (end.tv_sec - start.tv_sec) * (int64_t) 1000000 +
		end.tv_usec - start.tv_usec;
}
#endif

/* End of backports */

#define UNUSED(x) ((void)x)

#ifndef AST_MODULE
#define AST_MODULE "app_fax"
#endif

static const char *app_sndfax_name = "SendFAX";
static const char *app_txfax_name = "TxFAX";
static const char *app_sndfax_synopsis = "Send a FAX";
static const char *app_sndfax_desc = 
"  SendFAX(filename[|options]):\n"
"Send a given TIFF file to the channel as a FAX.\n"
"The option string may contain zero or more of the following characters:\n"
"     'a' - makes the application behave as an answering machine\n"
"           The default behaviour is to behave as a calling machine.\n"
"\n"
"This application uses following variables:\n"
"     LOCALSTATIONID to identify itself to the remote end.\n"
"     LOCALHEADERINFO to generate a header line on each page.\n"
"\n"
"This application sets the following channel variables upon completion:\n"
"     FAXSTATUS       - status of operation:\n"
"                           SUCCESS | FAILED\n"
"     FAXERROR        - Error when FAILED\n"
"     FAXMODE         - Mode used:\n"
"                           audio | T38\n"
"     REMOTESTATIONID - CSID of the remote side.\n"
"     FAXPAGES        - number of pages sent.\n"
"     FAXBITRATE      - transmition rate.\n"
"     FAXRESOLUTION   - resolution.\n"
"\n"
"Returns -1 in case of user hang up or any channel error.\n"
"Returns 0 on success.\n";

static const char *app_rcvfax_name = "ReceiveFAX";
static const char *app_rxfax_name = "RxFAX";
static const char *app_rcvfax_synopsis = "Receive a FAX";
static const char *app_rcvfax_desc = 
"  ReceiveFAX(filename[|options]):\n"
"Receives a fax from the channel into the given filename overwriting\n"
"the file if it already exists. File created will have TIFF format.\n"
"The option string may contain zero or more of the following characters:\n"
"     'c' -- makes the application behave as a calling machine\n"
"            The default behaviour is to behave as an answering machine.\n"
"\n"
"This application uses following variables:\n"
"     LOCALSTATIONID to identify itself to the remote end.\n"
"     LOCALHEADERINFO to generate a header line on each page.\n"
"\n"
"This application sets the following channel variables upon completion:\n"
"     FAXSTATUS       - status of operation:\n"
"                           SUCCESS | FAILED\n"
"     FAXERROR        - Error when FAILED\n"
"     FAXMODE         - Mode used:\n"
"                           audio | T38\n"
"     REMOTESTATIONID - CSID of the remote side.\n"
"     FAXPAGES        - number of pages sent.\n"
"     FAXBITRATE      - transmition rate.\n"
"     FAXRESOLUTION   - resolution.\n"
"\n"
"Returns -1 in case of user hang up or any channel error.\n"
"Returns 0 on success.\n";

//#define MAX_SAMPLES 240
#define MAX_SAMPLES 320

/* Watchdog. I have seen situations when remote fax disconnects (because of poor line
   quality) while SpanDSP continues staying in T30_STATE_IV_CTC state forever.
   To avoid this, we terminate when we see that T30 state does not change for 5 minutes.
   We also terminate application when more than 30 minutes passed regardless of
   state changes. This is just a precaution measure - no fax should take that long */

#define WATCHDOG_TOTAL_TIMEOUT	30 * 60
#define WATCHDOG_STATE_TIMEOUT	5 * 60

#undef AST_HAS_T38_SUPPORT

typedef struct {
	struct ast_channel *chan;
#ifdef AST_HAS_T38_SUPPORT
	enum ast_t38_state t38state;	/* T38 state of the channel */
#endif
	int direction;			/* Fax direction: 0 - receiving, 1 - sending */
	int caller_mode;
	char *file_name;
	
	volatile int finished;
} fax_session;

static void span_message(int level, const char *msg)
{
	if (level == SPAN_LOG_ERROR) {
		ast_log(LOG_ERROR, "%s", msg);
	} else if (level == SPAN_LOG_WARNING) {
		ast_log(LOG_WARNING, "%s", msg);
	} else {
		ast_log(LOG_DEBUG, "%s", msg);
	}
}

#ifdef AST_HAS_T38_SUPPORT
static int t38_tx_packet_handler(t38_core_state_t *s, void *user_data, const uint8_t *buf, int len, int count)
{
	struct ast_channel *chan = (struct ast_channel *) user_data;

	struct ast_frame outf = {
		.frametype = AST_FRAME_MODEM,
		.subclass = AST_MODEM_T38,
		.src = __FUNCTION__,
	};

	/* TODO: Asterisk does not provide means of resending the same packet multiple
	  times so count is ignored at the moment */

	AST_FRAME_SET_BUFFER(&outf, buf, 0, len);

	if (ast_write(chan, &outf) < 0) {
		ast_log(LOG_WARNING, "Unable to write frame to channel; %s\n", strerror(errno));
		return -1;
	}

	return 0;
}
#endif

static void phase_e_handler(t30_state_t *f, void *user_data, int result)
{
	const char *local_ident;
	const char *far_ident;
	char buf[20];
	char phase_estring[128];
	fax_session *s = (fax_session *) user_data;
	t30_stats_t xstat;
	int pages_transferred;

	ast_debug(1, "Fax phase E handler. result=%d\n", result);

	t30_get_transfer_statistics(f, &xstat);

	s = (fax_session *) user_data;
	
	local_ident = t30_get_tx_ident(f);
	far_ident = t30_get_rx_ident(f);

	pbx_builtin_setvar_helper(s->chan, "REMOTESTATIONID", far_ident);
	snprintf(phase_estring, sizeof(phase_estring), "%s", t30_completion_code_to_str(result));

#if SPANDSP_RELEASE_DATE >= 20090220
	pages_transferred = (s->direction) ? xstat.pages_tx : xstat.pages_rx;
#else
	pages_transferred = xstat.pages_transferred;
#endif
	
	manager_event(EVENT_FLAG_CALL,
		      s->direction ? "FaxSent" : "FaxReceived", 
		      "Channel: %s\r\n"
		      "Context: %s\r\n"
		      "Exten: %s\r\n"
		      "CallerID: %s\r\n"
		      "RemoteStationID: %s\r\n"
		      "LocalStationID: %s\r\n"
		      "PagesTransferred: %d\r\n"
		      "Resolution: %d\r\n"
		      "TransferRate: %d\r\n"
		      "FileName: %s\r\n"
		      "PhaseEStatus: %d\r\n"
		      "PhaseEString: %s\r\n",
		      s->chan->name,
		      s->chan->context,
		      s->chan->exten,
		      S_OR(s->chan->cid.cid_num, ""),
		      far_ident,
		      local_ident,
		      pages_transferred,
		      xstat.y_resolution,
		      xstat.bit_rate,
		      s->file_name,
		      result,
		      phase_estring);

	if (result != T30_ERR_OK) {
		s->finished = -1;

		/* FAXSTATUS is already set to FAILED */
		pbx_builtin_setvar_helper(s->chan, "FAXERROR", phase_estring);

		ast_log(LOG_WARNING, "Error transmitting fax. result=%d: %s.\n", result, phase_estring);

		return;
	}
	
	s->finished = 1; 
	
	pbx_builtin_setvar_helper(s->chan, "FAXSTATUS", "SUCCESS"); 
	pbx_builtin_setvar_helper(s->chan, "FAXERROR", NULL); 
	
	snprintf(buf, sizeof(buf), "%d", pages_transferred);
	pbx_builtin_setvar_helper(s->chan, "FAXPAGES", buf);
	snprintf(buf, sizeof(buf), "%d", xstat.y_resolution);
	pbx_builtin_setvar_helper(s->chan, "FAXRESOLUTION", buf);
	snprintf(buf, sizeof(buf), "%d", xstat.bit_rate);
	pbx_builtin_setvar_helper(s->chan, "FAXBITRATE", buf); 
	
	ast_debug(1, "Fax transmitted successfully.\n");
	ast_debug(1, "  Remote station ID: %s\n", far_ident);
	ast_debug(1, "  Pages transferred: %d\n", pages_transferred);
	ast_debug(1, "  Image resolution:  %d x %d\n", xstat.x_resolution, xstat.y_resolution);
	ast_debug(1, "  Transfer Rate:     %d\n", xstat.bit_rate);
}

/* === Helper functions to configure fax === */

/* Setup SPAN logging according to Asterisk debug level */
static int set_logging(logging_state_t *state)
{
	int level = SPAN_LOG_WARNING + option_debug;

	span_log_set_message_handler(state, span_message);
	span_log_set_level(state, SPAN_LOG_SHOW_SEVERITY | SPAN_LOG_SHOW_PROTOCOL | level); 

	return 0;
}

static void set_local_info(t30_state_t *state, fax_session *s)
{
	const char *x;

	x = pbx_builtin_getvar_helper(s->chan, "LOCALSTATIONID");
	if (!ast_strlen_zero(x))
		t30_set_tx_ident(state, x);

	x = pbx_builtin_getvar_helper(s->chan, "LOCALHEADERINFO");
	if (!ast_strlen_zero(x))
		t30_set_tx_page_header_info(state, x);
}

static void set_file(t30_state_t *state, fax_session *s)
{
	if (s->direction)
		t30_set_tx_file(state, s->file_name, -1, -1);
	else
		t30_set_rx_file(state, s->file_name, -1);
}

static void set_ecm(t30_state_t *state, int ecm)
{
	t30_set_ecm_capability(state, ecm);
	t30_set_supported_compressions(state, T30_SUPPORT_T4_1D_COMPRESSION | T30_SUPPORT_T4_2D_COMPRESSION | T30_SUPPORT_T6_COMPRESSION);
}

/* === Generator === */

/* This function is only needed to return passed params so
   generator_activate will save it to channel's generatordata */
static void *fax_generator_alloc(struct ast_channel *chan, void *params)
{
	UNUSED(chan);
	return params;
}

static int fax_generator_generate(struct ast_channel *chan, void *data, int len, int samples)
{
	fax_state_t *fax = (fax_state_t*) data;
	uint8_t buffer[AST_FRIENDLY_OFFSET + MAX_SAMPLES * sizeof(uint16_t)];
	int16_t *buf = (int16_t *) (buffer + AST_FRIENDLY_OFFSET);
    
	struct ast_frame outf = {
		.frametype = AST_FRAME_VOICE,
		.subclass = AST_FORMAT_SLINEAR,
		.src = __FUNCTION__,
	};

	if (samples > MAX_SAMPLES) {
		ast_log(LOG_WARNING, "Only generating %d samples, where %d requested\n", MAX_SAMPLES, samples);
		samples = MAX_SAMPLES;
	}
	
	if ((len = fax_tx(fax, buf, samples)) > 0) {
		outf.samples = len;
		AST_FRAME_SET_BUFFER(&outf, buffer, AST_FRIENDLY_OFFSET, len * sizeof(int16_t));

		if (ast_write(chan, &outf) < 0) {
			ast_log(LOG_WARNING, "Failed to write frame to '%s': %s\n", chan->name, strerror(errno));
			return -1;
		}
	}

	return 0;
}

struct ast_generator generator = {
	alloc:		fax_generator_alloc,
	generate: 	fax_generator_generate,
};


/* === Transmission === */

static int transmit_audio(fax_session *s)
{
	int res = -1;
	int original_read_fmt = AST_FORMAT_SLINEAR;
	int original_write_fmt = AST_FORMAT_SLINEAR;
	fax_state_t fax;
	t30_state_t *t30state;
	struct ast_dsp *dsp = NULL;
	struct ast_frame *inf = NULL;
	int last_state = 0;
	struct timeval now, start, state_change;
#ifdef AST_HAS_T38_SUPPORT
	int detect_tone = 0;
	struct ast_frame *fr;
	enum ast_control_t38 t38control;
#endif

#if SPANDSP_RELEASE_DATE >= 20081012
        /* for spandsp shaphots 0.0.6 and higher */
        t30state = &fax.t30;
#else
        /* for spandsp release 0.0.5 */
        t30state = &fax.t30_state;
#endif

	original_read_fmt = s->chan->readformat;
	if (original_read_fmt != AST_FORMAT_SLINEAR) {
		res = ast_set_read_format(s->chan, AST_FORMAT_SLINEAR);
		if (res < 0) {
			ast_log(LOG_WARNING, "Unable to set to linear read mode, giving up\n");
			goto done;
		}
	}

	original_write_fmt = s->chan->writeformat;
	if (original_write_fmt != AST_FORMAT_SLINEAR) {
		res = ast_set_write_format(s->chan, AST_FORMAT_SLINEAR);
		if (res < 0) {
			ast_log(LOG_WARNING, "Unable to set to linear write mode, giving up\n");
			goto done;
		}
	}

	/* Initialize T30 terminal */
	fax_init(&fax, s->caller_mode);

	/* Setup logging */
	set_logging(&fax.logging);
	set_logging(&t30state->logging);

	/* Configure terminal */
	set_local_info(t30state, s);
	set_file(t30state, s);
	set_ecm(t30state, TRUE);

	fax_set_transmit_on_idle(&fax, TRUE);

	t30_set_phase_e_handler(t30state, phase_e_handler, s);

#ifdef AST_HAS_T38_SUPPORT
	if (s->t38state == T38_STATE_UNAVAILABLE) {
		ast_debug(1, "T38 is unavailable on %s\n", s->chan->name);
	} else if (!s->direction) {
		/* We are receiving side and this means we are the side which should
		   request T38 when the fax is detected. Use DSP to detect fax tone */
		ast_debug(1, "Setting up CNG detection on %s\n", s->chan->name);
		dsp = ast_dsp_new();
		ast_dsp_set_features(dsp, DSP_FEATURE_FAX_DETECT);
		ast_dsp_set_faxmode(dsp, DSP_FAXMODE_DETECT_CNG);
		detect_tone = 1;
	}
#endif

	start = state_change = ast_tvnow();

	ast_activate_generator(s->chan, &generator, &fax);

	while (!s->finished) {

		/* Watchdog */
		now = ast_tvnow();
		if (ast_tvdiff_sec(now, start) > WATCHDOG_TOTAL_TIMEOUT || ast_tvdiff_sec(now, state_change) > WATCHDOG_STATE_TIMEOUT) {
			ast_log(LOG_WARNING, "It looks like we hung. Aborting.\n");
			res = -1;
			break;
		}

		res = ast_waitfor(s->chan, 50);
		if (res > 0)
			res = 0;
		else if (!res)
			continue;
		else if (res < 0)
			break;

		inf = ast_read(s->chan);
		if (inf == NULL) {
			ast_debug(1, "Channel hangup\n");
			res = -1;
			break;
		}

		ast_debug(10, "frame %d/%d, len=%d\n", inf->frametype, inf->subclass, inf->datalen);

/* FIXME: that part should probably be included, not cut off (Tzafrir)*/
#ifdef AST_HAS_T38_SUPPORT
		/* Detect fax tone */
		if (detect_tone && inf->frametype == AST_FRAME_VOICE) {
			/* Duplicate frame because ast_dsp_process may free the frame passed */
			fr = ast_frdup(inf);

			/* Do not pass channel to ast_dsp_process otherwise it may queue modified audio frame back */
			fr = ast_dsp_process(NULL, dsp, fr);
			if (fr && fr->frametype == AST_FRAME_DTMF && fr->subclass == 'f') {
				ast_debug(1, "Fax tone detected. Requesting T38\n");
				t38control = AST_T38_REQUEST_NEGOTIATE;
				ast_indicate_data(s->chan, AST_CONTROL_T38, &t38control, sizeof(t38control));
				detect_tone = 0;
			}

			ast_frfree(fr);
		}
#endif


		/* Check the frame type. Format also must be checked because there is a chance
		   that a frame in old format was already queued before we set chanel format
		   to slinear so it will still be received by ast_read */
		if (inf->frametype == AST_FRAME_VOICE && inf->subclass == AST_FORMAT_SLINEAR) {

			if (fax_rx(&fax, inf->data, inf->samples) < 0) {
				/* I know fax_rx never returns errors. The check here is for good style only */
				ast_log(LOG_WARNING, "fax_rx returned error\n");
				res = -1;
				break;
			}

			/* Watchdog */
			if (last_state != t30state->state) {
				state_change = ast_tvnow();
				last_state = t30state->state;
			}
#ifdef AST_HAS_T38_SUPPORT
		} else if (inf->frametype == AST_FRAME_CONTROL && inf->subclass == AST_CONTROL_T38 &&
				inf->datalen == sizeof(enum ast_control_t38)) {
			t38control =*((enum ast_control_t38 *) inf->data.ptr);
			if (t38control == AST_T38_NEGOTIATED) {
				/* T38 switchover completed */
				ast_debug(1, "T38 negotiated, finishing audio loop\n");
				res = 1;
				break;
			} else if (t38control == AST_T38_REQUEST_NEGOTIATE) {
				ast_debug(1, "T38 reinvite received from remote peer. Accepting.\n");
				t38control = AST_T38_REQUEST_NEGOTIATE;
				ast_indicate_data(s->chan, AST_CONTROL_T38, &t38control, sizeof(t38control));
				detect_tone = 0;
			}
#endif
		}

		ast_frfree(inf);
		inf = NULL;
	}

	ast_debug(1, "Loop finished, res=%d\n", res);

	if (inf)
		ast_frfree(inf);

	if (dsp)
		ast_dsp_free(dsp);

	ast_deactivate_generator(s->chan);

	/* If we are switching to T38, remove phase E handler. Otherwise it will be executed
	   by t30_terminate, display diagnostics and set status variables although no transmittion
	   has taken place yet. */
	if (res > 0) {
		t30_set_phase_e_handler(t30state, NULL, NULL);
	}

	t30_terminate(t30state);
	fax_release(&fax);

done:
	if (original_write_fmt != AST_FORMAT_SLINEAR) {
		if (ast_set_write_format(s->chan, original_write_fmt) < 0)
			ast_log(LOG_WARNING, "Unable to restore write format on '%s'\n", s->chan->name);
	}

	if (original_read_fmt != AST_FORMAT_SLINEAR) {
		if (ast_set_read_format(s->chan, original_read_fmt) < 0)
			ast_log(LOG_WARNING, "Unable to restore read format on '%s'\n", s->chan->name);
	}

	return res;

}

#ifdef AST_HAS_T38_SUPPORT
static int transmit_t38(fax_session *s)
{
	t38_terminal_state_t t38;
	int res = 0;
	struct ast_frame *inf = NULL;
	int last_state = 0;
	struct timeval now, start, state_change, last_frame;
	enum ast_control_t38 t38control;

	t30_state_t *t30state;
	t38_core_state_t *t38state;

#if SPANDSP_RELEASE_DATE >= 20081012
	/* for spandsp shaphots 0.0.6 and higher */
	t30state = &t38.t30;
	t38state = &t38.t38_fe.t38;
#else
	/* for spandsp releases 0.0.5 */
	t30state = &t38.t30_state;
	t38state = &t38.t38;
#endif

	/* Initialize terminal */
	memset(&t38, 0, sizeof(t38));
	if (t38_terminal_init(&t38, s->caller_mode, t38_tx_packet_handler, s->chan) == NULL) {
		ast_log(LOG_WARNING, "Unable to start T.38 termination.\n");
		return -1;
	}

	/* Setup logging */
	set_logging(&t38.logging);
	set_logging(&t30state->logging);
	set_logging(&t38state->logging);

	/* Configure terminal */
	set_local_info(t30state, s);
	set_file(t30state, s);
	set_ecm(t30state, TRUE);

	t30_set_phase_e_handler(t30state, phase_e_handler, s);

	now = start = state_change = ast_tvnow();

	while (!s->finished) {

		/* Watchdog */
		if (ast_tvdiff_sec(now, start) > WATCHDOG_TOTAL_TIMEOUT || ast_tvdiff_sec(now, state_change) > WATCHDOG_STATE_TIMEOUT) {
			ast_log(LOG_WARNING, "It looks like we hung. Aborting.\n");
			res = -1;
			break;
		}

		res = ast_waitfor(s->chan, 20);
		if (res < 0)
			break;

		last_frame = now;
		now = ast_tvnow();
		t38_terminal_send_timeout(&t38, ast_tvdiff_us(now, last_frame) / (1000000 / 8000));

		if (res > 0)
			res = 0;
		else if (!res)
			continue;

		inf = ast_read(s->chan);
		if (inf == NULL) {
			ast_debug(1, "Channel hangup\n");
			res = -1;
			break;
		}

		ast_debug(10, "frame %d/%d, len=%d\n", inf->frametype, inf->subclass, inf->datalen);

		if (inf->frametype == AST_FRAME_MODEM && inf->subclass == AST_MODEM_T38) {
			t38_core_rx_ifp_packet(t38state, inf->data.ptr, inf->datalen, inf->seqno);

			/* Watchdog */
			if (last_state != t30state->state) {
				state_change = ast_tvnow();
				last_state = t30state->state;
			}
		} else if (inf->frametype == AST_FRAME_CONTROL && inf->subclass == AST_CONTROL_T38 &&
				inf->datalen == sizeof(enum ast_control_t38)) {

			t38control = *((enum ast_control_t38 *) inf->data.ptr);

			if (t38control == AST_T38_TERMINATED || t38control == AST_T38_REFUSED) {
				ast_debug(1, "T38 down, terminating\n");
				break;
			}
		}

		ast_frfree(inf);
		inf = NULL;
	}

	ast_debug(1, "Loop finished, res=%d\n", res);

	if (inf)
		ast_frfree(inf);

	t30_terminate(t30state);
	t38_terminal_release(&t38);

	return res;
}
#endif

static int transmit(fax_session *s)
{
	int res = 0;

	/* Clear all channel variables which to be set by the application.
	   Pre-set status to error so in case of any problems we can just leave */
	pbx_builtin_setvar_helper(s->chan, "FAXSTATUS", "FAILED"); 
	pbx_builtin_setvar_helper(s->chan, "FAXERROR", "Channel problems"); 

	pbx_builtin_setvar_helper(s->chan, "FAXMODE", NULL);
	pbx_builtin_setvar_helper(s->chan, "REMOTESTATIONID", NULL);
	pbx_builtin_setvar_helper(s->chan, "FAXPAGES", "0");
	pbx_builtin_setvar_helper(s->chan, "FAXRESOLUTION", NULL);
	pbx_builtin_setvar_helper(s->chan, "FAXBITRATE", NULL); 

	if (s->chan->_state != AST_STATE_UP) {
		/* Shouldn't need this, but checking to see if channel is already answered
		 * Theoretically asterisk should already have answered before running the app */
		res = ast_answer(s->chan);
		if (res) {
			ast_log(LOG_WARNING, "Could not answer channel '%s'\n", s->chan->name);
			return res;
		}
	}

#ifdef AST_HAS_T38_SUPPORT
	s->t38state = ast_channel_get_t38_state(s->chan);
	if (s->t38state != T38_STATE_NEGOTIATED) {
#endif
		/* T38 is not negotiated on the channel yet. First start regular transmission. If it switches to T38, follow */	
		pbx_builtin_setvar_helper(s->chan, "FAXMODE", "audio"); 
		res = transmit_audio(s);
		if (res > 0) {
			/* transmit_audio reports switchover to T38. Update t38state */
#ifdef AST_HAS_T38_SUPPORT
			s->t38state = ast_channel_get_t38_state(s->chan);
			if (s->t38state != T38_STATE_NEGOTIATED) {
				ast_log(LOG_ERROR, "Audio loop reports T38 switchover but t38state != T38_STATE_NEGOTIATED\n");
			}
#endif
		}
#ifdef AST_HAS_T38_SUPPORT
	}

	if (s->t38state == T38_STATE_NEGOTIATED) {
		pbx_builtin_setvar_helper(s->chan, "FAXMODE", "T38"); 
		res = transmit_t38(s);
	}
#endif

	if (res) {
		ast_log(LOG_WARNING, "Transmission error\n");
		res = -1;
	} else if (s->finished < 0) {
		ast_log(LOG_WARNING, "Transmission failed\n");
	} else if (s->finished > 0) {
		ast_debug(1, "Transmission finished Ok\n");
	}

	return res;
}

/* === Application functions === */

static int sndfax_exec(struct ast_channel *chan, void *data)
{
	int res = 0;
	char *parse;
	struct ast_module_user *u;
	fax_session session;

	AST_DECLARE_APP_ARGS(args,
		AST_APP_ARG(file_name);
		AST_APP_ARG(options);
	);

	if (chan == NULL) {
		ast_log(LOG_ERROR, "Fax channel is NULL. Giving up.\n");
		return -1;
	}

	/* The next few lines of code parse out the filename and header from the input string */
	if (ast_strlen_zero(data)) {
		/* No data implies no filename or anything is present */
		ast_log(LOG_ERROR, "SendFAX requires an argument (filename)\n");
		return -1;
	}

	parse = ast_strdupa(data);
	AST_STANDARD_APP_ARGS(args, parse);
	
	session.caller_mode = TRUE;

	if (args.options) {
		if (strchr(args.options, 'a'))
			session.caller_mode = FALSE;
	}

	/* Done parsing */

	u = ast_module_user_add(chan);

	session.direction = 1;
	session.file_name = args.file_name;
	session.chan = chan;
	session.finished = 0;

	res = transmit(&session);

	ast_module_user_remove(u);

	return res;
}

static int rcvfax_exec(struct ast_channel *chan, void *data)
{
	int res = 0;
	char *parse;
	struct ast_module_user *u;
	fax_session session;

	AST_DECLARE_APP_ARGS(args,
		AST_APP_ARG(file_name);
		AST_APP_ARG(options);
	);

	if (chan == NULL) {
		ast_log(LOG_ERROR, "Fax channel is NULL. Giving up.\n");
		return -1;
	}

	/* The next few lines of code parse out the filename and header from the input string */
	if (ast_strlen_zero(data)) {
		/* No data implies no filename or anything is present */
		ast_log(LOG_ERROR, "ReceiveFAX requires an argument (filename)\n");
		return -1;
	}

	parse = ast_strdupa(data);
	AST_STANDARD_APP_ARGS(args, parse);
	
	session.caller_mode = FALSE;

	if (args.options) {
		if (strchr(args.options, 'c'))
			session.caller_mode = TRUE;
	}

	/* Done parsing */

	u = ast_module_user_add(chan);

	session.direction = 0;
	session.file_name = args.file_name;
	session.chan = chan;
	session.finished = 0;

	res = transmit(&session);

	ast_module_user_remove(u);

	return res;
}

static int unload_module(void)
{
	int res;

	res = ast_unregister_application(app_sndfax_name);	
	res |= ast_unregister_application(app_txfax_name);
	res |= ast_unregister_application(app_rcvfax_name);	
	res |= ast_unregister_application(app_rxfax_name);

	return res;
}

static int load_module(void)
{
	int res ;

	res = ast_register_application(app_sndfax_name, sndfax_exec, app_sndfax_synopsis, app_sndfax_desc);
	res |= ast_register_application(app_txfax_name, sndfax_exec, app_sndfax_synopsis, app_sndfax_desc);
	res |= ast_register_application(app_rcvfax_name, rcvfax_exec, app_rcvfax_synopsis, app_rcvfax_desc);
	res |= ast_register_application(app_rxfax_name, rcvfax_exec, app_rcvfax_synopsis, app_rcvfax_desc);

	/* The default SPAN message handler prints to stderr. It is something we do not want */
	span_set_message_handler(NULL);

	return res;
}


AST_MODULE_INFO(ASTERISK_GPL_KEY, AST_MODFLAG_DEFAULT, "Simple FAX Application",
		.load = load_module,
		.unload = unload_module,
		);/* WWW [1] */

/*
 *
 * [1]: 'static' is not at beginning of declaration
 *      harmless
 */
