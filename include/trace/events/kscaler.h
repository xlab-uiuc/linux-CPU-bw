/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM kscaler

#if !defined(_TRACE_KSCALER_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_kscaler_H

#include <linux/tracepoint.h>

/*
 * Tracepoint for current active enqueues observed by kscaler
*/
TRACE_EVENT(kscaler_se_enqueue,

	TP_PROTO(struct sched_entity *se, u64 num_se_active),

	TP_ARGS(se, num_se_active),

	TP_STRUCT__entry(
		__field(	u64,	se_p	)
		__field(	int,	num_se_active	)
	),

	TP_fast_assign(
		__entry->se_p	= (u64) se;
		__entry->num_se_active	= num_se_active;
	),

	TP_printk("se=0x%llx num_active_se=%d",
		  __entry->se_p,
		  __entry->num_se_active)
);


/*
 * Tracepoint for current period's runtime-yield as observed by period-bound scaler
*/
TRACE_EVENT(kscaler_bound_record,

	TP_PROTO(u64 runtime, u64 yield),

	TP_ARGS(runtime, yield),

	TP_STRUCT__entry(
		__field(	u64,	runtime	)
		__field(	u64,	yield	)
	),

	TP_fast_assign(
		__entry->runtime	= runtime;
		__entry->yield	= yield;
	),

	TP_printk("runtime=%llu yield=%llu",
		  __entry->runtime,
		  __entry->yield)
);

/*
 * Tracepoint for current se's runtime-yield as observed by period-agnostic scaler
 * We also record the sched entity pointer just in case userspace wants to track it
*/
TRACE_EVENT(kscaler_agnostic_ind_record,

	TP_PROTO(struct sched_entity *se, u64 runtime, u64 yield),

	TP_ARGS(se, runtime, yield),

	TP_STRUCT__entry(
		__field(	u64,	se_p)
		__field(	u64,	runtime	)
		__field(	u64,	yield	)
	),

	TP_fast_assign(
		__entry->se_p	= (u64) se;
		__entry->runtime	= runtime;
		__entry->yield	= yield;
	),

	TP_printk("se=0x%llx runtime=%llu yield=%llu",
		  __entry->se_p,
		  __entry->runtime,
		  __entry->yield)
);

/*
 * Tracepoint for recommendations made by kscaler
 */
DECLARE_EVENT_CLASS(kscaler_recommend_template,

	TP_PROTO(u64 quota, u64 period),

	TP_ARGS(quota, period),

	TP_STRUCT__entry(
		__field(	u64,	quota			)
		__field(	u64,	period			)
	),

	TP_fast_assign(
		__entry->quota	= quota;
		__entry->period	= period;
	),

	TP_printk("quota=%llu period=%llu",
		  __entry->quota,
		  __entry->period)
);

/*
 * Tracepoint called when period bound recommendations are made
 */
DEFINE_EVENT(kscaler_recommend_template, kscaler_bound_recommendation,
	     TP_PROTO(u64 quota, u64 period),
	     TP_ARGS(quota, period));
/*
 * Tracepoint called when period agnostic recommendations are made
 */
DEFINE_EVENT(kscaler_recommend_template, kscaler_agnostic_recommendation,
	     TP_PROTO(u64 quota, u64 period),
	     TP_ARGS(quota, period));

/*
 * Tracepoint called when the final recommendation is made
 */
DEFINE_EVENT(kscaler_recommend_template, kscaler_recommendation,
	     TP_PROTO(u64 quota, u64 period),
	     TP_ARGS(quota, period));

#define MAX_HISTORY 10
TRACE_DEFINE_SIZEOF(u64);
/*
 * Tracepoint for all the current active se runtime-yield as observed by
 * period-agnostic scaler
*/
TRACE_EVENT(kscaler_agnostic_record,

    TP_PROTO(struct sched_entity *se),

    TP_ARGS(se),

    TP_STRUCT__entry(
        __field(	u64,	se_p	)
        __field(	int,	len	)
        __array(	u64,	runtime,	MAX_HISTORY	)
        __array(	u64,	yield,	MAX_HISTORY	)
    ),

    TP_fast_assign(
        __entry->se_p	= (u64) se;
        __entry->len = se->pa_hist_idx;
        memcpy(__entry->runtime, se->pa_runtime_hist, sizeof(u64) * se->pa_hist_idx);
        memcpy(__entry->yield, se->pa_yield_hist, sizeof(u64) * se->pa_hist_idx);
    ),

    TP_printk("se=0x%llx num_entries=%d runtime=%s yield=%s",
          __entry->se_p,
	  __entry->len,
          __print_int_array(__entry->runtime, __entry->len, sizeof(u64)),
          __print_int_array(__entry->yield, __entry->len, sizeof(u64)))
);

/*
 * Tracepoint for all the active se's most recent runtime, yeild
 * Why is this different from kscaler_agnostic_record?
 * kscaler record gives up all the history for one se. This gives all se's last
 * record.
*/
TRACE_EVENT(kscaler_agnostic_se_record,

    TP_PROTO(u64 *last_runtime, u64 *last_yieldtime, int num_se),

    TP_ARGS(last_runtime, last_yieldtime, num_se),

    TP_STRUCT__entry(
	 __field(	int,	len	)
        __array(	u64,	last_runtime,	MAX_HISTORY	)
        __array(	u64,	last_yieldtime,	MAX_HISTORY	)
    ),

    TP_fast_assign(
        __entry->len = num_se;
        memcpy(__entry->last_runtime, last_runtime, sizeof(u64) * num_se);
        memcpy(__entry->last_yieldtime, last_yieldtime, sizeof(u64) * num_se);
    ),

    TP_printk("num_entries=%d runtime=%s yield=%s",
	  __entry->len,
          __print_int_array(__entry->last_runtime, __entry->len, sizeof(u64)),
          __print_int_array(__entry->last_yieldtime, __entry->len, sizeof(u64)))
);

#endif /* _TRACE_KSCALER_H */
/* This part must be outside protection */
#include <trace/define_trace.h>