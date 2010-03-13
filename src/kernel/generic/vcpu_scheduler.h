struct scheduler_t
{
	process_queue_t current_q, waiting_q, best_effort_q, blocked_q;
};
