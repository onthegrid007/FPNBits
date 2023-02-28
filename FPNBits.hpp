#ifndef FPNBITS_HPP_
#define FPNBITS_HPP_
#include <utility>
#include <string>

namespace FPNBits {
    #define DEADBITS(bits__) ((((bits__) % 8) > 0) ? (8 - ((bits__) % 8)) : 0)
    #define RNDBITS(__bits) ((__bits) + DEADBITS(__bits))
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
    
    // template<short BYTES>
    // struct ChrAry {
    //     unsigned char m_data[BYTES];
    // };
    
    template<short _ExpBits, short _ManBits, bool _Signed>
    class StaticFloat {
        public:
        static constexpr short ExpBits = _ExpBits;
        static constexpr short ManBits = _ManBits;
        static constexpr bool Signed = _Signed;
        static constexpr char MinSupportedManAccessBits = 4;
        static constexpr short MaxSupportedManAccessBits = (sizeof(__int128) * 8);
        static constexpr char MinSupportedExpAccessBits = 4;
        static constexpr short MaxSupportedExpAccessBits = MaxSupportedManAccessBits - (Signed ? 1 : 0);
        static constexpr char DeadBits = DEADBITS(ExpBits + ManBits);
        static constexpr bool HasDeadBits = (DeadBits > 0);
        static constexpr bool ExpBitsInRange = ((ExpBits >= MinSupportedExpAccessBits) && ((ExpBits - (Signed ? 1 : 0)) <= MaxSupportedExpAccessBits));
        static constexpr bool ManBitsInRange = ((ManBits >= MinSupportedManAccessBits) && (ManBits <= MaxSupportedManAccessBits));
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
            short bitIdx = ((TotalBytes * 8) - 1);

            while(bitIdx > 0) {
                if(other & (typename StaticFloat<ExpBits, ManBits, Signed>::ManAccessType(1) << bitIdx)) {
                    if(bitIdx >= ManBits) {
                        other >>= (bitIdx - ManBits);
                    } else {
                        other <<= (ManBits - bitIdx);
                    }
                    break;
                }
                bitIdx--;
            }
            
            m_IEEE754.Exp = (bitIdx + (StaticFloat<ExpBits, ManBits, Signed>::ExpMax >> 1));
            m_IEEE754.Man = (other & ~(typename StaticFloat<ExpBits, ManBits, Signed>::ManAccessType(1) << ManBits));
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

