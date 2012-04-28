#ifndef HPP_CPP_MULTI_PRECISION_PRIMITIVE_MULTIPRECISION_FLOAT
#define HPP_CPP_MULTI_PRECISION_PRIMITIVE_MULTIPRECISION_FLOAT

#include "unsigned_integer.hpp"

namespace cpp_multi_precision{
    namespace aux{
        template<class RadixType, std::size_t RadixLog2, class Radix2Type, class URadix2Type, class Container>
        class fractional : public unsigned_integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container>{
        public:
            typedef unsigned_integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container> base_type;
            using base_type::radix_log2;
            using base_type::container;
            using base_type::assign;
            using base_type::square_multi;
            using base_type::div;

        public:
            fractional() : base_type(){
                assign_normalize_container();
            }

            fractional(const fractional &other) : base_type(other){
                assign_normalize_container();
            }

            fractional(fractional &&other) : base_type(other){
                assign_normalize_container();
            }

            std::size_t deg() const{
                std::size_t i = 1;
                while(container[i] != 0){ ++i; }
                return i - 1;
            }

            int normalize(int exp){
                std::size_t n = container.size() - 1, m = n - 1;
                //step1:;
                if(container[n] > 0){
                    goto step4;
                }else{
                    for(std::size_t i = 0; i < container.size(); ++i){
                        if(container[i] != 0){ goto step2; }
                    }
                    exp = 0;
                    goto step7;
                }

                step2:;
                if((container[m] >> (radix_log2 - 1)) == 1){ goto step5; }

                //step3:;
                --exp;
                *this <<= 1;
                goto step2;

                step4:;
                ++exp;
                *this >>= 1;
                container[n] = 0;

                step5:;
                if(container[0] != 0){
                    *this += static_cast<typename base_type::radix_type>((static_cast<typename base_type::radix2_type>(1) << radix_log2) - container[0]);
                }
                if(container[n] != 0){ goto step4; }

                //step6:;

                step7:;
                return exp;
            }

            fractional &operator =(const fractional &other){
                base_type::operator =(other);
                return *this;
            }

            fractional &operator =(fractional &&other){
                base_type::operator =(other);
                return *this;
            }

            fractional &operator -=(const fractional &rhs){
                base_type::operator -=(rhs);
                return *this;
            }

            fractional operator -(const fractional &rhs) const{
                fractional r(*this);
                r -= rhs;
                return std::move(r);
            }

        private:
            void assign_normalize_container(){
                base_type::normalize_container = &base_type::normalize_container_dummy;
            }
        };

        struct primitive_mp_float_non_resize{};
    }

    template<class RadixType, std::size_t RadixLog2, class Radix2Type, class URadix2Type, class Container>
    class primitive_mp_float{
    private:
        typedef primitive_mp_float<RadixType, RadixLog2, Radix2Type, URadix2Type, Container> this_type;

    public:
        typedef RadixType radix_type;
        typedef Radix2Type radix2_type;
        typedef URadix2Type unsigned_radix2_type;
        typedef aux::fractional<radix_type, RadixLog2, radix2_type, unsigned_radix2_type, Container> fractional_part_type;
        static const std::size_t radix_log2 = RadixLog2;
        static const unsigned_radix2_type radix_num = static_cast<unsigned_radix2_type>(1) << radix_log2;

    public:
        primitive_mp_float() : fractional_part(), exponent_part(0), sign(true){
            fractional_part.container.resize(get_precision() + 2);
        }

        primitive_mp_float(const primitive_mp_float &other) : fractional_part(other.fractional_part), exponent_part(other.exponent_part), sign(other.sign){}
        primitive_mp_float(primitive_mp_float &&other) : fractional_part(other.fractional_part), exponent_part(other.exponent_part), sign(other.sign){}

    private:
        primitive_mp_float(aux::primitive_mp_float_non_resize) : fractional_part(), exponent_part(0), sign(true){}

    public:
        primitive_mp_float(double x){
            assign(x);
        }

        void assign(double x){
            sign = x >= 0.0;
            if(x < 0.0){ x = -x; }
            fractional_part.container.resize(get_precision() + 2);
            std::size_t prec = get_precision();
            double x_fractional_part;
            int x_exponent_part;
            x_fractional_part = std::frexp(x, &x_exponent_part);
            exponent_part = x_exponent_part;
            std::size_t bit_counter = 0;
            for(double fractional_bit = 0.5; x_fractional_part > 0 && fractional_bit > 0; ++bit_counter){
                if(x_fractional_part >= fractional_bit){
                    fractional_part.container[prec - (bit_counter / radix_log2)] |= 1 << (radix_log2 - 1 - (bit_counter % radix_log2));
                    x_fractional_part -= fractional_bit;
                }
                fractional_bit = fractional_bit / 2.0;
            }
        }

