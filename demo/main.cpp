#include <format>
#include <iostream>
#include <ysEvent.hpp>

using namespace std;
using namespace YS;

class foo
{
public:
    foo(int id) : id(id) {}

    void Print() { cout << format("foo's Print()\nfoo's id : {}\n", id); }
    void Print() const { cout << format("foo's Print() const\nfoo's id : {}\n", id); }

    int id;
};

template <class T>
T const& GetFoo()
{
    static T t(123);
    return t;
}

void Normal_void_void() { cout << "called Normal_void_void()\n"; }
void Normal_void_int(int i) { cout << format("called Normal_void_int(int : {})\n", i); }
int Normal_int_void() { cout << "called Normal_int_void(), return 7\n"; return 7; }
int Normal_int_int(int i) { cout << format("called Normal_int_int(int : {}), return {}\n", i, i); return i; }
float Normal_float_ifsb(int i, float f, string s, bool b) { cout << format("called Normal_int_ifsb(i : {}, f : {}, s : {}, b : {}), return i + f : {}\n", i, f, s, b, i + f); return i + f; }
template <typename... _Args>
tuple<int, float, string> Normal_complex(_Args... args) { cout << format("called Normal_complex()"); return { 1, 3135.592f, "sys"s }; }

int main()
{
    auto pFoo = make_shared<foo>(5);
    auto pConstFoo = make_shared<const foo>(5);

    // 생성자 테스트
    Event<void()> ctorDefault;
    Event<void()> ctor_NCobj_NCfn(pFoo, SelectNonConstFn(&foo::Print));
    Event<void()> ctor_NCobj_Cfn(pFoo, SelectConstFn(&foo::Print));
    //Event<void()> ctor_Cobj_NCfn(pConstFoo, SelectNonConstFn(&foo::Print));
    Event<void()> ctor_Cobj_Cfn(pConstFoo, SelectConstFn(&foo::Print));

    // 다양한 타입에 대한 Event 적용 테스트
    Event<void()> void_void_fn;
    Event<void(int)> void_int_fn;
    Event<int()> int_void_fn;
    Event<int(int)> int_int_fn;
    Event<float(int, float, string, bool)> float_ifsb_fn;
    Event<tuple<int, float, string>(int, bool, tuple<int, float, foo, const foo, foo&, foo*>)> complex_fn;

    // operator= 테스트
    // lambda 테스트
    cout << "Test operator=\n\n";
    cout << R"(void_void_fn = []() { cout << "called void() lambda\n"; })" << endl;
    void_void_fn = []() { cout << "called void() lambda\n"; };
    cout << "void_void_fn()\n";
    void_void_fn();
    cout << R"(void_int_fn = [](int i) { cout << format("called void(int : {}) lambda\n", i); })" << endl;
    void_int_fn = [](int i) { cout << format("called void(int : {}) lambda\n", i); };
    cout << "void_int_fn(3)\n";
    void_int_fn(3);
    cout << R"(int_void_fn = []() { cout << "called int() lambda, return 2\n"; return 2; })" << endl;
    int_void_fn = []() { cout << "called int() lambda, return 2\n"; return 2; };
    cout << "int_void_fn()\n";
    auto l1 = int_void_fn();
    cout << R"(int_int_fn = [](int i) { cout << format("called int(int : {}) lambda, return {}\n", i, i); return i; })" << endl;
    int_int_fn = [](int i) { cout << format("called int(int : {}) lambda, return {}\n", i, i); return i; };
    cout << "int_int_fn(92)\n";
    auto l2 = int_int_fn(92);
    cout << R"(float_ifsb_fn = [](int i, float f, string s, bool b){ cout << format("called float(i : {}, f : {}, s : {}, b : {}) lambda, return i + f : {}\n", i, f, s, b, i + f); return i + f; })" << endl;
    float_ifsb_fn = [](int i, float f, string s, bool b){ cout << format("called float(i : {}, f : {}, s : {}, b : {}) lambda, return i + f : {}\n", i, f, s, b, i + f); return i + f; };
    cout << R"(float_ifsb_fn(2, 3.7, "hi", true))" << endl;
    auto l3 = float_ifsb_fn(2, 3.7, "hi", true);
    cout << R"(complex_fn = [](int i, bool b, tuple<int, float, foo, const foo, foo&, foo*> t) ...)" << endl;
    complex_fn = [](int i, bool b, tuple<int, float, foo, const foo, foo&, foo*> t)
    {
        auto& [ti, tf, tfoo, tcf, tfr, tfp] = t;
        cout << format("called tuple<int, float, string>(i : {}, b : {}, tuple<i : {}, f : {}, foo : {}, const foo : {}, foo& : {}, foo* : {}>) lambda, return tuple<i : {}, f : {}, s : {}>\n", i, b, ti, tf, tfoo.id, tcf.id, tfr.id, tfp->id, i, tf, "complex lambda"s); return make_tuple(i, tf, "complex lambda"s);
    };
    cout << R"(complex_fn(16, true, {931, 386.182f, foo(136), *pConstFoo, *pFoo, pFoo.get()}))" << endl;
    auto l4 = complex_fn(16, true, {931, 386.182f, foo(136), *pConstFoo, *pFoo, pFoo.get()});
    cout << "\n\n";

    // operator+= 테스트
    cout << "test operator+=\n\n";
    cout << R"(void_void_fn += Normal_void_void)" << endl;
    void_void_fn += Normal_void_void;
    cout << "void_void_fn()\n";
    void_void_fn();
    cout << R"(void_int_fn += Normal_void_int)" << endl;
    void_int_fn += Normal_void_int;
    cout << "void_int_fn(9)\n";
    void_int_fn(9);
    cout << R"(int_void_fn += Normal_int_void)" << endl;
    int_void_fn += Normal_int_void;
    cout << "int_void_fn()\n";
    auto l5 = int_void_fn();
    cout << R"(int_int_fn += Normal_int_int)" << endl;
    int_int_fn += Normal_int_int;
    cout << "int_int_fn(1)\n";
    auto l6 = int_int_fn(1);
    cout << R"(float_ifsb_fn += Normal_float_ifsb)" << endl;
    float_ifsb_fn += Normal_float_ifsb;
    cout << R"(float_ifsb_fn(9, 21.683, "hello", false))" << endl;
    auto l7 = float_ifsb_fn(9, 21.683, "hello", false);
    cout << R"(complex_fn += Normal_complex<int, bool, tuple<int, float, foo, const foo, foo&, foo*>>)" << endl;
    complex_fn += Normal_complex<int, bool, tuple<int, float, foo, const foo, foo&, foo*>>;
    cout << R"(complex_fn(25, false, {69, 26.1260f, foo(25), *pConstFoo, *pFoo, pFoo.get()}))" << endl;
    auto l8 = complex_fn(25, false, {69, 26.1260f, foo(25), *pConstFoo, *pFoo, pFoo.get()});
    cout << "\n\n";

    // operator-= 테스트
    cout << "test operator-=\n\n";
    cout << R"(void_void_fn -= Normal_void_void)" << endl;
    void_void_fn -= Normal_void_void;
    cout << "void_void_fn()\n";
    void_void_fn();
    cout << R"(void_int_fn -= Normal_void_int)" << endl;
    void_int_fn -= Normal_void_int;
    cout << "void_int_fn(9)\n";
    void_int_fn(9);
    cout << "int_void_fn()\n";
    auto l9 = int_void_fn();
    cout << R"(int_int_fn -= Normal_int_int)" << endl;
    int_int_fn -= Normal_int_int;
    cout << "int_int_fn(1)\n";
    auto l10 = int_int_fn(1);
    cout << R"(float_ifsb_fn -= Normal_float_ifsb)" << endl;
    float_ifsb_fn -= Normal_float_ifsb;
    cout << R"(float_ifsb_fn(9, 21.683, "hello", false))" << endl;
    auto l11 = float_ifsb_fn(9, 21.683, "hello", false);
    cout << R"(complex_fn -= Normal_complex<int, bool, tuple<int, float, foo, const foo, foo&, foo*>>)" << endl;
    complex_fn -= Normal_complex<int, bool, tuple<int, float, foo, const foo, foo&, foo*>>;
    cout << R"(complex_fn(25, false, {69, 26.1260f, foo(25), *pConstFoo, *pFoo, pFoo.get()}))" << endl;
    auto l12 = complex_fn(25, false, {69, 26.1260f, foo(25), *pConstFoo, *pFoo, pFoo.get()});
    cout << "\n\n";

    // RemoveAllListner 테스트
    cout << "Test RemoveAllListener\n\n";
    cout << R"(void_void_fn.RemoveAllListener())" << endl;
    void_void_fn.RemoveAllListener();
    cout << R"(void_int_fn.RemoveAllListener())" << endl;
    void_int_fn.RemoveAllListener();
    cout << R"(int_void_fn.RemoveAllListener())" << endl;
    int_void_fn.RemoveAllListener();
    cout << R"(int_int_fn.RemoveAllListener())" << endl;
    int_int_fn.RemoveAllListener();
    cout << R"(float_ifsb_fn.RemoveAllListener())" << endl;
    float_ifsb_fn.RemoveAllListener();
    cout << R"(complex_fn.RemoveAllListener())" << endl;
    complex_fn.RemoveAllListener();
    cout << "void_void_fn()\n";
    void_void_fn();
    cout << "void_int_fn()\n";
    void_int_fn(931);
    cout << "int_void_fn()\n";
    auto l13 = int_void_fn();
    cout << "int_int_fn()\n";
    auto l14 = int_int_fn(162);
    cout << "float_ifsb_fn()\n";
    auto l15 = float_ifsb_fn(1, 1.3, "asf"s, true);
    cout << "complex_fn()\n";
    auto l16 = complex_fn(25, false, {69, 26.1260f, foo(25), *pConstFoo, *pFoo, pFoo.get()});
    cout << "\n\n";

    Event<void()> e1;
    // AddListener 테스트
    cout << "test AddListener\n\n";
    cout << R"(e1.AddListener([]() {cout << "lambda\n"; }))" << endl;
    e1.AddListener([]() { cout << "lambda\n"; });
    cout << R"(e1.AddListener(Normal_void_void))" << endl;
    e1.AddListener(Normal_void_void);
    cout << R"(e1.AddListener(pFoo, SelectNonConstFn(&foo::Print)))" << endl;
    e1.AddListener(pFoo, SelectNonConstFn(&foo::Print));
    cout << R"(e1.AddListener(pFoo, SelectConstFn(&foo::Print)))" << endl;
    e1.AddListener(pFoo, SelectConstFn(&foo::Print));
    //e1.AddListener(pConstFoo, SelectNonConstFn(&foo::Print));
    cout << R"(e1.AddListener(pConstFoo, SelectConstFn(&foo::Print)))" << endl;
    e1.AddListener(pConstFoo, SelectConstFn(&foo::Print));

    cout << "e1()\n";
    e1();
    cout << "\n\n";

    // RemoveListener 테스트
    cout << "test RemoveListener\n\n";
    cout << R"(e1.RemoveListener([]() {cout << "lambda\n"; }))" << endl;
    e1.RemoveListener([]() { cout << "lambda\n"; });
    cout << "e1()\n";
    e1();
    cout << endl;
    cout << R"(e1.RemoveListener(pFoo, SelectConstFn(&foo::Print)))" << endl;
    e1.RemoveListener(pFoo, SelectConstFn(&foo::Print));
    cout << "e1()\n";
    e1();
    cout << endl;
    cout << R"(e1.RemoveListener(pFoo, SelectNonConstFn(&foo::Print)))" << endl;
    e1.RemoveListener(pFoo, SelectNonConstFn(&foo::Print));
    cout << "e1()\n";
    e1();
    cout << endl;
    cout << R"(e1.RemoveListener(Normal_void_void))" << endl;
    e1.RemoveListener(Normal_void_void);
    cout << "e1()\n";
    e1();
    cout << endl;
    cout << R"(e1.RemoveListener(pFoo, SelectConstFn(&foo::Print)))" << endl;
    e1.RemoveListener(pFoo, SelectConstFn(&foo::Print));
    cout << "e1()\n";
    e1();
    cout << endl;
    cout << R"(e1.RemoveListener(pConstFoo, SelectConstFn(&foo::Print)))" << endl;
    e1.RemoveListener(pConstFoo, SelectConstFn(&foo::Print));
    cout << "e1()\n";
    e1();
}