#ifndef BIGINT_NJAROS
#define BIGINT_NJAROS

#include <iostream>
#include <string.h>

typedef uint8_t byte;

/**
 * In cryptography we need numbers with 32, and 20 bytes long.
 * That doesn't exists on std, so I had to implement a type
 * with a templated number of bytes.
 */
template < size_t S >
class bigInt {

    byte n[S];
    static constexpr const size_t _SIZE = S;
    static constexpr const char hexaBase[] = "0123456789abcdef";

    void _init() {
        for (int i = 0; i < _SIZE; ++i)
            n[i] = 0;
    }

    std::string _byteToBitsString(byte b) const {
        std::string bits;

        for (int i = 7; i >= 0; --i) {
            bits += (b & (1 << i)) != 0
                    ? '1'
                    : '0';
        }

        return bits;
    }

    byte _hexaValue(char c) const
    {
        for (int i = 0; i < 16; ++i) {
            if (std::tolower(c) == hexaBase[i])
                return i;
        }

        std::string error = "char read is not an hexadecimal, have ";
        error += c;
        error += '.';
        throw std::logic_error(error);
    }

public:

    // Assuming a have _SIZE higher than b _SIZE
    template < size_t K, size_t U >
    static bool _isEqual(const bigInt<U>& a, const bigInt<K>& b) {
        const size_t sizeA = a.size();
        const size_t sizeB = b.size();
        const size_t sizeDiff = sizeA - sizeB;

        for (size_t idx = 0; idx < sizeDiff; ++idx) {
            if (a[idx] != 0)
                return false;
        }

        for (size_t idx = sizeDiff; idx < sizeA; ++idx) {
            if (a[idx] != b[idx - sizeDiff])
                return false;
        }

        return true;
    }

    // Compare function assuming a have a _SIZE higher than b.
    // Return true if a is smaller than b.
    template < size_t K, size_t U >
    static bool _isSmaller(const bigInt<U>& a, const bigInt<K>& b) {
        const size_t sizeA = a.size();
        const size_t sizeB = b.size();
        const size_t sizeDiff = sizeA - sizeB;

        for (size_t idx = 0; idx < sizeDiff; ++idx) {
            if (a[idx] != 0)
                return false;
        }

        for (size_t idx = sizeDiff; idx < sizeA; ++idx) {
            if (a[idx] > b[idx - sizeDiff])
                return false;
            if (a[idx] < b[idx - sizeDiff])
                return true;
        }

        return false;
    }

    // Assuming a have _SIZE higher than b _SIZE
    template < size_t K, size_t U >
    static bool _isHigher(const bigInt<U>& a, const bigInt<K>& b) {
        const size_t sizeA = a.size();
        const size_t sizeB = b.size();
        const size_t sizeDiff = sizeA - sizeB;

        for (size_t idx = 0; idx < sizeDiff; ++idx) {
            if (a[idx] != 0) {
                return true;
            }
        }

        for (size_t idx = sizeDiff; idx < sizeA; ++idx) {
            if (a[idx] < b[idx - sizeDiff])
                return false;
            if (a[idx] > b[idx - sizeDiff])
                return true;
        }

        return false;
    }

    template < size_t K >
    static size_t _getRank(const bigInt<K>& b) {
        size_t rank = b.size() * 8;
        while (rank > 0 && !b.getBitAt(rank - 1))
            --rank;

        return rank;
    }

    // If an overflow occurs, the operation stop and the first element is true and the
    // second element is the incomplete result.
    // If operation reach ends without overflow, first element is false and second element
    // is the multiplication result.
    template < size_t U, size_t K >
    static std::pair< bool, bigInt<U> > _multOverflowDetect(const bigInt<U>& a, const bigInt<K>& b) {
        bigInt<U> result;
        bigInt<U> temp;

        for (size_t i = 0; i != 8 * b.size(); ++i) {
            if (b.getBitAt(i))
            {
                if ((a.rank() + i) > (a.size() * 8))
                {
                    return std::make_pair(true, result);
                }
                result += (a << i);
                if (result < temp)
                {
                    return std::make_pair(true, result);
                }
                temp = result;
            }
        }
        return std::make_pair(false, result);
    }

public:

    bigInt() { _init(); }

    bigInt(const std::string& hex) {

        if (hex.size() != 2 * _SIZE)
        {
            std::string error = "the hexa string has to be ";
            error += std::to_string(2 * _SIZE);
            error += " char len, have ";
            error += std::to_string(hex.size());
            error += '.';
            throw std::logic_error(error);
        }

        for (int i = 0; i < _SIZE; ++i) {
            n[i] = (_hexaValue(hex[2 * i]) * 16)
                  + _hexaValue(hex[2 * i + 1]);
        }
    }

