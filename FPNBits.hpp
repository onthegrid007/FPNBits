#ifndef FPNBITS_HPP_
#define FPNBITS_HPP_
#include <utility>
#include <string>
#include <atomic>
#include <functional>

template<typename T>
inline constexpr T PowerUpVal(const T v1, const T v2) {
    return v1 * v2;
}

template<typename T, short N, T... Vals>
inline constexpr typename std::enable_if<(N == sizeof...(Vals)), std::array<T, N>>::type MakePowerUpArr() {
    return std::array<T, N>{{Vals...}};
}

template<typename T, short N, T... Vals>
inline constexpr typename std::enable_if<(N != sizeof...(Vals)), std::array<T, N>>::type MakePowerUpArr() {
    return MakePowerUpArr<T, N, Vals..., PowerUpVal(*(std::end({Vals...})), *(std::end({Vals...})))>();
}

namespace FPNBits {
    #define DEADBITS(bits__) ((((bits__) % 8) > 0) ? (8 - ((bits__) % 8)) : 0)
    #define RNDBITS(__bits) ((__bits) + DEADBITS(__bits))
    #define BITINBYTE(bit) (char(1) << (bit))
    #define CHKBITINBYTE(byte, bit) ((*(byte)) & BITINBYTE(bit))
    #define SETBITINBYTE(byte, bit) ((*(byte)) |= BITINBYTE(bit))
    #define CLRBITINBYTE(byte, bit) ((*(byte)) &= ~BITINBYTE(bit))
    #define CPYBITTOBYTE(from, bitF, to, bitT) CHKBITINBYTE(from, bitF) ? SETBITINBYTE(to, bitT) : CLRBITINBYTE(to, bitT)
    
    namespace __Internal {
        template<typename T>
        std::string to_str(const T& f, unsigned short Places = 16);
    };
    
    template<short _ExpBits, short _ManBits, bool _Signed>
    class StaticFloat {
        public:
        typedef StaticFloat<_ExpBits, _ManBits, _Signed> Type;
        typedef StaticFloat<_ExpBits, _ManBits, !_Signed> InvtType;
        typedef __int128 MaxAccessType;
        static constexpr char MinSupportedManAccessBits = 4;
        static constexpr short MaxSupportedManAccessBits = (sizeof(MaxAccessType) * 8);
        static constexpr char MinSupportedExpAccessBits = 4;
        static constexpr short MaxSupportedExpAccessBits = MaxSupportedManAccessBits - (_Signed ? 1 : 0);
        static constexpr char DeadBits = DEADBITS(_ExpBits + _ManBits);
        static constexpr bool HasDeadBits = (DeadBits > 0);
        static constexpr bool ExpBitsInRange = ((_ExpBits >= MinSupportedExpAccessBits) && ((_ExpBits - (_Signed ? 1 : 0)) <= MaxSupportedExpAccessBits));
        static constexpr bool ManBitsInRange = ((_ManBits >= MinSupportedManAccessBits) && (_ManBits <= MaxSupportedManAccessBits));
        static constexpr typename std::enable_if<ExpBitsInRange && ManBitsInRange, short>::type TotalBytes = (RNDBITS(_ExpBits + _ManBits) / 8);
        // static std::atomic<short> DigitPlaces;
        
        private:
        static constexpr bool E8 = (RNDBITS(_ExpBits) / 8) <= 1;
        static constexpr bool E16 = (RNDBITS(_ExpBits) / 8) <= 2;
        static constexpr bool E32 = (RNDBITS(_ExpBits) / 8) <= 4;
        static constexpr bool E64 = (RNDBITS(_ExpBits) / 8) <= 8;
        static constexpr bool E128 = (RNDBITS(_ExpBits) / 8) <= 16;
        static constexpr bool M8 = (RNDBITS(_ManBits) / 8) <= 1;
        static constexpr bool M16 = (RNDBITS(_ManBits) / 8) <= 2;
        static constexpr bool M32 = (RNDBITS(_ManBits) / 8) <= 4;
        static constexpr bool M64 = (RNDBITS(_ManBits) / 8) <= 8;
        static constexpr bool M128 = (RNDBITS(_ManBits) / 8) <= 16;
        
