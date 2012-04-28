#ifndef HPP_CPP_MULTI_PRECISION_NS_AUX
#define HPP_CPP_MULTI_PRECISION_NS_AUX

#include <string>
#include <vector>
#include <deque>
#include <list>
#include <stack>
#include <queue>
#include <set>
#include <map>
#include <ostream>
#include <algorithm>
#include <utility>
#include <type_traits>
#include <exception>
#include <stdexcept>
#include <cmath>
#include <cassert>
#include <sstream>
#include <boost/utility/enable_if.hpp>
#include <boost/random.hpp>

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

        //template<class Signature>
        //struct rebind_container;

        //template<class CType, class CAlloc>
        //struct rebind_container<std::vector<CType, CAlloc>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef std::vector<NewType, CAlloc> other; };
        //};

        //template<class CType, class CAlloc>
        //struct rebind_container<std::deque<CType, CAlloc>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef std::deque<NewType, CAlloc> other; };
        //};

        //template<class CType, class CAlloc>
        //struct rebind_container<std::list<CType, CAlloc>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef std::list<NewType, CAlloc> other; };
        //};

        //template<class CType, class CCont>
        //struct rebind_container<std::stack<CType, CCont>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef std::stack<NewType, CCont> other; };
        //};

        //template<class CType, class CCont>
        //struct rebind_container<std::queue<CType, CCont>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef std::queue<NewType, CCont> other; };
        //};

        //template<class CType, class CCont, class CPr>
        //struct rebind_container<std::priority_queue<CType, CCont, CPr>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef std::priority_queue<NewType, CCont, CPr> other; };
        //};

        //template<class CType, class CPr, class CAl>
        //struct rebind_container<std::set<CType, CPr, CAl>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef std::set<NewType, CPr, CAl> other; };
        //};

        //template<class CType, class CPr, class CAl>
        //struct rebind_container<std::multiset<CType, CPr, CAl>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef std::multiset<NewType, CPr, CAl> other; };
        //};

        //template<class CKey, class CType, class CPr, class CAl>
        //struct rebind_container<std::map<CKey, CType, CPr, CAl>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef std::map<CKey, NewType, CPr, CAl> other; };
        //};

        //template<class CKey, class CType, class CPr, class CAl>
        //struct rebind_container<std::multimap<CKey, CType, CPr, CAl>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef std::multimap<CKey, NewType, CPr, CAl> other; };
        //};

        //template<class CType1, class CType2, class CType3, template<class T1, class T2, class T3> class Container>
        //struct rebind_container<Container<CType1, CType2, CType3>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef Container<NewType, NewAlloc, CType3> other; };
        //    template<class NewType> struct rebind_first{ typedef Container<NewType, CType2, CType3> other; };
        //};

        //template<class CType1, class CType2, class CType3, class CType4, template<class T1, class T2, class T3, class T4> class Container>
        //struct rebind_container<Container<CType1, CType2, CType3, CType4>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef Container<NewType, NewAlloc, CType3, CType4> other; };
        //    template<class NewType> struct rebind_first{ typedef Container<NewType, CType2, CType3, CType4> other; };
        //};

        //template<class CType1, class CType2, class CType3, class CType4, class CType5, template<class T1, class T2, class T3, class T4, class T5> class Container>
        //struct rebind_container<Container<CType1, CType2, CType3, CType4, CType5>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef Container<NewType, NewAlloc, CType3, CType4, CType5> other; };
        //    template<class NewType> struct rebind_first{ typedef Container<NewType, CType2, CType3, CType4, CType5> other; };
        //};

        //template<class CType1, class CType2, class CType3, class CType4, class CType5, class CType6, template<class T1, class T2, class T3, class T4, class T5, class T6> class Container>
        //struct rebind_container<Container<CType1, CType2, CType3, CType4, CType5, CType6>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef Container<NewType, NewAlloc, CType3, CType4, CType5, CType6> other; };
        //    template<class NewType> struct rebind_first{ typedef Container<NewType, CType2, CType3, CType4, CType5, CType6> other; };
        //};

        //template<class CType1, class CType2, class CType3, class CType4, class CType5, class CType6, class CType7, template<class T1, class T2, class T3, class T4, class T5, class T6, class T7> class Container>
        //struct rebind_container<Container<CType1, CType2, CType3, CType4, CType5, CType6, CType7>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef Container<NewType, NewAlloc, CType3, CType4, CType5, CType6, CType7> other; };
        //    template<class NewType> struct rebind_first{ typedef Container<NewType, CType2, CType3, CType4, CType5, CType6, CType7> other; };
        //};

        //template<class CType1, class CType2, class CType3, class CType4, class CType5, class CType6, class CType7, class CType8, template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8> class Container>
        //struct rebind_container<Container<CType1, CType2, CType3, CType4, CType5, CType6, CType7, CType8>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef Container<NewType, NewAlloc, CType3, CType4, CType5, CType6, CType7, CType8> other; };
        //    template<class NewType> struct rebind_first{ typedef Container<NewType, CType2, CType3, CType4, CType5, CType6, CType7, CType8> other; };
        //};

        //template<class CType1, class CType2, class CType3, class CType4, class CType5, class CType6, class CType7, class CType8, class CType9, template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9> class Container>
        //struct rebind_container<Container<CType1, CType2, CType3, CType4, CType5, CType6, CType7, CType8, CType9>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef Container<NewType, NewAlloc, CType3, CType4, CType5, CType6, CType7, CType8, CType9> other; };
        //    template<class NewType> struct rebind_first{ typedef Container<NewType, CType2, CType3, CType4, CType5, CType6, CType7, CType8, CType9> other; };
        //};

        //template<class CType1, class CType2, class CType3, class CType4, class CType5, class CType6, class CType7, class CType8, class CType9, class CType10, template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10> class Container>
        //struct rebind_container<Container<CType1, CType2, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef Container<NewType, NewAlloc, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10> other; };
        //    template<class NewType> struct rebind_first{ typedef Container<NewType, CType2, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10> other; };
        //};

        //template<class CType1, class CType2, class CType3, class CType4, class CType5, class CType6, class CType7, class CType8, class CType9, class CType10, class CType11, template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11> class Container>
        //struct rebind_container<Container<CType1, CType2, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef Container<NewType, NewAlloc, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11> other; };
        //    template<class NewType> struct rebind_first{ typedef Container<NewType, CType2, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11> other; };
        //};

        //template<class CType1, class CType2, class CType3, class CType4, class CType5, class CType6, class CType7, class CType8, class CType9, class CType10, class CType11, class CType12, template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12> class Container>
        //struct rebind_container<Container<CType1, CType2, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef Container<NewType, NewAlloc, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12> other; };
        //    template<class NewType> struct rebind_first{ typedef Container<NewType, CType2, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12> other; };
        //};

        //template<class CType1, class CType2, class CType3, class CType4, class CType5, class CType6, class CType7, class CType8, class CType9, class CType10, class CType11, class CType12, class CType13, template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13> class Container>
        //struct rebind_container<Container<CType1, CType2, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12, CType13>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef Container<NewType, NewAlloc, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12, CType13> other; };
        //    template<class NewType> struct rebind_first{ typedef Container<NewType, CType2, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12, CType13> other; };
        //};

        //template<class CType1, class CType2, class CType3, class CType4, class CType5, class CType6, class CType7, class CType8, class CType9, class CType10, class CType11, class CType12, class CType13, class CType14, template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14> class Container>
        //struct rebind_container<Container<CType1, CType2, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12, CType13, CType14>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef Container<NewType, NewAlloc, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12, CType13, CType14> other; };
        //    template<class NewType> struct rebind_first{ typedef Container<NewType, CType2, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12, CType13, CType14> other; };
        //};

        //template<class CType1, class CType2, class CType3, class CType4, class CType5, class CType6, class CType7, class CType8, class CType9, class CType10, class CType11, class CType12, class CType13, class CType14, class CType15, template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15> class Container>
        //struct rebind_container<Container<CType1, CType2, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12, CType13, CType14, CType15>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef Container<NewType, NewAlloc, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12, CType13, CType14, CType15> other; };
        //    template<class NewType> struct rebind_first{ typedef Container<NewType, CType2, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12, CType13, CType14, CType15> other; };
        //};

        //template<class CType1, class CType2, class CType3, class CType4, class CType5, class CType6, class CType7, class CType8, class CType9, class CType10, class CType11, class CType12, class CType13, class CType14, class CType15, class CType16, template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16> class Container>
        //struct rebind_container<Container<CType1, CType2, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12, CType13, CType14, CType15, CType16>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef Container<NewType, NewAlloc, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12, CType13, CType14, CType15, CType16> other; };
        //    template<class NewType> struct rebind_first{ typedef Container<NewType, CType2, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12, CType13, CType14, CType15, CType16> other; };
        //};

        //template<class CType1, class CType2, class CType3, class CType4, class CType5, class CType6, class CType7, class CType8, class CType9, class CType10, class CType11, class CType12, class CType13, class CType14, class CType15, class CType16, class CType17, template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17> class Container>
        //struct rebind_container<Container<CType1, CType2, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12, CType13, CType14, CType15, CType16, CType17>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef Container<NewType, NewAlloc, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12, CType13, CType14, CType15, CType16, CType17> other; };
        //    template<class NewType> struct rebind_first{ typedef Container<NewType, CType2, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12, CType13, CType14, CType15, CType16, CType17> other; };
        //};

        //template<class CType1, class CType2, class CType3, class CType4, class CType5, class CType6, class CType7, class CType8, class CType9, class CType10, class CType11, class CType12, class CType13, class CType14, class CType15, class CType16, class CType17, class CType18, template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18> class Container>
        //struct rebind_container<Container<CType1, CType2, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12, CType13, CType14, CType15, CType16, CType17, CType18>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef Container<NewType, NewAlloc, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12, CType13, CType14, CType15, CType16, CType17, CType18> other; };
        //    template<class NewType> struct rebind_first{ typedef Container<NewType, CType2, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12, CType13, CType14, CType15, CType16, CType17, CType18> other; };
        //};

        //template<class CType1, class CType2, class CType3, class CType4, class CType5, class CType6, class CType7, class CType8, class CType9, class CType10, class CType11, class CType12, class CType13, class CType14, class CType15, class CType16, class CType17, class CType18, class CType19, template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19> class Container>
        //struct rebind_container<Container<CType1, CType2, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12, CType13, CType14, CType15, CType16, CType17, CType18, CType19>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef Container<NewType, NewAlloc, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12, CType13, CType14, CType15, CType16, CType17, CType18, CType19> other; };
        //    template<class NewType> struct rebind_first{ typedef Container<NewType, CType2, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12, CType13, CType14, CType15, CType16, CType17, CType18, CType19> other; };
        //};

        //template<class CType1, class CType2, class CType3, class CType4, class CType5, class CType6, class CType7, class CType8, class CType9, class CType10, class CType11, class CType12, class CType13, class CType14, class CType15, class CType16, class CType17, class CType18, class CType19, class CType20, template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19, class T20> class Container>
        //struct rebind_container<Container<CType1, CType2, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12, CType13, CType14, CType15, CType16, CType17, CType18, CType19, CType20>>{
        //    template<class NewType, class NewAlloc> struct rebind{ typedef Container<NewType, NewAlloc, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12, CType13, CType14, CType15, CType16, CType17, CType18, CType19, CType20> other; };
        //    template<class NewType> struct rebind_first{ typedef Container<NewType, CType2, CType3, CType4, CType5, CType6, CType7, CType8, CType9, CType10, CType11, CType12, CType13, CType14, CType15, CType16, CType17, CType18, CType19, CType20> other; };
        //};

        template<class Dummy>
        struct random_template{
            static boost::mt19937 &engine(){ static boost::mt19937 e; return e; }
            template<class T>
            static T gen(const T &mod){
                return static_cast<T>(engine()()) % mod;
            }
        };

        typedef random_template<void> random;

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
        std::size_t ceil_pow2(T n){
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

        std::size_t ceil_log2(std::size_t n){
            std::size_t m = n, r = 0;
            for(std::size_t i = 0; i < sizeof(std::size_t)  * 8; ++i, m >>= 1){
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

#define CPP_MULTI_PRECISION_AUX_SIGUNATURE_TO_STRING template<class T, std::string (T::*Func)() const>
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(to_string, CPP_MULTI_PRECISION_AUX_SIGUNATURE_TO_STRING);
        template<class T>
        std::string to_string_dispatch(const T &value, typename boost::enable_if<has_to_string<T>>::type* = 0){
            return value.to_string();
        }

        template<class T>
        std::string to_string_dispatch(const T &value, typename boost::disable_if<has_to_string<T>>::type* = 0){
            std::ostringstream os;
            os << value;
            return os.str();
        }

#define CPP_MULTI_PRECISION_AUX_SIGUNATURE_TO_WSTRING template<class T, std::wstring (T::*Func)() const>
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(to_wstring, CPP_MULTI_PRECISION_AUX_SIGUNATURE_TO_WSTRING);
        template<class T>
        std::wstring to_wstring_dispatch(const T &value, typename boost::enable_if<has_to_string<T>>::type* = 0){
            return value.to_wstring();
        }

        template<class T>
        std::wstring to_wstring_dispatch(const T &value, typename boost::disable_if<has_to_string<T>>::type* = 0){
            std::wostringstream os;
            os << value;
            return os.str();
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

        template<class RadixType, std::size_t RadixLog2, class Radix2Type, class URadix2Type, class Container>
        class fractional;
    }
}

#endif
