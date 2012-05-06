#ifndef HPP_CPP_MULTI_PRECISION_INTEGER
#define HPP_CPP_MULTI_PRECISION_INTEGER

#include "unsigned_integer.hpp"

namespace cpp_multi_precision{
    template<class RadixType, unsigned int RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator = std::allocator<RadixType>>
    class integer : public unsigned_integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>{
    public:
        static const std::size_t radix_log2 = RadixLog2;
        static const std::size_t radix2_log2 = RadixLog2 * 2;

        typedef RadixType radix_type;
        typedef Radix2Type radix2_type;
        typedef URadix2Type unsigned_radix2_type;
        typedef unsigned_integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> unsigned_integer_type;
        typedef unsigned_integer_type base_type;
        typedef typename unsigned_integer_type::container_type container_type;
        typedef Allocator allocator_type;

        integer() : unsigned_integer_type(), sign(true){}
        integer(radix2_type value) : unsigned_integer_type(static_cast<unsigned_radix2_type>(value < 0 ? -value : value)), sign(value >= 0){}
        integer(const integer &other) : unsigned_integer_type(other.container), sign(other.sign){}
        integer(integer &&other) : unsigned_integer_type(other), sign(other.sign){}
        integer(const container_type &other_container, bool sign_ = true) : unsigned_integer_type(other_container), sign(sign_){}
        template<class Iterator>
        integer(const Iterator &first, const Iterator &last, bool sign_ = true) : unsigned_integer_type(first, last), sign(sign_){}
        virtual ~integer(){}

        void assign(const unsigned_integer_type &other){
            unsigned_integer_type::assign(other);
            sign = true;
        }

        void assign(radix2_type v){
            if(v < 0){
                unsigned_integer_type::assign(static_cast<unsigned_radix2_type>(-v));
                sign = false;
            }else{
                unsigned_integer_type::assign(static_cast<unsigned_radix2_type>(v));
                sign = true;
            }
        }

        void assign(const integer &other){
            unsigned_integer_type::assign(static_cast<const unsigned_integer_type&>(other));
            sign = other.sign;
        }

        void assign(integer &&other){
            unsigned_integer_type::assign(static_cast<unsigned_integer_type&&>(other));
            sign = other.sign;
        }

        void assign_double(double v){
            if(v < 1.0){ assign(0); return; }
            unsigned_integer_type::container.clear();
            for(; v >= 1.0; v /= static_cast<double>(static_cast<radix2_type>(1) << radix_log2)){
                radix_type r = static_cast<radix_type>(std::fmod(v, static_cast<double>(static_cast<radix2_type>(1) << radix_log2)));
                unsigned_integer_type::container.push_front(0);
                *this += r;
            }
        }

        template<class Iterator>
        void assign(const Iterator &first, const Iterator &last){
            unsigned_integer_type::assign(first, last);
            sign = true;
        }

        void read(const char *str){
            read_impl<const char*, char>(str, '0', '9', '+', '-', 0);
        }

        void read(const wchar_t *str){
            read_impl<const wchar_t*, wchar_t>(str, L'0', L'9', L'+', L'-', 0);
        }

        void read(const std::string &str){
            read_impl<std::string::const_iterator, char>(str.begin(), '0', '9', '+', '-', str.end());
        }

        void read(const std::wstring &str){
            read_impl<std::wstring::const_iterator, wchar_t>(str.begin(), L'0', L'9', L'+', L'-', str.end());
        }

        std::string to_string() const{
            return to_string_impl<std::string, char>('0', '-');
        }

        std::wstring to_wstring() const{
            return to_string_impl<std::wstring, wchar_t>(L'0', L'-');
        }

        integer &operator =(const integer &other){
            assign(other);
            return *this;
        }

        integer &operator =(integer &&other){
            assign(other);
            return *this;
        }

        integer &operator +=(const integer &rhs){
            unsigned_integer_type &ulhs(*this);
            const unsigned_integer_type &urhs(rhs);
            if(sign == rhs.sign){
                ulhs += urhs;
            }else{
                if(ulhs < urhs){
                    unsigned_integer_type temp(ulhs);
                    ulhs.assign(rhs);
                    ulhs -= temp;
                    sign = !sign;
                }else{
                    ulhs -= rhs;
                    if(ulhs == 0){ sign = true; }
                }
            }
            return *this;
        }

        integer operator +(const integer &rhs) const{
            integer r(*this);
            r += rhs;
            return std::move(r);
        }

