#ifndef HPP_CPP_MULTI_PRECISION_UNSIGNED_INTEGER
#define HPP_CPP_MULTI_PRECISION_UNSIGNED_INTEGER

#include "ns_aux.hpp"

namespace cpp_multi_precision{
    template<class RadixType, std::size_t RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator = std::allocator<int>>
    class unsigned_integer{
    public:
        static const std::size_t radix_log2 = RadixLog2, radix2_log2 = RadixLog2 * 2;

        typedef RadixType radix_type;
        typedef Radix2Type radix2_type;
        typedef URadix2Type unsigned_radix2_type;
        typedef typename aux::rebind_container<Container>::template rebind<radix_type, typename Allocator::template rebind<radix_type>::other>::other container_type;

        unsigned_integer() : normalize_container(&unsigned_integer::normalize_container_impl){ assign(0); }
        unsigned_integer(unsigned_radix2_type value) : normalize_container(&unsigned_integer::normalize_container_impl){ assign(value); }
        unsigned_integer(const unsigned_integer &other) : normalize_container(other.normalize_container), container(other.container){}
        unsigned_integer(unsigned_integer &&other) : normalize_container(other.normalize_container), container(other.container){}
        unsigned_integer(const container_type &other_container) : normalize_container(&unsigned_integer::normalize_container_impl), container(other_container){}
        template<class Iterator>
        unsigned_integer(const Iterator &first, const Iterator &last) : normalize_container(&unsigned_integer::normalize_container_impl){ assign(first, last); }

        virtual ~unsigned_integer(){}

        void assign(const unsigned_integer &other){
            container.assign(other.container.begin(), other.container.end());
        }

        void assign(unsigned_integer &&other){
            container = other.container;
        }

        void assign(unsigned_radix2_type v){
            if((v >> radix_log2)){
                container.resize(2);
                container.front() = static_cast<radix_type>(v & ~static_cast<radix_type>(0));
                container.back() = static_cast<radix_type>(v >> radix_log2);
            }else{
                container.resize(1);
                container.front() = static_cast<radix_type>(v);
            }
        }

        template<class Iterator>
        void assign(const Iterator &first, const Iterator &last){
            if(first == last){ assign(0); return; }
            std::size_t size = std::distance(first, last);
            if(container.size() == size){
                Iterator iter = first;
                for(std::size_t i = 0; i < size; ++i, ++iter){
                    container[i] = *iter;
                }
            }else{
                container.assign(first, last);
            }
            (this->*normalize_container)();
        }

        void read(const char *str){
            read_impl<const char*, char>(str, '0', '9', 0);
        }

        void read(const wchar_t *str){
            read_impl<const wchar_t*, wchar_t>(str, L'0', L'9', 0);
        }

        void read(const std::string &str){
            read_impl<std::string::const_iterator, char>(str.begin(), '0', '9', str.end());
        }

        void read(const std::wstring &str){
            read_impl<std::wstring::const_iterator, wchar_t>(str.begin(), L'0', L'9', str.end());
        }

        std::string to_string() const{
            return to_string_impl<std::string, char>('0');
        }

        std::wstring to_wstring() const{
            return to_string_impl<std::wstring, wchar_t>(L'0');
        }

        unsigned_integer &operator =(const unsigned_integer &rhs){
            container = rhs.container;
            return *this;
        }

        unsigned_integer &operator =(unsigned_integer &&rhs){
            container = std::move(rhs.container);
            return *this;
        }

        unsigned_integer &operator +=(const unsigned_integer &rhs){
            return add_range(rhs.container.begin(), rhs.container.end());
        }

        unsigned_integer operator +(const unsigned_integer &rhs) const{
            unsigned_integer r(*this);
            r += rhs;
            return std::move(r);
        }

        unsigned_integer &operator -=(const unsigned_integer &rhs){
            return sub_range(rhs.container.begin(), rhs.container.end());
        }

        unsigned_integer operator -(const unsigned_integer &rhs) const{
            unsigned_integer r(*this);
            r -= rhs;
            return std::move(r);
        }