        // template<typename T>
        // constexpr void assign(T other) {
        //     if constexpr(_Signed) {
        //         if((m_IEEE754.Sign = (other < 0))) other = -other;
        //     }
        //     else {
        //         if(other < 0) return;
        //     }
        //     short bitIdx = ((TotalBytes * 8) - 1);
        //     while(bitIdx > 0) {
        //         if(other & (typename Type::ManAccessType(1) << bitIdx)) {
        //             if(bitIdx >= _ManBits) {
        //                 other >>= (bitIdx - _ManBits);
        //             } else {
        //                 other <<= (_ManBits - bitIdx);
        //             }
        //             break;
        //         }
        //         bitIdx--;
        //     }
            
        //     m_IEEE754.Exp = (bitIdx + (Type::ExpMax >> 1));
        //     m_IEEE754.Man = (other & ~(typename Type::ManAccessType(1) << _ManBits));
        // }
        
        public:
        typedef typename std::enable_if<E128,
                    typename std::conditional<E8, unsigned char, 
                        typename std::conditional<E16, unsigned short,
                            typename std::conditional<E32, unsigned int,
                                typename std::conditional<E64, unsigned long int, unsigned __int128>::type>::type>::type>::type>::type ExpAccessType;

        typedef typename std::enable_if<M128, 
                    typename std::conditional<M8, unsigned char, 
                        typename std::conditional<M16, unsigned short,
                            typename std::conditional<M32, unsigned int,
                                typename std::conditional<M64, unsigned long int, unsigned __int128>::type>::type>::type>::type>::type ManAccessType;
        
        static constexpr typename Type::ExpAccessType ExpBias = ((typename Type::ExpAccessType(1) << (_ExpBits - (_Signed ? 2 : 1))) - 1);
        static constexpr typename Type::ExpAccessType ExpMax = ((ExpBias << 1) + 1);
        static constexpr typename Type::ManAccessType ManBias = ((typename Type::ManAccessType(1) << (_ManBits - 1)) - 1);
        static constexpr typename Type::ManAccessType ManMax = ((ManBias << 1) + 1);
        
        public:
        typedef struct __S {
            constexpr __S(const bool _Neg = false, const ExpAccessType _Exp = 0, const ManAccessType _Man = 0) : Man(_Man), Exp(_Exp), Sign(_Neg) {}
            ManAccessType Man : _ManBits;
            ExpAccessType Exp : _ExpBits - 1;
            bool Sign : 1;
            static constexpr short ExpBits{_ExpBits};
            static constexpr short ManBits{_ManBits};
            static constexpr bool Signed{true};
        } _S;
        typedef struct __SwP {
            constexpr __SwP(const bool _Neg, const ExpAccessType _Exp, const ManAccessType _Man) : Man(_Man), Padding(0), Exp(_Exp), Sign(_Neg) {}
            ManAccessType Man : _ManBits;
            const char Padding : DeadBits;
            ExpAccessType Exp : _ExpBits - 1;
            bool Sign : 1;
            static constexpr short ExpBits{_ExpBits};
            static constexpr short ManBits{_ManBits};
            static constexpr bool Signed{true};
        } _SwP;
        typedef struct __U {
            constexpr __U(const ExpAccessType _Exp, const ManAccessType _Man) : Man(_Man), Exp(_Exp) {}
            ManAccessType Man : _ManBits;
            ExpAccessType Exp : _ExpBits;
            static constexpr short ExpBits{_ExpBits};
            static constexpr short ManBits{_ManBits};
            static constexpr bool Signed{false};
        } _U;
        typedef struct __UwP{
            constexpr __UwP(const ExpAccessType _Exp, const ManAccessType _Man) : Man(_Man), Padding(0), Exp(_Exp) {}
            ManAccessType Man : _ManBits;
            const char Padding : DeadBits;
            ExpAccessType Exp : _ExpBits;
            static constexpr short ExpBits{_ExpBits};
            static constexpr short ManBits{_ManBits};
            static constexpr bool Signed{false};
        } _UwP;
        
