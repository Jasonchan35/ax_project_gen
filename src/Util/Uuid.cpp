#include "Uuid.h"

namespace ax_gen {

Uuid::Uuid() {
	memset(data, 0, kSize);
}

void Uuid::generate() {
#if ax_OS_Windows
	UUID tmp;
	if (RPC_S_OK != ::UuidCreate(&tmp)) {
		throw Error("error genUuid");
	}

	data[ 0] = (uint8_t)(tmp.Data1 >> 24);
	data[ 1] = (uint8_t)(tmp.Data1 >> 16);
	data[ 2] = (uint8_t)(tmp.Data1 >> 8);
	data[ 3] = (uint8_t)(tmp.Data1);

	data[ 4] = (uint8_t)(tmp.Data2 >> 8);
	data[ 5] = (uint8_t)(tmp.Data2);

	data[ 6] = (uint8_t)(tmp.Data3 >> 8);
	data[ 7] = (uint8_t)(tmp.Data3);

	data[ 8] = (uint8_t)(tmp.Data4[0]);
	data[ 9] = (uint8_t)(tmp.Data4[1]);
	data[10] = (uint8_t)(tmp.Data4[2]);
	data[11] = (uint8_t)(tmp.Data4[3]);
	data[12] = (uint8_t)(tmp.Data4[4]);
	data[13] = (uint8_t)(tmp.Data4[5]);
	data[14] = (uint8_t)(tmp.Data4[6]);
	data[15] = (uint8_t)(tmp.Data4[7]);
#else
	uuid_generate(data);
#endif
}

void Uuid::toString(String& outStr) {
	char buf[64];
	snprintf(buf, 64, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
		data[0],  data[1],  data[2],  data[3],
		data[4],  data[5],
		data[6],  data[7],
		data[8],  data[9],  data[10], data[11],
		data[12], data[13], data[14], data[15]);
	outStr.set(StrView(buf, 32 + 4));
}

bool Uuid::isValid() const {
	for (int i=0; i<kSize; i++) {
		if (data[i] != 0) return false;
	}
	return true;
}

} //namespace