        unsigned_integer operator *(const unsigned_integer &rhs) const{
            unsigned_integer r;
            square_multi(r, *this, rhs);
            return std::move(r);
        }

        unsigned_integer &operator *=(const unsigned_integer &rhs){
            assign(std::move(*this * rhs));
            return *this;
        }

        unsigned_integer operator /(const unsigned_integer &rhs) const{
            unsigned_integer quo;
            div(quo, *this, rhs);
            return std::move(quo);
        }

        unsigned_integer &operator /=(const unsigned_integer &rhs){
            assign(std::move(*this / rhs));
            return *this;
        }

        unsigned_integer operator %(const unsigned_integer &rhs) const{
            unsigned_integer quo, rem;
            div(quo, rem, *this, rhs);
            return std::move(rem);
        }

        unsigned_integer &operator %=(const unsigned_integer &rhs){
            assign(*this % rhs);
            return *this;
        }

        unsigned_integer &operator <<=(std::size_t n){
            bit_shift_lsr(n);
            return *this;
        }

        unsigned_integer operator <<(std::size_t n) const{
            unsigned_integer r(*this);
            r <<= n;
            return std::move(r);
        }

        unsigned_integer &operator >>=(std::size_t n){
            bit_shift_rsl(n);
            return *this;
        }

        unsigned_integer operator >>(std::size_t n) const{
            unsigned_integer r(*this);
            r >>= n;
            return std::move(r);
        }

        bool operator <(const unsigned_integer &rhs) const{ return less(rhs.container.begin(), rhs.container.end()); }
        bool operator <=(const unsigned_integer &rhs) const{ return less_eq(rhs.container.begin(), rhs.container.end()); }
        bool operator >(const unsigned_integer &rhs) const{ return rhs < *this; }
        bool operator >=(const unsigned_integer &rhs) const{ return rhs <= *this; }

        bool operator ==(const radix_type &v) const{
            return container.size() == 1 && container.front() == v;
        }

        bool operator ==(const unsigned_integer &rhs) const{
            if(container.size() != rhs.container.size()){ return false; }
            for(typename container_type::const_iterator it = container.begin(), end = container.end(), rhs_it = rhs.container.begin(); it != end; ++it, ++rhs_it){
                if(*it != *rhs_it){ return false; }
            }
            return true;
        }

        bool operator !=(const radix_type &v) const{
            return container.size() != 1 || container.front() != v;
        }

        bool operator !=(const unsigned_integer &rhs) const{
            if(container.size() != rhs.container.size()){ return true; }
            for(typename container_type::const_iterator it = container.begin(), end = container.end(), rhs_it = rhs.container.begin(); it != end; ++it, ++rhs_it){
                if(*it != *rhs_it){ return true; }
            }
            return false;
        }

    public:
        static unsigned_integer &div(unsigned_integer &result, unsigned_integer &rem, const unsigned_integer &lhs, const unsigned_integer &rhs){
            result.assign(lhs);
            result.div_impl(rhs, rem);
            return result;
        }

        void radix_shift(std::size_t n){
            container.insert(container.begin(), n, 0);
        }

        std::size_t deg() const{
            if(*this == 0){
                return 0;
            }else{
                return container.size();
            }
        }

        radix_type lc() const{ return container.back(); }

        static unsigned_integer &normal(unsigned_integer &result, const unsigned_integer &x){
            result.container = x.container;
            const radix_type &lc_value(x.lc());
            for(typename container_type::iterator iter = result.container.begin(), end = result.container.end(); iter != end; ++iter){
                *iter /= lc_value;
            }
            return result;
        }

        void normalize(){
            if(container.size() == 0){ return; }
            radix_type lc_value(lc());
            for(typename container_type::iterator iter = container.begin(), end = container.end(); iter != end; ++iter){
                *iter /= lc_value;
            }
        }

        radix_type infinity_norm() const{
            const radix_type r = 0;
            for(typename container_type::const_iterator iter = container.begin(), end = container.end(); iter != end; ++iter){
                const radix_type &iter_coe(*iter);
                if(iter_coe > r){ r = iter_coe; }
            }
            return r;
        }