        typedef typename std::conditional<_Signed,
            typename std::conditional<HasDeadBits, _SwP, _S>::type,
            typename std::conditional<HasDeadBits, _UwP, _U>::type>::type BitManipType;
        
        // private:
        // typedef struct _B : BitManipType {
        //         bool Bit[TotalBytes * 8] : TotalBytes * 8;
        // } BitBoolAccessType;
        
        public:
        union {
            unsigned char m_data[TotalBytes];
            BitManipType m_IEEE754;
            // BitBoolAccessType m_bitBool;
        };
        
        friend std::string __Internal::to_str<Type>(const Type&, unsigned short);
        
        static constexpr bool isSigned() { return _Signed; };
        
        constexpr operator const InvtType() const {
            InvtType rtn;
            rtn.m_data = this->m_data;
            if constexpr(_Signed) rtn.m_data[0] &= ~(char(1) << 7);
            return rtn;
        }
        
        constexpr StaticFloat(const Type::BitManipType other) : m_IEEE754(other) {}        
        
        constexpr StaticFloat(const int& other) : m_data() {
            if(other ==  0) return;
            ManAccessType o;
            if constexpr(_Signed) {
                o = ((m_IEEE754.Sign = (other < 0)) ? -other : other);
            }
            else {
                if(other < 0) return;
                o = other;
            }
            short bitIdx = ((TotalBytes * 8) - 1);
            while(bitIdx > 0) {
                if(o & (ManAccessType(1) << bitIdx)) {
                    if(bitIdx >= _ManBits) {
                        o >>= (bitIdx - _ManBits);
                    } else {
                        o <<= (_ManBits - bitIdx);
                    }
                    break;
                }
                bitIdx--;
            }
            // if constexpr(_Signed) {
            //     this->StaticFloat({(other < 0), (bitIdx + (ExpMax >> 1)), (o & ~(ManAccessType(1) << (_ManBits - 1)))});
            // }
            // else {
            //     *this = StaticFloat({(bitIdx + (ExpMax >> 1)), (o & ~(ManAccessType(1) << (_ManBits - 1)))});
            // }
            // if constexpr(_Signed) m_IEEE754.Sign = (other < 0);
            m_IEEE754.Exp = (bitIdx + ExpBias);
            m_IEEE754.Man = (o & ~ManBias);
            // assign(other);
        }
        
        template<short ExpBitsO, short ManBitsO, bool SignedO>
        constexpr StaticFloat(const StaticFloat<ExpBitsO, ManBitsO, SignedO>& other) : m_data() {
            constexpr bool IsSameSign = (_Signed == SignedO);
            constexpr bool IsExact = ((_ExpBits == ExpBitsO) && (_ManBits == ManBitsO));
            if((other.m_IEEE754.Exp == 0) && (other.m_IEEE754.Man == 0)) {
                if constexpr(_Signed && IsSameSign) m_IEEE754 = ((m_IEEE754.Sign ^ other.m_IEEE754.Sign) ? NegZero : PosZero);
                m_IEEE754 = PosZero;
                // return *this;
            }
            if constexpr(IsExact && IsSameSign) {
                m_IEEE754 = other.m_IEEE754;
            }
            else if constexpr(IsExact && !IsSameSign) {
                constexpr StaticFloat<ExpBitsO, ManBitsO, !SignedO> O{other};
                m_IEEE754 = O.m_IEEE754;
            }
            else if constexpr(!IsExact && IsSameSign) {
                if constexpr(_Signed) m_IEEE754.Sign = other.m_IEEE754.Sign;
                m_IEEE754.Exp = (Type::ExpBias + other.m_IEEE754.Exp - StaticFloat<ExpBitsO, ManBitsO, SignedO>::ExpBias);
                m_IEEE754.Man = other.m_IEEE754.Man;
            }
            else if constexpr(!IsExact && !IsSameSign) {
                constexpr StaticFloat<ExpBitsO, ManBitsO, !SignedO> O{other};
                if constexpr(_Signed) m_IEEE754.Sign = O.m_IEEE754.Sign;
                m_IEEE754.Exp = (Type::ExpBias + O.m_IEEE754.Exp - StaticFloat<ExpBitsO, ManBitsO, !SignedO>::ExpBias);
                m_IEEE754.Man = O.m_IEEE754.Man;
            }
        }
        
