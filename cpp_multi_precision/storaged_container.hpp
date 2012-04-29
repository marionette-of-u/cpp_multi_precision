#ifndef CPP_MULTI_PRECISION_STORAGED_CONTAINER
#define CPP_MULTI_PRECISION_STORAGED_CONTAINER

#include <vector>
#include <iterator>
#include <type_traits>
#include <cassert>

namespace cpp_multi_precision{
    template<std::size_t ArraySize>
    struct storaged_container{
        static const std::size_t array_size = ArraySize;

        template<class Element, class Allocator = std::allocator<Element>>
        class vector{
        public:
            typedef typename Allocator::reference reference;
            typedef typename Allocator::const_reference const_reference;
            typedef Element *iterator;
            typedef const Element *const_iterator;
            typedef std::size_t size_type;
            typedef std::ptrdiff_t difference_type;
            typedef Element value_type;
            typedef Allocator allocator_type;
            typedef typename Allocator::pointer pointer;
            typedef typename Allocator::const_pointer const_pointer;
            typedef std::reverse_iterator<iterator> reverse_iterator;
            typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
            typedef typename std::aligned_storage<
                sizeof(value_type),
                std::alignment_of<value_type>::value
            >::type aligned_storage;

            vector() : pos(0){}
            vector(const vector &other) :
                pos(other.pos)
            {
                assign(other);
            }

            vector(vector &&other) :
                pos(other.pos)
            {
                assign(other);
            }

            ~vector(){
                for(size_type i = 0; i < pos; ++i){
                    reinterpret_cast<value_type*>(&element_array[i])->~value_type();
                }
            }

            template<class Iter>
            vector(const Iter &first, const Iter &last) : pos(std::distance(first, last)){
                assert(pos <= array_size);
                std::size_t i = 0;
                for(Iter iter = first; iter != last; ++iter, ++i){
                    new(reinterpret_cast<value_type*>(&element_array[i])) value_type(*iter);
                }
            }

            vector &operator =(const vector &other){
                pos = other.pos;
                assign(other);
                return *this;
            }

            vector &operator =(vector &&other){
                pos = other.pos;
                assign(other);
                return *this;
            }

            iterator begin(){
                assert(pos > 0);
                return &reinterpret_cast<value_type*>(element_array)[0];
            }

            const_iterator begin() const{
                assert(pos > 0);
                return &reinterpret_cast<const value_type*>(element_array)[0];
            }

            iterator end(){
                return &reinterpret_cast<value_type*>(element_array)[pos];
            }

            const_iterator end() const{
                return &reinterpret_cast<const value_type*>(element_array)[pos];
            }

            reverse_iterator rbegin(){
                assert(pos > 0);
                return reverse_iterator(&reinterpret_cast<value_type*>(element_array)[pos]);
            }

            const_reverse_iterator rbegin() const{
                assert(pos > 0);
                return const_reverse_iterator(&reinterpret_cast<const value_type*>(element_array)[pos]);
            }

            reverse_iterator rend(){
                return reverse_iterator(&reinterpret_cast<value_type*>(element_array)[0]);
            }

            const_reverse_iterator rend() const{
                return const_reverse_iterator(&reinterpret_cast<const value_type*>(element_array)[0]);
            }

            size_type size() const{
                return pos;
            }

            size_type max_size() const{
                return array_size;
            }

            size_type capacity() const{
                return array_size;
            }

            bool empty() const{
                return pos == 0;
            }

            void reserve(size_type n){
                assert(n <= array_size);
            }

            reference operator [](size_type n){
                return reinterpret_cast<value_type*>(element_array)[n];
            }

            const_reference operator [](size_type n) const{
                return reinterpret_cast<const value_type*>(element_array)[n];
            }

            reference at(size_type n){
                assert(n <= pos);
                return (*this)[n];
            }

            const_reference at(size_type n) const{
                assert(n <= pos);
                return (*this)[n];
            }

            reference front(){
                assert(pos > 0);
                return reinterpret_cast<value_type*>(element_array)[0];
            }

            const_reference front() const{
                assert(pos > 0);
                return reinterpret_cast<const value_type*>(element_array)[0];
            }

            reference back(){
                assert(pos > 0);
                return reinterpret_cast<value_type*>(element_array)[pos - 1];
            }

            const_reference back() const{
                assert(pos > 0);
                return reinterpret_cast<const value_type*>(element_array)[pos - 1];
            }

            template<class Iter>
            void assign(Iter first, Iter last){
                size_type dist = std::distance(first, last);
                assert(dist <= array_size);
                clear();
                pos = dist;
                Iter iter = first;
                for(std::size_t i = 0; i < dist; ++i, ++iter){
                    reinterpret_cast<value_type*>(element_array)[i] = *iter;
                }
            }

