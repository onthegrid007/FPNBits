#ifndef FPNBITS_HPP_
#define FPNBITS_HPP_
#include <utility>
#include <string>

namespace FPNBits {
    #define RNDBITS(__bits) ((__bits) + ((((__bits) % 8) > 0) ? (8 - ((__bits) % 8)) : 0))
    #define BITINBYTE(bit) (char(1) << (bit))
    #define CHKBITINBYTE(byte, bit) ((*(byte)) & BITINBYTE(bit))
    #define SETBITINBYTE(byte, bit) ((*(byte)) |= BITINBYTE(bit))
    #define CLRBITINBYTE(byte, bit) ((*(byte)) &= ~BITINBYTE(bit))
    #define CPYBITTOBYTE(from, bitF, to, bitT) CHKBITINBYTE(from, bitF) ? SETBITINBYTE(to, bitT) : CLRBITINBYTE(to, bitT)
    // template<int _Bits>
    // class StaticBitArray {
    //     public:
    //     // static constexpr short Bits = _Bits;
    //     StaticBitArray& operator<<(const int shiftL) {
    //     //     for(int shift = 0; shift < shiftL; shift++) {
    //     //         bool carry = false;
    //     //         for(short byte = (sizeof(m_data) / sizeof(m_data[0])) - 1; byte > 0; byte--) {
    //     //             m_data[byte] |= carry;
    //     //             carry = CHKBITINBYTE(m_data[byte], 7);
    //     //             m_data[byte] <<= 1;
    //     //         }
    //     //     }
    //         return *this;
    //     }
        
    //     StaticBitArray& operator>>(const int shiftR) {
    //         // for(int shift = 0; shift < shiftR; shift++) {
    //         //     bool carry = false;
    //         //     for(short byte = 0; byte < (sizeof(m_data) / sizeof(m_data[0])); byte++) {
    //         //         m_data[byte] |= (char(carry) << 7);
    //         //         carry = CHKBITINBYTE(m_data[byte], 0);
    //         //         m_data[byte] >>= 1;
    //         //     }
    //         // }
    //         return *this;
    //     }
    
    namespace __INTERNAL {
        #define ToIdx 0
        #define FromIdx 1
        inline void copy(char* From, const short& ExpBitsF, const short& ManBitsF, char* To, const short& ExpBitsT, const short& ManBitsT) {
            const int totalBitsT = RNDBITS(ExpBitsT + ManBitsT);
            const int totalBitsF = RNDBITS(ExpBitsF + ManBitsF);
            
            for(int bits[2] = {totalBitsT - ExpBitsT - 1, totalBitsF - ExpBitsF - 1}; bits[0] < totalBitsT; bits[ToIdx]++) {
                // const short byteF = (totalBitsF / 8) - ((totalBitsF - (bits[FromIdx] % 8)) / 8); // refactor
                const short byteT = (totalBitsT / 8) - ((totalBitsT - (bits[ToIdx] % 8)) / 8);
                if(bits[FromIdx] < totalBitsF) {
                    CPYBITTOBYTE(From + ((totalBitsF / 8) - ((totalBitsF - (bits[FromIdx] % 8)) / 8)), (bits[FromIdx] % 8), To + byteT, (bits[ToIdx] % 8)); // 7 - x ?
                    bits[FromIdx]++;
                }
                else {
                    CLRBITINBYTE(To + byteT, bits[ToIdx]);
                }
            }
            
            for(int bits[2] = { 0, 0 }; bits[0] < ManBitsT; bits[ToIdx]++) {
                // const short byteF = (totalBitsF / 8) - ((totalBitsF - (bits[FromIdx] % 8)) / 8); // refactor
                const short byteT = (totalBitsT / 8) - ((totalBitsT - (bits[ToIdx] % 8)) / 8);
                if(bits[FromIdx] < ManBitsF) {
                    CPYBITTOBYTE(From + ((totalBitsF / 8) - ((totalBitsF - (bits[FromIdx] % 8)) / 8)), (bits[FromIdx] % 8), To + byteT, (bits[ToIdx] % 8)); // 7 - x ?
                    bits[FromIdx]++;
                }
                else {
                    CLRBITINBYTE(To + byteT, bits[ToIdx]);
                }
            }
        }
    }
    
    template<short BYTES>
    struct ChrAry {
        unsigned char m_data[BYTES];
    };
    
    template<short _ExpBits, short _ManBits, bool _Signed>
    class StaticFloat {
        public:
        static constexpr short ExpBits = _ExpBits;
        static constexpr short ManBits = _ManBits;
        static constexpr bool Signed = _Signed;
        static constexpr short MaxSupportedManAccessBits = (sizeof(__int128) * 8);
        static constexpr short MaxSupportedExpAccessBits = MaxSupportedManAccessBits - (Signed ? 1 : 0);
        static constexpr char DeadBits = ((ExpBits + ManBits) % 8);
        static constexpr bool HasDeadBits = (DeadBits > 0);
        static constexpr bool ExpBitsInRange = ((ExpBits - (Signed ? 1 : 0)) <= MaxSupportedExpAccessBits);
        static constexpr bool ManBitsInRange = (ManBits <= MaxSupportedManAccessBits);
        static constexpr typename std::enable_if<ExpBitsInRange && ManBitsInRange, short>::type TotalBytes = (RNDBITS(ExpBits + ManBits) / 8);
        typedef StaticFloat<ExpBits, ManBits, !Signed> InvtType;
        
