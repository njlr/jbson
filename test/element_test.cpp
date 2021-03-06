//          Copyright Christian Manning 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <string>
using namespace std::literals;
#include <list>
#include <deque>

#include <jbson/detail/config.hpp>

JBSON_PUSH_DISABLE_DOCUMENTATION_WARNING
#include <boost/container/stable_vector.hpp>
#include <boost/container/vector.hpp>
JBSON_CLANG_POP_WARNINGS

#include <jbson/element.hpp>
#include <jbson/document.hpp>
#include <jbson/builder.hpp>
using namespace jbson;

#include <gtest/gtest.h>

// compile-time tests

static_assert(detail::is_nothrow_swappable<element>::value, "");
static_assert(!detail::is_nothrow_swappable<boost::container::vector<char>>::value, "");

// static_assert(std::is_nothrow_move_assignable<basic_element<std::vector<char>>>::value, "");
// static_assert(std::is_nothrow_move_constructible<basic_element<std::vector<char>>>::value, "");
// static_assert(std::is_nothrow_move_assignable<basic_element<std::deque<char>>>::value, "");
// static_assert(std::is_nothrow_move_constructible<basic_element<std::deque<char>>>::value, "");

static_assert(!std::is_constructible<std::string,
                                     std::tuple<std::experimental::string_view, std::experimental::string_view>>::value,
              "");

static_assert(detail::is_valid_element_value_type<std::vector<char>, bool>::value, "");
static_assert(detail::is_valid_element_value_type<std::vector<char>, int32_t>::value, "");
static_assert(detail::is_valid_element_value_type<std::vector<char>, std::experimental::string_view>::value, "");

static_assert(!detail::is_valid_element_value_type<std::vector<char>, std::chrono::milliseconds>::value, "");
static_assert(!detail::is_valid_element_value_type<std::vector<char>, std::set<int>>::value, "");

TEST(ElementTest, ElementParseTest1) {
    auto el1 = element{boost::make_iterator_range("\x02hello\x00\x06\x00\x00\x00world\x00"s)};
    ASSERT_EQ(element_type::string_element, el1.type());
    EXPECT_EQ("hello", el1.name());
    EXPECT_NO_THROW(EXPECT_EQ("world", get<element_type::string_element>(el1)));
    el1.value("test");
    EXPECT_NO_THROW(EXPECT_EQ("test", get<element_type::string_element>(el1)));

    EXPECT_THROW(get<element_type::boolean_element>(el1), incompatible_element_conversion);
    EXPECT_NO_THROW(el1.value<std::experimental::string_view>());
    EXPECT_THROW(el1.value<bool>(), incompatible_type_conversion);

    EXPECT_NO_THROW(el1.value(element_type::boolean_element, false));
    EXPECT_NO_THROW(EXPECT_FALSE(get<element_type::boolean_element>(el1)));

    EXPECT_NO_THROW(el1.value(true));
    ASSERT_EQ(element_type::boolean_element, el1.type());
    EXPECT_NO_THROW(EXPECT_TRUE(get<element_type::boolean_element>(el1)));
    EXPECT_EQ(8, el1.size());
    EXPECT_NO_THROW(el1.value(element_type::boolean_element, 432));

    ASSERT_EQ(element_type::boolean_element, el1.type());
    EXPECT_NO_THROW(EXPECT_TRUE(get<element_type::boolean_element>(el1)));
    EXPECT_EQ(8, el1.size());

    EXPECT_NO_THROW(el1.value(static_cast<bool>(432)));
    ASSERT_EQ(element_type::boolean_element, el1.type());
    EXPECT_NO_THROW(EXPECT_TRUE(get<element_type::boolean_element>(el1)));
    EXPECT_EQ(8, el1.size());

    EXPECT_NO_THROW(el1.value(static_cast<bool>(0)));
    ASSERT_EQ(element_type::boolean_element, el1.type());
    EXPECT_NO_THROW(EXPECT_FALSE(get<element_type::boolean_element>(el1)));
    EXPECT_EQ(8, el1.size());

    EXPECT_NO_THROW(el1.value<bool>());
    EXPECT_THROW(el1.value<int64_t>(), invalid_element_size);

    EXPECT_NO_THROW(el1.value<element_type::int64_element>(24));
    EXPECT_THROW(el1.value<int32_t>(), invalid_element_size);
    ASSERT_EQ(element_type::int64_element, el1.type());
    EXPECT_NO_THROW(EXPECT_EQ(24, get<element_type::int64_element>(el1)));
    EXPECT_EQ(15, el1.size());

    EXPECT_NO_THROW(el1.value((int8_t)24));
    EXPECT_EQ(element_type::boolean_element, el1.type());
    EXPECT_NO_THROW(EXPECT_EQ(true, get<element_type::boolean_element>(el1)));
    EXPECT_EQ(8, el1.size());
}