        unsigned_integer &norm1(unsigned_integer &result) const{
            result.assign(0);
            for(typename container_type::const_iterator iter = container.begin(), end = container.end(); iter != end; ++iter){
                result += *iter;
            }
            return result;
        }

        radix_type cont() const{
            if(container.size() == 1){ return container.front(); }
            radix_type a = container.front();
            typename container_type::const_iterator iter = container.begin();
            ++iter;
            for(typename container_type::const_iterator end = container.end(); iter != end; ++iter){
                if(a == 1){ break; }
                a = gcd(a, *iter);
            }
            return a;
        }

        void rev(){
            std::reverse(container.begin(), container.end());
            (this->*normalize_container)();
        }

        static unsigned_integer &ceil_pow2(unsigned_integer &result, const unsigned_integer &a){
            std::size_t n = 0, m = 0, floor_log2 = 0;
            typename container_type::const_reverse_iterator a_iter = a.container.rbegin(); ++a_iter;
            for(typename container_type::const_reverse_iterator a_end = a.container.rend(); a_iter != a_end; ++a_iter){
                if(*a_iter > 0){ ++n; }
            }
            for(radix_type x = a.container.back(); x > 0; x >>= 1){
                if((x & 1) == 1){ ++m; }
                ++floor_log2;
            }
            if(n > 0 || m > 1){
                if(floor_log2 == (radix_log2 - 1)){
                    result.container.resize(a.container.size() + 1);
                    result.container.back() = 1;
                }else{
                    result.container.resize(a.container.size());
                    result.container.back() = 1 << (floor_log2);
                }
            }else{
                result.assign(a);
            }
            return result;
        }

        void ceil_pow2(){
            unsigned_integer a(*this);
            ceil_pow2(*this, a);
        }

        std::size_t ceil_log2() const{
            radix_type x = container.back();
            std::size_t n = aux::index_of_leftmost_flag(x);
            if((x & aux::ceil_pow2(x)) != 0){
                return (container.size() - 1) * radix_log2 + n + 1;
            }
            std::size_t m = 0;
            for(typename container_type::const_iterator iter = container.begin(), end = container.end() - 1; iter != end; ++iter){
                if(*iter != 0){
                    m = 1;
                    break;
                }
            }
            return (container.size() - 1) * radix_log2 + n + m;
        }

        static unsigned_integer &pp(unsigned_integer &result, const unsigned_integer &x){
            unsigned_integer y(x.cont());
            div(result, x, y);
            return result;
        }

        static radix_type gcd(radix_type a, radix_type b){
            if(a < b){ std::swap(a, b); }
            if(b == 1){ return b; }
            radix_type c;
            for(; ; ){
                c = a % b;
                if(c == 0){ return b; }
                a = b, b = c;
            }
            return c;
        }

        static unsigned_integer &gcd(unsigned_integer &result, const unsigned_integer &lhs, const unsigned_integer &rhs){
            if(lhs >= rhs){
                return gcd_impl(result, lhs, rhs);
            }else{
                return gcd_impl(result, rhs, lhs);
            }
        }

        const container_type &get_container() const{ return container; }

        static unsigned_integer &multi(unsigned_integer &result, const unsigned_integer &lhs, unsigned_radix2_type rhs){
            return multi(result, lhs.container.begin(), lhs.container.end(), rhs);
        }

        static unsigned_integer &multi(unsigned_integer &result, typename container_type::const_iterator lhs_it, const typename container_type::const_iterator &lhs_end, unsigned_radix2_type rhs){
            result.assign(0);
            std::size_t m = 0;
            for(; lhs_it != lhs_end; ++lhs_it, ++m){
                radix_type lhs = *lhs_it;
                result.add_radix_n(static_cast<unsigned_radix2_type>(lhs) * rhs, m);
            }
            return result;
        }

        static unsigned_integer &multi(unsigned_integer &result, radix_type x, radix_type y){
            result.assign(0);
            unsigned_radix2_type r = static_cast<unsigned_radix2_type>(x) * static_cast<unsigned_radix2_type>(y);
            result.container.front() = static_cast<unsigned_radix2_type>(r % (static_cast<unsigned_radix2_type>(1) << radix_log2));
            r >>= radix_log2;
            if(r > 0){ result.container.push_back(static_cast<radix_type>(r)); }    
            return result;
        }