        std::string to_string() const{
            std::string r;
            if(!sign){ r += "-"; }
            r += integer_portion().to_string();
            r += ".";
            primitive_mp_float f(fractional_portion()), ten(10.0), zero(0.0);
            if(f == zero){
                r += "0";
            }else{
                for(f *= ten; f != zero; f = f.fractional_portion(), f *= ten){
                    r += f.integer_portion().to_string();
                }
            }
            return std::move(r);
        }

        typename fractional_part_type::base_type integer_portion() const{
            std::size_t prec = fractional_part.container.size() - 2;
            if(exponent_part <= 0){
                typename fractional_part_type::base_type y;
                return std::move(y);
            }
            fractional_part_type x;
            x.container.resize(prec + 2);
            x = fractional_part >> (prec * radix_log2 - static_cast<std::size_t>(exponent_part));
            typename fractional_part_type::base_type y(x.container.begin() + 1, x.container.end() - 1);
            return std::move(y);
        }

        primitive_mp_float fractional_portion() const{
            if(exponent_part <= 0){ return *this; }
            primitive_mp_float x(*this);
            x.fractional_part <<= static_cast<std::size_t>(exponent_part);
            x.exponent_part = 0;
            x.normalize();
            return std::move(x);
        }

        primitive_mp_float &operator =(const primitive_mp_float &rhs){
            if(fractional_part.container.size() != rhs.fractional_part.container.size()){
                fractional_part = rhs.fractional_part;
            }else{
                for(std::size_t i = 1, n = fractional_part.container.size() - 1; i < n; ++i){
                    fractional_part.container[i] = rhs.fractional_part.container[i];
                }
            }
            exponent_part = rhs.exponent_part;
            sign = rhs.sign;
            return *this;
        }

        primitive_mp_float &operator =(primitive_mp_float &&rhs){
            if(fractional_part.container.size() != rhs.fractional_part.container.size()){
                fractional_part = std::move(rhs.fractional_part);
            }else{
                for(std::size_t i = 1, n = fractional_part.container.size() - 1; i < n; ++i){
                    fractional_part.container[i] = rhs.fractional_part.container[i];
                }
            }
            exponent_part = rhs.exponent_part;
            sign = rhs.sign;
            return *this;
        }

        primitive_mp_float operator +(const primitive_mp_float &rhs) const{
            primitive_mp_float r = add(*this, rhs);
            return std::move(r);
        }

        primitive_mp_float &operator +=(const primitive_mp_float &rhs){
            primitive_mp_float r = add(*this, rhs);
            *this = std::move(r);
            return *this;
        }

        primitive_mp_float operator -(const primitive_mp_float &rhs) const{
            primitive_mp_float r = sub(*this, rhs);
            return std::move(r);
        }

        primitive_mp_float &operator -=(const primitive_mp_float &rhs){
            primitive_mp_float r = sub(*this, rhs);
            *this = std::move(r);
            return *this;
        }

        primitive_mp_float operator *(const primitive_mp_float &rhs) const{
            primitive_mp_float r = mul(*this, rhs);
            return std::move(r);
        }

        primitive_mp_float &operator *=(const primitive_mp_float &rhs){
            primitive_mp_float r = mul(*this, rhs);
            *this = std::move(r);
            return *this;
        }

        primitive_mp_float operator /(const primitive_mp_float &rhs) const{
            primitive_mp_float r = div(*this, rhs);
            return std::move(r);
        }

        primitive_mp_float &operator /=(const primitive_mp_float &rhs){
            primitive_mp_float r = div(*this, rhs);
            *this = std::move(r);
            return *this;
        }

        primitive_mp_float &operator +(){
            return *this;
        }

        primitive_mp_float operator -() const{
            primitive_mp_float r(*this);
            r.sign = !r.sign;
            return std::move(r);
        }

        bool operator ==(const primitive_mp_float &rhs) const{
            return sign == rhs.sign && exponent_part == rhs.exponent_part && fractional_part == rhs.fractional_part;
        }

        bool operator !=(const primitive_mp_float &rhs) const{
            return sign != rhs.sign || exponent_part != rhs.exponent_part || fractional_part != rhs.fractional_part;
        }

        bool operator <(const primitive_mp_float &rhs) const{
            if(sign == rhs.sign){
                if(sign){
                    return exponent_part < rhs.exponent_part || (exponent_part == rhs.exponent_part && fractional_part < rhs.fractional_part);
                }else{
                    return exponent_part > rhs.exponent_part || (exponent_part == rhs.exponent_part && fractional_part > rhs.fractional_part);
                }
            }else{
                return !sign;
            }
        }

