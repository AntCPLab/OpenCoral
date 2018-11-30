/*
 * YaoGate.h
 *
 */

#ifndef YAO_YAOGATE_H_
#define YAO_YAOGATE_H_

#include "config.h"
#include "BMR/Key.h"
#include "YaoGarbleWire.h"
#include "YaoEvalWire.h"

class YaoGate
{
	Key entries[2][2];
public:
	static Key E_input(const Key& left, const Key& right, long T);

	YaoGate() {}
	YaoGate(const YaoGarbleWire& out, const YaoGarbleWire& left,
			const YaoGarbleWire& right, Function func);
	void garble(const YaoGarbleWire& out, const Key* hashes, bool left_mask,
			bool right_mask, Function func, Key delta);
	void eval(YaoEvalWire& out, const YaoEvalWire& left, const YaoEvalWire& right);
	void eval(YaoEvalWire& out, const Key& hash,
			const Key& entry);
	const Key& get_entry(bool left, bool right) { return entries[left][right]; }
};

inline Key YaoGate::E_input(const Key& left, const Key& right, long T)
{
	Key res = left.doubling(1) ^ right.doubling(2) ^ T;
#ifdef DEBUG
	cout << "E " << res << ": " << left.doubling(1) << " " << right.doubling(2)
			<< " " << T << endl;
#endif
	return res;
}

inline void YaoGate::garble(const YaoGarbleWire& out, const Key* hashes,
		bool left_mask, bool right_mask, Function func, Key delta)
{
	for (int left_value = 0; left_value < 2; left_value++)
		for (int right_value = 0; right_value < 2; right_value++)
		{
			Key key = out.key;
			if (func.call(left_value, right_value) ^ out.mask)
				key += delta;
#ifdef DEBUG
			cout << "start key " << key << endl;
#endif
			key += hashes[2 * (left_value ^ left_mask) + (right_value ^ right_mask)];
#ifdef DEBUG
			cout << "after left " << key << endl;
#endif
			entries[left_value ^ left_mask][right_value ^ right_mask] = key;
		}
#ifdef DEBUG
	//cout << "counter " << YaoGarbler::s().counter << endl;
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			cout << "entry " << i << " " << j << " " << entries[i][j] << endl;
#endif
}

inline void YaoGate::eval(YaoEvalWire& out, const Key& hash, const Key& entry)
{
	Key key = entry;
	key -= hash;
#ifdef DEBUG
	cout << "after left " << key << endl;
#endif
	out.set(key);
}

#endif /* YAO_YAOGATE_H_ */
