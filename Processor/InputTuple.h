/*
 * InputTuple.h
 *
 */

#ifndef PROCESSOR_INPUTTUPLE_H_
#define PROCESSOR_INPUTTUPLE_H_


template <class T>
struct InputTuple
{
  T share;
  typename T::clear value;

  static int size()
    { return T::clear::size() + T::size(); }

  static string type_string()
    { return T::type_string(); }

  void assign(const char* buffer)
    {
      share.assign(buffer);
      value.assign(buffer + T::size());
    }
};


template <class T>
struct RefInputTuple
{
  T& share;
  typename T::clear& value;
  RefInputTuple(T& share, typename T::clear& value) : share(share), value(value) {}
  void operator=(InputTuple<T>& other) { share = other.share; value = other.value; }
};


#endif /* PROCESSOR_INPUTTUPLE_H_ */