        integer &operator -=(const integer &rhs){
            unsigned_integer_type &ulhs(*this);
            const unsigned_integer_type &urhs(rhs);
            if(sign != rhs.sign){
                ulhs += urhs;
            }else{
                if(ulhs < urhs){
                    unsigned_integer_type temp(ulhs);
                    ulhs.assign(rhs);
                    ulhs -= temp;
                    sign = !sign;
                }else{
                    ulhs -= rhs;
                    if(ulhs == 0){ sign = true; }
                }
            }
            return *this;
        }

        integer operator -(const integer &rhs) const{
            integer r(*this);
            r -= rhs;
            return std::move(r);
        }

        integer operator *(const integer &rhs) const{
            integer r;
            kar_multi(r, *this, rhs);
            return std::move(r);
        }

        integer &operator *=(const integer &rhs){
            assign(*this * rhs);
            return *this;
        }

        integer operator /(const integer &rhs) const{
            integer r;
            div(r, *this, rhs);
            return std::move(r);
        }

        integer &operator /=(const integer &rhs){
            assign(*this / rhs);
            return *this;
        }

        integer operator %(const integer &rhs) const{
            integer quo, rem;
            div(quo, rem, *this, rhs);
            return std::move(rem);
        }

        integer operator %=(const integer &rhs){
            assign(*this % rhs);
            return *this;
        }

        integer &operator <<=(std::size_t n){
            unsigned_integer_type::operator <<=(n);
            return *this;
        }

        integer operator <<(std::size_t n) const{
            integer r(*this);
            r <<= n;
            return std::move(r);
        }

        integer &operator >>=(std::size_t n){
            unsigned_integer_type::operator >>=(n);
            return *this;
        }

        integer operator >>(std::size_t n) const{
            integer r(*this);
            r >>= n;
            return std::move(r);
        }

        integer &operator +(){
            return *this;
        }

        integer operator -() const{
            integer r(*this);
            r.sign = !r.sign;
            return std::move(r);
        }

        bool operator <(const integer &rhs) const{
            if(sign == rhs.sign){
                if(sign == true){
                    return static_cast<const unsigned_integer_type&>(*this).operator <(rhs);
                }else{
                    return static_cast<const unsigned_integer_type&>(*this).operator >(rhs);
                }
            }else if(sign == true){
                return false;
            }else{
                return true;
            }
        }

        bool operator >(const integer &rhs) const{
            return rhs < *this;
        }

        bool operator <=(const integer &rhs) const{
            if(sign == rhs.sign){
                if(sign == true){
                    return static_cast<const unsigned_integer_type&>(*this).operator <=(rhs);
                }else{
                    return static_cast<const unsigned_integer_type&>(*this).operator >=(rhs);
                }
            }else if(sign == true){
                return false;
            }else{
                return true;
            }
        }

        bool operator >=(const integer &rhs) const{
            return rhs <= *this;
        }

        bool operator ==(const integer &rhs) const{
            const unsigned_integer_type &ulhs(*this);
            const unsigned_integer_type &urhs(rhs);
            return sign == rhs.sign && ulhs == urhs;
        }

        bool operator !=(const integer &rhs) const{
            const unsigned_integer_type &ulhs(*this);
            const unsigned_integer_type &urhs(rhs);
            return sign != rhs.sign || ulhs != urhs;
        }

    private:
        struct kar_pair{
            kar_pair(typename container_type::iterator f, typename container_type::iterator s, std::size_t n) : first(f), second(s), size(n){}
            const typename container_type::iterator first, second;
            const std::size_t size;
        };

        struct kar_const_pair{
            kar_const_pair(typename container_type::const_iterator f, typename container_type::const_iterator s, std::size_t n) : first(f), second(s), size(n){}
            const typename container_type::const_iterator first, second;
            const std::size_t size;
        };

    public:
        static integer &multi(integer &result, const integer &lhs, radix_type rhs){
            result.sign = lhs.sign;
            unsigned_integer_type::multi(result, lhs, rhs);
            return result;
        }

        static integer &multi(integer &result, radix_type x, radix_type y){
            result.sign = true;
            unsigned_integer_type::multi(result, x, y);
            return result;
        }

        static integer &square_multi(integer &result, const integer &lhs, const integer &rhs){
            result.sign = lhs.sign == rhs.sign;
            unsigned_integer_type::square_multi(result, lhs, rhs);
            return result;
        }

        static integer &square_multi(
            integer &result,
            const typename container_type::const_iterator &lhs_begin, const typename container_type::const_iterator &lhs_end,
            const typename container_type::const_iterator &rhs_begin, const typename container_type::const_iterator &rhs_end,
            std::size_t lhs_size, std::size_t rhs_size
        ){
            result.sign = true;
            unsigned_integer_type::square_multi(result, lhs_begin, lhs_end, rhs_begin, rhs_end, lhs_size, rhs_size);
            return result;
        }