        static unsigned_integer &pow(unsigned_integer &result, const unsigned_integer &x, const unsigned_integer &y){
            return pow_impl<unsigned_integer>(result, x, y, square_multi);
        }

        radix_type n_bit(std::size_t n) const{ return (container[n / radix_log2] >> (n % radix_log2)) & 1; }
        std::size_t bit_num() const{
            std::size_t i = 0;
            radix_type t = container.back();
            while(t >>= 1){ ++i; }
            return i + (container.size() - 1) * radix_log2;
        }

        static const unsigned_integer &max(const unsigned_integer &a, const unsigned_integer &b){
            return a > b ? a : b;
        }

        static const unsigned_integer &min(const unsigned_integer &a, const unsigned_integer &b){
            return a < b ? a : b;
        }

    protected:
        bool less(const typename container_type::const_iterator &rhs_it, const typename container_type::const_iterator &rhs_end) const{
            return base_less_eq(false, rhs_it, rhs_end);
        }

        bool less_eq(const typename container_type::const_iterator &rhs_it, const typename container_type::const_iterator &rhs_end) const{
            return base_less_eq(true, rhs_it, rhs_end);
        }

        bool greater(const typename container_type::const_iterator &rhs_it, const typename container_type::const_iterator &rhs_end) const{
            return base_greater_eq(false, rhs_it, rhs_end);
        }

        bool greater_eq(const typename container_type::const_iterator &rhs_it, const typename container_type::const_iterator &rhs_end) const{
            return base_greater_eq(true, rhs_it, rhs_end);
        }

        static unsigned_integer &square_multi(unsigned_integer &result, const unsigned_integer &lhs, const unsigned_integer &rhs, bool is_float = false){
            return square_multi(result, lhs.container.begin(), lhs.container.end(), rhs.container.begin(), rhs.container.end(), lhs.container.size(), rhs.container.size(), is_float);
        }

        static unsigned_integer &square_multi(
            unsigned_integer &result,
            const typename container_type::const_iterator &lhs_begin, const typename container_type::const_iterator &lhs_end,
            const typename container_type::const_iterator &rhs_begin, const typename container_type::const_iterator &rhs_end,
            std::size_t lhs_size, std::size_t rhs_size,
            bool is_float = false
        ){
            container_type &r_container(result.container);
            if(!is_float){
                r_container.resize(lhs_size + rhs_size - 1);
                if(r_container.size() == 0){
                    result.assign(0);
                    return result;
                }
            }else{
                r_container.resize(lhs_size + rhs_size - 3);
            }
            std::size_t n = 0;
            for(typename container_type::const_iterator rhs_it = rhs_begin; rhs_it != rhs_end; ++rhs_it, ++n){
                radix_type rhs = *rhs_it;
                std::size_t m = 0;
                for(typename container_type::const_iterator lhs_it = lhs_begin; lhs_it != lhs_end; ++lhs_it, ++m){
                    radix_type lhs = *lhs_it;
                    result.add_radix_n(static_cast<unsigned_radix2_type>(lhs) * rhs, n + m);
                }
            }
            return result;
        }

        radix_type div_n_digit(const unsigned_integer &v){
            std::size_t digit = v.container.size();
            unsigned_radix2_type q = container[digit - 1];
            if(container.size() > digit){
                q += static_cast<unsigned_radix2_type>(container[digit]) << radix_log2;
            }
            q /= v.container[digit - 1];
            unsigned_integer qv;
            multi(qv, v, q);
            while(*this < qv){
                --q;
                *this += v;
            }
            *this -= qv;
            return static_cast<radix_type>(q);
        }

        void div_copy_impl(const unsigned_integer &num, std::size_t s, std::size_t e){
            container.clear();
            while(s <= e){
                if(s >= num.container.size()){ break; }
                container.push_back(num.container[s++]);
            }
            if(container.size() == 0){ assign(0); }
            (this->*normalize_container)();
        }

