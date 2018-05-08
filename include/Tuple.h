#ifndef TUPLE_H
#define TUPLE_H

template <class T>
class Tuple
{
// I genuinly just wanted a generic struct...
public:
    Tuple() = default;
    Tuple( T _x, T _y ) { x = _x; y = _y; }
    T x;
    T y;
};

#endif // TUPLE_H