        static integer &kar_multi(integer &result, const integer &lhs, const integer &rhs){
            result.assign(0);
            aux::reserve_dispatch(result.container, lhs.container.size() + rhs.container.size());
            kar_multi_impl(
                result,
                kar_const_pair(lhs.container.begin(), lhs.container.end(), lhs.container.size()),
                kar_const_pair(rhs.container.begin(), rhs.container.end(), rhs.container.size())
            );
            if(static_cast<unsigned_integer_type&>(result) == 0){
                result.sign = true;
            }else{
                result.sign = lhs.sign == rhs.sign;
            }
            return result;
        }

    private:
        static std::size_t mshift_left(std::size_t x, std::size_t i){
            if(i < sizeof(std::size_t) * 8){
                return x << i;
            }else{
                return 0;
            }
        }

        static std::size_t mshift_right(std::size_t x, std::size_t i){
            if(i < sizeof(std::size_t) * 8){
                return x >> i;
            }else{
                return 0;
            }
        }

    private:
        template<bool Rem>
        static integer &div_impl(integer &result, integer &rem, const integer &lhs, const integer &rhs){
            if(lhs == 0){
                result = 0, rem = 0;
                return result;
            }
            monic_div_impl<Rem>(result, rem, lhs, rhs);
            if(static_cast<unsigned_integer_type&>(result) == 0){
                result.sign = true;
            }else{
                result.sign = lhs.sign == rhs.sign;
            }
            if(static_cast<unsigned_integer_type&>(rem) == 0){ rem.sign = true; }
            return result;
        }

    public:
        static integer &div(integer &result, integer &rem, const integer &lhs, const integer &rhs){
            return div_impl<true>(result, rem, lhs, rhs);
        }

        static integer &div(integer &result, const integer &lhs, const integer &rhs){
            integer rem;
            return div_impl<false>(result, rem, lhs, rhs);
        }

        static integer &mod(integer &rem, const integer &lhs, const integer &rhs){
            integer result;
            div_impl<true>(result, rem, lhs, rhs);
            return rem;
        }

        static integer &pow(integer &result, const integer &x, const integer &y){
            if(y.sign == true){
                if(y == 0){ result.assign(1); }else{
                    unsigned_integer_type::template pow_impl<false>(result, x, y, kar_multi);
                    if(!x.sign && (x.container[0] & 1) == 0){
                        result.sign = true;
                    }
                }
            }else{
                result.assign(0);
            }
            return result;
        }

        static integer &pow_mod(integer &result, const integer &x, const integer &y, const integer &m){
            if(y.sign == true){
                if(y == 0){ result.assign(1); }else{
                    unsigned_integer_type::template pow_impl<true>(result, x, y, kar_multi, m);
                    if(!x.sign && (x.container[0] & 1) == 0){
                        result.sign = true;
                    }
                }
            }else{
                result.assign(0);
            }
            return result;
        }

        static integer &gcd(integer &result, const integer &lhs, const integer &rhs){
            const unsigned_integer_type &ulhs(lhs), &urhs(rhs);
            if(ulhs >= urhs){
                return gcd_impl(result, lhs, rhs);
            }else{
                return gcd_impl(result, rhs, lhs);
            }
        }

        template<class MContainer, class VContainer>
        static integer &cra(integer &result, const MContainer &m_container, const VContainer& v_container){
            integer prod_m(1), &c_sum(result);
            c_sum.assign(0);
            for(typename MContainer::const_iterator iter = m_container.begin(), end = m_container.end(); iter != end; ++iter){
                integer temp(prod_m);
                multi(prod_m, temp, *iter);
            }
            typename VContainer::const_iterator v_iter = v_container.begin();
            for(typename MContainer::const_iterator m_iter = m_container.begin(), end = m_container.end(); m_iter != end; ++m_iter, ++v_iter){
                integer rhs(*m_iter), d, s, v(*v_iter), c_temp;
                div(d, prod_m, rhs);
                eea(integer(), s, integer(), d, rhs);
                integer vs = s * v;
                c_temp = vs * rhs;
                if(!c_temp.sign){
                    unsigned_integer_type &u_temp(c_temp), urhs(rhs);
                    urhs -= u_temp;
                    u_temp.assign(urhs);
                    c_temp.sign = true;
                }
                c_sum += c_temp * d;
            }
            return result;
        }

        radix_type lc() const{ return unsigned_integer_type::lc(); }

        static integer &normal(integer &result, const integer &x){
            if(x == 0){
                result = 0;
                return result;
            }
            unsigned_integer_type::normal(result, x);
            result.sign = true;
            return result;
        }