        void div_impl(const unsigned_integer &v, unsigned_integer &r){
            const radix_type half = static_cast<radix_type>(static_cast<radix2_type>(1) << (radix_log2 - 1));
            if(*this == 0 || *this < v){
                r.assign(*this);
                assign(0);
                return;
            }
            unsigned_integer v_prime(v);
            container_type buffer;
            radix_type leftmost = v_prime.container.back();
            std::size_t shift_num = 0;
            while(leftmost < half){
                leftmost <<= 1;
                ++shift_num;
            }
            v_prime <<= shift_num;
            *this <<= shift_num;
            std::size_t s = container.size() - v_prime.container.size();
            r.div_copy_impl(*this, s, s + v_prime.container.size() - 1);
            if(r < v_prime){
                r.container.insert(r.container.begin(), container[--s]);
            }
            while(r >= v_prime){
                buffer.push_back(r.div_n_digit(v_prime));
                while(s > 0){
                    r.container.insert(r.container.begin(), container[--s]);
                    if(r >= v_prime){ break; }
                    buffer.push_back(0);
                }
            }
            container.clear();
            for(std::size_t i = buffer.size(); i > 0; --i){
                container.push_back(buffer[i - 1]);
            }
            r >>= shift_num;
        }

        static unsigned_integer &div(unsigned_integer &result, const unsigned_integer &lhs, const unsigned_integer &rhs){
            unsigned_integer rem;
            return div(result, rem, lhs, rhs);
        }

        static unsigned_integer &mod(unsigned_integer &rem, const unsigned_integer &lhs, const unsigned_integer &rhs){
            unsigned_integer result;
            div(result, rem, lhs, rhs);
            return rem;
        }

        bool base_less_eq(bool final, typename container_type::const_iterator rhs_it, typename container_type::const_iterator rhs_end) const{
            std::size_t lhs_n = container.size(), rhs_n = std::distance(rhs_it, rhs_end);
            typename container_type::const_iterator lhs_begin = container.begin(), lhs_it = lhs_begin;
            if(lhs_n < rhs_n){
                std::size_t n = rhs_n - lhs_n;
                for(std::size_t i = 0; i < n; ++i){
                    if(*(rhs_it + (rhs_n - i - 1)) > 0){ return true; }
                }
                rhs_it += lhs_n - 1;
                lhs_it = lhs_begin + (lhs_n - 1);
            }else if(lhs_n > rhs_n){
                std::size_t n = lhs_n - rhs_n;
                for(std::size_t i = 0; i < n; ++i){
                    if(*(container.begin() + (lhs_n - i - 1)) > 0){ return false; }
                }
                lhs_it += rhs_n - 1;
                rhs_it = rhs_it + (rhs_n - 1);
            }else{
                lhs_it += lhs_n - 1;
                rhs_it += lhs_n - 1;
            }
            for(; ; ){
                radix_type l = *lhs_it, r = *rhs_it;
                if(l < r){ return true; }
                if(l > r){ return false; }
                if(lhs_begin == lhs_it){ break; }
                --lhs_it, --rhs_it;
            }
            return final;
        }

        bool base_greater_eq(bool final, typename container_type::const_iterator rhs_it, typename container_type::const_iterator rhs_end) const{
            std::size_t lhs_n = container.size(), rhs_n = std::distance(rhs_it, rhs_end);
            typename container_type::const_iterator lhs_begin = container.begin(), lhs_it = lhs_begin;
            if(lhs_n < rhs_n){
                std::size_t n = rhs_n - lhs_n;
                for(std::size_t i = 0; i < n; ++i){
                    if(*(rhs_it + (rhs_n - i - 1)) > 0){ return false; }
                }
                rhs_it += lhs_n - 1;
                lhs_it = lhs_begin + (lhs_n - 1);
            }else if(lhs_n > rhs_n){
                std::size_t n = lhs_n - rhs_n;
                for(std::size_t i = 0; i < n; ++i){
                    if(*(container.begin() + (lhs_n - i - 1)) > 0){ return true; }
                }
                lhs_it += rhs_n - 1;
                rhs_it = rhs_it + (rhs_n - 1);
            }else{
                lhs_it += lhs_n - 1;
                rhs_it += lhs_n - 1;
            }
            for(; ; ){
                radix_type l = *lhs_it, r = *rhs_it;
                if(l < r){ return false; }
                if(l > r){ return true; }
                if(lhs_begin == lhs_it){ break; }
                --lhs_it, --rhs_it;
            }
            return final;
        }

