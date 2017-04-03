#pragma once

#include "String.h"

namespace ax_pjgen {

class Uuid {
public:
	Uuid();	
	void generate();

	void toString(String& outStr);

	bool isValid() const;
	explicit operator bool() const { return isValid(); }

	static const int kSize = 16;
	uint8_t data[16];
};

} //namespace