        static constexpr typename StaticFloat<ExpBits, ManBits, Signed>::ExpAccessType ExpMax = ((((typename StaticFloat<ExpBits, ManBits, Signed>::ExpAccessType(1) << (ExpBits - (Signed ? 2 : 1))) - 1) << 1) + 1);
        static constexpr typename StaticFloat<ExpBits, ManBits, Signed>::ManAccessType ManMax = ((((typename StaticFloat<ExpBits, ManBits, Signed>::ManAccessType(1) << (ManBits - 1)) - 1) << 1) + 1);
        private:
        typedef struct {
            ManAccessType Man : ManBits;
            ExpAccessType Exp : ExpBits - 1;
            bool Sign : 1;
        } _S;
        typedef struct {
            ManAccessType Man : ManBits;
            const char Padding : DeadBits;
            ExpAccessType Exp : ExpBits - 1;
            bool Sign : 1;
        } _SwD;
        typedef struct {
            ManAccessType Man : ManBits;
            ExpAccessType Exp : ExpBits;
        } _U;
        typedef struct {
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
        
        explicit StaticFloat(BitManipType&& IEEE754) : m_IEEE754(IEEE754) {}
        
        template<bool Signed_ = Signed>
        static const StaticFloat<ExpBits, ManBits, Signed_> _PosInf;
        template<>
        inline static const StaticFloat<ExpBits, ManBits, true> _PosInf<true> {{
            .Sign = false,
            .Exp = StaticFloat<ExpBits, ManBits, true>::ExpMax,
            .Man = 0
        }};
        template<>
        inline static const StaticFloat<ExpBits, ManBits, false> _PosInf<false> {{
            .Exp = StaticFloat<ExpBits, ManBits, false>::ExpMax,
            .Man = 0
        }};
        inline static const auto& PosInf = _PosInf<Signed>;
        
        template<bool Signed_ = Signed>
        inline static const StaticFloat<ExpBits, ManBits, Signed_> PosZero;
        template<>
        inline static const StaticFloat<ExpBits, ManBits, true> PosZero<true> {{
            .Sign = false,
            .Exp = 0,
            .Man = 0
        }};
        template<>
        inline static const StaticFloat<ExpBits, ManBits, false> PosZero<false> {{
            .Exp = 0,
            .Man = 0
        }};
        // inline static const auto& NegZero = _NegZero<Signed>;
        
        template<bool Signed_ = Signed>
        static const StaticFloat<ExpBits, ManBits, Signed_> PosOne;
        template<>
        inline static const StaticFloat<ExpBits, ManBits, true> PosOne<true> {{
            .Sign = false,
            .Exp = StaticFloat<ExpBits, ManBits, true>::ExpMax >> 1,
            .Man = 0
        }};
        template<>
        inline static const StaticFloat<ExpBits, ManBits, false> PosOne<false> {{
            .Exp = StaticFloat<ExpBits, ManBits, false>::ExpMax >> 1,
            .Man = 0
        }};
        // inline static const auto& NegZero = _NegZero<Signed>;
        
        template<bool Signed_ = Signed>
        static const StaticFloat<ExpBits, ManBits, Signed_> Max;
        template<>
        inline static const StaticFloat<ExpBits, ManBits, true> Max<true> {{
            .Sign = false,
            .Exp = StaticFloat<ExpBits, ManBits, true>::ExpMax,
            .Man = StaticFloat<ExpBits, ManBits, true>::ManMax
        }};
        template<>
        inline static const StaticFloat<ExpBits, ManBits, false> Max<false> {{
            .Exp = StaticFloat<ExpBits, ManBits, false>::ExpMax,
            .Man = StaticFloat<ExpBits, ManBits, false>::ManMax
        }};
        // inline static const auto& NegZero = _NegZero<Signed>;
        
        template<bool Signed_ = Signed>
        static const StaticFloat<ExpBits, ManBits, Signed_> Min;
        template<>
        inline static const StaticFloat<ExpBits, ManBits, true> Min<true> {{
            .Sign = true,
            .Exp = StaticFloat<ExpBits, ManBits, true>::ExpMax,
            .Man = StaticFloat<ExpBits, ManBits, true>::ManMax
        }};
        template<>
        inline static const StaticFloat<ExpBits, ManBits, false> Min<false> {{
            .Exp = 0,
            .Man = 0
        }};
        // inline static const auto& NegZero = _NegZero<Signed>;
        
        template<bool Signed_ = Signed>
        static const StaticFloat<ExpBits, ManBits, Signed_> NegInf;
        template<>
        inline static const StaticFloat<ExpBits, ManBits, true> NegInf<true> {{
            .Sign = true,
            .Exp = StaticFloat<ExpBits, ManBits, true>::ExpMax,
            .Man = 0
        }};
        template<>
        inline static const StaticFloat<ExpBits, ManBits, false> NegInf<false> {{
            .Exp = 0,
            .Man = 0
        }}; 
        // inline static const auto& NegZero = _NegZero<Signed>;
        
        template<bool Signed_ = Signed>
        static const StaticFloat<ExpBits, ManBits, Signed_> _NegZero;
        template<>
        inline static const StaticFloat<ExpBits, ManBits, true> _NegZero<true> {{
            .Sign = true,
            .Exp = 0,
            .Man = 0
        }};
        template<>
        inline static const StaticFloat<ExpBits, ManBits, false> _NegZero<false> {{
            .Exp = 0,
            .Man = 0
        }};
        inline static const auto& NegZero = _NegZero<Signed>;
        
        template<bool Signed_ = Signed>
        static const StaticFloat<ExpBits, ManBits, Signed_> _NegOne;
        template<>
        inline static const StaticFloat<ExpBits, ManBits, true> _NegOne<true> {{
            .Sign = true,
            .Exp = (StaticFloat<ExpBits, ManBits, true>::ExpMax >> 1),
            .Man = 0
        }};
        template<>
        inline static const StaticFloat<ExpBits, ManBits, false> _NegOne<false> {{
            .Exp = 0,
            .Man = 0
        }};
        inline static const auto& NegOne = _NegOne<Signed>;
        
        inline static const StaticFloat<ExpBits, ManBits, Signed> Lowest {{
            .Exp = 0,
            .Man = 1
        }};
        // inline static const auto& Lowest = _Lowest<Signed>;

        inline static const StaticFloat<ExpBits, ManBits, Signed> qNaN {{
            .Exp = 0,
            .Man = (typename StaticFloat<ExpBits, ManBits, Signed>::ManAccessType(1) << (ManBits - 1))
        }};
        
        inline static const StaticFloat<ExpBits, ManBits, Signed> sNaN {{
            .Exp = 0,
            .Man = (StaticFloat<ExpBits, ManBits, Signed>::ManMax >> 1)
        }};

        StaticFloat(int other = 0) : m_data() {
            if(other == 0) return;
            assign(other);
        }

        std::string RawBits() {
            std::string rtn;
            for(unsigned char* a = (this->m_data + TotalBytes - 1); a >= this->m_data; a--) { //const unsigned char& c : this->m_data) {
                for(unsigned char b = 8; b > 0; b--) {
                    rtn.push_back(CHKBITINBYTE(a, b - 1) ? '1' : '0');
                }
            }
            return rtn;
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
        
        static constexpr bool isSigned() { return Signed; };
        
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

typedef FPNBits::StaticFloat<128, 128, false> float128_max_u;
typedef FPNBits::StaticFloat<128, 128, true> float128_max_s;
typedef float128_max_s float128_max;

#endif