        void normalize(){
            unsigned_integer_type::normalize();
            sign = true;
        }

        void lu(){
            if(*this == 0){
                *this = 1;
                return;
            }
            integer a(*this), na;
            normal(na, a);
            div(*this, a, na);
        }

        radix_type infinity_norm() const{ return unsigned_integer_type::infinity_norm(); }

        integer &norm1(integer &result) const{
            unsigned_integer_type::norm1(result);
            result.sign = true;
            return result;
        }

        radix_type cont() const{ return unsigned_integer_type::cont(); }

        static integer &pp(integer &result, const integer &x){
            unsigned_integer_type::pp(result, x);
            result.sign = true;
            return result;
        }

    private:
        static void kar_multi_impl(integer &result, kar_const_pair x, kar_const_pair y){
            std::size_t n = aux::ceil_pow2((std::max)(x.size, y.size));
            if(n < 2){
                unsigned_integer_type &uresult(result);
                unsigned_integer_type::square_multi(uresult, x.first, x.second, y.first, y.second, x.size, y.size);
                return;
            }
            n /= 2;
            std::size_t xn = x.size < n ? x.size : n, yn = y.size < n ? y.size : n;
            kar_const_pair x1(x.first + xn, x.second, x.size - xn), y1(y.first + yn, y.second, y.size - yn);
            kar_const_pair x0(x.first, x.first + xn, xn), y0(y.first, y.first + yn, yn);
            integer z2, z0;
            {
                integer tx, ty;
                if(x1.first != x1.second){
                    tx.assign(x1.first, x1.second), tx.sub_iterator_n(true, x0.first, x0.second, 0);
                }else{
                    tx.assign(x0.first, x0.second), tx.sign = false;
                }
                if(y1.first != y1.second){
                    ty.assign(y1.first, y1.second), ty.sub_iterator_n(true, y0.first, y0.second, 0);
                }else{
                    ty.assign(y0.first, y0.second), ty.sign = false;
                }
                kar_multi_impl(result, kar_const_pair(tx.container.begin(), tx.container.end(), tx.container.size()), kar_const_pair(ty.container.begin(), ty.container.end(), ty.container.size()));
                result.sign = tx.sign != ty.sign;
            }
            kar_multi_impl(z0, x0, y0);
            (z0.*(&integer::normalize_container_impl))();
            result += z0;
            if(x.size >= n && y.size >= n){
                kar_multi_impl(z2, x1, y1);
                result += z2;
            }
            result.radix_shift(n);
            result += z0;
            if(x.size >= n && y.size >= n){
                result.add_radix_n(z2, n * 2);
            }
            (result.*(&integer::normalize_container_impl))();
        }

        void sub_iterator_n(bool rhs_sign, const typename container_type::const_iterator &rhs_first, typename container_type::const_iterator rhs_last, std::size_t n){
            unsigned_integer_type &ulhs(*this);
            if(sign != rhs_sign){
                unsigned_integer_type::add_iterator_n(rhs_first, rhs_last, n);
            }else{
                if(unsigned_integer_type::less(rhs_first, rhs_last)){
                    unsigned_integer_type temp(ulhs);
                    ulhs.assign(rhs_first, rhs_last);
                    unsigned_integer_type::radix_shift(n);
                    ulhs -= temp;
                    sign = !sign;
                }else{
                    std::size_t n = std::distance(rhs_first, rhs_last);
                    if(n > this->container.size()){
                        this->unsigned_integer_type::sub_range(rhs_first, rhs_first + this->container.size());
                    }else{
                        this->unsigned_integer_type::sub_range(rhs_first, rhs_last);
                    }
                    if(ulhs == 0){ sign = true; }
                }
            }
        }

        void sub_n(const integer &rhs, std::size_t n){
            sub_iterator_n(rhs.sign, rhs.container.begin(), rhs.container.end(), n);
        }

    private:
        template<bool Rem>
        static integer &monic_div_impl(integer &result, integer &rem, const integer &lhs, const integer &rhs){
            if(!rhs.is_monic()){
                unsigned_integer_type::div(result, rem, lhs, rhs);
                return result;
            }
            result = 0;
            if(rhs.container.size() > lhs.container.size()){
                if(Rem){ rem = lhs; }
                return result;
            }
            std::size_t m = lhs.container.size() - rhs.container.size() + 1;
            integer inv_rev_rhs;
            {
                integer rev_rhs(rhs);
                rev_rhs.rev();
                inverse(inv_rev_rhs, rev_rhs, m);
            }
            {
                integer rev_lhs(lhs);
                rev_lhs.rev();
                result = rev_lhs * std::move(inv_rev_rhs);
            }
            if(result.container.size() > m){ result.container.resize(m); }
            result.rev();
            if(Rem){ rem = lhs - rhs * result; }
            return result;
        }

