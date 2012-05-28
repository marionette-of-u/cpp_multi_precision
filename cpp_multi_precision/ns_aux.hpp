#ifndef HPP_CPP_MULTI_PRECISION_NS_AUX
#define HPP_CPP_MULTI_PRECISION_NS_AUX

#include <string>
#include <ostream>
#include <queue>
#include <vector>
#include <algorithm>
#include <utility>
#include <type_traits>
#include <exception>
#include <stdexcept>
#include <cmath>
#include <cassert>
#include <climits>
#include <boost/utility/enable_if.hpp>

namespace cpp_multi_precision{
    namespace aux{
        static_assert(CHAR_BIT == 8, "cpp_multi_precision::aux - 'CHAR_BIT == 8'");

        template<class Container>
        struct rebind_container;

        template<template<class, class> class Container, class Elem, class Alloc>
        struct rebind_container<Container<Elem, Alloc>>{
            template<class NewType, class NewAlloc> struct rebind{ typedef Container<NewType, NewAlloc> other; };
        };

        template<class Container, class NewType>
        struct wrapped_rebind_container{
            typedef typename rebind_container<Container>::template rebind<NewType, typename Container::allocator_type::template rebind<NewType>::other>::other type;
        };

        template<class Container, class Alloc>
        class bit_array{
        public:
            typedef typename aux::rebind_container<Container>::template rebind<
                unsigned int,
                typename Alloc::template rebind<unsigned int>::other
            >::other container_type;

            bool get(std::size_t i) const{
                std::size_t idx = (i / 32), rem = i % 32;
                return container.size() >= idx + 1 && ((container[idx] >> rem) & 1) == 1;
            }

            void set(std::size_t i, bool flag){
                std::size_t idx = i / 32, rem = i % 32;
                if(!flag && container.size() < idx + 1){ return; }
                if(flag){
                    if(container.size() < idx + 1){ container.resize(idx + 1); }
                    container[idx] |= 1 <<  rem;
                }else{
                    if(((container[idx] >> rem) & 1) == 1){
                        container[idx] -= 1 << rem;
                    }
                }
            }

        private:
            container_type container;
        };

        template<class T>
        T ceil_pow2(T n){
            --n;
            for(std::size_t i = 1; i < sizeof(T) * 8; i <<= 1){
                n = n | (n >> i);
            }
            return n + 1;
        }

        template<class T>
        std::size_t index_of_leftmost_flag(T x){
            T y;
            std::size_t n = sizeof(T) * 8, m = n / 2;
            while(m > 0){
                y = x >> m;
                if(y != 0){
                    n -= m;
                    x = y;
                }
                m /= 2;
            }
            return 1 + sizeof(T) * 8 - n - x;
        }

        template<class T>
        std::size_t ceil_log2(T n){
            std::size_t m = n, r = 0;
            for(std::size_t i = 0; i < sizeof(T) * 8; ++i, m >>= 1){
                if((m & 1) == 1){
                    ++r;
                    if(r > 1){ break; }
                }
            }
            return index_of_leftmost_flag(n) + (r > 1 ? 1 : 0);
        }

#define CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(fn_name, signature) \
        template<class Type> \
        class has_ ## fn_name{ \
        private: \
            typedef char yes; \
            typedef struct{ char a[2]; } no; \
            signature \
            struct ptmf_helper{}; \
            template<class T> \
            static yes check(ptmf_helper<T, &T::fn_name>* p); \
            template<class T> \
            static no check(...); \
        public: \
            BOOST_STATIC_CONSTANT(bool, value = sizeof(check<Type>(0)) == sizeof(yes)); \
        }

#define CPP_MULTI_PRECISION_AUX_SIGNATURE_RESERVE template<class T, void (T::*Func)(typename T::size_type)>
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(reserve, CPP_MULTI_PRECISION_AUX_SIGNATURE_RESERVE);
        template<class T>
        void reserve_dispatch(T &container, std::size_t n, typename boost::enable_if<has_reserve<T>>::type* = 0){
            container.reserve(n);
        }