        bool operator >(const primitive_mp_float &rhs) const{
            if(sign == rhs.sign){
                if(sign){
                    return exponent_part > rhs.exponent_part || (exponent_part == rhs.exponent_part && fractional_part > rhs.fractional_part);
                }else{
                    return exponent_part < rhs.exponent_part || (exponent_part == rhs.exponent_part && fractional_part < rhs.fractional_part);
                }
            }else{
                return sign;
            }
        }

        bool operator <=(const primitive_mp_float &rhs) const{
            if(sign == rhs.sign){
                if(sign){
                    if(exponent_part < rhs.exponent_part){
                        return true;
                    }else if(exponent_part == rhs.exponent_part){
                        return fractional_part <= rhs.fractional_part;
                    }else{
                        return false;
                    }
                }else{
                    if(exponent_part > rhs.exponent_part){
                        return true;
                    }else if(exponent_part == rhs.exponent_part){
                        return fractional_part >= rhs.fractional_part;
                    }else{
                        return false;
                    }
                }
            }else{
                return (!sign && rhs.sign) || (fractional_part == 0 && rhs.fractional_part == 0);
            }
        }

        bool operator >=(const primitive_mp_float &rhs) const{
            if(sign == rhs.sign){
                if(sign){
                    if(exponent_part > rhs.exponent_part){
                        return true;
                    }else if(exponent_part == rhs.exponent_part){
                        return fractional_part >= rhs.fractional_part;
                    }else{
                        return false;
                    }
                }else{
                    if(exponent_part < rhs.exponent_part){
                        return true;
                    }else if(exponent_part == rhs.exponent_part){
                        return fractional_part <= rhs.fractional_part;
                    }else{
                        return false;
                    }
                }
            }else{
                return (sign && !rhs.sign) || (fractional_part == 0 && rhs.fractional_part == 0);
            }
        }

    public:
        static primitive_mp_float add(const primitive_mp_float &lhs, const primitive_mp_float &rhs){
            if(lhs.sign == rhs.sign){
                if(lhs.exponent_part >= rhs.exponent_part){
                    primitive_mp_float result(add_impl(lhs, rhs));
                    result.sign = lhs.sign;
                    return std::move(result);
                }else{
                    primitive_mp_float result(add_impl(rhs, lhs));
                    result.sign = lhs.sign;
                    return std::move(result);
                }
            }else{
                if(lhs >= rhs){
                    aux::sign sign = lhs.sign;
                    primitive_mp_float result(sub_impl(lhs, rhs));
                    result.sign = sign;
                    return std::move(result);
                }else{
                    aux::sign sign = rhs.sign;
                    primitive_mp_float result(sub_impl(rhs, lhs));
                    result.sign = sign;
                    return std::move(result);
                }
            }
        }

        static primitive_mp_float sub(const primitive_mp_float &lhs, const primitive_mp_float &rhs){
            if(lhs.sign == rhs.sign){
                if(lhs >= rhs){
                    aux::sign sign = lhs.sign;
                    primitive_mp_float result(sub_impl(lhs, rhs));
                    result.sign = sign;
                    return std::move(result);
                }else{
                    aux::sign sign = rhs.sign;
                    primitive_mp_float result(sub_impl(rhs, lhs));
                    result.sign = sign;
                    return std::move(result);
                }
            }else{
                if(lhs.exponent_part >= rhs.exponent_part){
                    primitive_mp_float result(add_impl(lhs, rhs));
                    result.sign = lhs.sign;
                    return std::move(result);
                }else{
                    primitive_mp_float result(add_impl(rhs, lhs));
                    result.sign = lhs.sign;
                    return std::move(result);
                }
            }
        }

        static primitive_mp_float mul(const primitive_mp_float &lhs, const primitive_mp_float &rhs){
            int prec = lhs.fractional_part.container.size() - 2;
            aux::primitive_mp_float_non_resize non_resize_instance;
            primitive_mp_float result(non_resize_instance);
            result.sign = lhs.sign == rhs.sign;
            result.exponent_part = lhs.exponent_part + rhs.exponent_part;
            fractional_part_type::square_multi(result.fractional_part, lhs.fractional_part, rhs.fractional_part, true);
            for(std::size_t i = prec * 2 + 1, n = i - prec, j = prec; i >= n; --i, --j){
                result.fractional_part.container[j] = result.fractional_part.container[i];
            }
            if(result.fractional_part.container[0] != 0){
                result.fractional_part += static_cast<radix_type>(radix_num - result.fractional_part.container[0]);
            }
            result.fractional_part.container.resize(prec + 2);
            result.fractional_part.container[prec + 1] = 0;
            result.normalize();
            return std::move(result);
        }