TEST(ElementTest, ElementParseTest2) {
    try {
    auto bson_data = "\x02hello\x00\x06\x00\x00\x00world\x00"s;
    auto bson = boost::make_iterator_range(bson_data);
    auto el1 = basic_element<std::vector<char>>{bson};
    EXPECT_EQ(bson.size(), el1.size());
    ASSERT_EQ(element_type::string_element, el1.type());
    EXPECT_EQ("hello", el1.name());
    EXPECT_EQ("world", get<element_type::string_element>(el1));
    el1.name("some name");
    EXPECT_EQ("some name", el1.name());
    el1.value("some value");
    EXPECT_EQ("some value", get<element_type::string_element>(el1));
    el1.value(element_type::int32_element, 1234);
    ASSERT_EQ(element_type::int32_element, el1.type());
    EXPECT_EQ(1234, get<element_type::int32_element>(el1));
    EXPECT_EQ(15, el1.size());
    }
    catch(...) {
        FAIL() << boost::current_exception_diagnostic_information();
    }
}

TEST(ElementTest, ElementParseTest3) {
    auto bson = boost::make_iterator_range("\x00hello\x00\x06\x00\x00\x00world\x00"s);
    ASSERT_THROW(element{bson}, invalid_element_type);
    bson = "\x02hello\x06\x00\x00\x00world\x00"s;
    ASSERT_THROW(element{bson}, invalid_element_type);

    bson = "\x02hello\x00\x06\x00\x00\x00world"s;
    ASSERT_THROW(element{bson}, invalid_element_type);
}

TEST(ElementTest, ElementTypeConversionsTest1) {
    auto el1 = element{"some name", 123};
    ASSERT_EQ("some name", el1.name());
    ASSERT_EQ(element_type::int32_element, el1.type());
    EXPECT_EQ(123, get<element_type::int32_element>(el1));

    ASSERT_NO_THROW(el1.type(element_type::int64_element));

    EXPECT_THROW(get<element_type::int32_element>(el1), incompatible_element_conversion);
    EXPECT_THROW(get<element_type::int64_element>(el1), invalid_element_size);
    EXPECT_EQ(123, el1.value<int32_t>());
    EXPECT_EQ(123, el1.value<int>());
    EXPECT_THROW(el1.value<int64_t>(), invalid_element_size);

    try {
        EXPECT_EQ(123, el1.value<short>());
    } catch(...) {
        FAIL() << boost::current_exception_diagnostic_information();
    }

    ASSERT_NO_THROW(el1.type(element_type::string_element));

    EXPECT_THROW(get<element_type::string_element>(el1), invalid_element_size);

    ASSERT_NO_THROW(el1.type(element_type::document_element));

    EXPECT_THROW(get<element_type::document_element>(el1), invalid_document_size);
}