    public:
        static integer &gcd_impl(integer &result, const integer &lhs, const integer &rhs){
            unsigned_integer_type::gcd_impl(result, lhs, rhs);
            result.sign = true;
            return result;
        }

        static integer &inverse(integer &result, const integer &f, std::size_t l){
            integer &g(result);
            g = 1;
            std::size_t r = aux::ceil_log2(l), rem = 1;
            for(std::size_t i = 0; i < r; ++i){
                rem <<= 1;
                integer next_g(g);
                for(
                    typename container_type::iterator double_g_iter = next_g.container.begin(), double_g_end = next_g.container.end();
                    double_g_iter != double_g_end;
                    ++double_g_iter
                ){ *double_g_iter = *double_g_iter * 2; }
                next_g -= f * g * g;
                if(next_g.container.size() >= rem){ next_g.container.resize(rem - 1); }
                g = std::move(next_g);
            }
            return result;
        }

    private:
        template<class Ptr, class Char>
        void read_impl(Ptr str, int zero, int nine, int plus, int minus, const Ptr &delim = Ptr()){
            sign = true;
            Char c = *str;
            if(c == plus){ ++str; }else if(c == minus){ sign = false, ++str; }
            unsigned_integer_type::template read_impl<Ptr, Char>(str, zero, nine, delim);
        }

        template<class String, class Char>
        String to_string_impl(int zero, int minus) const{
            String str(unsigned_integer_type::template to_string_impl<String, Char>(zero));
            if(sign == false){ str.insert(str.begin(), static_cast<Char>(minus)); }
            return str;
        }

    public:
        aux::sign sign;
    };

    template<class RadixType, unsigned int RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator>
    integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>
    operator +(
        typename integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>::radix2_type lhs,
        const integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> &rhs
    ){
        integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> result(lhs);
        result += rhs;
        return std::move(result);
    }

    template<class RadixType, unsigned int RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator>
    integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>
    operator -(
        typename integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>::radix2_type lhs,
        const integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> &rhs
    ){
        integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> result(lhs);
        result -= rhs;
        return std::move(result);
    }

    template<class RadixType, unsigned int RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator>
    integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>
    operator /(
        typename integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>::radix2_type lhs,
        const integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> &rhs
    ){  return integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>(lhs) / rhs; }

    template<class RadixType, unsigned int RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator>
    integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>
    operator *(
        typename integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>::radix2_type lhs,
        const integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> &rhs
    ){  return integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>(lhs) * rhs; }

    template<class RadixType, unsigned int RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator>
    integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>
    operator %(
        typename integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>::radix2_type lhs,
        const integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> &rhs
    ){  return integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>(lhs) % rhs; }

    template<class RadixType, unsigned int RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator>
    bool operator <(
        typename integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>::radix2_type lhs,
        const integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> &rhs
    ){  return integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>(lhs) < rhs; }

    template<class RadixType, unsigned int RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator>
    bool operator >(
        typename integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>::radix2_type lhs,
        const integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> &rhs
    ){  return integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>(lhs) > rhs; }

    template<class RadixType, unsigned int RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator>
    bool operator <=(
        typename integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>::radix2_type lhs,
        const integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> &rhs
    ){  return integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>(lhs) <= rhs; }

    template<class RadixType, unsigned int RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator>
    bool operator >=(
        typename integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>::radix2_type lhs,
        const integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> &rhs
    ){  return integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>(lhs) >= rhs; }

    template<class RadixType, unsigned int RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator>
    bool operator ==(
        typename integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>::radix2_type lhs,
        const integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> &rhs
    ){  return integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>(lhs) == rhs; }

    template<class RadixType, unsigned int RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator>
    bool operator !=(
        typename integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>::radix2_type lhs,
        const integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> &rhs
    ){  return integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator>(lhs) != rhs; }

    template<class RadixType, std::size_t RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator>
    std::ostream &operator <<(
        std::ostream &ostream,
        const integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> &value
    ){
        ostream << value.to_string();
        return ostream;
    }

    template<class RadixType, std::size_t RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator>
    std::wostream &operator <<(
        std::wostream &ostream,
        const integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> &value
    ){
        ostream << value.to_wstring();
        return ostream;
    }
}

#endif