        static primitive_mp_float div(const primitive_mp_float &lhs, const primitive_mp_float &rhs){
            int prec = lhs.fractional_part.container.size() - 2;
            aux::primitive_mp_float_non_resize non_resize_instance;
            primitive_mp_float result(non_resize_instance);
            typename fractional_part_type::base_type::container_type::const_iterator
                lhs_first,
                lhs_last,
                rhs_first = rhs.fractional_part.container.begin(),
                rhs_last = rhs.fractional_part.container.end();
            ++rhs_first, --rhs_last;
            result.fractional_part.container.resize(prec + 2);
            result.sign = lhs.sign == rhs.sign;
            result.exponent_part = lhs.exponent_part - rhs.exponent_part;
            primitive_mp_float divided(non_resize_instance), rem(non_resize_instance), quo(non_resize_instance);
            divided.fractional_part.container.resize(prec + 2);
            rem.fractional_part.container.resize(prec + 2);
            quo.fractional_part.container.resize(prec + 2);
            for(int i = 1; i <= prec; ++i){
                divided.fractional_part.container[i] = lhs.fractional_part.container[i];
            }
            for(
                std::size_t word_counter = 0, word_counter_length = prec + 1;
                word_counter < word_counter_length;
                ++word_counter
            ){
                if(rhs.fractional_part > divided.fractional_part){
                    for(int i = 0, n = prec; i < n; ++i){
                        divided.fractional_part.container[prec - i + 1] = divided.fractional_part.container[prec - i];
                    }
                    divided.fractional_part.container[0] = 0;
                    continue;
                }
                fractional_part_type::div(quo.fractional_part, rem.fractional_part, divided.fractional_part, rhs.fractional_part);
                result.fractional_part.container[prec + 1 - word_counter] = quo.fractional_part.container[1];
                divided.fractional_part.container[0] = 0;
                divided.fractional_part.container[1] = 0;
                for(int i = 1; i <= prec; ++i){
                    divided.fractional_part.container[i + 1] = rem.fractional_part.container[i];
                }
                for(int i = 0; i < prec; ++i){
                    quo.fractional_part.container[i] = 0;
                    rem.fractional_part.container[i] = 0;
                }
            }
            result.normalize();
            return std::move(result);
        }

        static void set_precision(std::size_t value){
            prec_impl() = value;
        }

        static std::size_t get_precision(){
            return prec_impl();
        }

    private:
        static primitive_mp_float add_impl(const primitive_mp_float &lhs, const primitive_mp_float &rhs){
            int prec = lhs.fractional_part.container.size() - 2;
            aux::primitive_mp_float_non_resize non_resize_instance;
            primitive_mp_float result(non_resize_instance);
            result.fractional_part.container.resize(prec + 2);
            result.exponent_part = lhs.exponent_part;
            int exponent_diff = lhs.exponent_part - rhs.exponent_part;
            if(exponent_diff >= prec + prec / 2){
                result.fractional_part.container = lhs.fractional_part.container;
                return std::move(result);
            }
            result.fractional_part.container = rhs.fractional_part.container;
            result.fractional_part >>= exponent_diff;
            result.fractional_part += lhs.fractional_part;
            result.normalize();
            return std::move(result);
        }

        static primitive_mp_float sub_impl(const primitive_mp_float &lhs, const primitive_mp_float &rhs){
            int prec = lhs.fractional_part.container.size() - 2;
            aux::primitive_mp_float_non_resize non_resize_instance;
            primitive_mp_float result(non_resize_instance);
            result.fractional_part.container.resize(prec + 2);
            result.exponent_part = lhs.exponent_part;
            int exponent_diff = lhs.exponent_part - rhs.exponent_part;
            if(exponent_diff >= prec + prec / 2){
                result.fractional_part.container = lhs.fractional_part.container;
                return std::move(result);
            }
            result.fractional_part.container = rhs.fractional_part.container;
            result.fractional_part >>= exponent_diff;
            result.fractional_part = lhs.fractional_part - result.fractional_part;
            result.normalize();
            return std::move(result);
        }

        static std::size_t &prec_impl(){
            static std::size_t value = 0;
            return value;
        }

        void normalize(){
            exponent_part = fractional_part.normalize(exponent_part);
        }

    private:
        fractional_part_type fractional_part;
        int exponent_part;
        aux::sign sign;
    };