TEST(ElementTest, ElementConstructTest1) {
    auto el1 = element{"Pi 6dp", element_type::double_element, 3.141592};
    ASSERT_EQ(element_type::double_element, el1.type());
    EXPECT_EQ("Pi 6dp", el1.name());
    EXPECT_DOUBLE_EQ(3.141592, get<element_type::double_element>(el1));
    const auto val = 44.854;
    el1.value(val);
    ASSERT_EQ(element_type::double_element, el1.type());
    EXPECT_EQ(val, get<element_type::double_element>(el1));

    el1 = element{};
    EXPECT_EQ("", el1.name());
    ASSERT_EQ(element_type::null_element, el1.type());
    EXPECT_THROW(el1.value<bool>(), incompatible_type_conversion);
    EXPECT_NO_THROW(el1.value<bool>(true));
    ASSERT_EQ(element_type::boolean_element, el1.type());

    EXPECT_NO_THROW(el1.value(true));
    ASSERT_EQ(element_type::boolean_element, el1.type());
    EXPECT_NO_THROW(el1.type(element_type::null_element));
    EXPECT_THROW(el1.value<bool>(), incompatible_type_conversion);
    EXPECT_NO_THROW(el1.value<bool>(true));
    ASSERT_EQ(element_type::boolean_element, el1.type());

    {
        decltype(auto) a = "name";
        decltype(auto) b = "value";
        auto el2 = element{a, b};
        ASSERT_EQ(element_type::string_element, el2.type());
        EXPECT_EQ("name", el2.name());
        EXPECT_EQ("value", get<element_type::string_element>(el2));
    }
    {
        auto a = "name";
        auto b = "value";
        auto el2 = element{a, b};
        ASSERT_EQ(element_type::string_element, el2.type());
        EXPECT_EQ("name", el2.name());
        EXPECT_EQ("value", get<element_type::string_element>(el2));
    }

    ASSERT_NO_THROW((el1 = element{"doc", builder{}}));
    ASSERT_EQ(element_type::document_element, el1.type());
    ASSERT_EQ(document(builder{}), get<element_type::document_element>(el1));
}

TEST(ElementTest, ElementConstructTest2) {
    element el1{"asd"};
    ASSERT_NO_THROW((el1 = element{"Pi 6dp", 3.141592}));
    ASSERT_EQ(element_type::double_element, el1.type());
    EXPECT_EQ("Pi 6dp", el1.name());
    EXPECT_DOUBLE_EQ(3.141592, get<element_type::double_element>(el1));

    ASSERT_NO_THROW((el1 = element{"Pi 6dp", 3.141592f}));
    ASSERT_EQ(element_type::double_element, el1.type());
    EXPECT_EQ("Pi 6dp", el1.name());
    EXPECT_FLOAT_EQ(3.141592, get<element_type::double_element>(el1));

    ASSERT_NO_THROW((el1 = element{"val", "literal"}));
    ASSERT_EQ(element_type::string_element, el1.type());
    EXPECT_EQ("val", el1.name());
    EXPECT_EQ("literal", get<element_type::string_element>(el1));

    ASSERT_NO_THROW((el1 = element{"val", "literal"s}));
    ASSERT_EQ(element_type::string_element, el1.type());
    EXPECT_EQ("val", el1.name());
    EXPECT_EQ("literal", get<element_type::string_element>(el1));

    std::string str1 = "str"s;
    ASSERT_NO_THROW((el1 = element{"val", str1}));
    ASSERT_EQ(element_type::string_element, el1.type());
    EXPECT_EQ("val", el1.name());
    EXPECT_EQ("str", get<element_type::string_element>(el1));

    const char* str2 = "str";
    ASSERT_NO_THROW((el1 = element{"val", str2}));
    ASSERT_EQ(element_type::string_element, el1.type());
    EXPECT_EQ("val", el1.name());
    EXPECT_EQ("str", get<element_type::string_element>(el1));

    ASSERT_NO_THROW((el1 = element{"val", 123}));
    ASSERT_EQ(element_type::int32_element, el1.type());
    EXPECT_EQ("val", el1.name());
    EXPECT_EQ(123, get<element_type::int32_element>(el1));

    EXPECT_NO_THROW((el1 = element{"nest", builder{}}));
    ASSERT_EQ(element_type::document_element, el1.type());
    EXPECT_EQ("nest", el1.name());
    EXPECT_EQ(0, boost::distance(get<element_type::document_element>(el1)));

    EXPECT_NO_THROW((el1 = element{"nest", element_type::document_element, builder{}}));
    ASSERT_EQ(element_type::document_element, el1.type());
    EXPECT_EQ("nest", el1.name());
    EXPECT_EQ(0, boost::distance(get<element_type::document_element>(el1)));

    ASSERT_NO_THROW((el1 = element{"val", false}));
    ASSERT_EQ(element_type::boolean_element, el1.type());
    EXPECT_EQ("val", el1.name());
    EXPECT_EQ(false, get<element_type::boolean_element>(el1));
}

