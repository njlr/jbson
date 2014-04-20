//          Copyright Christian Manning 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef JBSON_ERROR_HPP
#define JBSON_ERROR_HPP

#include <exception>
#include <typeindex>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdocumentation"
#include <boost/exception/all.hpp>
#pragma GCC diagnostic pop

namespace jbson {

/*!
 * \brief Exception type. Base class of all exceptions thrown directly by jbson.
 */
struct jbson_error : virtual std::exception, virtual boost::exception {
    const char* what() const noexcept override { return "jbson_error"; }
};

/*!
 * \brief Exception type thrown when an element has a type value not represented by element_type.
 */
struct invalid_element_type : jbson_error {
    const char* what() const noexcept override { return "invalid_element_type"; }
};

/*!
 * \brief Exception type thrown when a call to get<element_type>() has an incorrect type parameter.
 */
struct incompatible_element_conversion : jbson_error {
    const char* what() const noexcept override { return "incompatible_element_conversion"; }
};

/*!
 * \brief Exception thrown when an element has a value not convertible to that requested.
 */
struct incompatible_type_conversion : jbson_error {
    const char* what() const noexcept override { return "incompatible_type_conversion"; }
};

/*!
 * \brief Exception thrown when an element's data size differs from that reported.
 */
struct invalid_element_size : jbson_error {
    const char* what() const noexcept override { return "invalid_element_size"; }
};

namespace detail {

using expected_type = boost::error_info<struct expected_type_, std::type_index>;
using actual_type = boost::error_info<struct actual_type_, std::type_index>;
using expected_size = boost::error_info<struct expected_size_, ptrdiff_t>;
using actual_size = boost::error_info<struct actual_size_, ptrdiff_t>;

} // namespace detail
} // namespace jbson

#endif // JBSON_ERROR_HPP
