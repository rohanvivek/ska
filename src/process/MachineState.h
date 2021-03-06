/*----------------------------------------------------------------------------*
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*
 * Machine state class.
 *----------------------------------------------------------------------------*/

#ifndef MachineState_h
#define MachineState_h

namespace ska {

class machine_state_t
{
public:

	/*-------------------------------------------------------------------------*
	 * Meyer's singleton instance.
 	 *-------------------------------------------------------------------------*/

	static machine_state_t & instance() {
		static machine_state_t m;
		return m;
	} // instance

	/*-------------------------------------------------------------------------*
	 * Advance the cycle count by cycles.
 	 *-------------------------------------------------------------------------*/

	size_t advance(size_t cycles = 1) {
		cycle_ += cycles;
		return cycle_;
	} // advance

	/*-------------------------------------------------------------------------*
	 * Reset the cycle counter.
 	 *-------------------------------------------------------------------------*/

	size_t clear() {
		// this has to do with the ordering of updates to
		// various states (machine, alus, instructions)
		cycle_ = 0;
		return cycle_;
	} // reset

	/*-------------------------------------------------------------------------*
	 * Absolute cycle count.
 	 *-------------------------------------------------------------------------*/

	size_t current() const { return cycle_; }

	/*-------------------------------------------------------------------------*
	 * Base-ten counter for instruction status.
 	 *-------------------------------------------------------------------------*/

	size_t counter() const { return cycle_%10; }

private:

	machine_state_t() : cycle_(0) {}
	machine_state_t(const machine_state_t &);

	~machine_state_t() {}

	size_t cycle_;

}; // class machine_state_t

} // namespace ska

#endif // MachineState_h

/*----------------------------------------------------------------------------*
 * Local Variables:
 * mode:c++
 * c-basic-offset:3
 * indent-tabs-mode:t
 * tab-width:3
 * End:
 *
 * vim: set ts=3 :
 *----------------------------------------------------------------------------*/
