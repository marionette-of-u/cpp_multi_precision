#ifndef HPP_CPP_MULTI_PRECISION_SPARSE_POLY
#define HPP_CPP_MULTI_PRECISION_SPARSE_POLY

#include <utility>
#include <vector>
#include <map>
#include <algorithm>
#include <iterator>
#include <cmath>
#include <boost/iterator_adaptors.hpp>
#include "storaged_container.hpp"
#include "ns_aux.hpp"

namespace cpp_multi_precision{
    template<
        class OrderType,
        class CoefficientType,
        class Alloc = std::allocator<int>
    > class sparse_poly{
    public:
        class container_type : public std::map<
            OrderType, CoefficientType, std::less<OrderType>,
            typename Alloc::template rebind<std::pair<const OrderType, CoefficientType> >::other
        >{
        private:
            typedef std::map<
                OrderType, CoefficientType, std::less<OrderType>,
                typename Alloc::template rebind<std::pair<const OrderType, CoefficientType> >::other
            > base_type;
            typedef typename base_type::key_type order_type;
            typedef typename base_type::mapped_type coefficient_type;

        public:
            container_type() : base_type(){}
            template<class Iter>
            container_type(const Iter &first, const Iter &last) : base_type(first, last){}

            struct ref_value_type{
                ref_value_type(const order_type &first_, const coefficient_type &second_) : first(first_), second(second_){}
                ref_value_type(const ref_value_type &other) : first(other.first), second(other.second){}
                ref_value_type(const std::pair<const order_type&, const coefficient_type&> &pair) : first(pair.first), second(pair.second){}
                const order_type &first;
                const coefficient_type &second;
            };

            typename base_type::iterator assign(const ref_value_type &v){
                if(v.second == 0){
                    typename base_type::iterator iter = base_type::find(v.first);
                    if(iter != base_type::end()){
                        base_type::erase(iter);
                    }
                    return base_type::end();
                }else{
                    typename base_type::iterator iter = base_type::find(v.first);
                    if(iter != base_type::end()){
                        iter->second = v.second;
                    }else{
                        base_type::insert(iter, std::make_pair(v.first, v.second));
                    }
                    return iter;
                }
            }

            typename base_type::iterator add(const ref_value_type &v){
                std::pair<typename base_type::iterator, bool> result = base_type::insert(std::make_pair(v.first, v.second));
                coefficient_type &coe(result.first->second);
                if(!result.second){
                    coe += v.second;
                }
                if(coe == 0){
                    base_type::erase(result.first);
                    return base_type::end();
                }else{
                    return result.first;
                }
            }

            typename base_type::iterator sub(const ref_value_type &v){
                std::pair<typename base_type::iterator, bool> result = base_type::insert(std::make_pair(v.first, v.second));
                coefficient_type &coe(result.first->second);
                if(result.second){
                    set_sign(coe, get_sign(result.first->second));
                }else{
                    coe -= v.second;
                }
                if(coe == 0){
                    base_type::erase(result.first);
                    return base_type::end();
                }else{
                    return result.first;
                }
            }

            typename base_type::iterator multi(const ref_value_type &v){
                typename base_type::iterator iter = base_type::find(v.first);
                if(iter == base_type::end()){ return base_type::end(); }
                coefficient_type &coe(iter.first->second), lhs(coe);
                aux::multi_dispatch(coe, lhs, v.second);
                if(coe == 0){
                    base_type::erase(iter.first);
                    return base_type::end();
                }else{
                    return iter.first;
                }
            }

            typename base_type::iterator div(const ref_value_type &v){
                typename base_type::iterator iter = base_type::find(v.first);
                if(iter == base_type::end()){ return base_type::end(); }
                coefficient_type &coe(iter.first->second), lhs(coe);
                coe = lhs / v.second;
                if(coe == 0){
                    base_type::erase(iter.first);
                    return base_type::end();
                }else{
                    return iter.first;
                }
            }

            int compare_constant(const container_type &rhs_container) const{
                bool l_empty = base_type::empty(), r_empty = rhs_container.base_type::empty();
                if(l_empty && r_empty){
                    return 0;
                }else if(!l_empty && r_empty){
                    const coefficient_type &a(base_type::begin()->second);
                    return a < 0 ? -1 : 1;
                }else if(l_empty && !r_empty){
                    const coefficient_type &a(rhs_container.base_type::begin()->second);
                    return a < 0 ? 1 : -1;
                }
                int r = 0;
                const coefficient_type &lhs_coe(base_type::begin()->second), &rhs_coe(rhs_container.base_type::begin()->second);
                if(lhs_coe < rhs_coe){
                    r = -1;
                }else if(rhs_coe < lhs_coe){
                    r = 1;
                }
                return r;
            }

            bool is_constant() const{
                return base_type::empty() || (base_type::size() == 1 && base_type::find(order_type(0)) != base_type::end());
            }
        };

        typedef typename container_type::key_type order_type;
        typedef typename container_type::mapped_type coefficient_type;

        sparse_poly() : container(){}
        sparse_poly(const sparse_poly &other) : container(other.container){}
        sparse_poly(sparse_poly &&other) : container(other.container){}
        sparse_poly(const coefficient_type &coe) : container(){
            if(coe != 0){
                container.insert(typename container_type::value_type(0, coe));
            }
        }

    private:
        sparse_poly(const container_type &other_container) : container(other_container){}
        sparse_poly(typename container_type::const_iterator first, typename container_type::const_iterator last) : container(first, last){}

    public:
        virtual ~sparse_poly(){}

        const container_type &get_container() const{ return container; }
        void assign(const sparse_poly &other){ container = other.container; }
        void assign(sparse_poly &&other){ container = other.container; }

    private:
        void assign(const container_type &other_container){ container = other_container; }
        void assign(typename container_type::const_iterator first, typename container_type::const_iterator last){
            container.clear();
            container.insert(first, last);
        }

    private:
        struct coefficient_proxy{
            coefficient_proxy(){}
            const coefficient_type &get() const{ return *coe; }
            coefficient_proxy(const coefficient_proxy &other) : container(other.container), order(other.order), coe(other.coe){}
            const coefficient_proxy &operator =(const coefficient_type &v) const{
                if(v == 0){
                    container->clear();
                }else{
                    container->assign(typename container_type::ref_value_type(order, v));
                }
                return *this;
            }

            const coefficient_proxy &operator +=(const coefficient_type &v) const{
                container->add(typename container_type::ref_value_type(order, v));
                return *this;
            }

            const coefficient_proxy &operator -=(const coefficient_type &v) const{
                container->sub(typename container_type::ref_value_type(order, v));
                return *this;
            }

            const coefficient_proxy &operator *=(const coefficient_type &v) const{
                container->multi(typename container_type::ref_value_type(order, v));
                return *this;
            }

            const coefficient_proxy &operator /=(const coefficient_type &v) const{
                container->div(typename container_type::ref_value_type(order, v));
                return *this;
            }

            const coefficient_proxy &operator ()(const coefficient_type &v) const{
                operator =(v);
                return *this;
            }

            coefficient_proxy operator [](const order_type &v) const{
                coefficient_proxy ret;
                ret.container = container;
                ret.order = v;
                return std::move(ret);
            }

            container_type *container;
            order_type order;
            coefficient_type *coe;
        };

    public:
        sparse_poly &operator =(const coefficient_type &rhs){
            assign(rhs);
            return *this;
        }

        sparse_poly &operator =(const coefficient_type &&rhs){
            assign(rhs);
            return *this;
        }

        sparse_poly &operator =(const sparse_poly &rhs){
            assign(rhs);
            return *this;
        }

        sparse_poly &operator =(const sparse_poly &&rhs){
            assign(rhs);
            return *this;
        }

        const sparse_poly &operator +() const{
            return *this;
        }

        sparse_poly operator -() const{
            sparse_poly r;
            r.sub_iterator(container.begin(), container.end());
            return std::move(r);
        }

        sparse_poly &operator +=(const sparse_poly &rhs){
            add_order_n(rhs, 0);
            return *this;
        }

        sparse_poly &operator +=(const coefficient_type &rhs){
            if(container.empty()){
                assign(rhs);
            }else{
                typename container_type::iterator iter = container.find(0);
                if(iter == container.end()){
                    container.insert(typename container_type::value_type(order_type(0), rhs));
                }else{
                    iter->second += rhs;
                    if(iter->second == 0){ container.erase(iter); }
                }
            }
            return *this;
        }

        sparse_poly operator +(const sparse_poly &rhs) const{
            sparse_poly result(*this);
            result += rhs;
            return std::move(result);
        }

        sparse_poly &operator -=(const sparse_poly &rhs){
            sub_iterator(rhs.container.begin(), rhs.container.end());
            return *this;
        }

        sparse_poly &operator -=(const coefficient_type &rhs){
            if(container.empty()){
                assign(rhs);
            }else{
                typename container_type::iterator iter = container.find(0);
                if(iter == container.end()){
                    container.insert(typename container_type::value_type(order_type(0), rhs));
                }else{
                    iter->second -= rhs;
                    if(iter->second == 0){ container.erase(iter); }
                }
            }
            return *this;
        }

        sparse_poly operator -(const sparse_poly &rhs) const{
            sparse_poly result(*this);
            result -= rhs;
            return std::move(result);
        }

        sparse_poly operator *(const sparse_poly &rhs) const{
            sparse_poly r;
            kar_multi(r, *this, rhs);
            return std::move(r);
        }

        sparse_poly &operator *=(const sparse_poly &rhs){
            assign(*this * rhs);
            return *this;
        }

        sparse_poly &operator *=(const coefficient_type &rhs){
            if(container.empty()){ return *this; }
            for(typename container_type::iterator iter = container.begin(); ; ){
                coefficient_type &coe(iter->second);
                coe *= rhs;
                if(coe == 0){
                    iter = container.erase(iter);
                }else{
                    ++iter;
                }
                if(iter == container.end()){ break; }
            }
            return *this;
        }

        sparse_poly operator /(const sparse_poly &rhs) const{
            sparse_poly r;
            if(container.empty()){
                return std::move(r);
            }
            monic_div(r, *this, rhs);
            return std::move(r);
        }

        sparse_poly &operator /=(const sparse_poly &rhs){
            assign(*this / rhs);
            return *this;
        }

        sparse_poly &operator /=(const coefficient_type &rhs){
            if(container.empty()){ return *this; }
            for(typename container_type::iterator iter = container.begin(); ; ){
                coefficient_type &coe(iter->second);
                coe /= rhs;
                if(coe == 0){
                    iter = container.erase(iter);
                }else{
                    ++iter;
                }
                if(iter == container.end()){ break; }
            }
            return *this;
        }

        sparse_poly operator %(const sparse_poly &rhs) const{
            sparse_poly r, rem;
            monic_div(r, rem, *this, rhs);
            return std::move(rem);
        }

        sparse_poly &operator %=(const sparse_poly &rhs){
            assign(*this % rhs);
            return *this;
        }

        bool operator <(const sparse_poly &rhs) const{
            return base_less_eq(false, rhs.container);
        }

        bool operator >(const sparse_poly &rhs) const{
            return rhs.operator <(*this);
        }

        bool operator <=(const sparse_poly &rhs) const{
            return base_less_eq(true, rhs.container);
        }

        bool operator <=(const coefficient_type &rhs) const{
            return base_less_eq(true, sparse_poly(rhs).container);
        }

        bool operator >=(const sparse_poly &rhs) const{
            return rhs.operator <=(*this);
        }

        bool operator >=(const coefficient_type &rhs) const{
            return sparse_poly(rhs).operator <=(*this);
        }

        bool operator ==(const sparse_poly &rhs) const{
            return container == rhs.container;
        }

        bool operator ==(const coefficient_type &rhs) const{
            return container == sparse_poly(rhs).container;
        }

        bool operator !=(const sparse_poly &rhs) const{
            return container != rhs.container;
        }

        bool operator !=(const coefficient_type &rhs) const{
            return container != sparse_poly(rhs).container;
        }

        coefficient_proxy operator [](const order_type &order){
            coefficient_proxy ret;
            ret.container = &container;
            ret.order = order;
            ret.coe = &container[order];
            return ret;
        }

        static sparse_poly &eea(
            sparse_poly &result,
            sparse_poly &c_lhs,
            sparse_poly &c_rhs,
            const sparse_poly &lhs,
            const sparse_poly &rhs
        ){
            if(lhs >= rhs){
                eea_default_impl(result, c_lhs, c_rhs, lhs, rhs);
            }else{
                eea_default_impl(result, c_rhs, c_lhs, rhs, lhs);
            }
            return result;
        }

        static sparse_poly eea(sparse_poly &c_lhs, sparse_poly &c_rhs, const sparse_poly &lhs, const sparse_poly &rhs){
            sparse_poly r;
            eea(r, c_lhs, c_rhs, lhs, rhs);
            return std::move(r);
        }

        static sparse_poly &eea_classic(
            sparse_poly &result,
            sparse_poly &c_lhs,
            sparse_poly &c_rhs,
            const sparse_poly &f,
            const sparse_poly &g
        ){
            if(f >= g){
                eea_classic_impl(result, c_lhs, c_rhs, f, g);
            }else{
                eea_classic_impl(result, c_rhs, c_lhs, g, f);
            }
            return result;
        }

        static sparse_poly eea_classic(sparse_poly &c_lhs, sparse_poly &c_rhs, const sparse_poly &f, const sparse_poly &g){
            sparse_poly r;
            eea_classic(r, c_lhs, c_rhs, f, g);
            return std::move(r);
        }

        static sparse_poly &gcd_coefficient(sparse_poly &r, const sparse_poly &f, const coefficient_type &p){
            r = f;
            for(typename container_type::iterator iter = r.container.begin(), end = r.container.end(); iter != end; ++iter){
                coefficient_type &coe(iter->second);
                coefficient_type r;
                gcd_dispatch(r, coe, p);
                coe = std::move(r);
            }
            return r;
        }

        sparse_poly gcd_coefficient(const coefficient_type &p) const{
            sparse_poly r;
            gcd_coefficient(r, *this, p);
            return std::move(r);
        }

        static sparse_poly &modular_gcd(
            sparse_poly &result,
            const sparse_poly &f,
            const sparse_poly &g,
            const coefficient_type &p
        ){
            if(f >= g){
                modular_gcd_impl(result, f, g, p);
            }else{
                modular_gcd_impl(result, f + g, g, p);
            }
            return result;
        }

        static sparse_poly modular_gcd(
            const sparse_poly &f,
            const sparse_poly &g,
            const coefficient_type &p
        ){
            sparse_poly r;
            modular_gcd(r, f, g, p);
            return std::move(r);
        }

        static sparse_poly &modular_eea(
            sparse_poly &result,
            sparse_poly &c_lhs,
            sparse_poly &c_rhs,
            const sparse_poly &f,
            const sparse_poly &g,
            const coefficient_type &p
        ){
            if(g.base_modular_less_eq(true, f.container, p)){
                modular_eea_impl(result, c_lhs, c_rhs, f, g, p);
            }else{
                modular_eea_impl(result, c_rhs, c_lhs, g, f, p);
            }
            return result;
        }

        static sparse_poly modular_eea(
            sparse_poly &c_rhs,
            sparse_poly &c_lhs,
            const sparse_poly &f,
            const sparse_poly &g,
            const coefficient_type &p
        ){
            sparse_poly r;
            modular_eea(r, c_rhs, c_lhs, f, g, p);
            return std::move(r);
        }

        static sparse_poly &primitive_gcd(sparse_poly &result, const sparse_poly &f, const sparse_poly &g){
            if(f >= g){
                primitive_gcd_impl(result, f, g);
            }else{
                primitive_gcd_impl(result, g, f);
            }
            return result;
        }

        static sparse_poly primitive_gcd(const sparse_poly &f, const sparse_poly &g){
            sparse_poly r;
            primitive_gcd(r, f, g);
            return std::move(r);
        }

        order_type deg() const{
            if(container.empty()){
                return order_type(0);
            }else{
                return container.rbegin()->first;
            }
        }

        const order_type &ref_deg() const{
            return container.rbegin()->first;
        }

        const coefficient_type &lc() const{ return container.rbegin()->second; }

        static sparse_poly &normal(sparse_poly &result, const sparse_poly &x){
            if(x.container.empty()){
                result.container.clear();
                return result;
            }
            result.container = x.container;
            result /= x.lc();
            return result;
        }

        sparse_poly normal() const{
            sparse_poly r;
            normal(r, *this);
            return std::move(r);
        }

        sparse_poly &normalize(){
            sparse_poly r;
            normal(r, *this);
            *this = std::move(r);
            return *this;
        }

        static sparse_poly &modular_normal(sparse_poly &result, const sparse_poly &x, const coefficient_type &p){
            if(x.container.empty()){
                result.container.clear();
                return result;
            }
            result.container = x.container;
            coefficient_type xlc = coefficient_inverse(x.lc(), p);
            for(
                typename container_type::iterator iter = result.container.begin(), end = result.container.end();
                iter != end;
                ++iter
            ){
                coefficient_type &coe(iter->second);
                coe = (coe * xlc) % p;
            }
            return result;
        }

        sparse_poly &modular_normal(const coefficient_type &p) const{
            sparse_poly r;
            modular_normal(r, *this, p);
            return std::move(r);
        }

        sparse_poly &modular_normalize(const coefficient_type &p){
            sparse_poly r;
            modular_normal(r, *this, p);
            *this = std::move(r);
            return *this;
        }

        sparse_poly lu() const{
            if(container.empty()){ return coefficient_type(1); }
            sparse_poly na;
            normal(na, *this);
            return *this / na;
        }

        coefficient_type norm1() const{
            coefficient_type s;
            for(typename container_type::const_iterator iter = container.begin(), end = container.end(); iter != end; ++iter){
                s += abs_dispatch(iter->second);
            }
            return std::move(s);
        }

        coefficient_type infinity_norm() const{
            coefficient_type r = 0;
            for(typename container_type::const_iterator iter = container.begin(), end = container.end(); iter != end; ++iter){
                const coefficient_type &coe(iter->second);
                max_dispatch(r, coe);
            }
            set_sign_dispatch(r, true);
            return std::move(r);
        }

        bool is_monic() const{
            return !container.empty() && container.rbegin()->second == 1;
        }

        static sparse_poly &pp(sparse_poly &result, const sparse_poly &x){
            result = x;
            coefficient_type c = x.cont();
            if(c == 1){ return result; }
            for(
                typename container_type::iterator iter = result.container.begin(), end = result.container.end();
                iter != end;
                ++iter
            ){
                coefficient_type &coe(iter->second);
                coe /= c;
                if(coe == 0){ iter = result.container.erase(iter); }
            }
            return result;
        }

        sparse_poly pp() const{
            sparse_poly r;
            pp(r, *this);
            return std::move(r);
        }

        static sparse_poly &inverse(sparse_poly &result, const sparse_poly &f, const order_type &l){
            result.container.clear();
            sparse_poly &g(result);
            g[0] = 1;
            std::size_t r = ceil_log2_dispatch(l);
            order_type rem(1);
            for(std::size_t i = 0; i < r; ++i){
                rem <<= 1;
                sparse_poly next_g(g);
                for(typename container_type::iterator double_g_iter = next_g.container.begin(), double_g_end = next_g.container.end(); double_g_iter != double_g_end; ++double_g_iter){
                    coefficient_type coe_double_g(double_g_iter->second);
                    double_g_iter->second = coe_double_g * 2;
                }
                next_g -= f * g * g;
                if(next_g.ref_deg() >= rem){
                    next_g.container.erase(next_g.container.find(rem), next_g.container.end());
                }
                g = std::move(next_g);
            }
            return result;
        }

        sparse_poly inverse(const order_type &l) const{
            sparse_poly r;
            inverse(r, *this, l);
            return std::move(r);
        }

        static sparse_poly &modular_inverse(sparse_poly &result, const sparse_poly &a, const coefficient_type &m){
            if(a >= m){
                modular_inverse_impl(result, a, m);
            }else{
                modular_inverse_impl(result, a + m, m);
            }
            return result;
        }

        sparse_poly modular_inverse(const coefficient_type &m) const{
            sparse_poly result;
            modular_inverse(result, *this, m);
            return std::move(result);
        }

        static coefficient_type coefficient_inverse(const coefficient_type &f, const coefficient_type &g){
            coefficient_type r;
            if(f >= g){
                r = modular_reduce_coefficient(coefficient_inverse_impl(f, g), g);
            }else{
                r = modular_reduce_coefficient(coefficient_inverse_impl(f + g, g), g);
            }
            return std::move(r);
        }

        coefficient_type cont() const{
            coefficient_type result;
            if(container.empty()){
                result = coefficient_type(0);
                return result;
            }
            result = container.begin()->second;
            if(container.size() == 1){
                result.normalize();
            }
            typename container_type::const_iterator iter = container.begin(), end = container.end(); ++iter;
            for(; iter != end; ++iter){
                coefficient_type temp;
                gcd_dispatch(temp, result, iter->second);
                result = std::move(temp);
                if(result == 1){ break; }
            }
            return std::move(result);
        }

        static sparse_poly &gcd(sparse_poly &result, const sparse_poly &f, const sparse_poly &g){
            if(f >= g){
                return gcd_default_impl(result, f, g);
            }else{
                return gcd_default_impl(result, g, f);
            }
        }

        static sparse_poly gcd(const sparse_poly &f, const sparse_poly &g){
            sparse_poly r;
            gcd(r, f, g);
            std::move(r);
        }

        static sparse_poly &mod_coefficient(sparse_poly &r, const sparse_poly &f, const coefficient_type &p){
            r = f;
            for(typename container_type::iterator iter = r.container.begin(); iter != r.container.end(); ){
                coefficient_type &coe(iter->second);
                coe = modular_reduce_coefficient(coe, p);
                if(coe == 0){ r.container.erase(iter++); }else{ ++iter; }
            }
            return r;
        }

        sparse_poly mod_coefficient(const coefficient_type &p) const{
            sparse_poly r;
            mod_coefficient(r, *this, p);
            return std::move(r);
        }

        template<class VIter, class MIter>
        static sparse_poly &cra(
            sparse_poly &result,
            VIter v_first,
            VIter v_end,
            MIter m_first
        ){
            result.container.clear();
            sparse_poly m_div_mi;
            coefficient_type prod_m = 1;
            {
                MIter m_iter = m_first;
                for(VIter v_iter = v_first; v_iter != v_end; ++v_iter, ++m_iter){
                    prod_m *= *m_iter;
                }
            }
            VIter v_iter = v_first;
            MIter m_iter = m_first;
            for(; v_iter != v_end; ++v_iter, ++m_iter){
                const coefficient_type &m(*m_iter);
                m_div_mi = prod_m / m;
                result = (result + *v_iter * m_div_mi * m_div_mi.modular_inverse(m)) % prod_m;
            }
            return result;
        }

        template<class VIter, class MIter>
        static sparse_poly cra(VIter v_first, VIter v_end, MIter m_first){
            sparse_poly r;
            cra(r, v_first, v_end, m_first);
            return std::move(r);
        }

        template<class Variable>
        std::string to_string(const Variable &v) const{
            return to_string_impl<std::string, char, Variable, std::ostringstream>(
                v,
                '*', '^', '+', '-', '0', '(', ')'
            );
        }

        std::string to_string() const{
            return to_string("x");
        }

        template<class Variable>
        std::wstring to_wstring(const Variable &v) const{
            return to_string_impl<std::wstring, wchar_t, Variable, std::wostringstream>(
                v,
                L'*', L'^', L'+', L'-', L'0', L'(', L')'
            );
        }

        std::wstring to_wstring() const{
            return to_wstring(L"x");
        }

    private:
#define CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_TO_DOUBLE template<class T, double (T::*Func)() const>
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(to_double, CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_TO_DOUBLE);
        template<class T>
        static double to_double_dispatch(const T &a, typename boost::enable_if<has_to_double<T>>::type* = nullptr){
            return a.to_double();
        }

        template<class T>
        static double to_double_dispatch(const T &a, typename boost::disable_if<has_to_double<T>>::type* = nullptr){
            return static_cast<double>(a);
        }

#define CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_TO_UNSIGNED_INT template<class T, unsigned int (T::*Func)() const>
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(to_unsigned_int, CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_TO_UNSIGNED_INT);
        template<class T>
        static unsigned int to_unsigned_int_dispatch(const T &a, typename boost::enable_if<has_to_unsigned_int<T>>::type* = nullptr){
            return a.to_unsigned_int();
        }

        template<class T>
        static unsigned int to_unsigned_int_dispatch(const T &a, typename boost::disable_if<has_to_unsigned_int<T>>::type* = nullptr){
            return static_cast<unsigned int>(a);
        }

#define CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_EEA template<class T, T &(Func)(T&, T&, T&, const T&, const T&)>
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(eea, CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_EEA);
        template<class T>
        static T &eea_dispatch(T &result, T &s, T &t, const T &f, const T &g, typename boost::enable_if<has_eea<T>>::type* = nullptr){
            T::eea(result, s, t, f, g);
            return result;
        }

        template<class T>
        static T &eea_dispatch(T &result, T &s, T &t, const T &f, const T &g, typename boost::disable_if<has_eea<T>>::type* = nullptr){
            result = aux::eea_classic(s, t, f, g);
            return result;
        }

#define CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_SQRT template<class T, T &(Func)(T&, const T&)>
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(sqrt, CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_SQRT);
        template<class T>
        static T sqrt_dispatch(const T &a, typename boost::enable_if<has_sqrt<T>>::type* = nullptr){
            T result;
            return T::sqrt(result, a);
        }

        template<class T>
        static T sqrt_dispatch(const T &a, typename boost::disable_if<has_sqrt<T>>::type* = nullptr){
            T result = a / T(2), prev_x = result;
            do{
                prev_x = result;
                result = (result + a / result) / T(2);
                if(result * result <= a){ break; }
            }while(prev_x != result);
            return std::move(result);
        }

#define CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_POW template<class T, T &(Func)(T&, const T&, const T&)>
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(pow, CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_POW);
        template<class T>
        static T pow_dispatch(const T &a, const T &b, typename boost::enable_if<has_pow<T>>::type* = nullptr){
            T r;
            T::pow(r, a, b);
            return std::move(r);
        }

        template<class T>
        static T pow_dispatch(const T &a, const T &b, typename boost::disable_if<has_pow<T>>::type* = nullptr){
            T result = T(1);
            for(T count = T(0); count < b; count += T(1)){
                result *= a;
            }
            return std::move(result);
        }

#define CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_GCD template<class T, T &(Func)(T&, const T&, const T&)>
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(gcd, CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_GCD);
        template<class T>
        static void gcd_dispatch(T &result, const T &a, const T &b, typename boost::enable_if<has_gcd<T>>::type* = nullptr){
            T::gcd(result, a, b);
        }

        template<class T>
        static void gcd_dispatch(T &result, const T &a, const T &b, typename boost::disable_if<has_gcd<T>>::type* = nullptr){
            result = aux::gcd(a, b);
        }

#define CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_ABS_MAX template<class T, const T &(Func)(const T&, const T&)>
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(abs_max, CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_ABS_MAX);
        template<class T>
        static void max_dispatch(T &a, const T &b, typename boost::enable_if<has_abs_max<T>>::type* = nullptr){
            const T *ptr = &((T::abs_max)(a, b));
            if(ptr != &a){ a = b; }
        }

        template<class T>
        static void max_dispatch(T &a, const T &b, typename boost::disable_if<has_abs_max<T>>::type* = nullptr){
            a = (std::max)(std::abs(a), std::abs(b));
        }

#define CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_CEIL_POW2 template<class T, T (T::*Func)() const>
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(ceil_pow2, CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_CEIL_POW2);
        template<class T>
        static void ceil_pow2_dispatch(T &x, const T &y, typename boost::enable_if<has_ceil_pow2<T>>::type* = nullptr){
            x = y.ceil_pow2();
        }

        template<class T>
        static void ceil_pow2_dispatch(T &x, const T &y, typename boost::disable_if<has_ceil_pow2<T>>::type* = nullptr){
            x = aux::ceil_pow2(y);
        }

#define CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_CEIL_LOG2 template<class T, std::size_t (T::*Func)() const>
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(ceil_log2, CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_CEIL_LOG2);
        template<class T>
        static std::size_t ceil_log2_dispatch(const T &x, typename boost::enable_if<has_ceil_log2<T>>::type* = nullptr){
            return x.ceil_log2();
        }

        template<class T>
        static std::size_t ceil_log2_dispatch(const T &x, typename boost::disable_if<has_ceil_log2<T>>::type* = nullptr){
            return aux::ceil_log2(x);
        }

#define CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_SIGN template<class T, aux::sign T::*Sign>
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(sign, CPP_MULTI_PRECISION_AUX_SIGNATURE_SPARSE_POLY_SIGN);
        template<class T>
        static T abs_dispatch(T x, typename boost::enable_if<has_sign<T>>::type* = nullptr){
            x.sign = true;
            return std::move(x);
        }

        template<class T>
        static T abs_dispatch(T x, typename boost::disable_if<has_sign<T>>::type* = nullptr){
            if(x >= 0){
                return std::move(x);
            }else{
                return std::move(-x);
            }
        }

        template<class T>
        static bool get_sign_dispatch(const T &x, typename boost::enable_if<has_sign<T>>::type* = nullptr){
            return x.sign;
        }

        template<class T>
        static bool get_sign_dispatch(const T &x, typename boost::disable_if<has_sign<T>>::type* = nullptr){
            return x >= 0;
        }

        template<class T>
        static void set_sign_dispatch(T &x, bool b, typename boost::enable_if<has_sign<T>>::type* = nullptr){
            x.sign = b;
        }

        template<class T>
        static void set_sign_dispatch(T &x, bool b, typename boost::disable_if<has_sign<T>>::type* = nullptr){
            if(b){
                if(x < 0){ x = -x; }
            }else{
                if(x > 0){ x = -x; }
            }
        }

        template<class T>
        static void negate_dispatch(T &x, typename boost::enable_if<has_sign<T>>::type* = 0){
            x.sign = !x.sign;
        }

        template<class T>
        static void negate_dispatch(T &x, typename boost::disable_if<has_sign<T>>::type* = 0){
            x = -x;
        }

        template<class T>
        static void assign_reverse_dispatch(T &x, const T &y, typename boost::enable_if<has_sign<T>>::type* = 0){
            x.sign = !y.sign;
        }

        template<class T>
        static void assign_reverse_dispatch(T &x, const T &y, typename boost::disable_if<has_sign<T>>::type* = 0){
            if(y > 0){
                if(x > 0){ x = -x; }
            }else{
                if(x < 0){ x = -x; }
            }
        }

        template<class T>
        static bool get_sign(const T &x, typename boost::enable_if<has_sign<T>>::type* = 0){
            return x.sign;
        }

        template<class T>
        static bool get_sign(const T &x, typename boost::disable_if<has_sign<T>>::type* = 0){
            return x >= 0;
        }

        template<class T>
        static void set_sign(T &x, bool b, typename boost::enable_if<has_sign<T>>::type* = 0){
            x.sign = b;
        }

        template<class T>
        static void set_sign(T &x, bool b, typename boost::disable_if<has_sign<T>>::type* = 0){
            if(b){
                if(x < T(0)){ x = -x; }
            }else{
                if(x > T(0)){ x = -x; }
            }
        }

        static sparse_poly &square_multi(sparse_poly &result, const sparse_poly &lhs, const sparse_poly &rhs){
            square_multi_impl(result, lhs.container.begin(), lhs.container.end(), rhs.container.begin(), rhs.container.end());
            return result;
        }

        struct divisor_default{
            template<class T>
            T operator ()(const T &x, const T &y) const{ return x / y; }
        };

        struct modulo_default{
            template<class T>
            void operator ()(const T&) const{}
        };

        template<bool Rem, class Modulo, class CoefficientModulo, class Divisor>
        static sparse_poly &square_div(
            sparse_poly &q,
            sparse_poly &r,
            const sparse_poly &a,
            const sparse_poly &b,
            const Modulo &modulo,
            const CoefficientModulo &coe_modulo,
            const Divisor &div
        ){
            square_div_impl<Rem>(q, r, a, b, modulo, coe_modulo, div);
            if(Rem){ set_sign(r, get_sign_dispatch(b)); }
            return q;
        }

        template<bool Rem, class Modulo, class CoefficientModulo, class Divisor>
        static sparse_poly &square_div_impl(
            sparse_poly &q,
            sparse_poly &r,
            const sparse_poly &a,
            const sparse_poly &b,
            const Modulo &modulo,
            const CoefficientModulo &coe_modulo,
            const Divisor &div
        ){
            r = a;
            q.container.clear();
            if(a.container.empty() || a.deg() < b.deg()){ return q; }
            const order_type &m(b.ref_deg());
            const coefficient_type &u(b.lc());
            for(typename container_type::reverse_iterator iter = r.container.rbegin(); ; ){
                order_type n = iter->first - m;
                if(n < 0){ break; }
                coefficient_type qn = div(iter->second, u);
                coe_modulo(qn);
                if(qn == 0){
                    if(r.ref_deg() <= b.ref_deg()){ break; }
                    ++iter;
                    if(iter == r.container.rend()){ break; }else{ continue; }
                }else{
                    q.addition_order_coe(n, qn);
                    std::size_t r_size = r.container.size();
                    r.sub_n_q(b, n, qn);
                    if(r_size > r.container.size()){
                        iter = r.container.rbegin();
                    }else{ ++iter; }
                    modulo(r);
                    if(r.container.empty()){ break; }
                    iter = r.container.rbegin();
                }
            }
            if(Rem && !q.affirmation_plus()){
                r = minus_rem(a, b, q);
                modulo(r);
            }
            return q;
        }

        bool affirmation_plus() const{
            typename container_type::const_iterator iter = container.find(order_type(0));
            return iter == container.end() || iter->second >= coefficient_type(0);
        }

        coefficient_type constant_part() const{
            typename container_type::const_iterator iter = container.find(order_type(0));
            if(iter == container.end()){
                return coefficient_type(0);
            }else{
                return iter->second;
            }
        }

        static sparse_poly plus_rem(const sparse_poly &lhs, const sparse_poly &rhs, const sparse_poly &quo){
            return lhs - quo * rhs;
        }

        static sparse_poly minus_rem(const sparse_poly &lhs, const sparse_poly &rhs, const sparse_poly &quo){
            return plus_rem(lhs, rhs, quo);
        }

        static sparse_poly &monic_div(sparse_poly &result, sparse_poly &rem, const sparse_poly &lhs, const sparse_poly &rhs){
            return monic_div_impl<true>(result, rem, lhs, rhs);
        }
        
        static sparse_poly &monic_div(sparse_poly &result, const sparse_poly &lhs, const sparse_poly &rhs){
            sparse_poly rem;
            return monic_div_impl<false>(result, rem, lhs, rhs);
        }

        static sparse_poly &monic_mod(sparse_poly &rem, const sparse_poly &lhs, const sparse_poly &rhs){
            sparse_poly result;
            monic_div_impl<true>(result, rem, lhs, rhs);
            return rem;
        }

        bool base_less_eq(bool final, const container_type &rhs_container) const{
            if(container.is_constant() && rhs_container.is_constant()){
                switch(container.compare_constant(rhs_container)){
                case 0:
                    return final;
                case -1:
                    return true;
                case 1:
                    return false;
                }
            }
            typename container_type::const_reverse_iterator
                lhs_iter = container.rbegin(),
                lhs_end = container.rend(),
                rhs_iter = rhs_container.rbegin(),
                rhs_end = rhs_container.rend();
            bool end_l = lhs_iter == lhs_end, end_r = rhs_iter == rhs_end;
            if(!end_l || !end_r){
                if(end_l && !end_r){ return true; }
                if(!end_l && end_r){ return false; }
                for(; lhs_iter != lhs_end && rhs_iter != rhs_end; ++lhs_iter, ++rhs_iter){
                    const order_type &lhs_order(lhs_iter->first), &rhs_order(rhs_iter->first);
                    if(lhs_order == rhs_order){
                        const coefficient_type &lhs_coe(lhs_iter->second), &rhs_coe(rhs_iter->second);
                        if(lhs_coe < rhs_coe){
                            return true;
                        }else if(lhs_coe > rhs_coe){
                            return false;
                        }
                    }else if(lhs_order < rhs_order){
                        return true;
                    }else{
                        return false;
                    }
                }
                if(lhs_iter == lhs_end && rhs_iter != rhs_end){ return false; }
                if(lhs_iter != lhs_end && rhs_iter == rhs_end){ return true; }
            }
            return final;
        }

        bool base_modular_less_eq(bool final, const container_type &rhs_container, const coefficient_type &p) const{
            if(container.is_constant() && rhs_container.is_constant()){
                switch(container.compare_constant(rhs_container)){
                case 0:
                    return final;
                case -1:
                    return true;
                case 1:
                    return false;
                }
            }
            typename container_type::const_reverse_iterator
                lhs_iter = container.rbegin(),
                lhs_end = container.rend(),
                rhs_iter = rhs_container.rbegin(),
                rhs_end = rhs_container.rend();
            bool end_l = lhs_iter == lhs_end, end_r = rhs_iter == rhs_end;
            if(!end_l || !end_r){
                if(end_l && !end_r){ return true; }
                if(!end_l && end_r){ return false; }
                for(; lhs_iter != lhs_end && rhs_iter != rhs_end; ++lhs_iter, ++rhs_iter){
                    const order_type &lhs_order(lhs_iter->first), &rhs_order(rhs_iter->first);
                    if(lhs_order == rhs_order){
                        const coefficient_type
                            lhs_coe(modular_reduce_coefficient(lhs_iter->second, p)),
                            rhs_coe(modular_reduce_coefficient(rhs_iter->second, p));
                        if(lhs_coe < rhs_coe){
                            return true;
                        }else if(lhs_coe > rhs_coe){
                            return false;
                        }
                    }else if(lhs_order < rhs_order){
                        return true;
                    }else{
                        return false;
                    }
                }
                if(lhs_iter == lhs_end && rhs_iter != rhs_end){ return false; }
                if(lhs_iter != lhs_end && rhs_iter == rhs_end){ return true; }
            }
            return final;
        }

        void sign_reverse(){
            coefficient_type a = -1;
            for(typename container_type::iterator iter = container.begin(), end = container.end(); iter != end; ++iter){
                iter->second *= a;
            }
        }

        static coefficient_type &modular_reduce_coefficient(
            coefficient_type &r,
            const coefficient_type &a,
            const coefficient_type &m
        ){
            r = a;
            if(get_sign_dispatch(r)){
                r %= m;
            }else{
                set_sign_dispatch(r, true);
                coefficient_type s = r % m;
                if(s > 0){
                    r = m - s;
                }else{
                    r = 0;
                }
            }
            return r;
        }

        static coefficient_type modular_reduce_coefficient(const coefficient_type &a, const coefficient_type &m){
            coefficient_type r;
            modular_reduce_coefficient(r, a, m);
            return std::move(r);
        }

        static sparse_poly &kar_multi(sparse_poly &result, const sparse_poly &lhs, const sparse_poly &rhs){
            return result = kar_multi_impl_n(lhs, rhs);
        }

        void add_order_n(const sparse_poly &rhs, const order_type &n){
            for(typename container_type::const_iterator rhs_iter = rhs.container.begin(), rhs_end = rhs.container.end(); rhs_iter != rhs_end; ++rhs_iter){
                order_type order = rhs_iter->first;
                order += n;
                const coefficient_type &coe(rhs_iter->second);
                typename container_type::iterator iter = container.find(order);
                if(iter == container.end()){
                    iter = container.insert(iter, std::make_pair(order, coe));
                }else{
                    coefficient_type &lhs_coe(iter->second);
                    lhs_coe += coe;
                    if(lhs_coe == 0){ container.erase(iter); }
                }
            }
        }

        static void square_multi_impl(
            sparse_poly &result,
            const typename container_type::const_iterator &lhs_first, const typename container_type::const_iterator &lhs_last,
            const typename container_type::const_iterator &rhs_first, const typename container_type::const_iterator &rhs_last
        ){
            result.container.clear();
            order_type temp_order;
            coefficient_type temp_coefficient;
            for(typename container_type::const_iterator lhs_iter = lhs_first, lhs_end = lhs_last; lhs_iter != lhs_end; ++lhs_iter){
                const order_type &lhs_order(lhs_iter->first);
                const coefficient_type &lhs_coe(lhs_iter->second);
                for(typename container_type::const_iterator rhs_iter = rhs_first, rhs_end = rhs_last; rhs_iter != rhs_end; ++rhs_iter){
                    const order_type &rhs_order(rhs_iter->first);
                    const coefficient_type &rhs_coe(rhs_iter->second);
                    temp_order = rhs_order;
                    temp_order += lhs_order;
                    temp_coefficient = lhs_coe * rhs_coe;
                    result.addition_order_coe(temp_order, temp_coefficient);
                }
            }
        }

        static sparse_poly kar_multi_impl_n(const sparse_poly &f, const sparse_poly &g){
            if(f.container.empty() || g.container.empty()){
                return sparse_poly();
            }
            const order_type &order_f(f.ref_deg()), &order_g(g.ref_deg());
            order_type n;
            ceil_pow2_dispatch(n, order_f > order_g ? order_f : order_g);
            if(n < 2){
                sparse_poly result;
                square_multi(result, f, g);
                return std::move(result);
            }
            n >>= 1;
            sparse_poly f_0, f_1, g_0, g_1, ff, gg;
            typename container_type::const_iterator
                upper_bound_f = f.container.upper_bound(n),
                upper_bound_g = g.container.upper_bound(n);
            kar_multi_add_ffgg(f_0, f.container.begin(), upper_bound_f, 0);
            kar_multi_add_ffgg(g_0, g.container.begin(), upper_bound_g, 0);
            kar_multi_add_ffgg(f_1, upper_bound_f, f.container.end(), n);
            kar_multi_add_ffgg(g_1, upper_bound_g, g.container.end(), n);
            ff = f_0 + f_1, gg = g_0 + g_1;
            sparse_poly
                fg_0 = kar_multi_impl_n(f_0, g_0),
                fg_1 = kar_multi_impl_n(f_1, g_1),
                ffgg = kar_multi_impl_n(ff, gg);
            f_0.container.clear(), f_1.container.clear();
            g_0.container.clear(), g_1.container.clear();
            ff.container.clear(), gg.container.clear();
            ffgg -= fg_0, ffgg -= fg_1;
            sparse_poly result;
            result += fg_0;
            ffgg.radix_shift(n);
            result += ffgg;
            n <<= 1;
            fg_1.radix_shift(n);
            result += fg_1;
            return std::move(result);
        }

        static void kar_multi_add_ffgg(
            sparse_poly &result,
            typename container_type::const_iterator first,
            typename container_type::const_iterator last,
            const order_type &n
        ){
            for(; first != last; ++first){
                result.container[first->first - n] = first->second;
            }
        }

        void sub_iterator(typename container_type::const_iterator rhs_iter, typename container_type::const_iterator rhs_end){
            sub_iterator(rhs_iter, rhs_end, 0);
        }

        void sub_iterator(typename container_type::const_iterator rhs_iter, typename container_type::const_iterator rhs_end, const order_type &xrhs){
            for(; rhs_iter != rhs_end; ++rhs_iter){
                order_type order(rhs_iter->first);
                order -= xrhs;
                const coefficient_type &coe(rhs_iter->second);
                typename container_type::iterator iter = container.find(order);
                if(iter == container.end()){
                    iter = container.add(typename container_type::ref_value_type(order, coe));
                    if(iter != container.end()){ negate_dispatch(iter->second); }
                }else{
                    coefficient_type &lhs_coe(iter->second);
                    lhs_coe -= coe;
                    if(lhs_coe == 0){ container.erase(iter); }
                }
            }
        }

        void radix_shift(const order_type &n){
            for(typename container_type::iterator iter = container.begin(), end = container.end(); iter != end; ++iter){
                const_cast<order_type&>(iter->first) += n;
            }
        }

        template<bool Rem>
        static sparse_poly &monic_div_impl(sparse_poly &result, sparse_poly &rem, const sparse_poly &lhs, const sparse_poly &rhs){
            if(!rhs.is_monic()){
                return square_div<Rem>(result, rem, lhs, rhs, modulo_default(), modulo_default(), divisor_default());
            }
            result.container.clear();
            if(rhs.ref_deg() > lhs.ref_deg()){
                if(Rem){ rem.assign(lhs); }
                return result;
            }
            order_type m(lhs.ref_deg() - rhs.ref_deg());
            sparse_poly inv_rev_rhs;
            {
                sparse_poly rev_rhs;
                rev(rev_rhs, rhs);
                inverse(inv_rev_rhs, rev_rhs, m + 1);
            }
            {
                sparse_poly rev_lhs;
                rev(rev_lhs, lhs);
                result = rev_lhs * inv_rev_rhs;
            }
            result.container.erase(result.container.upper_bound(m), result.container.end());
            result.rev(m);
            if(Rem){
                if(result.affirmation_plus()){
                    rem = plus_rem(lhs, rhs, result);
                }else{
                    rem = minus_rem(lhs, rhs, result);
                }
            }
            return result;
        }

        static void rev(sparse_poly &result, const sparse_poly &a){
            result.container.clear();
            if(a.container.empty()){ return; }
            const order_type &order(a.ref_deg());
            for(
                typename container_type::const_reverse_iterator iter = a.container.rbegin(),
                end = a.container.rend();
                iter != end; ++iter
            ){
                order_type x(order);
                x -= iter->first;
                result.container.insert(std::make_pair(std::move(x), iter->second));
            }
        }

        void rev(){
            sparse_poly result;
            rev(result, *this);
            assign(std::move(result));
        }

        void rev(const order_type &n){
            if(container.find(n) != container.end()){
                sparse_poly result;
                rev(result, *this);
                assign(std::move(result));
            }else{
                sparse_poly result;
                container.insert(std::make_pair(n, 0));
                for(
                    typename container_type::const_iterator iter = container.begin(), end = container.lower_bound(n);
                    iter != end;
                    ++iter
                ){
                    order_type x(n);
                    x -= iter->first;
                    result.container.insert(std::make_pair(std::move(x), iter->second));
                }
                assign(std::move(result));
            }
        }

        static coefficient_type coefficient_inverse_impl(const coefficient_type &f, const coefficient_type &g){
            coefficient_type r = 0;
            if(f == 0){ return std::move(r); }
            coefficient_type r_0 = f, r_1 = g, s_0 = 1, s_1 = 0;
            while(r_1 != 0){
                coefficient_type q = r_0 / r_1, r_m = r_1, s_m = s_1;
                r_1 = r_0 - q * r_1;
                s_1 = s_0 - q * s_1;
                r_0 = std::move(r_m);
                s_0 = std::move(s_m);
            }
            return std::move(s_0);
        }

        static sparse_poly &modular_inverse_impl(sparse_poly &result, const sparse_poly &a, const sparse_poly &m){
            sparse_poly r_0 = a, r_1 = m, s_0 = coefficient_type(1), s_1 = coefficient_type(0);
            while(!r_1.container.empty()){
                sparse_poly q = r_0 / r_1, r_m = r_1, s_m = s_1;
                r_1 = r_0 - q * r_1;
                s_1 = s_0 - q * s_1;
                r_0 = std::move(r_m);
                s_0 = std::move(s_m);
            }
            result = std::move(s_0);
            result = result.mod_coefficient(m.container.begin()->second);
            return result;
        }

        static sparse_poly &gcd_default_impl(sparse_poly &result, const sparse_poly &f, const sparse_poly &g){
            result.container.clear();
            if(g.container.empty()){ return result; }
            sparse_poly r_0, r_1;
            bool zero_flag[2] = { false, false };
            if(f.ref_deg() > 0){
                normal(r_0, f);
            }else{
                r_0 = f;
                zero_flag[0] = true;
            }
            if(g.ref_deg() > 0){
                normal(r_1, g);
            }else{
                r_1 = g;
                zero_flag[1] = true;
            }
            if(zero_flag[0] && zero_flag[1]){
                coefficient_type r;
                gcd_dispatch(r, r_0.container.rbegin()->second, r_1.container.rbegin()->second);
                result.container.insert(std::make_pair(order_type(0), std::move(r)));
                return result;
            }
            if(zero_flag[0] || zero_flag[1]){
                result.container.insert(std::make_pair(order_type(0), coefficient_type(1)));
                return result;
            }
            while(!r_1.container.empty()){
                sparse_poly q = r_0 / r_1, rho = r_0 - q * r_1, r_m = r_1;
                rho.lu();
                r_1 = (r_0 - q * r_1) / rho;
                r_0 = std::move(r_m);
            }
            result = std::move(r_0);
            return result;
        }

        static sparse_poly &eea_classic_impl(
            sparse_poly &result,
            sparse_poly &c_lhs,
            sparse_poly &c_rhs,
            const sparse_poly &f,
            const sparse_poly &g
        ){
            sparse_poly
                r_0 = f, r_1 = g,
                s_0 = coefficient_type(1), s_1 = coefficient_type(0),
                t_0 = coefficient_type(0), t_1 = coefficient_type(1);
            while(!r_1.container.empty()){
                sparse_poly q = r_0 / r_1, r_m = r_1, s_m = s_1, t_m = t_1;
                if(q.container.empty()){
                    result = 1;
                    c_lhs = 0;
                    c_rhs = 1 / g;
                    return result;
                }
                r_1 = r_0 - q * r_1;
                s_1 = s_0 - q * s_1;
                t_1 = t_0 - q * t_1;
                r_0 = std::move(r_m);
                s_0 = std::move(s_m);
                t_0 = std::move(t_m);
            }
            result = std::move(r_0);
            c_lhs = std::move(s_0);
            c_rhs = std::move(t_0);
            return result;
        }

        template<class ValueType>
        static ValueType cra_inner_eea(const ValueType &f, const ValueType &g){
            ValueType s, t;
            if(f >= g){
                cra_inner_eea_impl(s, t, f, g);
            }else{
                cra_inner_eea_impl(t, s, g, f);
            }
            return std::move(s);
        }

        template<class ValueType>
        static void cra_inner_eea_impl(ValueType &s, ValueType &t, const ValueType &f, const ValueType &g){
            ValueType
                r_0 = f, r_1 = g,
                s_0 = ValueType(1), s_1 = ValueType(0),
                t_0 = ValueType(0), t_1 = ValueType(1);
            while(r_1 != 0){
                ValueType q = r_0 / r_1, r_m = r_1, s_m = s_1, t_m = t_1;
                if(q == 0){
                    s = 0;
                    t = 1 / g;
                    return;
                }
                r_1 = r_0 - q * r_1;
                s_1 = s_0 - q * s_1;
                t_1 = t_0 - q * t_1;
                r_0 = std::move(r_m);
                s_0 = std::move(s_m);
                t_0 = std::move(t_m);
            }
            s = std::move(s_0);
            t = std::move(t_0);
        }

        static sparse_poly &eea_default_impl(
            sparse_poly &result,
            sparse_poly &c_lhs,
            sparse_poly &c_rhs,
            const sparse_poly &f,
            const sparse_poly &g
        ){
            result.container.clear();
            if(g.container.empty()){
                c_lhs.container.clear();
                c_rhs.container.clear();
                return result;
            }
            sparse_poly
                rho_0 = f.lc(), rho_1 = g.lc(),
                r_0, r_1,
                s_0 = coefficient_type(1) / rho_0, s_1 = coefficient_type(0),
                t_0 = coefficient_type(0), t_1 = coefficient_type(1) / rho_1;
            normal(r_0, f), normal(r_1, g);
            while(!r_1.container.empty()){
                sparse_poly q = r_0 / r_1, r_m = r_1, s_m = s_1, t_m = t_1, rho_m = rho_1;
                rho_1 = r_0 - q * r_1;
                rho_1 = rho_1.lu();
                r_1 = (r_0 - q * r_1) / rho_1;
                s_1 = (s_0 - q * s_1) / rho_1;
                t_1 = (t_0 - q * t_1) / rho_1;
                rho_0 = std::move(rho_m);
                r_0 = std::move(r_m);
                s_0 = std::move(s_m);
                t_0 = std::move(t_m);
            }
            result = std::move(r_0);
            c_lhs = std::move(s_0);
            c_rhs = std::move(t_0);
            return result;
        }

        static sparse_poly &modular_gcd_impl(
            sparse_poly &result,
            const sparse_poly &f,
            const sparse_poly &g,
            const coefficient_type &p
        ){
            result.container.clear();
            if(g.container.empty()){ return result; }
            sparse_poly
                rho_0 = f.lc(), rho_1 = g.lc(),
                r_0, r_1;
            modular_normal(r_0, f, p), modular_normal(r_1, g, p);
            rho_0 %= p, rho_1 %= p;
            r_0 = r_0.mod_coefficient(p), r_1 = r_1.mod_coefficient(p);
            while(!r_1.container.empty()){
                sparse_poly q, inv_rho_1 = rho_1.modular_inverse(p), rho_m = rho_1;
                sparse_poly r_m = r_1;
                {
                    sparse_poly dummy_rem;
                    square_div<false>(
                        q,
                        dummy_rem,
                        r_0,
                        r_1,
                        [&](sparse_poly &r){ r = r.mod_coefficient(p); },
                        [&](coefficient_type &r){ r %= p; },
                        [&](const coefficient_type &x, const coefficient_type &y) -> coefficient_type{
                            return x * coefficient_inverse(y, p) % p;
                        }
                    );
                }
                sparse_poly q_r_1 = q * r_1;
                q = q.mod_coefficient(p);
                q_r_1 = q_r_1.mod_coefficient(p);
                rho_1 = (r_0 - q_r_1).lu();
                rho_1 = rho_1.mod_coefficient(p);
                r_1 = (r_0 - q_r_1) * inv_rho_1;
                r_1 = r_1.mod_coefficient(p);
                rho_0 = std::move(rho_m);
                r_0 = std::move(r_m);
            }
            result = std::move(r_0);
            return result;
        }

        static sparse_poly &modular_eea_impl(
            sparse_poly &result,
            sparse_poly &c_lhs,
            sparse_poly &c_rhs,
            const sparse_poly &f,
            const sparse_poly &g,
            const coefficient_type &p
        ){
            result.container.clear();
            if(g.container.empty()){
                c_lhs.container.clear();
                c_rhs.container.clear();
                return result;
            }
            sparse_poly
                rho_0 = f.lc(), rho_1 = g.lc(),
                r_0, r_1,
                s_0 = rho_0.modular_inverse(p), s_1 = coefficient_type(0),
                t_0 = coefficient_type(0), t_1 = rho_1.modular_inverse(p);
            modular_normal(r_0, f, p), modular_normal(r_1, g, p);
            rho_0 %= p, rho_1 %= p;
            r_0 = r_0.mod_coefficient(p), r_1 = r_1.mod_coefficient(p);
            s_0 = s_0.mod_coefficient(p), t_1 = t_1.mod_coefficient(p);
            while(!r_1.container.empty()){
                sparse_poly q, inv_rho_1 = rho_1.modular_inverse(p), rho_m = rho_1;
                sparse_poly r_m = r_1, s_m = s_1, t_m = t_1;
                {
                    sparse_poly dummy_result, dummy_rem;
                    square_div<false>(
                        dummy_result,
                        dummy_rem,
                        r_0,
                        r_1,
                        [&](sparse_poly &r){ r = r.mod_coefficient(p); },
                        [&](coefficient_type &r){ r %= p; },
                        [&](const coefficient_type &x, const coefficient_type &y) -> coefficient_type{
                            return x * coefficient_inverse(y, p) % p;
                        }
                    );
                    q = std::move(dummy_result);
                }
                sparse_poly q_r_1 = q * r_1;
                q = q.mod_coefficient(p);
                q_r_1 = q_r_1.mod_coefficient(p);
                rho_1 = ((r_0 - q_r_1).lu()).mod_coefficient(p);
                r_1 = ((r_0 - q_r_1) * inv_rho_1).mod_coefficient(p);
                s_1 = ((s_0 - q * s_1) * inv_rho_1).mod_coefficient(p);
                t_1 = ((t_0 - q * t_1) * inv_rho_1).mod_coefficient(p);
                rho_0 = std::move(rho_m);
                r_0 = std::move(r_m);
                s_0 = std::move(s_m);
                t_0 = std::move(t_m);
            }
            result = std::move(r_0);
            c_lhs = std::move(s_0);
            c_rhs = std::move(t_0);
            return result;
        }

        template<class ConcurrentOrthogonalWIterator>
        class orthogonal_w_iterrator : public boost::iterator_adaptor<
            orthogonal_w_iterrator<ConcurrentOrthogonalWIterator>,
            orthogonal_w_iterrator<ConcurrentOrthogonalWIterator>*,
            const coefficient_type,
            boost::forward_traversal_tag
        >{
        public:
            std::size_t i;
            const ConcurrentOrthogonalWIterator *concurrent;

            orthogonal_w_iterrator(const orthogonal_w_iterrator &other) :
                i(other.i), concurrent(other.concurrent)
            {}

            orthogonal_w_iterrator(const ConcurrentOrthogonalWIterator *concurrent_) :
                concurrent(concurrent_), i(0)
            {}

            void increment(){ ++i; }

            const coefficient_type &dereference() const{
                return concurrent->iterator_container[i]->second;
            }

            bool equal(const orthogonal_w_iterrator &x) const{
                return x.i == i;
            }

            static orthogonal_w_iterrator end(std::size_t i){
                orthogonal_w_iterrator r(nullptr);
                r.i = i;
                return r;
            }
        };

        class concurrent_orthogonal_w_iterator : public boost::iterator_adaptor<
            concurrent_orthogonal_w_iterator,
            concurrent_orthogonal_w_iterator*,
            orthogonal_w_iterrator<concurrent_orthogonal_w_iterator>,
            boost::forward_traversal_tag
        >{
        public:
            typedef std::vector<typename container_type::const_iterator> iterator_container_type;
            typedef orthogonal_w_iterrator<concurrent_orthogonal_w_iterator> orthogonal_w_iterrator_type;
            iterator_container_type &iterator_container;
            mutable orthogonal_w_iterrator_type orthogonal_w_iterrator_cache;
            concurrent_orthogonal_w_iterator(iterator_container_type &iterator_container_) :
                iterator_container(iterator_container_), orthogonal_w_iterrator_cache(nullptr)
            {}

            void increment(){
                for(
                    typename iterator_container_type::iterator iter = iterator_container.begin(), end = iterator_container.end();
                    iter != end;
                    ++iter
                ){ ++(*iter); }
            }

            orthogonal_w_iterrator_type &dereference() const{
                orthogonal_w_iterrator_cache.i = 0;
                orthogonal_w_iterrator_cache.concurrent = this;
                return orthogonal_w_iterrator_cache;
            };
        };

        static sparse_poly &primitive_gcd_impl(sparse_poly &result, const sparse_poly &f, const sparse_poly &g){
            result.container.clear();
            if(g.container.empty()){ return result; }
            const order_type &n(f.ref_deg());
            coefficient_type n_ = coefficient_type(n), large_a = f.infinity_norm();
            {
                coefficient_type large_a_prime = g.infinity_norm();
                if(large_a_prime > large_a){ large_a = std::move(large_a_prime); }
            }
            coefficient_type
                b = aux::gcd(f.lc(), g.lc()),
                k = 2 * ceil_log2_dispatch(pow_dispatch(n_ + 1, n_) * b * pow_dispatch(large_a, 2 * n_)) + 1,
                large_b = sqrt_dispatch(n_ + 1) * pow_dispatch(coefficient_type(2), n_) * large_a * b,
                l = ceil_log2_dispatch(2 * large_b + 1);
            double double_k = to_double_dispatch(k);
            typedef aux::prime_list<unsigned int> prime_list_type;
            prime_list_type::ext_prime_vec_type set =
                prime_list_type::get_prime_set(
                    static_cast<prime_list_type::value_type>(double_k * 2 * std::log(double_k)),
                    2 * to_unsigned_int_dispatch(l)
                );
            set.erase(
                std::remove_if(
                    set.begin(),
                    set.end(),
                    [&](prime_list_type::value_type p) -> bool{ return p == 0 || b % p == 0; }
                ),
                set.end()
            );
            std::vector<sparse_poly> v_set;
            {
                std::vector<std::pair<sparse_poly, prime_list_type::value_type>> vp_set;
                vp_set.reserve(set.size());
                for(std::size_t i = 0, length = set.size(); i < length; ++i){
                    // !!
                    sparse_poly result_modular_gcd = modular_gcd(f, g, set[i]);
                    std::cout
                        << "PolynomialGCD[" << f << "," << g << ",Modulus->" << set[i] << "]=\n"
                        << result_modular_gcd << "\n\n";
                    vp_set.push_back(std::make_pair(result_modular_gcd, set[i]));

                    vp_set.back().first = vp_set.back().first.mod_coefficient(set[i]);
                }
                {
                    order_type e = vp_set[0].first.deg();
                    for(std::size_t i = 1, length = vp_set.size(); i < length; ++i){
                        if(vp_set[i].first.ref_deg() < e){ e = vp_set[i].first.ref_deg(); }
                    }
                    set.clear();
                    vp_set.erase(
                        std::remove_if(
                            vp_set.begin(),
                            vp_set.end(),
                            [&](const std::pair<sparse_poly, prime_list_type::value_type> &vp) -> bool{
                                return vp.first.ref_deg() != e;
                            }
                        ),
                        vp_set.end()
                    );
                }
                v_set.resize(vp_set.size()), set.resize(vp_set.size());
                for(std::size_t i = 0, length = vp_set.size(); i < length; ++i){
                    v_set[i] = vp_set[i].first, set[i] = vp_set[i].second;
                }
                std::size_t l_ = to_unsigned_int_dispatch(l);
                std::cout
                    << "set.size()   = " << set.size() << "\n"
                    << "v_set.size() = " << v_set.size() << "\n"
                    << "l = " << l_ << "\n";
                if(set.size() >= l_){
                    set.erase(set.begin(), set.begin() + (set.size() - l_));
                    v_set.erase(v_set.begin(), v_set.begin() + (v_set.size() - l_));
                }
            }

            std::cout
                << "set.size()   = " << set.size() << "\n"
                << "v_set.size() = " << v_set.size() << "\n";

            std::cout << "ChineseRemainder[{";
            for(std::size_t i = 0, length = v_set.size(); i < length; ++i){
                v_set[i] *=b;
                std::cout << v_set[i];
                if(i + 1 != length){
                    std::cout << ", ";
                }else{
                    std::cout << "}, {";
                }
            }
            for(std::size_t i = 0, length = set.size(); i < length; ++i){
                std::cout << set[i];
                if(i + 1 != length){
                    std::cout << ", ";
                }else{
                    std::cout << "}]\n";
                }
            }
            result = cra(v_set.begin(), v_set.end(), set.begin());
            std::cout << result << "\n";
            result = result.pp();

            //{
            //    std::size_t i_length = 0;
            //    for(coefficient_type p = 1; p < large_a; ){ p *= set[i_length++]; }
            //    std::vector<sparse_poly> w;
            //    {
            //        storaged_container<1>::vector<coefficient_type> m;
            //        storaged_container<1>::vector<sparse_poly> v;
            //        std::vector<typename container_type::iterator> w_iter;
            //        w.reserve(i_length);
            //        for(std::size_t i = 0; i < i_length; ++i){
            //            m.clear(), v.clear();
            //            m.push_back(set[i]), v.push_back(v_set[i] * b);
            //            w.push_back(cra(v.begin(), v.end(), m.begin()));
            //        }
            //    }
            //    std::vector<typename container_type::const_iterator> orthogonal_w_iterator_vec;
            //    orthogonal_w_iterator_vec.reserve(set.size());
            //    for(std::size_t i = 0 ,length = w.size(); i < length; ++i){
            //        orthogonal_w_iterator_vec.push_back(w[i].container.begin());
            //    }
            //    concurrent_orthogonal_w_iterator d(orthogonal_w_iterator_vec);
            //    typename container_type::const_iterator
            //        w_element_iter = w.front().container.begin(),
            //        w_element_end = w.front().container.end();
            //    for(; w_element_iter != w_element_end; ++w_element_iter, ++d){
            //        sparse_poly r =
            //            cra(
            //                *d,
            //                orthogonal_w_iterrator<concurrent_orthogonal_w_iterator>::end(i_length),
            //                set.begin()
            //            );
            //        r.radix_shift(w_element_iter->first);
            //        result += r;
            //    }
            //}
            //result = result.pp();
            
            return result;
        }

        void sub_n_q(const sparse_poly &rhs, const order_type &n, const coefficient_type &q){
            for(typename container_type::const_iterator rhs_iter = rhs.container.begin(), rhs_end = rhs.container.end(); rhs_iter != rhs_end; ++rhs_iter){
                const order_type &order(rhs_iter->first);
                order_type new_order(order);
                new_order += n;
                const coefficient_type &coe(rhs_iter->second);
                coefficient_type new_coe = coe * q;
                typename container_type::iterator iter = container.find(new_order);
                if(iter == container.end()){
                    iter = container.add(typename container_type::ref_value_type(new_order, new_coe));
                    if(iter != container.end()){ assign_reverse_dispatch(iter->second, new_coe); }
                }else{
                    coefficient_type &lhs_coe(iter->second);
                    lhs_coe -= new_coe;
                    if(lhs_coe == 0){ container.erase(iter); }
                }
            }
        }

        void addition_order_coe(const order_type &order, const coefficient_type &coe){
            typename container_type::iterator iter = container.find(order);
            if(iter == container.end()){
                container.add(typename container_type::ref_value_type(order, coe));
            }else{
                coefficient_type &lhs_coe(iter->second);
                lhs_coe += coe;
                if(lhs_coe == 0){ container.erase(iter); }
            }
        }

        void subtraction_order_coe(const order_type &order, const coefficient_type &coe){
            typename container_type::iterator iter = container.find(order);
            if(iter == container.end()){
                container.sub(typename container_type::ref_value_type(order, coe));
            }else{
                coefficient_type &lhs_coe(iter->second);
                lhs_coe -= coe;
                if(lhs_coe == 0){ container.erase(iter); }
            }
        }

        template<class Str, class Char, class Variable, class OStringStream>
        Str to_string_impl(
            const Variable &v,
            Char multi,
            Char pow,
            Char plus,
            Char mn,
            Char zero,
            Char l_pare,
            Char r_pare
        ) const{
            Str result;
            if(container.empty()){
                result += zero;
                return std::move(result);
            }
            typename container_type::const_reverse_iterator first = container.rbegin();
            for(typename container_type::const_reverse_iterator iter = container.rbegin(), end = container.rend(); iter != end; ++iter){
                const order_type &order(iter->first);
                const coefficient_type &coe(iter->second);
                if(coe == 0){ continue; }
                if(first != iter && coe > 0){
                    result += plus;
                }
                if(coe == 1){
                    if(order == 0){
                        OStringStream ostringstream;
                        ostringstream << coe;
                        result += ostringstream.str();
                    }
                }else if(coe == -1){
                    if(order == 0){
                        OStringStream ostringstream;
                        ostringstream << coe;
                        result += ostringstream.str();
                    }else{
                        result += mn;
                    }
                }else if(coe > 0 || coe < 0){
                    OStringStream ostringstream;
                    ostringstream << coe;
                    result += ostringstream.str();
                }
                if(order != 0){
                    result += v;
                    if(order != 1){
                        result += pow;
                        if(order < 0){ result += l_pare; }
                        OStringStream ostringstream;
                        ostringstream << order;
                        result += ostringstream.str();
                        if(order < 0){ result += r_pare; }
                    }
                }
            }
            return std::move(result);
        }

    private:
        container_type container;
    };