    template < size_t K >
    bigInt(const bigInt<K>& other) {
        *this = other;
    }

    template < size_t K >
    bigInt& operator=(const bigInt<K>& other) {
        size_t idx = _SIZE;
        size_t idx_other = other.size();

        while (idx-- != 0) {
            if (idx_other != 0)
                n[idx] = other[--idx_other];
            else
                n[idx] = 0;
        }

        return *this;
    }

    // GETTERS

    size_t size() const { return this->_SIZE; }

    size_t rank() const {
        size_t rank = _SIZE * 8;

        while (rank-- != 0)
        {
            if (getBitAt(rank))
                return rank + 1;
        }
        return 0;
    }

    // Use with care.
    byte* base() { return n; }

    const byte* base() const { return n; }

    // ARITHMETIC OPERATORS

    template < size_t K >
    bigInt& operator+=(const bigInt<K>& other) {
        bool carry = false;
        byte temp1;

        size_t idx = _SIZE;
        size_t other_idx = other.size();

        while (idx-- != 0) {
            temp1 = n[idx];
            n[idx] = n[idx] + carry;
            if (other_idx != 0)
                n[idx] += other[--other_idx];
                carry = (n[idx] - carry) < temp1;
        }

        return *this;
    }

    template < size_t K >
    bigInt operator+(const bigInt<K>& other) const {
        bigInt result(*this);
        result += other;

        return result;
    }

    template < size_t K >
    bigInt& operator-=(const bigInt<K>& other ) {
        bigInt complement(~bigInt(other) + bigInt<1>("01"));

        return *this += complement;
    }

    template < size_t K >
    bigInt operator-(const bigInt<K>& other) const {
        bigInt result(*this);
        result -= other;

        return result;
    }

    template < size_t K >
    bigInt operator*(const bigInt<K>& other) const {
        bigInt result;

        for (size_t i = 0; i != 8 * other.size(); ++i) {
            if (other.getBitAt(i))
                result += (*this << i);
        }
        return result;
    }

    template < size_t K >
    bigInt& operator*=(const bigInt<K>& other) {
        return *this = *this * other;
    }

    template < size_t K >
    bigInt operator/(const bigInt<K>& other) const {
        bigInt comparer(*this);
        bigInt result;
        size_t bitIdx;
        std::pair<bool, bigInt> multRes;

        if (*this < other)
            return bigInt();
        
        if (other == bigInt<0>())
            throw std::runtime_error("Divide by 0.");

        // To detect the overflow in the good range, I have to recast
        // to the size of current object.
        bigInt castedOther(other);

        // create a number like "b(100...0)"
        bigInt multiplier(bigInt<1>("01")); 
        multiplier <<= (_SIZE * 8) - 1;
        bitIdx = size() * 8;
        while (bitIdx-- != 0)
        {
            multRes = _multOverflowDetect(castedOther, multiplier);
            
            if (!multRes.first)
            {
                if (comparer >= multRes.second) {
                    comparer -= multRes.second;
                    result += multiplier;
                }
            }
            multiplier >>= 1;
        }

        return result;
    }

    template <size_t K >
    bigInt& operator/=(const bigInt<K>& other) {
        return *this = *this / other;
    }

    template < size_t K >
    bigInt operator%(const bigInt<K>& other) {
        if (other.size() >= _SIZE)
            return *this - (other * (*this / other));
        else
            return *this - (bigInt(other) * (*this / other));
    }

    template <size_t K >
    bigInt& operator%=(const bigInt<K>& other) {
        return *this = *this % other;
    }

    // BOOLEAN OPERATORS

    /**
     * Algo from a collegue:
     * - at each loop i in the take the byte where the shift is
     *  and his left neighboor.
     * - then builds a short with them and shift this short
     *  with the 8 module of the shift.
     * - then recast the number as a byte.
     * - then assign the i nth byte to this built byte.
     */
    bigInt operator>>(size_t shift) const {
        bigInt result;

        for (int i = _SIZE - 1; i >= 0; --i)
        {
            const int idx = i - shift / 8;
            const byte right = idx >= 0
                            ? this->n[idx]
                            : 0;
            const byte left = idx > 0
                            ? this->n[idx - 1]
                            : 0;
            const short group = ((short)left << 8) | (short)right;
            result.n[i] = byte(group >> (shift % 8));
        }
        return result;
    }

    bigInt& operator>>=(uint shift) {
        *this = *this >> shift;

        return *this;
    }