        template<class T>
        void reserve_dispatch(T &container, std::size_t, typename boost::disable_if<has_reserve<T>>::type* = 0){
            return;
        }

#define CPP_MULTI_PRECISION_AUX_SIGNATURE_MULTI template<class T, T& (*Func)(T&, const T&, const T&)>
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(square_multi, CPP_MULTI_PRECISION_AUX_SIGNATURE_MULTI);
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(kar_multi, CPP_MULTI_PRECISION_AUX_SIGNATURE_MULTI);

        template<class T>
        T &multi_dispatch_2(T &result, const T &lhs, const T &rhs, typename boost::enable_if<has_square_multi<T>>::type* = 0){
            return T::square_multi(result, lhs, rhs);
        }

        template<class T>
        T &multi_dispatch_2(T &result, const T &lhs, const T &rhs, typename boost::disable_if<has_square_multi<T>>::type* = 0){
            return T::multi(result, lhs, rhs);
        }

        template<class T>
        T &multi_dispatch(T &result, const T &lhs, const T &rhs, typename boost::enable_if<has_kar_multi<T>>::type* = 0){
            return T::kar_multi(result, lhs, rhs);
        }

        template<class T>
        T &multi_dispatch(T &result, const T &lhs, const T &rhs, typename boost::disable_if<has_kar_multi<T>>::type* = 0){
            return multi_dispatch_2(result, lhs, rhs);
        }

        template<class Ptr, class Char>
        Ptr reading_str_rbegin(Ptr str, Char zero, Char nine, Ptr delim){
            for(; str != delim && *str >= zero && *str <= nine; ++str);
            return --str;
        }

        class sign{
        public:
            sign() : value(false){}
            sign(const sign &other) : value(other.value){}
            sign(bool value_) : value(value_){}

            sign &operator =(const sign &other){
                value = other.value;
                return *this;
            }

            sign &operator =(const bool value){
                this->value = value;
                return *this;
            }

            bool operator ==(const sign &other){
                return (value && other.value) || (!value && !other.value);
            }

            bool operator ==(const bool other_value){
                return (value && other_value) || (!value && !other_value);
            }

            bool operator !=(const sign &other){
                return (value && !other.value) || (!value && other.value);
            }
            
            bool operator !=(const bool other_value){
                return (value && !other_value) || (!value && other_value);
            }

            operator bool() const{
                return value;
            }

        private:
            bool value;
        };

        template<class Type>
        bool prime_div_test(Type n){
            for(Type i = 3; i * i <= n; i += 2){
                if(n % i == 0){ return false; }
            }
            return true;
        }

        template<class Type, std::size_t N = sizeof(Type) * 8>
        struct prime_list;

        template<class Type>
        struct prime_list<Type, 32>{
            typedef Type value_type;
            typedef std::vector<value_type> ext_prime_vec_type;

            static std::vector<value_type> get_prime_set(value_type k, std::size_t n){
                table(0);
                value_type u;
                std::vector<value_type> r;
                r.resize(n);
                if(k <= range_upper_bound()){
                    std::size_t index = get_table_index(k);
                    for(std::size_t i = 0; i < n; ++i){
                        r[n - i - 1] = table(index - i);
                    }
                    u = table(index + 1);
                }else{
                    std::size_t index = push_ext_prime(k);
                    if(index == 0){ index = get_ext_index(k); }
                    if(n > ext_prime_vec().size()){
                        for(std::size_t i = 0, end = ext_prime_vec().size(); i < end; ++i){
                            r[n - i - 1] = ext_prime_vec()[i];
                        }
                        for(std::size_t i = 0, end = n - ext_prime_vec().size(); i < end; ++i){
                            r[n - i - ext_prime_vec().size() - 1] = table(table_size() - i - 1);
                        }
                    }else{
                        for(std::size_t i = 0; i < n; ++i){
                            r[n - i - 1] = ext_prime_vec()[index - i - 1];
                        }
                    }
                    u = table(index + 1);
                }
                return std::move(r);
            }

