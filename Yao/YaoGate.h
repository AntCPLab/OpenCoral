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
	static void E_inputs(Key* output, const Key& left, const Key& right,
			const Key& left_delta, const Key& right_delta,
			long T);

	YaoGate() {}
	YaoGate(const YaoGarbleWire& out, const YaoGarbleWire& left,
			const YaoGarbleWire& right, Function func);
	void and_garble(const YaoGarbleWire& out, const Key* hashes, bool left_mask,
			bool right_mask, Key delta);
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

inline void YaoGate::E_inputs(Key* output, const Key& left, const Key& right,
		const Key& left_delta, const Key& right_delta, long T)
{
	auto l = left.doubling(1);
	auto r = right.doubling(2);

	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			output[2 * i + j] = l ^ (i ? left_delta : 0) ^ r
					^ (j ? right_delta : 0) ^ T;
}

inline void YaoGate::and_garble(const YaoGarbleWire& out, const Key* hashes,
		bool left_mask, bool right_mask, Key delta)
{
#define XX(L, R, O) \
	for (int left = 0; left < 2; left++) \
		for (int right = 0; right < 2; right++) \
		{ \
			int index = 2 * left + right; \
			Key key = out.key; \
			if (((left ^ L) & (right ^ R)) ^ O) \
				key += delta; \
			key += hashes[index]; \
			entries[left][right] = key; \
		}
#define Y(L, R) \
	if (out.mask) \
		XX(L, R, true) \
	else \
		XX(L, R, false)
#define Z(L) \
	if (right_mask) \
		Y(L, true) \
	else \
		Y(L, false)

	if (left_mask) \
		Z(true) \
	else \
		Z(false)
}

inline void YaoGate::garble(const YaoGarbleWire& out, const Key* hashes,
		bool left_mask, bool right_mask, Function func, Key delta)
{
	for (int left = 0; left < 2; left++)
		for (int right = 0; right < 2; right++)
		{
			Key key = out.key;
			if (func.call(left ^ left_mask, right ^ right_mask) ^ out.mask)
				key += delta;
#ifdef DEBUG
			cout << "start key " << key << endl;
#endif
			key += hashes[2 * (left) + (right)];
#ifdef DEBUG
			cout << "after left " << key << endl;
#endif
			entries[left][right] = key;
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