TEST(ElementTest, ElementCopyTest1) {
    auto el1 = element{"Pi 6dp", element_type::double_element, 3.141592};
    ASSERT_EQ(element_type::double_element, el1.type());
    EXPECT_EQ("Pi 6dp", el1.name());
    EXPECT_DOUBLE_EQ(3.141592, get<element_type::double_element>(el1));
    element el2 = el1;
    ASSERT_EQ(element_type::double_element, el2.type());
    EXPECT_EQ("Pi 6dp", el2.name());
    EXPECT_DOUBLE_EQ(3.141592, get<element_type::double_element>(el2));
    EXPECT_EQ(el1, el2);
}

TEST(ElementTest, ElementCopyTest2) {
    auto el1 = element{"Pi 6dp", element_type::double_element, 3.141592};
    ASSERT_EQ(element_type::double_element, el1.type());
    EXPECT_EQ("Pi 6dp", el1.name());
    EXPECT_DOUBLE_EQ(3.141592, get<element_type::double_element>(el1));
    element el2 = el1;
    const auto val = 44.854;
    el2.value(val);
    ASSERT_EQ(element_type::double_element, el2.type());
    EXPECT_EQ("Pi 6dp", el2.name());
    EXPECT_DOUBLE_EQ(val, get<element_type::double_element>(el2));
    EXPECT_NE(el1, el2);
}

TEST(ElementTest, ElementCopyConvertTest1) {
    auto el1 = element{"Pi 6dp", element_type::double_element, 3.141592};
    ASSERT_EQ(element_type::double_element, el1.type());
    EXPECT_EQ("Pi 6dp", el1.name());
    EXPECT_DOUBLE_EQ(3.141592, get<element_type::double_element>(el1));
    basic_element<std::list<char>> el2 = el1;
    EXPECT_EQ(el1, el2);
    const auto val = 44.854;
    el2.value(val);
    ASSERT_EQ(element_type::double_element, el2.type());
    EXPECT_EQ("Pi 6dp", el2.name());
    EXPECT_DOUBLE_EQ(val, get<element_type::double_element>(el2));
    EXPECT_NE(el1, el2);
}

TEST(ElementTest, ElementMoveTest1) {
    auto el1 = element{"Pi 6dp", element_type::double_element, 3.141592};
    ASSERT_EQ(element_type::double_element, el1.type());
    EXPECT_EQ("Pi 6dp", el1.name());
    EXPECT_DOUBLE_EQ(3.141592, get<element_type::double_element>(el1));
    auto old_size = el1.size();
    element el2 = std::move(el1);
    ASSERT_EQ(2, el1.size());
    ASSERT_EQ(old_size, el2.size());
    ASSERT_EQ(element_type::double_element, el2.type());
    EXPECT_EQ("Pi 6dp", el2.name());
    EXPECT_DOUBLE_EQ(3.141592, get<element_type::double_element>(el2));
    EXPECT_NE(el1, el2);
}

TEST(ElementTest, ElementVoidTest) {
    auto el1 = element{"null element", element_type::null_element};
    ASSERT_EQ(element_type::null_element, el1.type());
    EXPECT_EQ("null element", el1.name());
    ASSERT_EQ(el1.name().size() + sizeof('\0') + sizeof(element_type), el1.size());
    ASSERT_ANY_THROW(el1.value<bool>());
}

TEST(ElementTest, ElementRefTest1) {
    static_assert(std::is_same<std::experimental::string_view,
                               decltype(get<element_type::string_element>(
                                   std::declval<basic_element<std::experimental::string_view>>()))>::value,
                  "");
    static_assert(
        std::is_same<std::experimental::string_view, decltype(get<element_type::string_element>(
                                                         std::declval<basic_element<std::vector<char>>>()))>::value,
        "");
    static_assert(std::is_same<basic_document<boost::iterator_range<std::vector<char>::const_iterator>>,
                               decltype(get<element_type::document_element>(
                                   std::declval<basic_element<std::vector<char>>>()))>::value,
                  "");
    static_assert(std::is_same<std::string, decltype(get<element_type::string_element>(
                                                std::declval<basic_element<std::list<char>>>()))>::value,
                  "");
    ASSERT_TRUE((std::is_same<std::experimental::string_view,
                              decltype(get<element_type::string_element>(std::declval<element>()))>::value));
}