        private:
        static constexpr bool E8 = (RNDBITS(ExpBits) / 8) <= 1;
        static constexpr bool E16 = (RNDBITS(ExpBits) / 8) <= 2;
        static constexpr bool E32 = (RNDBITS(ExpBits) / 8) <= 4;
        static constexpr bool E64 = (RNDBITS(ExpBits) / 8) <= 8;
        static constexpr bool E128 = (RNDBITS(ExpBits) / 8) <= 16;
        static constexpr bool M8 = (RNDBITS(ManBits) / 8) <= 1;
        static constexpr bool M16 = (RNDBITS(ManBits) / 8) <= 2;
        static constexpr bool M32 = (RNDBITS(ManBits) / 8) <= 4;
        static constexpr bool M64 = (RNDBITS(ManBits) / 8) <= 8;
        static constexpr bool M128 = (RNDBITS(ManBits) / 8) <= 16;

        template<typename T>
        void assign(T other) {
            if constexpr(Signed) if((m_IEEE754.Sign = (other < 0))) other = -other;
            short bitIdx = (TotalBytes * 8) - 1;

            while(bitIdx > 0) {
                if(other & (ManAccessType(1) << bitIdx)) {
                    if(bitIdx >= ManBits) {
                        other >>= (bitIdx - ManBits);
                    } else {
                        other <<= (ManBits - bitIdx);
                    }
                    break;
                }
                bitIdx--;
            }
            
            m_IEEE754.Exp = bitIdx + (StaticFloat<ExpBits, ManBits, true>::ExpMax >> 1);
            m_IEEE754.Man = other & ~(ManAccessType(1) << ManBits);
        }
        
        public:
        typedef
            typename std::enable_if<E128,
                typename std::conditional<E8, unsigned char, 
                    typename std::conditional<E16, unsigned short,
                        typename std::conditional<E32, unsigned int,
                            typename std::conditional<E64, unsigned long int, unsigned __int128>::type>::type>::type>::type>::type ExpAccessType;

        typedef
            typename std::enable_if<M128, 
                typename std::conditional<M8, unsigned char, 
                    typename std::conditional<M16, unsigned short,
                        typename std::conditional<M32, unsigned int,
                            typename std::conditional<M64, unsigned long int, unsigned __int128>::type>::type>::type>::type>::type ManAccessType;

        static constexpr typename StaticFloat<ExpBits, ManBits, Signed>::ExpAccessType ExpMax = ((((StaticFloat<ExpBits, ManBits, Signed>::ExpAccessType(1) << (ExpBits - (Signed ? 2 : 1))) - 1) << 1) + 1);
        static constexpr typename StaticFloat<ExpBits, ManBits, Signed>::ManAccessType ManMax = ((((StaticFloat<ExpBits, ManBits, Signed>::ManAccessType(1) << (ManBits - 1)) - 1) << 1) + 1);
        
        static const StaticFloat<ExpBits, ManBits, true> PosInf {
            .m_IEEE754.Sign = false,
            .m_IEEE754.Exp = StaticFloat<ExpBits, ManBits, true>::ExpMax,
            .m_IEEE754.Man = 0
        };
        
        static const StaticFloat<ExpBits, ManBits, true> PosZero {
            .m_IEEE754.Sign = false,
            .m_IEEE754.Exp = 0,
            .m_IEEE754.Man = 0
        };
        
        static const StaticFloat<ExpBits, ManBits, true> PosOne {
            .m_IEEE754.Sign = false,
            .m_IEEE754.Exp = StaticFloat<ExpBits, ManBits, true>::ExpMax >> 1,
            .m_IEEE754.Man = 0
        };

        static const StaticFloat<ExpBits, ManBits, true> Max {
            .m_IEEE754.Sign = false,
            .m_IEEE754.Exp = StaticFloat<ExpBits, ManBits, true>::ExpMax,
            .m_IEEE754.Man = StaticFloat<ExpBits, ManBits, true>::ManMax
        };
        
        static const StaticFloat<ExpBits, ManBits, true> Min {
            .m_IEEE754.Sign = true,
            .m_IEEE754.Exp = StaticFloat<ExpBits, ManBits, true>::ExpMax,
            .m_IEEE754.Man = StaticFloat<ExpBits, ManBits, true>::ManMax 
        };
        
        static const StaticFloat<ExpBits, ManBits, true> NegInf {
            .m_IEEE754.Sign = true,
            .m_IEEE754.Exp = StaticFloat<ExpBits, ManBits, true>::ExpMax,
            .m_IEEE754.Man = 0
        };
        
        static const StaticFloat<ExpBits, ManBits, true> NegZero {
            .m_IEEE754.Sign = true,
            .m_IEEE754.Exp = 0,
            .m_IEEE754.Man = 0
        };

