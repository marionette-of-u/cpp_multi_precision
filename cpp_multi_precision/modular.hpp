#ifndef HPP_CPP_MULTI_PRECISION_MODULAR
#define HPP_CPP_MULTI_PRECISION_MODULAR

#include <sstream>
#include <functional>
#include "ns_aux.hpp"

namespace cpp_multi_precision{
    template<class ValueType>
    class modular{
    public:
        typedef ValueType value_type;

    public:
        modular() :
           value_(), modulus_(), modulus_1_5_()
        {}

       modular(const value_type &other_value) :
           value_(other_value), modulus_(), modulus_1_5_()
       {}

        modular(const value_type &other_value, const value_type &other_modulus) :
            value_(other_value), modulus_(other_modulus)
        {
            set_modulus_1_5();
            normalize();
        }

        modular(const modular &other) :
            value_(other.value_), modulus_(other.modulus_), modulus_1_5_(other.modulus_1_5_)
        { normalize(); }

        modular(modular &&other) :
            value_(std::move(other.value_)), modulus_(std::move(other.modulus_)), modulus_1_5_(std::move(other.modulus_1_5_))
        { normalize(); }

    public:
        std::string to_string() const{
            std::string r;
            r += to_string_dispatch(value_);
            r += " mod ";
            r += to_string_dispatch(modulus_);
            return r;
        }

        std::wstring to_wstring() const{
            std::wstring r;
            r += to_wstring_dispatch(value_);
            r += L" mod ";
            r += to_wstring_dispatch(modulus_);
            return r;
        }

        void normalize(){
            if(value_ >= modulus_1_5_){
                force_normalize();
            }
        }

        void force_normalize(){
            value_ = value_ % modulus_;
        }

        static modular &pow(modular &result, const modular &x, const modular &y){
            modular x_(x), y_(y);
            x_.force_normalize();
            y_.force_normalize();
            pow_dispatch(result, x_, y_);
            result.set_modulus(x_.modulus_);
            result.normalize();
            return result;
        }

    public:
        modular &operator =(const modular &other){
            value_ = other.value_;
            modulus_ = other.modulus_;
            modulus_1_5_ = other.modulus_1_5_;
            return *this;
        }

        modular &operator =(modular &&other){
            value_ = std::move(other.value_);
            modulus_ = std::move(other.modulus_);
            modulus_1_5_ = std::move(other.modulus_1_5_);
            return *this;
        }

        modular &operator +=(const modular &rhs){
            value_ += rhs.value_;
            normalize();
            return *this;
        }

        modular operator +(const modular &rhs) const{
            modular r(*this);
            r += rhs;
            return std::move(r);
        }

        modular &operator +=(const value_type &rhs){
            value_ += rhs;
            normalize();
            return *this;
        }

        modular operator +(const value_type &rhs){
            modular r(*this);
            r += rhs;
            return std::move(r);
        }

        modular &operator -=(const modular &rhs){
            value_ -= rhs.value_;
            normalize();
            return *this;
        }

        modular operator -(const modular &rhs) const{
            modular r(*this);
            r -= rhs;
            return std::move(r);
        }

        modular &operator -=(const value_type &rhs){
            value_ -= rhs;
            normalize();
            return *this;
        }

        modular operator -(const value_type &rhs){
            modular r(*this);
            r -= rhs;
            return std::move(r);
        }

        modular &operator *=(const modular &rhs){
            value_ *= rhs.value_;
            normalize();
            return *this;
        }

        modular operator *(const modular &rhs) const{
            modular r(*this);
            r *= rhs;
            return std::move(r);
        }

        modular &operator *=(const value_type &rhs){
            value_ *= rhs;
            normalize();
            return *this;
        }

        modular operator *(const value_type &rhs){
            modular r(*this);
            r *= rhs;
            return std::move(r);
        }

        modular &operator /=(const modular &rhs){
            value_ /= rhs.value_;
            normalize();
            return *this;
        }

        modular operator /(const modular &rhs) const{
            modular r(*this);
            r /= rhs;
            return std::move(r);
        }

        modular &operator /=(const value_type &rhs){
            value_ /= rhs;
            normalize();
            return *this;
        }

        modular operator /(const value_type &rhs){
            modular r(*this);
            r /= rhs;
            return std::move(r);
        }

        modular &operator %=(const modular &rhs){
            value_ %= rhs.value_;
            normalize();
            return *this;
        }

        modular operator %(const modular &rhs) const{
            modular r(*this);
            r %= rhs;
            return std::move(r);
        }

        modular &operator %=(const value_type &rhs){
            value_ %= rhs;
            normalize();
            return *this;
        }

        modular operator %(const value_type &rhs){
            modular r(*this);
            r %= rhs;
            return std::move(r);
        }

        modular operator +() const{
            modular r(*this);
            r.inverse_sign();
            return std::move(r);
        }

        modular operator -() const{
            modular r(*this);
            r.force_normalize();
            if(r.value_ > 0){
                r.value_ = r.modulus_ - r.value_;
            }else if(r.value_ < 0){
                r.value_ = -(r.modulus_ + r.value_);
            }
            return std::move(r);
        }

        bool operator ==(const modular &rhs) const{
            if(modulus_ != rhs.modulus_){
                return false;
            }
            modular rhs_(rhs), lhs_(*this);
            lhs_.force_normalize();
            rhs_.force_normalize();
            if(lhs_.value_ == rhs_.value_){
                return true;
            }else{
                rhs_.inverse_sign();
                if(lhs_.value_ == rhs_.value_){
                    return true;
                }
            }
            return false;
        }