        // template<typename T>
        // constexpr void operator=(const int other) const {
        //     if(other ==  0) return;
        //     ManAccessType o;
        //     if constexpr(_Signed) {
        //         o = ((other < 0) ? -other : other);
        //     }
        //     else {
        //         if(other < 0) return;
        //         o = other;
        //     }
        //     short bitIdx = ((TotalBytes * 8) - 1);
        //     while(bitIdx > 0) {
        //         if(o & (ManAccessType(1) << bitIdx)) {
        //             if(bitIdx >= _ManBits) {
        //                 o >>= (bitIdx - _ManBits);
        //             } else {
        //                 o <<= (_ManBits - bitIdx);
        //             }
        //             break;
        //         }
        //         bitIdx--;
        //     }
            
        //     if constexpr(_Signed) m_IEEE754.Sign = (other < 0);
        //     m_IEEE754.Exp = (bitIdx + (ExpMax >> 1));
        //     m_IEEE754.Man = (o & ~(ManAccessType(1) << (_ManBits - 1)));
        // }
        
        template<typename T>
        constexpr Type& operator=(const T other) {
            constexpr bool IsSameSign = (_Signed == T::Signed);
            constexpr bool IsExact = ((_ExpBits == T::ExpBits) && (_ManBits == T::ManBits));
            if((other.Exp == 0) && (other.Man == 0)) {
                if constexpr(_Signed && IsSameSign) m_IEEE754 = ((m_IEEE754.Sign ^ other.Sign) ? NegZero : PosZero);
                m_IEEE754 = PosZero;
                return *this;
            }
            if constexpr(IsExact && IsSameSign) {
                m_IEEE754 = other.m_IEEE754;
            }
            else if constexpr(IsExact && !IsSameSign) {
                constexpr StaticFloat<T::ExpBits, T::ManBits, !T::Signed> O{other};
                m_IEEE754 = O.m_IEEE754;
            }
            else if constexpr(!IsExact && IsSameSign) {
                if constexpr(_Signed) m_IEEE754.Sign = other.Sign;
                m_IEEE754.Exp = (Type::ExpBias + other.Exp - StaticFloat<T::ExpBits, T::ManBits, T::Signed>::ExpBias);
                m_IEEE754.Man = other.Man;
            }
            else if constexpr(!IsExact && !IsSameSign) {
                constexpr StaticFloat<T::ExpBits, T::ManBits, !T::Signed> O{other};
                if constexpr(_Signed) m_IEEE754.Sign = O.m_IEEE754.Sign;
                m_IEEE754.Exp = (Type::ExpBias + O.m_IEEE754.Exp - StaticFloat<T::ExpBits, T::ManBits, !T::Signed>::ExpBias);
                m_IEEE754.Man = O.m_IEEE754.Man;
            }
            return *this;
        }
        