        static const StaticFloat<ExpBits, ManBits, true> NegOne {
            .m_IEEE754.Sign = true,
            .m_IEEE754.Exp = (StaticFloat<ExpBits, ManBits, true>::ExpMax >> 1),
            .m_IEEE754.Man = 0
        };
        
        static const StaticFloat<ExpBits, ManBits, Signed> Lowest {
            .m_IEEE754.Exp = 0,
            .m_IEEE754.Man = 1
        };

        static const StaticFloat<ExpBits, ManBits, Signed> qNaN {
            .m_IEEE754.Exp = 0,
            .m_IEEE754.Man = (StaticFloat<ExpBits, ManBits, Signed>::ManAccessType(1) << (ManBits - 1))
        };
        
        static const StaticFloat<ExpBits, ManBits, Signed> sNaN {
            .m_IEEE754.Exp = 0,
            .m_IEEE754.Man = (StaticFloat<ExpBits, ManBits, true>::ManMax >> 1)
        };
        
        static const StaticFloat<ExpBits, ManBits, false> PosInf { StaticFloat<ExpBits, ManBits, true>::NegInf };
        static const StaticFloat<ExpBits, ManBits, false> PosZero { StaticFloat<ExpBits, ManBits, true>::PosZero };
        static const StaticFloat<ExpBits, ManBits, false> PosOne { StaticFloat<ExpBits, ManBits, true>::PosOne };
        static const StaticFloat<ExpBits, ManBits, false> Max { StaticFloat<ExpBits, ManBits, true>::Min };
        static const StaticFloat<ExpBits, ManBits, false> Min { StaticFloat<ExpBits, ManBits, true>::PosZero };
        static const StaticFloat<ExpBits, ManBits, false> NegInf { StaticFloat<ExpBits, ManBits, true>::PosZero };
        static const StaticFloat<ExpBits, ManBits, false> NegZero { StaticFloat<ExpBits, ManBits, true>::PosZero };
        static const StaticFloat<ExpBits, ManBits, false> NegOne { StaticFloat<ExpBits, ManBits, true>::PosZero };

        StaticFloat(int other = 0) : m_data() {
            if(other == 0) return;
            assign(other);
        }

        std::string&& RawBits() {
            std::string rtn;
            for(unsigned char* a = this->m_data + TotalBytes - 1; a >= this->m_data; a--) { //const unsigned char& c : this->m_data) {
                for(signed short b = 8; b >= 0; rtn.push_back(CHKBITINBYTE(a, b-=1) ? '1' : '0'));
            }
            return std::move(rtn);
        }
        
        operator std::string() {
            std::string rtn;
            if constexpr(Signed) CHKBITINBYTE(this->m_data, 7) ? rtn.push_back('-') : 0;
            return rtn;
        }

        operator InvtType() {
            InvtType rtn;
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
        typedef struct {
            ManAccessType Man : ManBits;
            ExpAccessType Exp : ExpBits - 1;
            bool Sign : 1;
        } _S;
        typedef struct SwD_ : ChrAry<TotalBytes> {
            ManAccessType Man : ManBits;
            const char Padding : DeadBits;
            ExpAccessType Exp : ExpBits - 1;
            bool Sign : 1;
        } _SwD;
        typedef struct {
            ManAccessType Man : ManBits;
            ExpAccessType Exp : ExpBits;
        } _U;
        typedef struct _UwD_ : ChrAry<TotalBytes> {
            ManAccessType Man : ManBits;
            const char Padding : DeadBits;
            ExpAccessType Exp : ExpBits;
        } _UwD;

        public:
        typedef typename std::conditional<Signed,
            typename std::conditional<HasDeadBits, _SwD, _S>::type,
            typename std::conditional<HasDeadBits, _UwD, _U>::type>::type BitManipType;

        private:
        // typedef struct _B : BitManipType {
        //         bool Bit[TotalBytes * 8] : TotalBytes * 8;
        // } BitBoolAccessType;

        public:
        union {
            unsigned char m_data[TotalBytes];
            BitManipType m_IEEE754;
            // BitBoolAccessType m_bitBool;
        };
    };
}

typedef FPNBits::StaticFloat<4, 4, false> float8_u;
typedef FPNBits::StaticFloat<4, 4, true> float8_s;
typedef float8_s float8;

typedef FPNBits::StaticFloat<6, 10, false> float16_u;
typedef FPNBits::StaticFloat<6, 10, true> float16_s;
typedef float16_s float16;

typedef FPNBits::StaticFloat<9, 23, false> float32_u;
typedef FPNBits::StaticFloat<9, 23, true> float32_s;
typedef float32_s float32;

typedef FPNBits::StaticFloat<12, 52, false> float64_u;
typedef FPNBits::StaticFloat<12, 52, true> float64_s;
typedef float64_s float64;

typedef FPNBits::StaticFloat<16, 112, false> float128_u;
typedef FPNBits::StaticFloat<16, 112, true> float128_s;
typedef float128_s float128;

#endif