TEST(ElementTest, ElementRefTest2) {
    std::vector<char> bson;
    boost::range::push_back(bson, "\x02hello\x00\x06\x00\x00\x00world\x00"s);
    auto el1 = basic_element<boost::iterator_range<decltype(bson)::const_iterator>>{bson};
    EXPECT_EQ(bson.size(), el1.size());
    ASSERT_EQ(element_type::string_element, el1.type());
    EXPECT_EQ("hello", el1.name());
    EXPECT_EQ("world", get<element_type::string_element>(el1));
    auto str_ref = get<element_type::string_element>(el1);
    EXPECT_GE(str_ref.data(), bson.data());
    EXPECT_LT(str_ref.data(), (bson.data() + bson.size()));
    el1.name("some name");
    EXPECT_EQ("some name", el1.name());
}

// test elements one type at a time
template <typename ElemType> struct VoidVisitor {
    ElemType m_v;

    explicit VoidVisitor(ElemType v) : m_v(v) {
    }

    void operator()(std::experimental::string_view /*name*/, element_type e, ElemType v) {
        EXPECT_EQ(element_type::double_element, e);
        EXPECT_EQ(m_v, v);
    }

    template <typename T> void operator()(std::experimental::string_view, element_type, T&&) {
        FAIL();
    }
    void operator()(std::experimental::string_view, element_type) {
        FAIL();
    }
};

TEST(ElementTest, ElementVisitTest1) {
    auto el1 = element{"Pi 6dp", element_type::double_element, 3.141592};
    ASSERT_EQ(element_type::double_element, el1.type());
    EXPECT_EQ("Pi 6dp", el1.name());
    EXPECT_DOUBLE_EQ(3.141592, get<element_type::double_element>(el1));
    el1.visit(VoidVisitor<double>(3.141592));
}

template <typename ElemType> struct BoolVisitor {
    ElemType m_v;

    explicit BoolVisitor(ElemType v) : m_v(v) {
    }

    bool operator()(std::experimental::string_view /*name*/, element_type e, ElemType v) {
        EXPECT_EQ(m_v, v);
        return element_type::double_element == e;
    }

    template <typename T> bool operator()(std::experimental::string_view, element_type, T&&) {
        return false;
    }
    bool operator()(std::experimental::string_view, element_type) {
        return false;
    }
};

TEST(ElementTest, ElementVisitTest2) {
    auto el1 = element{"Pi 6dp", element_type::double_element, 3.141592};
    ASSERT_EQ(element_type::double_element, el1.type());
    EXPECT_EQ("Pi 6dp", el1.name());
    EXPECT_DOUBLE_EQ(3.141592, get<element_type::double_element>(el1));
    EXPECT_TRUE(el1.visit(BoolVisitor<double>(3.141592)));
}

TEST(ElementTest, ElementGetDocumentTest1) {
    element::container_type data{{0x03, 'e', 'm', 'b', 'e', 'd', 'd', 'e', 'd', ' ', 'd', 'o', 'c', 'u', 'm', 'e', 'n',
                                  't', '\0', 0x05, 0x00, 0x00, 0x00, 0x00}};
    auto el1 = basic_element<boost::iterator_range<element::container_type::const_iterator>>{data};
    ASSERT_EQ(element_type::document_element, el1.type());
    EXPECT_EQ("embedded document", el1.name());
    auto doc = get<element_type::document_element>(el1);
    static_assert(detail::is_document<decltype(doc)>::value, "");
    static_assert(std::is_same<boost::iterator_range<element::container_type::const_iterator>,
                               decltype(doc)::container_type>::value,
                  "");
    EXPECT_EQ(5, doc.size());
}

template <typename Container> struct ParameterizedContainerTest : ::testing::Test {
    using container_type = Container;
    template <element_type EType> using ElementTypeMap = detail::ElementTypeMap<EType, container_type>;
    using string_type = ElementTypeMap<element_type::string_element>;
};
TYPED_TEST_CASE_P(ParameterizedContainerTest);

JBSON_PUSH_DISABLE_DEPRECATED_WARNING
TYPED_TEST_P(ParameterizedContainerTest, ElementOIDTest) {
    const typename TestFixture::template ElementTypeMap<element_type::oid_element> oid{
        {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}};
    basic_element<typename TestFixture::container_type> el{"_id", element_type::oid_element, oid};
    EXPECT_EQ(oid, get<element_type::oid_element>(el));

    EXPECT_NO_THROW(el.template value<element_type::db_pointer_element>(std::make_tuple("some collection", oid)));
    typename TestFixture::string_type coll;
    std::decay_t<decltype(oid)> new_oid;
    std::tie(coll, new_oid) = get<element_type::db_pointer_element>(el);
    EXPECT_EQ("some collection", coll);
    EXPECT_EQ(oid, new_oid);
}
JBSON_POP_WARNINGS

