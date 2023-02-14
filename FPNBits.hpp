#ifndef FPNBITS_HPP_
#define FPNBITS_HPP_
#include <utility>
// #include <cstdint>
namespace FPNBits {
    #define RNDBITS(__bits) (__bits + (__bits % 8 ? (8 - (__bits % 8)) : 0))
    #define BITINBYTE(bit) (char(1) << (bit))
    #define CHKBITINBYTE(byte, bit) ((*(byte)) & BITINBYTE(bit))
    #define SETBITINBYTE(byte, bit) ((*(byte)) |= BITINBYTE(bit))
    #define CLRBITINBYTE(byte, bit) ((*(byte)) &= ~BITINBYTE(bit))
    #define CPYBITTOBYTE(from, bitF, to, bitT) CHKBITINBYTE(from, bitF) ? SETBITINBYTE(to, bitT) : CLRBITINBYTE(to, bitT)
    template<int _Bits>
    class StaticBitArray {
        public:
        // static constexpr short Bits = _Bits;
        StaticBitArray& operator<<(const int shiftL) {
            for(int shift = 0; shift < shiftL; shift++) {
                bool carry = false;
                for(short byte = (sizeof(m_data) / sizeof(m_data[0])) - 1; byte < 0; byte--) {
                    m_data[byte] |= carry;
                    carry = CHKBITINBYTE(m_data[byte], 7);
                    m_data[byte] <<= 1;
                }
            }
            return *this;
        }
        
        StaticBitArray& operator>>(const int shiftR) {
            for(int shift = 0; shift < shiftR; shift++) {
                bool carry = false;
                for(short byte = 0; byte < (sizeof(m_data) / sizeof(m_data[0])); byte++) {
                    m_data[byte] |= carry;
                    carry = CHKBITINBYTE(m_data[byte], 0);
                    m_data[byte] >>= 1;
                }
            }
            return *this;
        }
        
        char m_data[RNDBITS(_Bits) / 8];
        
        private:
    };
    namespace __INTERNAL {
        #define ToIdx 0
        #define FromIdx 1
        inline void copy(char* From, const short& ExpBitsF, const short& ManBitsF, char* To, const short& ExpBitsT, const short& ManBitsT) {
            const int totalBitsT = RNDBITS(ExpBitsT + ManBitsT);
            const int totalBitsF = RNDBITS(ExpBitsF + ManBitsF);
            
            for(int bits[2] = { totalBitsT - ExpBitsT, totalBitsF - ExpBitsF}; bits[0] < totalBitsT; bits[ToIdx]++) {
                const short byteF = (totalBitsF / 8) - ((totalBitsF - (bits[FromIdx] % 8)) / 8);
                const short byteT = (totalBitsT / 8) - ((totalBitsT - (bits[ToIdx] % 8)) / 8);
                if(bits[FromIdx] < totalBitsF) {
                    CPYBITTOBYTE(From + (byteF), (bits[FromIdx] % 8), To + byteT, (bits[ToIdx] % 8)); // 7 - x ?
                    bits[FromIdx]++;
                }
                else {
                    CLRBITINBYTE(To + byteT, bits[ToIdx]);
                }
            }
            
            for(int bits[2] = { 0, 0 }; bits[0] < ManBitsT; bits[ToIdx]++) {
                const short byteF = (totalBitsF / 8) - ((totalBitsF - (bits[FromIdx] % 8)) / 8);
                const short byteT = (totalBitsT / 8) - ((totalBitsT - (bits[ToIdx] % 8)) / 8);
                if(bits[FromIdx] < ManBitsF) {
                    CPYBITTOBYTE(From + (byteF), (bits[FromIdx] % 8), To + byteT, (bits[ToIdx] % 8)); // 7 - x ?
                    bits[FromIdx]++;
                }
                else {
                    // CLRBITINBYTE(To + byteT, bits[ToIdx]);
                }
            }
        }
        inline void add(char* A, char* B, const short& ExpBitsA, const short& ManBits, const short& ExpBitsB, const short& ManBitsB, constexpr bool& Signed) {
            // const int totalBitsA = ExpBitsA + ManBitsA + (8 - ((ExpBitsA + ManBitsA) % 8));
            // const int totalBitsB = ExpBitsB + ManBitsB + (8 - ((ExpBitsB + ManBitsB) % 8));
            
            // for(int bits[2] = { totalBitsA - ExpBitsA, totalBitsB - ExpBitsB}; bits[0] < totalBitsA; bits[ToIdx]++) {
            //     const short byteB = (totalBitsB / 8) - ((totalBitsB - (bits[FromIdx] % 8)) / 8);
            //     const short byteA = (totalBitsA / 8) - ((totalBitsA - (bits[ToIdx] % 8)) / 8);
            //     if(bits[FromIdx] < totalBitsB) {
            //         CPYBITTOBYTE(B + (byteA), (bits[FromIdx] % 8), B + byteB, (bits[ToIdx] % 8)); // 7 - x ?
            //         bits[FromIdx]++;
            //     }
            //     else {
            //         CLRBITINBYTE(B + byteB, bits[FromIdx]);
            //     }
            // }
        }
        inline void subtract(char* A, const short& ExpBitsA, const short& ManBitsA, char* B, const short& ExpBitsB, const short& ManBitsB, constexpr bool& Signed) {
            
        }
        inline void multiply(char* A, const short& ExpBitsA, const short& ManBitsA, char* B, const short& ExpBitsB, const short& ManBitsB, constexpr bool& Signed) {
            
        }
        inline void divide(char* A, const short& ExpBitsA, const short& ManBitsA, char* B, const short& ExpBitsB, const short& ManBitsB, constexpr bool& Signed) {
            
        }
    }
    