            static std::size_t push_ext_prime(value_type k){
                if(k <= ext_prime_upper_bound()){ return 0; }
                if((k & 1) == 0){ ++k; }
                for(value_type i = ext_prime_upper_bound() + 2; i <= k; i += 2){
                    if(prime_div_test(i)){
                        ext_prime_vec().push_back(i);
                    }
                }
                ext_prime_upper_bound() = k;
                return ext_prime_vec().size() - 1;
            }

            static std::size_t get_ext_index(value_type k){
                std::size_t lower = 0, upper = ext_prime_vec().size(), n;
                for(; upper - lower != 1; ){
                    n = (upper + lower) / 2;
                    value_type l = ext_prime_vec()[n];
                    if(l == k){ break; }
                    if(l > k){
                        upper = n;
                    }else{
                        lower = n;
                    }
                }
                return n;
            }

            static std::size_t get_table_index(value_type k){
                std::size_t lower = 0, upper = table_size(), n;
                for(; upper - lower != 1; ){
                    n = (upper + lower) / 2;
                    value_type l = table(n);
                    if(l == k){ break; }
                    if(l > k){
                        upper = n;
                    }else{
                        lower = n;
                    }
                }
                return n;
            }

            static value_type table(std::size_t n){
                static const value_type table_[] = {
#include "prime32_262144.hpp"
                };

                struct init_type{
                    init_type(std::size_t n, value_type begin, value_type end){
                        prime_list::table_size() = n;
                        prime_list::range_lower_bound() = begin;
                        prime_list::range_upper_bound() = end;
                        prime_list::ext_prime_upper_bound() = end;
                    }
                };

                static init_type init(
                    sizeof(table_) / sizeof(value_type),
                    table_[0],
                    table_[sizeof(table_) / sizeof(value_type) - 1]
                );

                return table_[n];
            }

            static std::size_t &table_size(){
                static std::size_t n;
                return n;
            }

            static value_type &range_lower_bound(){
                static value_type n;
                return n;
            }

            static value_type &range_upper_bound(){
                static value_type n;
                return n;
            }

            static ext_prime_vec_type &ext_prime_vec(){
                static ext_prime_vec_type vec;
                return vec;
            }

            static value_type &ext_prime_upper_bound(){
                static value_type n = 0;
                return n;
            }
        };

        template<class Type>
        struct prime_list<Type, 64>{
            typedef Type value_type;
            typedef std::vector<value_type> ext_prime_vec_type;
            typedef prime_list<value_type, 32> prime32_type;

            static std::vector<value_type> get_prime_set(value_type k, std::size_t n){
                table(0);
                std::vector<value_type> r;
                r.resize(n);
                if(k < range_lower_bound()){
                    ext_prime_vec_type pair_set_u(prime32_type::get_prime_set(k, n));
                    r = std::move(pair_set_u);
                }else if(k <= range_upper_bound()){
                    std::size_t index = get_table_index(k);
                    if(index >= n){
                        for(std::size_t i = 0; i < n; ++i){
                            r[n - i - 1] = table(index - i);
                        }
                    }else{
                        for(std::size_t i = 0; i < index; ++i){
                            r[n - i - 1] = table(index - i);
                        }
                        std::size_t m = n - index;
                        std::vector<value_type> rest(std::move(prime32_type::get_prime_set(range_lower_bound(), m)));
                        for(std::size_t j = 0; j < m; ++j){
                            r[m - j - 1] = rest[m - j - 1];
                        }
                    }
                }else{
                    std::size_t index = push_ext_prime(k);
                    if(index == 0){ index = get_ext_index(k); }
                    if(n > ext_prime_vec().size()){
                        for(std::size_t i = 0, end = ext_prime_vec().size(); i < end; ++i){
                            r[n - i - 1] = ext_prime_vec()[i];
                        }
                        for(std::size_t i = 0, end = n - ext_prime_vec().size(); i < end; ++i){
                            r[n - i - ext_prime_vec().size() - 1] = table(table_size() - i - 1);
                        }
                    }else{
                        for(std::size_t i = 0; i < n; ++i){
                            r[n - i - 1] = ext_prime_vec()[index - i - 1];
                        }
                    }
                }
                return std::move(r);
            }

