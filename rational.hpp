#ifndef HPP_CPP_MULTI_PRECISION_RATIONAL
#define HPP_CPP_MULTI_PRECISION_RATIONAL

#include "integer.hpp"

namespace cpp_multi_precision{
    template<class RadixType, unsigned int RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator = std::allocator<RadixType>>
    class rational{
    public:
        static const std::size_t radix_log2 = RadixLog2;
        static const std::size_t radix2_log2 = RadixLog2 * 2;

        typedef RadixType radix_type;
        typedef Radix2Type radix2_type;
        typedef URadix2Type unsigned_radix2_type;
        typedef typename aux::rebind_container<Container>::template rebind<radix_type, Allocator>::other container_type;
        typedef integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> integer_type;

        rational() : numerator(0), denominator(1){}
        rational(const rational &other){ assign(other); }
        rational(rational &&other){ assign(other); }
        rational(radix_type num){ assign(num); }
        rational(const integer_type &other_integer){ assign(other_integer); }
        rational(integer_type &&other_integer){ assign(other_integer); }
        rational(const integer_type &num, const integer_type &den){ assign(num, den); }
        rational(integer_type &&num, integer_type &&den){ assign(num, den); }
        rational(const integer_type &num, integer_type &&den){ assign(num, den); }
        rational(integer_type &&num, const integer_type &den){ assign(num, den); }
        virtual ~rational(){}

        void assign(const rational &other){ numerator.assign(other.numerator), denominator.assign(other.denominator), sign = other.sign; }
        void assign(rational &&other){ numerator.assign(other.numerator), denominator.assign(other.denominator), assign_sign(), reduce(); }
        void assign(radix_type num){ numerator.assign(num), denominator.assign(1), sign = true; }
        void assign(const integer_type &other_integer){ numerator.assign(other_integer), denominator.assign(1), assign_sign(); }
        void assign(integer_type &&other_integer){ numerator.assign(other_integer), denominator.assign(1), assign_sign(); }
        void assign(const integer_type &num, const integer_type &den){ numerator.assign(num), denominator.assign(den), assign_sign(), reduce(); }
        void assign(integer_type &&num, integer_type &&den){ numerator.assign(num), denominator.assign(den), assign_sign(), reduce(); }
        void assign(const integer_type &num, integer_type &&den){ numerator.assign(num), denominator.assign(den), assign_sign(), reduce(); }
        void assign(integer_type &&num, const integer_type &den){ numerator.assign(num), denominator.assign(den), assign_sign(), reduce(); }

        rational &operator =(const rational &other){
            assign(other);
            return *this;
        }

        rational &operator =(rational &&other){
            assign(other);
            return *this;
        }

        rational &operator +=(const rational &rhs){
            integer_type a;
            integer_type::kar_multi(a, numerator, rhs.denominator);
            numerator.assign(a);
            integer_type::kar_multi(a, rhs.numerator, denominator);
            numerator.sign = sign;
            a.sign = rhs.sign;
            numerator += a;
            integer_type::kar_multi(a, denominator, rhs.denominator);
            denominator.assign(a);
            sign = numerator.sign;
            numerator.sign = true;
            if(numerator == 0){
                sign = true;
                denominator = 1;
            }else{
                reduce();
            }
            return *this;
        }

        rational operator +(const rational &rhs) const{
            rational r(*this);
            r += rhs;
            return std::move(r);
        }

        rational &operator -=(const rational &rhs){
            integer_type a;
            integer_type::kar_multi(a, numerator, rhs.denominator);
            numerator.assign(a);
            integer_type::kar_multi(a, rhs.numerator, denominator);
            numerator.sign = sign;
            a.sign = rhs.sign;
            numerator -= a;
            integer_type::kar_multi(a, denominator, rhs.denominator);
            denominator.assign(a);
            sign = numerator.sign;
            numerator.sign = true;
            if(numerator == 0){
                sign = true;
                denominator = 1;
            }else{
                reduce();
            }
            return *this;
        }

        rational operator -(const rational &rhs) const{
            rational r(*this);
            r -= rhs;
            return std::move(r);
        }

        rational &operator *=(const rational &rhs){
            assign(*this * rhs);
            return *this;
        }

        rational operator *(const rational &rhs) const{
            rational r;
            multi(r, *this, rhs);
            return std::move(r);
        }

        rational &operator /=(const rational &rhs){
            assign(*this / rhs);
            return *this;
        }

        rational operator /(const rational &rhs) const{
            rational r;
            div(r, *this, rhs);
            return std::move(r);
        }

        rational &operator +(){
            return *this;
        }

        rational operator -() const{
            rational r(*this);
            r.sign = !r.sign;
            return std::move(r);
        }

        bool operator <(const rational &rhs) const{
            integer_type a, b;
            if(sign != rhs.sign){ return sign == false; }
            integer_type::kar_multi(a, numerator, rhs.denominator);
            integer_type::kar_multi(b, rhs.numerator, denominator);
            a.sign = sign;
            b.sign = rhs.sign;
            return a < b;
        }

        bool operator <=(const rational &rhs) const{
            integer_type a, b;
            if(sign != rhs.sign){ return sign == false; }
            integer_type::kar_multi(a, numerator, rhs.denominator);
            integer_type::kar_multi(b, rhs.numerator, denominator);
            a.sign = sign;
            b.sign = rhs.sign;
            return a <= b;
        }