    template<short _ExpBits, short _ManBits, bool _Signed>
    class StaticFloat : private StaticBitArray<_ExpBits + _ManBits> {
        public:
        static constexpr short ExpBits = _ExpBits;
        static constexpr short ManBits = _ManBits;
        static constexpr bool Signed = _Signed;
        StaticFloat() :
            StaticBitArray<_ExpBits + _ManBits>::m_data({0}) {}

        template<>
        StaticFloat<ExpBits, ManBits, !Signed>() {
            StaticFloat<ExpBits, ManBits, !Signed> rtn;
            rtn.m_data = this->m_data;
            if constexpr(Signed) rtn.m_data[0] &= ~(char(1) << 7);
            return std::move(rtn);
        }
        
        template<short ExpBitsO, short ManBitsO, bool SignedO>
        StaticFloat<ExpBits, ManBits, Signed>(StaticFloat<ExpBitsO, ManBitsO, SignedO>&& other) {
            if constexpr((ExpBits == ExpBitsO) && (ManBits == ManBitsO)) {
                this->m_data = std::move(other.m_data);
            }
            else {
                __INTERNAL::copy(other.m_data, ExpBitsO, ManBitsO, this->m_data, ExpBits, ManBits);
            }
            if constexpr(!Signed && SignedO) this->m_data[0] &= ~(char(1) << 7);
        }
        template<short ExpBitsO, short ManBitsO, bool SignedO>
        StaticFloat<ExpBits, ManBits, Signed>& operator=(StaticFloat<ExpBitsO, ManBitsO, SignedO>&& other) {
            if constexpr((ExpBits == ExpBitsO) && (ManBits == ManBitsO)) {
                this->m_data = std::move(other.m_data);
            }
            else {
                __INTERNAL::copy(other.m_data, ExpBitsO, ManBitsO, this->m_data, ExpBits, ManBits);
            }
            if constexpr(!Signed && SignedO) this->m_data[0] &= ~(char(1) << 7);
            return *this;
        }
        template<short ExpBitsO, short ManBitsO, bool SignedO>
        StaticFloat<ExpBits, ManBits, Signed>(const StaticFloat<ExpBitsO, ManBitsO, SignedO>& other)  {
            if constexpr((ExpBits == ExpBitsO) && (ManBits == ManBitsO)) {
                this->m_data = std::move(other.m_data);
            }
            else {
                __INTERNAL::copy(other.m_data, ExpBitsO, ManBitsO, this->m_data, ExpBits, ManBits);
            }
            if constexpr(!Signed && SignedO) this->m_data[0] &= ~(char(1) << 7);
        }
        template<short ExpBitsO, short ManBitsO, bool SignedO>
        StaticFloat<ExpBits, ManBits, Signed>& operator=(const StaticFloat<ExpBitsO, ManBitsO, SignedO>& other)  {
            if constexpr((ExpBits == ExpBitsO) && (ManBits == ManBitsO)) {
                this->m_data = std::move(other.m_data);
            }
            else {
                __INTERNAL::copy(other.m_data, ExpBitsO, ManBitsO, this->m_data, ExpBits, ManBits);
            }
            return *this;
        }
        
        private:
        friend class DynamicFloat;
        // char m_data[((ExpBits + ManBits + (8 - ((ExpBits + ManBits) % 8))) / 8)];  
        
        public:
          
    };
    class DynamicFloat {
        private:
        short ExpBits = 0;
        short ManBits = 0;
        bool Signed = true;
        char* m_data = nullptr;
        
        public:
        
    };
    typedef FPNBits::StaticFloat<9, 23, false> float32u;
    typedef FPNBits::StaticFloat<9, 23, true> float32s;
    typedef FPNBits::StaticFloat<12, 52, false> float64u;
    typedef FPNBits::StaticFloat<12, 52, true> float64s;
    typedef FPNBits::StaticFloat<16, 112, false> float128u;
    typedef FPNBits::StaticFloat<16, 112, true> float128s;
}
#endif