        template<short ExpBitsO, short ManBitsO, bool SignedO>
        constexpr Type operator*(const StaticFloat<ExpBitsO, ManBitsO, SignedO>& other) const {
            constexpr bool IsSameSign = (_Signed == SignedO);
            if(((m_IEEE754.Exp == 0) && (m_IEEE754.Man == 0)) || ((other.m_IEEE754.Exp == 0) && (other.m_IEEE754.Man == 0))) {
                if constexpr(_Signed && IsSameSign) return ((m_IEEE754.Sign ^ other.m_IEEE754.Sign) ? NegZero : PosZero);
                return PosZero;
            }
            if constexpr(_Signed) {
                return Type::BitManipType{
                    static_cast<bool>(m_IEEE754.Sign ^ other.m_IEEE754.Sign),
                    static_cast<ExpAccessType>(((m_IEEE754.Exp > 0) ? m_IEEE754.Exp : Type::ExpBias) + ((other.m_IEEE754.Exp > 0) ? (other.m_IEEE754.Exp - StaticFloat<ExpBitsO, ManBitsO, SignedO>::ExpBias) : ((m_IEEE754.Exp > 0) ? 0 : - Type::ExpBias))),
                    static_cast<ManAccessType>(m_IEEE754.Man * other.m_IEEE754.Man)
                };
            }
            else {
                return Type::BitManipType{
                    static_cast<ExpAccessType>(((m_IEEE754.Exp > 0) ? m_IEEE754.Exp : Type::ExpBias) + ((other.m_IEEE754.Exp > 0) ? (other.m_IEEE754.Exp - StaticFloat<ExpBitsO, ManBitsO, SignedO>::ExpBias) : ((m_IEEE754.Exp > 0) ? 0 : - Type::ExpBias))),
                    static_cast<ManAccessType>(m_IEEE754.Man * other.m_IEEE754.Man)
                };
            }
        }
        
        constexpr StaticFloat() : m_data() {}
        
        // ~StaticFloat() = default;
        
        // explicit constexpr StaticFloat(const float other) : m_data() {
        //     if(other ==  0) return;
        //     *this = (*((FPNBits::StaticFloat<9, 23, _Signed>*)(&other)));
        // }
        
        inline static constexpr BitManipType PosInf{[]() constexpr->BitManipType{
            if constexpr(_Signed) {
                return {
                    false,
                    ExpMax,
                    0
                };
            }
            else {
                return {
                    0,
                    0
                };
            }
        }()};
        
        inline static constexpr BitManipType PosZero{[]() constexpr->BitManipType{
            if constexpr(_Signed) {
                return {
                    false,
                    0,
                    0
                };
            }
            else {
                return {
                    0,
                    0
                };
            }
        }()};
        
        inline static constexpr BitManipType Bias{[]() constexpr->BitManipType{
            if constexpr(_Signed) {
                return {
                    false,
                    (ExpAccessType(1) << (_ExpBits)),
                    0
                };
            }
            else {
                return {
                    (ExpAccessType(1) << (_ExpBits)),
                    0
                };
            }
        }()};
        
        inline static constexpr BitManipType PosTwo{[]() constexpr->BitManipType{
            if constexpr(_Signed) {
                return {
                    false,
                    (ExpBias + 1),
                    0
                };
            }
            else {
                return {
                    ((ExpAccessType(1) << (_ExpBits - 1)) + 1),
                    0
                };
            }
        }()};
        
        // inline static constexpr BitManipType PosTen{[]() constexpr->BitManipType{
        //     if constexpr(_Signed) {
        //         return {
        //             false,
        //             ((ExpAccessType(1) << (_ExpBits - 1)) + 1),
        //             5
        //         };
        //     }
        //     else {
        //         return {
        //             ((ExpAccessType(1) << (_ExpBits - 1)) + 1),
        //             5
        //         };
        //     }
        // }()};
        
        inline static constexpr BitManipType Max{[]() constexpr->BitManipType{
            if constexpr(_Signed) {
                return {
                    false,
                    ExpMax,
                    ManMax
                };
            }
            else {
                return {
                    ExpMax,
                    ManMax
                };
            }
        }()};
        
        inline static constexpr BitManipType Min{[]() constexpr->BitManipType{
            if constexpr(_Signed) {
                return {
                    true,
                    ExpMax,
                    ManMax
                };
            }
            else {
                return {
                    0,
                    0
                };
            }
        }()};
        
        inline static constexpr BitManipType NegInf{[]() constexpr->BitManipType{
            if constexpr(_Signed) {
                return {
                    true,
                    ExpMax,
                    0
                };
            }
            else {
                return {
                    0,
                    0
                };
            }
        }()};
        