    template<class RadixType, std::size_t RadixLog2, class Radix2Type, class URadix2Type, class Container>
    primitive_mp_float<RadixType, RadixLog2, Radix2Type, URadix2Type, Container>
    operator +(
        double lhs,
        const primitive_mp_float<RadixType, RadixLog2, Radix2Type, URadix2Type, Container> &rhs
    ){ return primitive_mp_float<RadixType, RadixLog2, Radix2Type, URadix2Type, Container>(lhs) + rhs; }

    template<class RadixType, std::size_t RadixLog2, class Radix2Type, class URadix2Type, class Container>
    primitive_mp_float<RadixType, RadixLog2, Radix2Type, URadix2Type, Container>
    operator -(
        double lhs,
        const primitive_mp_float<RadixType, RadixLog2, Radix2Type, URadix2Type, Container> &rhs
    ){ return primitive_mp_float<RadixType, RadixLog2, Radix2Type, URadix2Type, Container>(lhs) - rhs; }

    template<class RadixType, std::size_t RadixLog2, class Radix2Type, class URadix2Type, class Container>
    primitive_mp_float<RadixType, RadixLog2, Radix2Type, URadix2Type, Container>
    operator *(
        double lhs,
        const primitive_mp_float<RadixType, RadixLog2, Radix2Type, URadix2Type, Container> &rhs
    ){ return primitive_mp_float<RadixType, RadixLog2, Radix2Type, URadix2Type, Container>(lhs) * rhs; }

    template<class RadixType, std::size_t RadixLog2, class Radix2Type, class URadix2Type, class Container>
    primitive_mp_float<RadixType, RadixLog2, Radix2Type, URadix2Type, Container>
    operator /(
        double lhs,
        const primitive_mp_float<RadixType, RadixLog2, Radix2Type, URadix2Type, Container> &rhs
    ){ return primitive_mp_float<RadixType, RadixLog2, Radix2Type, URadix2Type, Container>(lhs) / rhs; }

    template<class RadixType, std::size_t RadixLog2, class Radix2Type, class URadix2Type, class Container>
    bool operator ==(
        double lhs,
        const primitive_mp_float<RadixType, RadixLog2, Radix2Type, URadix2Type, Container> &rhs
    ){ return primitive_mp_float<RadixType, RadixLog2, Radix2Type, URadix2Type, Container>(lhs) == rhs; }

    template<class RadixType, std::size_t RadixLog2, class Radix2Type, class URadix2Type, class Container>
    bool operator !=(
        double lhs,
        const primitive_mp_float<RadixType, RadixLog2, Radix2Type, URadix2Type, Container> &rhs
    ){ return primitive_mp_float<RadixType, RadixLog2, Radix2Type, URadix2Type, Container>(lhs) != rhs; }

    template<class RadixType, std::size_t RadixLog2, class Radix2Type, class URadix2Type, class Container>
    bool operator <(
        double lhs,
        const primitive_mp_float<RadixType, RadixLog2, Radix2Type, URadix2Type, Container> &rhs
    ){ return primitive_mp_float<RadixType, RadixLog2, Radix2Type, URadix2Type, Container>(lhs) < rhs; }

    template<class RadixType, std::size_t RadixLog2, class Radix2Type, class URadix2Type, class Container>
    bool operator >(
        double lhs,
        const primitive_mp_float<RadixType, RadixLog2, Radix2Type, URadix2Type, Container> &rhs
    ){ return primitive_mp_float<RadixType, RadixLog2, Radix2Type, URadix2Type, Container>(lhs) > rhs; }

    template<class RadixType, std::size_t RadixLog2, class Radix2Type, class URadix2Type, class Container>
    bool operator <=(
        double lhs,
        const primitive_mp_float<RadixType, RadixLog2, Radix2Type, URadix2Type, Container> &rhs
    ){ return primitive_mp_float<RadixType, RadixLog2, Radix2Type, URadix2Type, Container>(lhs) <= rhs; }

    template<class RadixType, std::size_t RadixLog2, class Radix2Type, class URadix2Type, class Container>
    bool operator >=(
        double lhs,
        const primitive_mp_float<RadixType, RadixLog2, Radix2Type, URadix2Type, Container> &rhs
    ){ return primitive_mp_float<RadixType, RadixLog2, Radix2Type, URadix2Type, Container>(lhs) >= rhs; }

    template<class RadixType, std::size_t RadixLog2, class Radix2Type, class URadix2Type, class Container>
    std::ostream &operator <<(
        std::ostream &ostream,
        const primitive_mp_float<RadixType, RadixLog2, Radix2Type, URadix2Type, Container> &value
    ){
        ostream << value.to_string();
        return ostream;
    }
}

#endif
