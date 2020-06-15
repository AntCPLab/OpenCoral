/*
 * YaoWire.h
 *
 */

#ifndef YAO_YAOWIRE_H_
#define YAO_YAOWIRE_H_

#include "BMR/Key.h"
#include "BMR/Register.h"

class YaoWire : public Phase
{
protected:
	Key key_;

public:
	template<class T>
	static void xors(GC::Processor<T>& processor, const vector<int>& args);
	template<class T>
	static void xors(GC::Processor<T>& processor, const vector<int>& args,
			size_t start, size_t end);

	void XOR(const YaoWire& left, const YaoWire& right)
	{
		key_ = left.key_ ^ right.key_;
	}
};

#endif /* YAO_YAOWIRE_H_ */