    template<class OrderType, class CoefficientType, class Alloc>
    sparse_poly<OrderType, CoefficientType, Alloc>
    operator +(
        const typename sparse_poly<OrderType, CoefficientType, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, Alloc>(lhs) + rhs; }

    template<class OrderType, class CoefficientType, class Alloc>
    sparse_poly<OrderType, CoefficientType, Alloc>
    operator -(
        const typename sparse_poly<OrderType, CoefficientType, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, Alloc>(lhs) - rhs; }

    template<class OrderType, class CoefficientType, class Alloc>
    sparse_poly<OrderType, CoefficientType, Alloc>
    operator *(
        const typename sparse_poly<OrderType, CoefficientType, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, Alloc>(lhs) * rhs; }

    template<class OrderType, class CoefficientType, class Alloc>
    sparse_poly<OrderType, CoefficientType, Alloc>
    operator /(
        const typename sparse_poly<OrderType, CoefficientType, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, Alloc>(lhs) / rhs; }

    template<class OrderType, class CoefficientType, class Alloc>
    sparse_poly<OrderType, CoefficientType, Alloc>
    operator %(
        const typename sparse_poly<OrderType, CoefficientType, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, Alloc>(lhs) % rhs; }

    template<class OrderType, class CoefficientType, class Alloc>
    bool operator <(
        const typename sparse_poly<OrderType, CoefficientType, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, Alloc>(lhs) < rhs; }