        inline static constexpr BitManipType NegZero{[]() constexpr->BitManipType{
            if constexpr(_Signed) {
                return {
                    true,
                    0,
                    0
                };
            }
            else {
                return {
                    0,
                    0
                };
            }
        }()};
        
        inline static constexpr BitManipType NegOne{[]() constexpr->BitManipType{
            if constexpr(_Signed) {
                return {
                    true,
                    (ExpMax >> 1),
                    0
                };
            }
            else {
                return {
                    (ExpMax >> 1),
                    0
                };
            }
        }()};
        
        inline static constexpr BitManipType Lowest{[]() constexpr->BitManipType{
            if constexpr(_Signed) {
                return {
                    false,
                    0,
                    1
                };
            }
            else {
                return {
                    0,
                    1
                };
            }
        }()};
        
        inline static constexpr BitManipType qNaN{[]() constexpr->BitManipType{
            if constexpr(_Signed) {
                return {
                    false,
                    0,
                    (ManAccessType(1) << (_ManBits - 1))
                };
            }
            else {
                return {
                    0,
                    (ManAccessType(1) << (_ManBits - 1))
                };
            }
        }()};
        
        inline static constexpr BitManipType sNaN{[]() constexpr->BitManipType{
            if constexpr(_Signed) {
                return {
                    false,
                    0,
                    (ManMax >> 1)
                };
            }
            else {
                return {
                    0,
                    (ManMax >> 1)
                };
            }
        }()};
        
        const std::string RawBits() const {
            std::string rtn;
            for(unsigned char* a = (unsigned char*)(m_data + TotalBytes - 1); a >= m_data; a--) {
                for(unsigned char b = 8; b > 0; b--) {
                    rtn.push_back(CHKBITINBYTE(a, b - 1) ? '1' : '0');
                }
            }
            return rtn;
        }
        
        operator const std::string() const {
            return __Internal::to_str(*this);
        }
        
        constexpr operator BitManipType() const {
            return m_IEEE754;
        }
    };
    
    // #define MakeTenGTO(bit) []() constexpr->typename StaticFloat<128, 128, false>::BitManipType{ \
    //     constexpr StaticFloat<128, 128, false>::const StaticFloat<const char> _B{bit}; \
    //     constexpr StaticFloat<128, 128, false>::const StaticFloat<const char> _T{10}; \
    //     return _B * _T; \
    // }()
    // #define MakeTenLTO(bit) (StaticFloat<128, 128, false>(1) / StaticFloat<128, 128, false>(1 << bit)).m_IEEE754
    inline static constexpr auto tenGTO{std::MakePowerUpArr<typename StaticFloat<128, 128, false>::Type, 128, StaticFloat<128, 128, false>::PosTen>()};
        // 
    // };
//         []() constexpr->typename StaticFloat<128, 128, false>::BitManipType{
//         constexpr int other = 1;
//         constexpr StaticFloat<128, 128, false> _B = StaticFloat<128, 128, false>({0, 0});
//         constexpr StaticFloat<128, 128, false> _T = StaticFloat<128, 128, false>{other};
//         // return {(bitIdx + (StaticFloat<128, 128, false>::ExpMax >> 1)), (o & ~(typename StaticFloat<128, 128, false>::ManAccessType(1) << (StaticFloat<128, 128, false>::BitManipType::ManBits - 1)))};
//         return (StaticFloat<128, 128, false>(1) * StaticFloat<128, 128, false>(10));
//     }()};


/*
ManAccessType o;
            if constexpr(_Signed) {
                o = ((other < 0) ? -other : other);
            }
            else {
                if(other < 0) return;
                o = other;
            }
            short bitIdx = ((TotalBytes * 8) - 1);
            while(bitIdx > 0) {
                if(o & (ManAccessType(1) << bitIdx)) {
                    if(bitIdx >= _ManBits) {
                        o >>= (bitIdx - _ManBits);
                    } else {
                        o <<= (_ManBits - bitIdx);
                    }
                    break;
                }
                bitIdx--;
            }
            
            if constexpr(_Signed) m_IEEE754.Sign = (other < 0);
            m_IEEE754.Exp = (bitIdx + (StaticFloat<128, 128, false>::ExpMax >> 1));
            m_IEEE754.Man = (o & ~(typename StaticFloat<128, 128, false>::ManAccessType(1) << (StaticFloat<128, 128, false>::ManBits - 1)));
*/
    // inline static constexpr StaticFloat<128, 128, false> o{[]() constexpr->typename StaticFloat<128, 128, false>::BitManipType{
    //     return {
    //                 0,
    //                 0
    //             };
    // }()};