        template<class OtherContainer>
        static void invert_bit(std::size_t n, OtherContainer &index_vec){
            std::size_t size = 1 << n;
            index_vec.resize(size);
            for(std::size_t i = 0; i < size; ++i){
                std::size_t j = i, mask = size >> 1;
                for(std::size_t k = 0; k < n; ++k){
                    if((j & 1) != 0){ index_vec[i] |= mask; }
                    j >>= 1;
                    mask >>= 1;
                }
            }
        }

        template<bool InModular, class Type, class Multi>
        static Type &pow_impl(Type &result, const Type &x, const Type &y, Multi multi, const Type &mod = 0){
            result.assign(x);
            typename container_type::const_reverse_iterator y_iter = y.container.rbegin(); 
            {
                radix_type n_y = *y_iter;
                std::size_t k = 0;
                for(; (n_y & (1 << (radix_log2 - 1))) == 0; n_y <<= 1){ ++k; }
                n_y = *y_iter;
                ++k;
                for(std::size_t n = k; n < radix_log2; ++n){
                    std::size_t m = radix_log2 - n - 1;
                    Type z(result);
                    multi(result, z, z);
                    if(((n_y >> m) & 1) == 1){
                        Type w(result);
                        multi(result, w, x);
                        if(InModular){ result = result % mod; }
                    }
                }
            }
            ++y_iter;
            for(typename container_type::const_reverse_iterator y_end = y.container.rend(); y_iter != y_end; ++y_iter){
                const radix_type &n_y(*y_iter);
                for(std::size_t n = 0; n < radix_log2; ++n){
                    std::size_t m = radix_log2 - n - 1;
                    Type z(result);
                    multi(result, z, z);
                    if(((n_y >> m) & 1) == 1){
                        Type w(result);
                        multi(result, w, x);
                        if(InModular){ result = result % mod; }
                    }
                }
            }
            return result;
        }

        void bit_shift_lsr(std::size_t n){
            if(*this == 0){ return; }
            std::size_t digit = n / radix_log2, shift = n % radix_log2;
            std::size_t rev_shift = radix_log2 - shift;
            radix_type c = 0;
            for(std::size_t i = 0; i < container.size(); ++i){
                radix_type x = container[i];
                container[i] = (x << shift) | c;
                if(rev_shift < radix_log2){
                    c = x >> rev_shift;
                }else{
                    c = 0;
                }
            }
            if(normalize_container == &unsigned_integer::normalize_container_impl){
                if(c > 0){ container.push_back(c); }
            }else{
                container.back() = c;
            }
            radix_shift(digit);
        }

        void bit_shift_rsl(std::size_t n){
            if(*this == 0){ return; }
            std::size_t digit = n / radix_log2, shift = n % radix_log2;
            std::size_t size = container.size(), rev_shift = radix_log2 - shift;
            if(digit > 0){
                for(std::size_t i = 0, length = size - digit; i < length; ++i){
                    container[i] = container[i + digit];
                }
                for(std::size_t i = 0; i < digit; ++i){
                    container[size - i - 1] = 0;
                }
            }
            radix_type c = 0;
            for(std::size_t i = 0; i < container.size(); ++i){
                std::size_t j = size - i - 1;
                radix_type x = container[j];
                container[j] = (x >> shift) | c;
                if(rev_shift < radix_log2){
                    c = x << rev_shift;
                }else{
                    c = 0;
                }
            }
            (this->*normalize_container)();
        }

        static unsigned_integer &gcd_impl(unsigned_integer &result, unsigned_integer lhs, unsigned_integer rhs){
            if(rhs == 0){ result = 0; return result; }
            unsigned_integer *operands[3] = { &lhs, &rhs, &result };
            for(; ; ){
                mod(*operands[2], *operands[0], *operands[1]);
                if(*operands[2] == 0){ break; }
                unsigned_integer *ptr = operands[0];
                for(int i = 0; i < 2; ++i){ operands[i] = operands[i + 1]; }
                operands[2] = ptr;
            }
            if(operands[1] != &result){ result.assign(*operands[1]); }
            return result;
        }