TYPED_TEST_P(ParameterizedContainerTest, ElementRegexTest) {
    basic_element<typename TestFixture::container_type> el{"some filter", element_type::regex_element};
    ASSERT_NO_THROW(el.value(std::make_tuple(".*", "i")));
    EXPECT_EQ(18, el.size());
    typename TestFixture::string_type regex, options;
    std::tie(regex, options) = get<element_type::regex_element>(el);
    EXPECT_EQ(".*", regex);
    EXPECT_EQ("i", options);
}

REGISTER_TYPED_TEST_CASE_P(ParameterizedContainerTest, ElementOIDTest, ElementRegexTest);
using ContainerTypes = ::testing::Types<std::vector<char>, std::deque<char>, std::list<char> /*,
                                        boost::container::stable_vector<char>*/>;
INSTANTIATE_TYPED_TEST_CASE_P(ParameterizedOIDTest, ParameterizedContainerTest, ContainerTypes);

namespace std {

template <typename Container, typename RepT, typename RatioT>
void value_set(basic_element<Container>&, std::chrono::duration<RepT, RatioT>) {
    throw 0;
}

} // namespace std

TEST(ExceptionSafetyTest, ElementExceptionSafetyTest1) {
    auto e = element{};

    // name(new_name)
    // not technically thrown in name() but might as well be.
    ASSERT_THROW(e.name({"abc"s, 4}), std::out_of_range);
    ASSERT_NO_THROW(EXPECT_EQ("", e.name()));

    // type(new_type)
    ASSERT_EQ(element_type::null_element, e.type());
    ASSERT_THROW(e.type((element_type)0), invalid_element_type);
    EXPECT_EQ(element_type::null_element, e.type());

    // type(new_type)
    ASSERT_NO_THROW(e.type(element_type::int32_element));
    ASSERT_EQ(element_type::int32_element, e.type());
    ASSERT_THROW(e.type((element_type)42), invalid_element_type);
    EXPECT_EQ(element_type::int32_element, e.type());

    // value(new_type, new_value) - new_value incompatible with new_type
    ASSERT_EQ(2, e.size());
    ASSERT_THROW(e.value(element_type::double_element, "not a number"), incompatible_type_conversion);
    EXPECT_EQ(element_type::int32_element, e.type());
    EXPECT_EQ(2, e.size());

    // value(new_type, new_value) - new_type invalid
    ASSERT_EQ(2, e.size());
    ASSERT_THROW(e.value((element_type)0, "not a number"), invalid_element_type);
    EXPECT_EQ(element_type::int32_element, e.type());
    EXPECT_EQ(2, e.size());

    const auto invalid_doc = basic_document<boost::iterator_range<char*>>{};

    // value(new_type, new_value) - new_value incompatible with new_type
    ASSERT_EQ(2, e.size());
    ASSERT_THROW(e.value(element_type::double_element, invalid_doc), incompatible_type_conversion);
    EXPECT_EQ(element_type::int32_element, e.type());
    EXPECT_EQ(2, e.size());

    // value(new_type, new_value) - new_value is an invalid document
    ASSERT_EQ(2, e.size());
    ASSERT_THROW(e.value(element_type::document_element, invalid_doc), invalid_document_size);
    EXPECT_EQ(element_type::int32_element, e.type());
    EXPECT_EQ(2, e.size());

    // value(new_value) - new_value forms an invalid element
    ASSERT_THROW(e.value(invalid_doc), invalid_document_size);
    EXPECT_EQ(element_type::int32_element, e.type());
    EXPECT_EQ(2, e.size());

    // value(new_value) - new_value is a user-defined type - value_set throws
    ASSERT_THROW(e.value(123ms), int);
    EXPECT_EQ(element_type::int32_element, e.type());
    EXPECT_EQ(2, e.size());

    // value(new_type, new_value) - new_value is a user-defined type - value_set throws
    ASSERT_THROW(e.value(element_type::date_element, 123ms), int);
    EXPECT_EQ(element_type::int32_element, e.type());
    EXPECT_EQ(2, e.size());
}