    // // { StaticFloat<128, 128, false>(1) * StaticFloat<128, 128, false>(10) }
    // // { StaticFloat<128, 128, false>(1) * StaticFloat<128, 128, false>(10) }
    // };
        // MakeTenGTO(1)
        // MakeTenGTO(2)
    // };
    
    namespace __Internal {
        template<typename T>
        std::string to_str(const T& f, unsigned short Places) {
            std::string rtn;
            /*
                if constexpr(sizeof(T::ExpAccessType) > sizeof(T::ManAccessType)) {
                    using ComputeType = typename T::ExpAccessType;
                }
                else {
                    using ComputeType = tyoename T::ManAccessType;
                }
                char digit;
                ComputeType TCount{0};
                ComputeType Log10f;
                // T MicroOffset{};
                if constexpr(T::BitManipType::Signed) {
                    const bool Neg{f < 0};
                    T Tmp{Neg ? T::NegHalf : T::PosHalf}; //0.5
                    if(Sign) {
                        
                    }
                    else {
                        
                    }
                }
                else {
                    
                }
            */
            return rtn;
        }
    };
    /*
    std::string rtn;
            if constexpr(Signed) CHKBITINBYTE((char*)(&(this->m_data)), 7) ? rtn.push_back('-') : void(0);
            
            *
            floatToString(char* outstr, float value, size_t places=NUM_DECIMAL_PLACES) {

    /// The order of magnitude on which the input resides
    /// i.e. tenscount = (int)log10f(abs(value))

    /// calculate rounding term d:   0.5/pow(10,places)
    float d = 

    /// divide by ten for each decimal place
    d = d * __inv_pow_10[places];

    /// this small addition, combined with truncation will round our values properly
    if (value < 0) {
        tempfloat = -(value + d);
    } else {
    	tempfloat = value + d;
    }

    /// This logic is similar to
    /// tenscount = (int)log10f(tempfloat)
    /// but is not miserably slow. 
    while (__pos_pow_10o[tenscount++] <= tempfloat) {}

    /// the number is negative
    if (value < 0) {
        outstr[c++] = '-';
    }

    if (tenscount == 0) {
        outstr[c++] = '0';
    }

    #pragma nounroll
    for (i = 0; i < tenscount; i++) {

    	const ssize_t idx = tenscount-i;

        digit = (int8_t) (tempfloat * __inv_pow_10p[idx]);

        to_digit_char_unsafe(digit, &outstr[c++]);
        tempfloat = tempfloat - ((float)digit * __pos_pow_10p[idx]);
    
    }

    /// if places is zero, then there is no decimal part
    if (places > 0) {
    	outstr[c++] = '.';
    }

    #pragma nounroll
    for (i = 0; i < places; i++) {

        tempfloat *= 10.0f;
        digit = (int8_t) tempfloat;

        /// convert digit to character
        /// If you replace this function with sprintf("%d", digit)
        /// you will see where sprintf most likely spends most of
        /// its time during their implementation of this routine :-)
        /// 
        /// Note that if for some reason digit is not within [0,9],
        /// your number will be a flaming pile of shit 
        to_digit_char_unsafe(digit, &outstr[c++]);

        tempfloat = tempfloat - (float) digit;

    }

    outstr[c++] = '\0';

    return c;
            
    */
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