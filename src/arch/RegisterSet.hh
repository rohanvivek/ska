/*----------------------------------------------------------------------------*
 * Copyright (c) 2012 Los Alamos National Security, LLC
 * All rights reserved
 *----------------------------------------------------------------------------*/

#ifndef RegisterSet_hh
#define RegisterSet_hh

namespace ska {

/*----------------------------------------------------------------------------*
 * Register Set class.
 * All rights reserved
 *----------------------------------------------------------------------------*/

class register_set_t
{
public:

	enum class register_type_t {
		Integer,
		Float,
		Vector
	}; // enum class type

	register_set_t(register_type_t type, size_t num_registers)
		: type_(type), num_registers_(num_registers) {}

	~register_set_t() {}

	register_type_t type() const { return type_; }
	size_t num_registers() const { return num_registers_; }

private:

	register_type_t type_;
	size_t num_registers_;

}; // class register_set_t

} // namespace ska

#endif // RegisterSet_hh

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