        bool operator >(const rational &rhs) const{ return rhs < *this; }
        bool operator >=(const rational &rhs) const{ return rhs <= *this; }
        bool operator ==(const rational &rhs) const{ return numerator == rhs.numerator && denominator == rhs.denominator; }
        bool operator !=(const rational &rhs) const{ return numerator != rhs.numerator || denominator != rhs.denominator; }

        std::string to_string() const{ return to_string_impl<std::string, char>('0', '-', '/', &integer_type::to_string); }
        std::wstring to_wstring() const{ return to_string_impl<std::wstring, wchar_t>(L'0', L'-', L'/', &integer_type::to_wstring); }

        static rational &multi(rational &result, const rational &lhs, const rational &rhs){
            integer_type::kar_multi(result.numerator, lhs.numerator, rhs.numerator);
            integer_type::kar_multi(result.denominator, lhs.denominator, rhs.denominator);
            result.reduce();
            if(result.numerator == 0){
                result.sign = true;
            }else{
                result.sign = lhs.sign == rhs.sign;
            }
            return result;
        }

        static rational &div(rational &result, const rational &lhs, const rational &rhs){
            integer_type::kar_multi(result.numerator, lhs.numerator, rhs.denominator);
            integer_type::kar_multi(result.denominator, lhs.denominator, rhs.numerator);
            result.reduce();
            if(result.numerator == 0){
                result.sign = true;
            }else{
                result.sign = lhs.sign == rhs.sign;
            }
            return result;
        }

        void inverse(){
            integer_type temp(numerator);
            numerator.assign(denominator);
            denominator.assign(temp);
        }

        void radix_shift(std::size_t n){
            numerator.radix_shift(n);
            reduce();
        }

        std::size_t deg() const{
            return numerator.deg() - denominator.deg();
        }

        rational lc() const{
            return rational(numerator.lc(), denominator.lc());
        }

        const integer_type &get_numerator() const{ return numerator; }
        const integer_type &get_denominator() const{ return denominator; }

    private:
        void reduce(){
            if(numerator == 0){ denominator = 1; return; }
            integer_type gcd, q;
            integer_type::gcd(gcd, numerator, denominator);
            integer_type::div(q, numerator, gcd);
            numerator.assign(q);
            integer_type::div(q, denominator, gcd);
            denominator.assign(q);
        }

        void assign_sign(){
            sign = numerator.sign == denominator.sign;
            numerator.sign = denominator.sign = true;
        }

        template<class Str, class Char, class IntegerToString>
        Str to_string_impl(Char zero, Char mn, Char dv, IntegerToString integer_to_string) const{
            Str result;
            if(numerator == 0){ result += zero; return std::move(result); }
            if(sign == false){ result += mn; }
            result += (numerator.*integer_to_string)();
            if(denominator == 1){ return std::move(result); }
            result += dv;
            result += (denominator.*integer_to_string)();
            return std::move(result);
        }

    private:
        integer_type numerator, denominator;

    public:
        aux::sign sign;
    };

    template<class RadixType, unsigned int RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator>
    rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>
    operator +(
        const typename rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>::integer_type &lhs,
        const rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> &rhs
    ){ return rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>(lhs) + rhs; }

    template<class RadixType, unsigned int RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator>
    rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>
    operator -(
        const typename rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>::integer_type &lhs,
        const rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> &rhs
    ){ return rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>(lhs) - rhs; }

    template<class RadixType, unsigned int RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator>
    rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>
    operator *(
        const typename rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>::integer_type &lhs,
        const rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> &rhs
    ){ return rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>(lhs) * rhs; }

    template<class RadixType, unsigned int RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator>
    rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>
    operator /(
        const typename rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>::integer_type &lhs,
        const rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> &rhs
    ){ return rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>(lhs) / rhs; }

    template<class RadixType, unsigned int RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator>
    bool operator <(
        const typename rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>::integer_type &lhs,
        const rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> &rhs
    ){ return rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>(lhs) < rhs; }

    template<class RadixType, unsigned int RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator>
    bool operator >(
        const typename rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>::integer_type &lhs,
        const rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> &rhs
    ){ return rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>(lhs) > rhs; }

    template<class RadixType, unsigned int RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator>
    bool operator <=(
        const typename rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>::integer_type &lhs,
        const rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> &rhs
    ){ return rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>(lhs) <= rhs; }

    template<class RadixType, unsigned int RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator>
    bool operator >=(
        const typename rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>::integer_type &lhs,
        const rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> &rhs
    ){ return rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>(lhs) >= rhs; }

    template<class RadixType, unsigned int RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator>
    bool operator ==(
        const typename rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>::integer_type &lhs,
        const rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> &rhs
    ){ return rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>(lhs) == rhs; }

    template<class RadixType, unsigned int RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator>
    bool operator !=(
        const typename rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>::integer_type &lhs,
        const rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> &rhs
    ){ return rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>(lhs) != rhs; }

    template<class RadixType, std::size_t RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator>
    std::ostream &operator <<(
        std::ostream &ostream,
        const rational<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> &value
    ){
        ostream << value.to_string();
        return ostream;
    }
}

#endif