        bool operator ==(const value_type &rhs) const{
            if(value_ == rhs){
                return true;
            }else if(value_ == -rhs){
                return true;
            }
            return false;
        }

        bool operator !=(const modular &rhs) const{
            return !operator ==(rhs);
        }

        bool operator !=(const value_type &rhs) const{
            return !operator ==(rhs);
        }

    public:
        const value_type &value() const{
            return value_;
        }

        const value_type &modulus() const{
            return modulus_;
        }

    private:
        void inverse_sign(){
            value_ = -value_;
        }

        void set_modulus(const value_type &x){
            modulus_ = x;
            set_modulus_1_5();
        }

        void set_modulus_1_5(){
            modulus_1_5_ = modulus_;
            modulus_1_5_ += modulus_ / 2;
        }

#define CPP_MULTI_PRECISION_SIGUNATURE_MODULAR_TO_STRING template<class T, std::string (T::*Func)() const>
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(to_string, CPP_MULTI_PRECISION_SIGUNATURE_MODULAR_TO_STRING);
        template<class T>
        static std::string to_string_dispatch(const T &value, typename boost::enable_if<has_to_string<T>>::type* = 0){
            return value.to_string();
        }

        template<class T>
        static std::string to_string_dispatch(const T &value, typename boost::disable_if<has_to_string<T>>::type* = 0){
            std::ostringstream os;
            os << value;
            return os.str();
        }

#define CPP_MULTI_PRECISION_SIGUNATURE_MODULAR_TO_WSTRING template<class T, std::wstring (T::*Func)() const>
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(to_wstring, CPP_MULTI_PRECISION_SIGUNATURE_MODULAR_TO_WSTRING);
        template<class T>
        static std::wstring to_wstring_dispatch(const T &value, typename boost::enable_if<has_to_string<T>>::type* = 0){
            return value.to_wstring();
        }

        template<class T>
        static std::wstring to_wstring_dispatch(const T &value, typename boost::disable_if<has_to_string<T>>::type* = 0){
            std::wostringstream os;
            os << value;
            return os.str();
        }

#define CPP_MULTI_PRECISION_SIGNATURE_MODULAR_POW template<class T, T &(*Func)(T&, const T&, const T&, const T&)>
        CPP_MULTI_PRECISION_AUX_HAS_MEM_FN(pow_mod, CPP_MULTI_PRECISION_SIGNATURE_MODULAR_POW);
        template<class T>
        static T &pow_dispatch(
            T &result,
            const T &x,
            const T &y,
            typename boost::enable_if<has_pow_mod<typename T::value_type>>::type* = 0
        ){
            T::value_type::pow_mod(result.value_, x.value_, y.value_, x.modulus_);
            return result;
        }

        template<class T>
        static T &pow_dispatch(
            T &result,
            const T &x,
            const T &y,
            typename boost::disable_if<has_pow_mod<typename T::value_type>>::type* = 0
        ){
            std::size_t k = sizeof(T) * 8;
            result.value_ = x.value_;
            for(std::size_t i = 0; i < k; ++i){
                std::size_t j = k - i - 1;
                if(((y.value_ >> j) & 1) == 1){
                    result.value_ = (result.value_ * result.value_ * x.value_) % x.modulus_;
                }else{
                    result.value_ = (result.value_ * result.value_) % x.modulus_;
                }
            }
            return result;
        }

    private:
        value_type value_, modulus_, modulus_1_5_;
    };

    template<class ValueType>
    modular<ValueType> operator +(const typename modular<ValueType>::value_type &lhs, const modular<ValueType> &rhs){
        modular<ValueType> r(lhs, rhs.modulus());
        r += rhs;
        return std::move(r);
    }

    template<class ValueType>
    modular<ValueType> operator -(const typename modular<ValueType>::value_type &lhs, const modular<ValueType> &rhs){
        modular<ValueType> r(lhs, rhs.modulus());
        r -= rhs;
        return std::move(r);
    }

    template<class ValueType>
    modular<ValueType> operator *(const typename modular<ValueType>::value_type &lhs, const modular<ValueType> &rhs){
        modular<ValueType> r(lhs, rhs.modulus());
        r *= rhs;
        return std::move(r);
    }

    template<class ValueType>
    modular<ValueType> operator /(const typename modular<ValueType>::value_type &lhs, const modular<ValueType> &rhs){
        modular<ValueType> r(lhs, rhs.modulus());
        r /= rhs;
        return std::move(r);
    }

    template<class ValueType>
    modular<ValueType> operator %(const typename modular<ValueType>::value_type &lhs, const modular<ValueType> &rhs){
        modular<ValueType> r(lhs, rhs.modulus());
        r %= rhs;
        return std::move(r);
    }

    template<class ValueType>
    bool operator ==(const typename modular<ValueType>::value_type &lhs, const modular<ValueType> &rhs){
        return lhs == rhs.value();
    }

    template<class ValueType>
    bool operator !=(const typename modular<ValueType>::value_type &lhs, const modular<ValueType> &rhs){
        return lhs != rhs.value();
    }

    template<class ValueType>
    std::ostream &operator <<(std::ostream &ostream, modular<ValueType> value){
        value.force_normalize();
        ostream << value.to_string();
        return ostream;
    }
}

#endif