        unsigned_integer &add_range(const typename container_type::const_iterator &first, const typename container_type::const_iterator &last){
            std::size_t rhs_size = std::distance(first, last);
            if(normalize_container == &unsigned_integer::normalize_container_impl && container.size() < rhs_size + 1){
                container.resize(rhs_size + 1);
            }
            typename container_type::iterator operand_it = container.begin();
            unsigned_radix2_type c = 0;
            typename container_type::const_iterator it = first, end = last;
            for(; it != end; ++it, ++operand_it){
                radix_type &operand(*operand_it);
                unsigned_radix2_type temp = static_cast<unsigned_radix2_type>(operand) + static_cast<unsigned_radix2_type>(*it) + c;
                operand = static_cast<radix_type>(temp % (static_cast<unsigned_radix2_type>(1) << radix_log2));
                c = temp >> radix_log2;
            }
            for(; c > 0; ++operand_it){
                radix_type &operand(*operand_it);
                unsigned_radix2_type temp = static_cast<unsigned_radix2_type>(operand) + c;
                operand = static_cast<radix_type>(temp % (static_cast<unsigned_radix2_type>(1) << radix_log2));
                c = temp >> radix_log2;
            }
            (this->*normalize_container)();
            return *this;
        }

        unsigned_integer &sub_range(const typename container_type::const_iterator &first, const typename container_type::const_iterator &last){
            typename container_type::iterator operand_it = container.begin();
            radix2_type c = 0;
            typename container_type::const_iterator it = first;
            for(; it != last; ++it, ++operand_it){
                radix_type &operand(*operand_it);
                radix2_type temp = static_cast<radix2_type>(operand) - static_cast<radix2_type>(*it) + c;
                if(temp < 0){
                    c = -(((-temp) >> radix_log2) + (((-temp) % (static_cast<radix2_type>(1) << radix_log2)) ? 1 : 0));
                    temp = (temp + (static_cast<radix2_type>(1) << radix_log2)) % (static_cast<radix2_type>(1) << radix_log2);
                }else{ c = 0; }
                operand = static_cast<radix_type>(temp);
            }
            if(c != 0){
                radix_type &operand(*operand_it);
                operand = static_cast<radix_type>(c + operand);
            }
            (this->*normalize_container)();
            return *this;
        }

        void sub_q_n(typename container_type::const_iterator rhs_begin, typename container_type::const_iterator rhs_end, radix2_type q, std::size_t n){
            typename container_type::iterator operand_it = container.begin();
            std::advance(operand_it, n);
            radix2_type c = 0;
            typename container_type::const_iterator it = rhs_begin, end = rhs_end;
            for(; it != end; ++it, ++operand_it){
                radix_type &operand(*operand_it);
                radix2_type temp = static_cast<radix2_type>(operand) - static_cast<radix2_type>(*it) * q + c;
                if(temp < 0){
                    temp = -temp;
                    c = -(temp / (static_cast<radix2_type>(1) << radix_log2) + (temp % (static_cast<radix2_type>(1) << radix_log2) ? 1 : 0));
                    temp = (static_cast<radix2_type>(1) << radix_log2) - (temp % (static_cast<radix2_type>(1) << radix_log2));
                }else{ c = 0; }
                operand = static_cast<radix_type>(temp);
            }
            if(c != 0){
                radix_type &operand(*operand_it);
                operand = static_cast<radix_type>(static_cast<radix2_type>(operand) + c);
            }
            (this->*normalize_container)();
        }

        void sub_q_n(const unsigned_integer &rhs, radix2_type q, std::size_t n){
            return sub_q_n(rhs.container.begin(), rhs.container.end(), q, n);
        }