    template<class OrderType, class CoefficientType, class Alloc>
    bool operator >(
        const typename sparse_poly<OrderType, CoefficientType, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, Alloc>(lhs) > rhs; }

    template<class OrderType, class CoefficientType, class Alloc>
    bool operator <=(
        const typename sparse_poly<OrderType, CoefficientType, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, Alloc>(lhs) <= rhs; }

    template<class OrderType, class CoefficientType, class Alloc>
    bool operator >=(
        const typename sparse_poly<OrderType, CoefficientType, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, Alloc>(lhs) >= rhs; }

    template<class OrderType, class CoefficientType, class Alloc>
    bool operator ==(
        const typename sparse_poly<OrderType, CoefficientType, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, Alloc>(lhs) == rhs; }

    template<class OrderType, class CoefficientType, class Alloc>
    bool operator !=(
        const typename sparse_poly<OrderType, CoefficientType, Alloc>::coefficient_type &lhs,
        const sparse_poly<OrderType, CoefficientType, Alloc> &rhs
    ){ return sparse_poly<OrderType, CoefficientType, Alloc>(lhs) != rhs; }

    template<class OrderType, class CoefficientType, class Alloc>
    std::ostream &operator <<(
        std::ostream &ostream,
        const sparse_poly<OrderType, CoefficientType, Alloc> &value
    ){
        ostream << value.to_string();
        return ostream;
    }

    template<class OrderType, class CoefficientType, class Alloc>
    std::wostream &operator <<(
        std::wostream &ostream,
        const sparse_poly<OrderType, CoefficientType, Alloc> &value
    ){
        ostream << value.to_wstring();
        return ostream;
    }
}

#endif
