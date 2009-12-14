struct process_t
{
	/* CPU QoS contract data */
	uint64_t period, cycles, next_period, remaining;
};