            void assign(size_type n, const Element &element){
                assert(n <= array_size);
                clear();
                for(size_type i = 0; i < pos; ++i){
                    reinterpret_cast<value_type*>(&element_array[i])->~value_type();
                }
                for(size_type i = 0; i < n; ++i){
                    new(&element_array[i]) value_type(element);
                }
                pos = n;
            }

            void push_back(const_reference element){
                assert(pos < array_size);
                new(&element_array[pos]) value_type(element);
                ++pos;
            }

            void pop_back(){
                assert(pos > 0);
                reinterpret_cast<value_type*>(element_array)[pos].~value_type();
                --pos;
            }

            iterator insert(const iterator &iter, const_reference value){
                assert(pos < array_size);
                for(iterator i = end() - 1, j = iter - 1; i != j; --i){
                    *reinterpret_cast<aligned_storage*>(&*(i + 1)) = *reinterpret_cast<aligned_storage*>(&*i);
                }
                new(&*iter) value_type(value);
                ++pos;
                return iter;
            }

            void insert(const iterator &iter, size_type num, const_reference value){
                assert((pos + num) <= array_size);
                for(size_type i = 0, n = end() - iter; i < n; ++i){
                    iterator p = iter + (n - i - 1);
                    *reinterpret_cast<aligned_storage*>(&*(p + num)) = *reinterpret_cast<aligned_storage*>(&*p);
                }
                for(std::size_t i = 0; i < num; ++i){
                    new(&*(iter + i)) value_type(value);
                }
                pos += num;
            }

            template<class Iter>
            void insert(const iterator &iter, const Iter &first, const Iter &last){
                size_type num = std::distance(first, last);
                assert((pos + num) <= array_size);
                for(iterator i = iter, j = iter + (pos - num); i != j; ++i){
                    *reinterpret_cast<aligned_storage*>(&*(i + num)) = *reinterpret_cast<aligned_storage*>(&*i);
                }
                size_type counter = 0;
                for(Iter i = first; i != last; ++i, ++counter){
                    new(&*(iter + counter)) value_type(*i);
                }
                pos += num;
            }

            iterator erase(const iterator &position){
                assert(pos > 0);
                iterator ret = position - 1;
                position->~value_type();
                for(iterator i = position, j = end(); i != j; ++i){
                    *reinterpret_cast<aligned_storage*>(&*(i)) = *reinterpret_cast<aligned_storage*>(&*(i + 1));
                }
                --pos;
                return ret;
            }

            iterator erase(const iterator &first, const iterator &last){
                size_type num = std::distance(first, last), rest_size = end() - last;
                assert(pos >= num);
                iterator ret = first - 1;
                for(iterator i = first; i != last; ++i){
                    i->~value_type();
                }
                iterator iter = first;
                for(size_type i = 0; i < rest_size; ++i, ++iter){
                    *reinterpret_cast<aligned_storage*>(&*(iter)) = *reinterpret_cast<aligned_storage*>(&*(iter + num));
                }
                pos -= num;
                return ret;
            }

            void resize(size_type n){
                assert(n <= array_size);
                if(n > pos){
                    for(std::size_t i = 0, m = n - pos; i < m; ++i){
                        new(reinterpret_cast<value_type*>(&element_array[pos + i])) value_type();
                    }
                }else{
                    for(std::size_t i = 0, m = pos - n; i < m; ++i){
                        reinterpret_cast<value_type*>(&element_array[n + i])->~value_type();
                    }
                }
                pos = n;
            }

            void swap(vector &other){
                aligned_storage temp_array[array_size];
                size_type temp_pos = other.pos;
                for(size_type i = 0; i < temp_pos; ++i){
                    temp_array[i] = other.element_array[i];
                }
                other.pos = pos;
                for(size_type i = 0; i < pos; ++i){
                    other.element_array[i] = element_array[i];
                }
                pos = temp_pos;
                for(size_type i = 0; i < temp_pos; ++i){
                    element_array[i] = temp_array[i];
                }
            }

            void clear(){
                for(size_type i = 0; i < pos; ++i){
                    reinterpret_cast<value_type*>(element_array)[i].~value_type();
                }
                pos = 0;
            }

        private:
            void assign(const vector &other){
                for(std::size_t i = 0; i < pos; ++i){
                    element_array[i] = other.element_array[i];
                }
            }

            std::size_t pos;
            aligned_storage element_array[array_size];
        };
    };
}

#endif