    bigInt operator<<(size_t shift) const {
        bigInt<S> result;

        for (int i = _SIZE - 1; i >= 0; --i)
        {
            const int idx = i + shift / 8;
            const byte left = idx <= _SIZE - 1
                            ? this->n[idx]
                            : 0;
            const byte right = idx <= _SIZE - 2
                            ? this->n[idx + 1]
                            : 0;
            const short group = ((short)left << 8) | (short)right;
            result.n[i] = byte((group << (shift % 8)) >> 8);
        }
        return result;
    }

    bigInt& operator<<=(uint shift) {
        *this = *this << shift;

        return *this;
    }

    bigInt operator|(const bigInt& other) const {
        bigInt result;
        size_t idx = 0;

        for (byte b: n) { result[idx] = b | other[idx++]; }
        return result;
    }

    bigInt& operator|=(const bigInt& other) { return *this = *this | other; }

    bigInt operator&(const bigInt& other) const {
        bigInt result;
        size_t idx = 0;

        for (byte b: n) { result[idx] = b & other[idx++]; }
        return result;
    }

    bigInt& operator&=(const bigInt& other) { return *this = *this & other; }

    bigInt operator^(const bigInt& other) const {
        bigInt result;
        size_t idx = 0;

        for (byte b: n) { result[idx] = b ^ other[idx++]; }
        return result;
    }

    bigInt& operator^=(const bigInt& other) { return *this = *this ^ other; }

    bigInt operator~() const {
        bigInt result;
        size_t idx = 0;

        for (byte b: n) { result[idx++] = ~b; }
        return result;
    }

    // COMPARISONS

    template < size_t K >
    bool operator<(const bigInt<K>& other) const {
        return  _SIZE >= other.size() ?
                _isSmaller(*this, other):
                _isHigher(other, *this);
    }

    template < size_t K >
    bool operator>(const bigInt<K>& other) const {
        return other < *this;
    }

    template < size_t K >
    bool operator<=(const bigInt<K>& other) const {
        return !(*this > other);
    }

    template < size_t K >
    bool operator>=(const bigInt<K>& other) const {
        return !(*this < other);
    }

    template < size_t K >
    bool operator==(const bigInt<K>& other) const {
        return  _SIZE >= other.size() ?
                _isEqual(*this, other):
                _isEqual(other, *this);
    }

    template < size_t K >
    bool operator!=(const bigInt<K>& other) const {
        return !(*this == other);
    }

    // ACCESSORS

    byte operator[](size_t idx) const {
        if (idx >= _SIZE) {
            throw std::out_of_range("index out of range.");
        }
        return n[idx];
    }

    byte& operator[](size_t idx) {
        if (idx >= _SIZE) {
            throw std::out_of_range("index out of range.");
        }
        return n[idx];
    }

    int asInt(size_t idx) const {
        return this->operator[](idx);
    }

    bool getBitAt(size_t bitPos) const {
        return ((1 << bitPos % 8) & n[(_SIZE - 1) - bitPos / 8]) != 0;
    }

    // CONVERSIONS

    // Return an allocated array of boolean. Do not forget to free.
    bool* toBits() const {
        const size_t bitsSize = 8 * _SIZE;
        bool* result = new bool[bitsSize];

        for (int i = 0; i < bitsSize; ++i)
            result[i] = getBitAt(i);
        return result;
    }

    // DISPLAY / DEBUG

    std::string getBitsString() const {
        std::string bits;

        bits += "[ ";
        for (byte b: n)
            bits += _byteToBitsString(b) + ' ';
        bits += ']';
        return bits;
    }

    std::string getIntsString() const {        
        if (_SIZE != 0) {
            std::string ints;

            ints += '[';
            for (byte b: n) {
                ints += std::to_string(b);
                if (b < 100)
                ints += ' ';
                if (b < 10)
                ints += ' ';
                ints += ", ";
            }
            ints.erase(ints.size() - 2, 2);
            ints += ']';
            
            return ints;
        }
        else
            return "[]";
    }

    std::string toHex() const {
        std::string hex;
        size_t hexaBaseLen = strlen(hexaBase);

        for (byte b: n) {
            hex += hexaBase[(b /  hexaBaseLen) % hexaBaseLen];
            hex += hexaBase[b % hexaBaseLen];
        }
        return hex;
    }

    std::string toHexWith0x() const {
        return std::string("0x") + toHex();
    }

};

template < size_t S >
std::ostream& operator<<(std::ostream& o, const bigInt<S>& n) {
    o << '[';
    for (int i = 0; i < n.size() - 1; ++i)
        o << n.asInt(i) << ", ";
    o << n.asInt(n.size() - 1) << ']';

    return o;
};

#endif