        template<class Iter>
        void add_iterator_n(const Iter &rhs_first, const Iter &rhs_last, std::size_t n){
            std::size_t rhs_container_size = std::distance(rhs_first, rhs_last);
            if(container.size() < rhs_container_size + n){ container.resize(rhs_container_size + n); }
            typename container_type::iterator operand_it = container.begin() + n;
            unsigned_radix2_type c = 0;
            Iter it = rhs_first, end = rhs_last; --end;
            for(; it != end; ++it, ++operand_it){
                radix_type &operand(*operand_it);
                unsigned_radix2_type temp = static_cast<unsigned_radix2_type>(operand) + static_cast<unsigned_radix2_type>(*it) + c;
                operand = static_cast<radix_type>(temp % (static_cast<unsigned_radix2_type>(1) << radix_log2));
                c = temp >> radix_log2;
            }
            {
                radix_type &operand(*operand_it);
                unsigned_radix2_type temp = static_cast<unsigned_radix2_type>(operand) + static_cast<unsigned_radix2_type>(*it) + c;
                operand = static_cast<radix_type>(temp % (static_cast<unsigned_radix2_type>(1) << radix_log2));
                c = temp >> radix_log2;
                if(c != 0){ container.push_back(static_cast<radix_type>(c)); }
            }
        }

        void add_radix_n(const unsigned_integer &rhs, std::size_t n){
            add_iterator_n(rhs.container.begin(), rhs.container.end(), n);
        }

        void add_radix_n(unsigned_radix2_type v, std::size_t n){
            if(container.size() < n){ container.resize(n); }
            typename container_type::iterator operand_it = container.begin();
            std::advance(operand_it, n);
            unsigned_radix2_type c = v;
            for(; c > 0; ++operand_it){
                if(operand_it == container.end()){
                    container.push_back(0);
                    operand_it = container.end(), --operand_it;
                }
                radix_type &operand(*operand_it);
                unsigned_radix2_type temp = static_cast<unsigned_radix2_type>(operand) + c;
                operand = static_cast<radix_type>(temp % (static_cast<unsigned_radix2_type>(1) << radix_log2));
                c = temp >> radix_log2;
            }
        }

        void normalize_container_impl(){
            while(container.size() > 1 && container.back() == 0){
                container.pop_back();
            }
        }

        void normalize_container_dummy(){}

        template<class Ptr, class Char>
        void read_impl(const Ptr &str, int zero, int nine, const Ptr &delim = Ptr()){
            assign(0);
            unsigned_integer a(1), t;
            Ptr it = aux::reading_str_rbegin<Ptr, Char>(str, zero, nine, delim);
            do{
                int c = *it;
                if(c >= zero && c <= nine){
                    *this += multi(t, a, static_cast<radix_type>(c - zero));
                }else{
                    break;
                }
                --it;
                t.assign(a);
                multi(a, t, 10);
            }while(it != str);
            {
                int c = *it;
                if(c >= zero && c <= nine){
                    *this += multi(t, a, static_cast<radix_type>(c - zero));
                    t.assign(a);
                    multi(a, t, 10);
                }
            }
        }

        template<class String, class Char>
        String to_string_impl(int zero) const{
            String result;
            if(*this == 0){ result = static_cast<Char>(zero); return std::move(result); }
            unsigned_integer a(10), temp[2], r;
            temp[0].assign(*this);
            for(; !(temp[0] == 0); ){
                div(temp[1], r, temp[0], a);
                temp[0].assign(temp[1]);
                result.insert(result.begin(), static_cast<Char>(r.container[0] + zero));
            }
            return std::move(result);
        }

    protected:
        void (unsigned_integer::*normalize_container)();
        container_type container;
    };

    template<class RadixType, std::size_t RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator>
    std::ostream &operator <<(
        std::ostream &ostream,
        const unsigned_integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> &value
    ){
        ostream << value.to_string();
        return ostream;
    }

    template<class RadixType, std::size_t RadixLog2, class Radix2Type, class URadix2Type, class Container, class Allocator>
    std::wostream &operator <<(
        std::wostream &ostream,
        const unsigned_integer<RadixType, RadixLog2, Radix2Type, URadix2Type, Container, Allocator> &value
    ){
        ostream << value.to_wstring();
        return ostream;
    }
}

#endif