            static std::size_t push_ext_prime(value_type k){
                if(k <= ext_prime_upper_bound()){ return 0; }
                if((k & 1) == 0){ ++k; }
                for(value_type i = ext_prime_upper_bound() + 2; i <= k; i += 2){
                    if(prime_div_test(i)){
                        ext_prime_vec().push_back(i);
                    }
                }
                ext_prime_upper_bound() = k;
                return ext_prime_vec().size() - 1;
            }

            static std::size_t get_ext_index(value_type k){
                std::size_t lower = 0, upper = ext_prime_vec().size(), n;
                for(; upper - lower != 1; ){
                    n = (upper + lower) / 2;
                    value_type l = ext_prime_vec()[n];
                    if(l == k){ break; }
                    if(l > k){
                        upper = n;
                    }else{
                        lower = n;
                    }
                }
                return n;
            }

            static std::size_t get_table_index(value_type k){
                std::size_t lower = 0, upper = table_size(), n;
                for(; upper - lower != 1; ){
                    n = (upper + lower) / 2;
                    value_type l = table(n);
                    if(l == k){ break; }
                    if(l > k){
                        upper = n;
                    }else{
                        lower = n;
                    }
                }
                return n;
            }

            static value_type table(std::size_t n){
                static const value_type table_[] = {
#include "prime64_262144.hpp"
                };

                struct init_type{
                    init_type(std::size_t n, value_type begin, value_type end){
                        prime_list::table_size() = n;
                        prime_list::range_lower_bound() = begin;
                        prime_list::range_upper_bound() = end;
                        prime_list::ext_prime_upper_bound() = end;
                    }
                };

                static init_type init(
                    sizeof(table_) / sizeof(value_type),
                    table_[0],
                    table_[sizeof(table_) / sizeof(value_type) - 1]
                );

                return table_[n];
            }

            static std::size_t &table_size(){
                static std::size_t n;
                return n;
            }

            static value_type &range_lower_bound(){
                static value_type n;
                return n;
            }

            static value_type &range_upper_bound(){
                static value_type n;
                return n;
            }

            static ext_prime_vec_type &ext_prime_vec(){
                static ext_prime_vec_type vec;
                return vec;
            }

            static value_type &ext_prime_upper_bound(){
                static value_type n = 0;
                return n;
            }
        };

        template<class T>
        T gcd_impl(T a, T b){
            if(b == 1){ return b; }
            T c;
            for(; ; ){
                c = a - (a / b * b);
                if(c == 0){ break; }
                a = b, b = c;
            }
            return std::move(b);
        }

        template<class T>
        T gcd(T a, T b){
            if(a >= b){
                return gcd_impl(a, b);
            }else{
                return gcd_impl(b, a);
            }
        }

        template<class T>
        T eea_classic(T &s, T &t, const T &f, const T &g){
            T result = 0;
            if(g == 0){
                s = 0, t = 0;
                return std::move(result);
            }
            T r_0 = f, r_1 = g, s_0 = 1, s_1 = 0, t_0 = 0, t_1 = 1;
            while(r_1 != 0){
                T &q(result);
                q = r_0 / r_1;
                T r_m = r_1, s_m = s_1, t_m = t_1;
                r_1 = r_0 - q * r_1;
                s_1 = s_0 - q * s_1;
                t_1 = t_0 - q * t_1;
                r_0 = std::move(r_m);
                s_0 = std::move(s_m);
                t_0 = std::move(t_m);
            }
            s = std::move(s_0);
            t = std::move(t_0);
            return std::move(result);
        }

        template<class T, class U>
        struct result_type{
            typedef decltype(T() + U()) type;
        };

        template<class T>
        struct remove_reference{
            typedef typename T::value_type type;
        };

        template<class T>
        struct remove_reference<T*>{
            typedef T type;
        };

        template<class T>
        struct remove_reference<const T*>{
            typedef T type;
        };
    }
}

#endif
