// Copyright (C) 2016-2017 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#include <standardese/doc_entity.hpp>

#include <catch.hpp>

#include <standardese/markup/document.hpp>
#include <standardese/markup/generator.hpp>
#include <standardese/linker.hpp>

#include "test_parser.hpp"

using namespace standardese;

std::vector<std::string> get_details(const std::string& str)
{
    std::vector<std::string> result;

    std::string::size_type start = 0u;
    while (true)
    {
        start = str.find("<details-section>\n", start);
        if (start == std::string::npos)
            break;
        start += std::strlen("<details-section>\n");

        auto end = str.find("</details-section>", start);
        REQUIRE(end != std::string::npos);
        result.push_back(str.substr(start, end - start));

        start = end;
    }

    return result;
}

// no need to test documentation comments extensively
// just the top-level documentation hierarchy
TEST_CASE("documentation")
{
    comment_registry         comments;
    cppast::cpp_entity_index index;

    SECTION("basic")
    {
        auto file = build_doc_entities(comments, index, "documentation__basic.cpp", R"(
/// A function.
/// \effects Effects.
void foo();

namespace ns
{
    /// A class.
    class bar
    {
        int exclude_me;

        /// A virtual function.
        virtual void f1(int i);

    public:
       /// A member function.
       /// \module module
       void f2() const {}
    };
}
)");

        auto doc = generate_documentation({}, {}, index, *file);
        REQUIRE(markup::as_xml(*doc) == R"*(<file-documentation id="documentation__basic.cpp">
<heading>Header file <code>documentation__basic.cpp</code></heading>
<code-block language="cpp"><code-block-keyword>void</code-block-keyword> <internal-link unresolved-destination-id="foo()"><code-block-identifier>foo</code-block-identifier></internal-link><code-block-punctuation>(</code-block-punctuation><code-block-punctuation>)</code-block-punctuation><code-block-punctuation>;</code-block-punctuation><soft-break></soft-break>
<soft-break></soft-break>
<code-block-keyword>namespace</code-block-keyword> <code-block-identifier>ns</code-block-identifier><soft-break></soft-break>
<code-block-punctuation>{</code-block-punctuation><soft-break></soft-break>
    <code-block-keyword>class</code-block-keyword> <internal-link unresolved-destination-id="ns::bar"><code-block-identifier>bar</code-block-identifier></internal-link><code-block-punctuation>;</code-block-punctuation><soft-break></soft-break>
<code-block-punctuation>}</code-block-punctuation><soft-break></soft-break>
</code-block>
<entity-documentation id="foo()">
<heading>Function <code>foo</code></heading>
<code-block language="cpp"><code-block-keyword>void</code-block-keyword> <code-block-identifier>foo</code-block-identifier><code-block-punctuation>(</code-block-punctuation><code-block-punctuation>)</code-block-punctuation><code-block-punctuation>;</code-block-punctuation><soft-break></soft-break>
</code-block>
<brief-section id="foo()-brief">A function.</brief-section>
<inline-section name="Effects">Effects.</inline-section>
</entity-documentation>
<entity-documentation id="ns">
<entity-documentation id="ns::bar">
<heading>Class <code>bar</code></heading>
<code-block language="cpp"><code-block-keyword>class</code-block-keyword> <code-block-identifier>bar</code-block-identifier><soft-break></soft-break>
<code-block-punctuation>{</code-block-punctuation><soft-break></soft-break>
    <code-block-keyword>virtual</code-block-keyword> <code-block-keyword>void</code-block-keyword> <internal-link unresolved-destination-id="ns::bar::f1(int)"><code-block-identifier>f1</code-block-identifier></internal-link><code-block-punctuation>(</code-block-punctuation><code-block-keyword>int</code-block-keyword> <code-block-identifier>i</code-block-identifier><code-block-punctuation>)</code-block-punctuation><code-block-punctuation>;</code-block-punctuation><soft-break></soft-break>
<soft-break></soft-break>
<code-block-keyword>public</code-block-keyword><code-block-punctuation>:</code-block-punctuation><soft-break></soft-break>
    <code-block-keyword>void</code-block-keyword> <internal-link unresolved-destination-id="ns::bar::f2()const"><code-block-identifier>f2</code-block-identifier></internal-link><code-block-punctuation>(</code-block-punctuation><code-block-punctuation>)</code-block-punctuation> <code-block-keyword>const</code-block-keyword><code-block-punctuation>;</code-block-punctuation><soft-break></soft-break>
<code-block-punctuation>};</code-block-punctuation><soft-break></soft-break>
</code-block>
<brief-section id="ns::bar-brief">A class.</brief-section>
<entity-documentation id="ns::bar::f1(int)">
<heading>Function <code>f1</code></heading>
<code-block language="cpp"><code-block-keyword>virtual</code-block-keyword> <code-block-keyword>void</code-block-keyword> <code-block-identifier>f1</code-block-identifier><code-block-punctuation>(</code-block-punctuation><code-block-keyword>int</code-block-keyword> <code-block-identifier>i</code-block-identifier><code-block-punctuation>)</code-block-punctuation><code-block-punctuation>;</code-block-punctuation><soft-break></soft-break>
</code-block>
<brief-section id="ns::bar::f1(int)-brief">A virtual function.</brief-section>
</entity-documentation>
<entity-documentation id="ns::bar::f2()const" module="module">
<heading>Function <code>f2</code></heading>
<code-block language="cpp"><code-block-keyword>void</code-block-keyword> <code-block-identifier>f2</code-block-identifier><code-block-punctuation>(</code-block-punctuation><code-block-punctuation>)</code-block-punctuation> <code-block-keyword>const</code-block-keyword><code-block-punctuation>;</code-block-punctuation><soft-break></soft-break>
</code-block>
<brief-section id="ns::bar::f2()const-brief">A member function.</brief-section>
</entity-documentation>
</entity-documentation>
</entity-documentation>
</file-documentation>
)*");
    }
    SECTION("inlines")
    {
        auto file = build_doc_entities(comments, index, "documentation__inlines.cpp", R"(
/// Class.
/// \tparam A A
/// \tparam B B
/// \base C C
template <typename A, typename B, typename C>
struct foo : C
{
    int d; //< d
    float e;
    void* f; //< f
};

/// Function.
/// \param a a
/// \param b b
void func(int a, int b);

/// Enum.
enum class bar
{
    a, //< a
    b, //< b
};
)");

        auto doc = generate_documentation({}, {}, index, *file);
        REQUIRE(markup::as_xml(*doc) == R"*(<file-documentation id="documentation__inlines.cpp">
<heading>Header file <code>documentation__inlines.cpp</code></heading>
<code-block language="cpp"><code-block-keyword>template</code-block-keyword> <code-block-punctuation>&lt;</code-block-punctuation><code-block-keyword>typename</code-block-keyword> <internal-link unresolved-destination-id="foo&lt;A,B,C&gt;.A"><code-block-identifier>A</code-block-identifier></internal-link><code-block-punctuation>,</code-block-punctuation> <code-block-keyword>typename</code-block-keyword> <internal-link unresolved-destination-id="foo&lt;A,B,C&gt;.B"><code-block-identifier>B</code-block-identifier></internal-link><code-block-punctuation>,</code-block-punctuation> <code-block-keyword>typename</code-block-keyword> <code-block-identifier>C</code-block-identifier><code-block-punctuation>&gt;</code-block-punctuation><soft-break></soft-break>
<code-block-keyword>struct</code-block-keyword> <internal-link unresolved-destination-id="foo&lt;A,B,C&gt;"><code-block-identifier>foo</code-block-identifier></internal-link><code-block-punctuation>;</code-block-punctuation><soft-break></soft-break>
<soft-break></soft-break>
<code-block-keyword>void</code-block-keyword> <internal-link unresolved-destination-id="func(int,int)"><code-block-identifier>func</code-block-identifier></internal-link><code-block-punctuation>(</code-block-punctuation><code-block-keyword>int</code-block-keyword> <internal-link unresolved-destination-id="func(int,int).a"><code-block-identifier>a</code-block-identifier></internal-link><code-block-punctuation>,</code-block-punctuation> <code-block-keyword>int</code-block-keyword> <internal-link unresolved-destination-id="func(int,int).b"><code-block-identifier>b</code-block-identifier></internal-link><code-block-punctuation>)</code-block-punctuation><code-block-punctuation>;</code-block-punctuation><soft-break></soft-break>
<soft-break></soft-break>
<code-block-keyword>enum</code-block-keyword> <code-block-keyword>class</code-block-keyword> <internal-link unresolved-destination-id="bar"><code-block-identifier>bar</code-block-identifier></internal-link><code-block-punctuation>;</code-block-punctuation><soft-break></soft-break>
</code-block>
<entity-documentation id="foo&lt;A,B,C&gt;">
<heading>Struct <code>foo</code></heading>
<code-block language="cpp"><code-block-keyword>template</code-block-keyword> <code-block-punctuation>&lt;</code-block-punctuation><code-block-keyword>typename</code-block-keyword> <internal-link unresolved-destination-id="foo&lt;A,B,C&gt;.A"><code-block-identifier>A</code-block-identifier></internal-link><code-block-punctuation>,</code-block-punctuation> <code-block-keyword>typename</code-block-keyword> <internal-link unresolved-destination-id="foo&lt;A,B,C&gt;.B"><code-block-identifier>B</code-block-identifier></internal-link><code-block-punctuation>,</code-block-punctuation> <code-block-keyword>typename</code-block-keyword> <code-block-identifier>C</code-block-identifier><code-block-punctuation>&gt;</code-block-punctuation><soft-break></soft-break>
<code-block-keyword>struct</code-block-keyword> <code-block-identifier>foo</code-block-identifier><soft-break></soft-break>
<code-block-punctuation>:</code-block-punctuation> <internal-link unresolved-destination-id="foo&lt;A,B,C&gt;::C"><code-block-identifier>C</code-block-identifier></internal-link><soft-break></soft-break>
<code-block-punctuation>{</code-block-punctuation><soft-break></soft-break>
    <code-block-keyword>int</code-block-keyword> <internal-link unresolved-destination-id="foo&lt;A,B,C&gt;::d"><code-block-identifier>d</code-block-identifier></internal-link><code-block-punctuation>;</code-block-punctuation><soft-break></soft-break>
<soft-break></soft-break>
    <code-block-keyword>float</code-block-keyword> <code-block-identifier>e</code-block-identifier><code-block-punctuation>;</code-block-punctuation><soft-break></soft-break>
<soft-break></soft-break>
    <code-block-keyword>void</code-block-keyword><code-block-punctuation>*</code-block-punctuation> <internal-link unresolved-destination-id="foo&lt;A,B,C&gt;::f"><code-block-identifier>f</code-block-identifier></internal-link><code-block-punctuation>;</code-block-punctuation><soft-break></soft-break>
<code-block-punctuation>};</code-block-punctuation><soft-break></soft-break>
</code-block>
<brief-section id="foo&lt;A,B,C&gt;-brief">Class.</brief-section>
<list-section name="Template parameters">
<term-description-item id="foo&lt;A,B,C&gt;.A">
<term><code>A</code></term>
<description>A</description>
</term-description-item>
<term-description-item id="foo&lt;A,B,C&gt;.B">
<term><code>B</code></term>
<description>B</description>
</term-description-item>
</list-section>
<list-section name="Base classes">
<term-description-item id="foo&lt;A,B,C&gt;::C">
<term><code>C</code></term>
<description>C</description>
</term-description-item>
</list-section>
<list-section name="Member variables">
<term-description-item id="foo&lt;A,B,C&gt;::d">
<term><code>d</code></term>
<description>d</description>
</term-description-item>
<term-description-item id="foo&lt;A,B,C&gt;::f">
<term><code>f</code></term>
<description>f</description>
</term-description-item>
</list-section>
</entity-documentation>
<entity-documentation id="func(int,int)">
<heading>Function <code>func</code></heading>
<code-block language="cpp"><code-block-keyword>void</code-block-keyword> <code-block-identifier>func</code-block-identifier><code-block-punctuation>(</code-block-punctuation><code-block-keyword>int</code-block-keyword> <internal-link unresolved-destination-id="func(int,int).a"><code-block-identifier>a</code-block-identifier></internal-link><code-block-punctuation>,</code-block-punctuation> <code-block-keyword>int</code-block-keyword> <internal-link unresolved-destination-id="func(int,int).b"><code-block-identifier>b</code-block-identifier></internal-link><code-block-punctuation>)</code-block-punctuation><code-block-punctuation>;</code-block-punctuation><soft-break></soft-break>
</code-block>
<brief-section id="func(int,int)-brief">Function.</brief-section>
<list-section name="Parameters">
<term-description-item id="func(int,int).a">
<term><code>a</code></term>
<description>a</description>
</term-description-item>
<term-description-item id="func(int,int).b">
<term><code>b</code></term>
<description>b</description>
</term-description-item>
</list-section>
</entity-documentation>
<entity-documentation id="bar">
<heading>Enumeration <code>bar</code></heading>
<code-block language="cpp"><code-block-keyword>enum</code-block-keyword> <code-block-keyword>class</code-block-keyword> <code-block-identifier>bar</code-block-identifier><soft-break></soft-break>
<code-block-punctuation>{</code-block-punctuation><soft-break></soft-break>
    <internal-link unresolved-destination-id="bar::a"><code-block-identifier>a</code-block-identifier></internal-link><code-block-punctuation>,</code-block-punctuation><soft-break></soft-break>
    <internal-link unresolved-destination-id="bar::b"><code-block-identifier>b</code-block-identifier></internal-link><soft-break></soft-break>
<code-block-punctuation>};</code-block-punctuation><soft-break></soft-break>
</code-block>
<brief-section id="bar-brief">Enum.</brief-section>
<list-section name="Enumerators">
<term-description-item id="bar::a">
<term><code>a</code></term>
<description>a</description>
</term-description-item>
<term-description-item id="bar::b">
<term><code>b</code></term>
<description>b</description>
</term-description-item>
</list-section>
</entity-documentation>
</file-documentation>
)*");
    }
    SECTION("groups")
    {
        auto file = build_doc_entities(comments, index, "documentation__groups.cpp", R"(
/// Documentation.
/// \group a The a
void a();

/// \group a
void a(int param);
)");

        auto doc = generate_documentation({}, {}, index, *file);
        REQUIRE(markup::as_xml(*doc) == R"*(<file-documentation id="documentation__groups.cpp">
<heading>Header file <code>documentation__groups.cpp</code></heading>
<code-block language="cpp">//=== The a ===//<soft-break></soft-break>
<code-block-keyword>void</code-block-keyword> <internal-link unresolved-destination-id="a"><code-block-identifier>a</code-block-identifier></internal-link><code-block-punctuation>(</code-block-punctuation><code-block-punctuation>)</code-block-punctuation><code-block-punctuation>;</code-block-punctuation><soft-break></soft-break>
<code-block-keyword>void</code-block-keyword> <internal-link unresolved-destination-id="a(int)"><code-block-identifier>a</code-block-identifier></internal-link><code-block-punctuation>(</code-block-punctuation><code-block-keyword>int</code-block-keyword> <code-block-identifier>param</code-block-identifier><code-block-punctuation>)</code-block-punctuation><code-block-punctuation>;</code-block-punctuation><soft-break></soft-break>
</code-block>
<entity-documentation id="a()">
<heading>The a</heading>
<code-block language="cpp">(1) <code-block-keyword>void</code-block-keyword> <code-block-identifier>a</code-block-identifier><code-block-punctuation>(</code-block-punctuation><code-block-punctuation>)</code-block-punctuation><code-block-punctuation>;</code-block-punctuation><soft-break></soft-break>
<soft-break></soft-break>
(2) <code-block-keyword>void</code-block-keyword> <code-block-identifier>a</code-block-identifier><code-block-punctuation>(</code-block-punctuation><code-block-keyword>int</code-block-keyword> <code-block-identifier>param</code-block-identifier><code-block-punctuation>)</code-block-punctuation><code-block-punctuation>;</code-block-punctuation><soft-break></soft-break>
</code-block>
<brief-section id="a()-brief">Documentation.</brief-section>
</entity-documentation>
</file-documentation>
)*");
    }
    SECTION("linking")
    {
        auto target_file =
            build_doc_entities(comments, index, "documentation__linking_target.cpp", R"(
/// A function.
void func(int a);

void func2(int a);

/// A struct.
template <typename T>
struct foo
{
    int member; //< A member.

    /// Doc.
    /// \unique_name *bar()
    void baz();
};
)");

        auto file = build_doc_entities(comments, index, "documentation__linking.cpp", R"(
/// Another function.
void other_func();

/// Documentation with links.
///
/// [other_func()]()
/// [func(int)]()
/// [func(int).a]()
/// [func2(int)]()
/// [func2(int).a]()
/// [foo<T>]()
/// [foo<T>::member]()
/// [foo<T>::bar()]()
void bar();

/// Documentation with short links.
///
/// [other_func]()
/// [func]()
/// [func().a]()
/// [foo]()
/// [foo::member]()
/// [foo::bar]()
void bar2();

namespace ns
{
    /// doc
    void a();

    /// doc
    template <typename T>
    struct b
    {
        /// doc
        void c();

        /// Documentation with relative links.
        ///
        /// [*a]()
        /// [?b]()
        /// [*c]()
        void bar3();
    };
}
)");

        auto target_doc = markup::main_document::builder("target", "target")
                              .add_child(generate_documentation({}, {}, index, *target_file))
                              .finish();
        auto doc = markup::main_document::builder("doc", "doc")
                       .add_child(generate_documentation({}, {}, index, *file))
                       .finish();

        linker l;
        register_documentations(*test_logger(), l, *target_doc);
        register_documentations(*test_logger(), l, *doc);

        resolve_links(*test_logger(), l, *target_doc);
        resolve_links(*test_logger(), l, *doc);

        auto xml_doc = markup::as_xml(*doc);
        auto details = get_details(xml_doc);
        REQUIRE(details.size() == 3u);

        // long links
        REQUIRE(
            details[0]
            == R"*(<paragraph><internal-link destination-document="doc" destination-id="other_func()"><code>other_func()</code></internal-link><soft-break></soft-break>
<internal-link destination-document="target" destination-id="func(int)"><code>func(int)</code></internal-link><soft-break></soft-break>
<internal-link destination-document="target" destination-id="func(int)"><code>func(int).a</code></internal-link><soft-break></soft-break>
<internal-link destination-document="target" destination-id="documentation__linking_target.cpp"><code>func2(int)</code></internal-link><soft-break></soft-break>
<internal-link destination-document="target" destination-id="documentation__linking_target.cpp"><code>func2(int).a</code></internal-link><soft-break></soft-break>
<internal-link destination-document="target" destination-id="foo&lt;T&gt;"><code>foo&lt;T&gt;</code></internal-link><soft-break></soft-break>
<internal-link destination-document="target" destination-id="foo&lt;T&gt;::member"><code>foo&lt;T&gt;::member</code></internal-link><soft-break></soft-break>
<internal-link destination-document="target" destination-id="foo&lt;T&gt;::bar()"><code>foo&lt;T&gt;::bar()</code></internal-link></paragraph>
)*");
        // short links
        REQUIRE(
            details[1]
            == R"*(<paragraph><internal-link destination-document="doc" destination-id="other_func()"><code>other_func</code></internal-link><soft-break></soft-break>
<internal-link destination-document="target" destination-id="func(int)"><code>func</code></internal-link><soft-break></soft-break>
<internal-link destination-document="target" destination-id="func(int)"><code>func().a</code></internal-link><soft-break></soft-break>
<internal-link destination-document="target" destination-id="foo&lt;T&gt;"><code>foo</code></internal-link><soft-break></soft-break>
<internal-link destination-document="target" destination-id="foo&lt;T&gt;::member"><code>foo::member</code></internal-link><soft-break></soft-break>
<internal-link destination-document="target" destination-id="foo&lt;T&gt;::bar()"><code>foo::bar</code></internal-link></paragraph>
)*");
        // relative links
        REQUIRE(
            details[2]
            == R"*(<paragraph><internal-link destination-document="doc" destination-id="ns::a()"><code>a</code></internal-link><soft-break></soft-break>
<internal-link destination-document="doc" destination-id="ns::b&lt;T&gt;"><code>b</code></internal-link><soft-break></soft-break>
<internal-link destination-document="doc" destination-id="ns::b&lt;T&gt;::c()"><code>c</code></internal-link></paragraph>
)*");
    }
}