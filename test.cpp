#include <iostream>
#include <complex>
#include "cpp_multi_precision/modular.hpp"
#include "cpp_multi_precision/sparse_poly.hpp"
#include "cpp_multi_precision/rational.hpp"
#include "cpp_multi_precision/primitive_mp_float.hpp"
#include "cpp_multi_precision/integer.hpp"

int test_sparse_poly(){
    // 次数となる整数型
    typedef cpp_multi_precision::integer<
        unsigned int,
        32,
        long long,
        unsigned long long,
        std::vector<unsigned int>
    > integer;

    // 係数となる有理数型
    typedef cpp_multi_precision::rational<
        unsigned int,
        32,
        long long,
        unsigned long long,
        std::vector<unsigned int>
    > rational;

    // 上記の二つをあわせて疎な多項式型を作る
    typedef cpp_multi_precision::sparse_poly<integer, rational> sparse_poly;

    std::cout << "start test_sparse_poly\n";

    sparse_poly poly_a, poly_b, poly_c, poly_r, poly_cl, poly_cr;
    // a += 18 * x^3
    poly_a[3] = 18;
    // a -= 42 * x^2
    poly_a[2] -= 42;
    // a += 30 * x
    poly_a[1] = 30;
    // a -= 6
    poly_a[0] -= 6;

    // b -= 12 * x^2
    poly_b[2] -= 12;
    // b += 10 * x
    poly_b[1] = 10;
    // b -= 2
    poly_b[0] -= 2;

    sparse_poly::kar_multi(poly_c, poly_a, poly_b);
    sparse_poly::eea(poly_r, poly_cl, poly_cr, poly_a, poly_b);

    // a, b の式は入力
    // c = a * b
    std::cout << "a  : " << poly_a << "\nb  : " << poly_b << "\nc  : " << poly_c << "\n";

    // r は GCD
    // cl, cr はそれぞれ、 r = a * cl + b * cr となる係数
    std::cout << "r  : " << poly_r << "\ncl : " << poly_cl << "\ncr : " << poly_cr << "\n";

    std::cout << "end of test_sparse_poly\n\n";

    return 0;
}

void test_rational(){
    typedef cpp_multi_precision::rational<
        unsigned int,
        sizeof(unsigned int) * 8,
        long long,
        unsigned long long,
        std::vector<int>
    > rational;
    rational zero, a(2), b(a), c(std::move(zero)), d(10), e(rational::integer_type(29)), f(a + a), g;
    bool ret_comp;

    std::cout << "start test_rational\n";

    g = 1 + a;
    g = 1 - a;
    g = 1 * a;
    g = 1 / a;
    ret_comp = 1 < a;
    ret_comp = 1 > a;
    ret_comp = 1 <= a;
    ret_comp = 1 >= a;
    ret_comp = 1 == a;
    ret_comp = 1 != a;

    g = a + 1;
    g = a - 1;
    g = a * 1;
    g = a / 1;
    ret_comp = a < 1;
    ret_comp = a > 1;
    ret_comp = a <= 1;
    ret_comp = a >= 1;
    ret_comp = a == 1;
    ret_comp = a != 1;

    g = f + a;
    g = f - a;
    g = f * a;
    g = f / a;
    ret_comp = f < a;
    ret_comp = f > a;
    ret_comp = f <= a;
    ret_comp = f >= a;
    ret_comp = f == a;
    ret_comp = f != a;

    std::cout << "end of test_rational\n\n";
}

void test_primitive_mp_float(){
    typedef cpp_multi_precision::primitive_mp_float<
        unsigned int,
        sizeof(unsigned int) * 8,
        long long,
        unsigned long long,
        std::vector<int>
    > floating_point;
    floating_point a(10.0), f(a + a), g;
    bool ret_comp;

    std::cout << "start test_primitive_mp_float\n";

    g = 1 + a;
    g = 1 - a;
    g = 1 * a;
    g = 1 / a;
    ret_comp = 1 < a;
    ret_comp = 1 > a;
    ret_comp = 1 <= a;
    ret_comp = 1 >= a;
    ret_comp = 1 == a;
    ret_comp = 1 != a;

    g = a + 1;
    g = a - 1;
    g = a * 1;
    g = a / 1;
    ret_comp = a < 1;
    ret_comp = a > 1;
    ret_comp = a <= 1;
    ret_comp = a >= 1;
    ret_comp = a == 1;
    ret_comp = a != 1;

    g = f + a;
    g = f - a;
    g = f * a;
    g = f / a;
    ret_comp = f < a;
    ret_comp = f > a;
    ret_comp = f <= a;
    ret_comp = f >= a;
    ret_comp = f == a;
    ret_comp = f != a;

    std::cout << "end of test_primitive_mp_float\n\n";
}

void test_integer(){
    typedef cpp_multi_precision::integer<
        unsigned int,
        sizeof(unsigned int) * 8,
        long long,
        unsigned long long,
        std::vector<int>
    > integer;
    integer a(1), f(a + a), g;
    bool ret_comp;

    std::cout << "start test_integer\n";

    g = 1 + a;
    g = 1 - a;
    g = 1 * a;
    g = 1 / a;
    ret_comp = 1 < a;
    ret_comp = 1 > a;
    ret_comp = 1 <= a;
    ret_comp = 1 >= a;
    ret_comp = 1 == a;
    ret_comp = 1 != a;

    g = a + 1;
    g = a - 1;
    g = a * 1;
    g = a / 1;
    ret_comp = a < 1;
    ret_comp = a > 1;
    ret_comp = a <= 1;
    ret_comp = a >= 1;
    ret_comp = a == 1;
    ret_comp = a != 1;

    g = f + a;
    g = f - a;
    g = f * a;
    g = f / a;
    ret_comp = f < a;
    ret_comp = f > a;
    ret_comp = f <= a;
    ret_comp = f >= a;
    ret_comp = f == a;
    ret_comp = f != a;

    std::cout << "end of test_integer\n\n";
}

void test_modular(){
    typedef cpp_multi_precision::integer<
        unsigned int,
        sizeof(unsigned int) * 8,
        long long,
        unsigned long long,
        std::vector<int>
    > integer;
    
    typedef cpp_multi_precision::modular<integer> modular;

    std::cout << "start test_modular\n";

    modular a(4, 5), f(a + a), g;
    bool ret_comp;

    g = 1 + a;
    g = 1 - a;
    g = 1 * a;
    g = 1 / a;
    ret_comp = 1 < a;
    ret_comp = 1 > a;
    ret_comp = 1 <= a;
    ret_comp = 1 >= a;
    ret_comp = 1 == a;
    ret_comp = 1 != a;

    g = a + 1;
    g = a - 1;
    g = a * 1;
    g = a / 1;
    ret_comp = a < 1;
    ret_comp = a > 1;
    ret_comp = a <= 1;
    ret_comp = a >= 1;
    ret_comp = a == 1;
    ret_comp = a != 1;

    g = f + a;
    g = f - a;
    g = f * a;
    g = f / a;
    ret_comp = f < a;
    ret_comp = f > a;
    ret_comp = f <= a;
    ret_comp = f >= a;
    ret_comp = f == a;
    ret_comp = f != a;

    std::cout << "end of test_modular\n\n";
}

#include <string>
#include <fstream>

int main(){
    test_modular();
    test_sparse_poly();
    test_rational();
    test_primitive_mp_float();
    test_integer();

    return 0;
}
