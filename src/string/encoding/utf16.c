/*
Copyright (C) 2001-2010, Parrot Foundation.

=head1 NAME

src/string/encoding/utf16.c - UTF-16 encoding

=head1 DESCRIPTION

UTF-16 encoding with the help of the ICU library.

=head2 Functions

=over 4

=cut

*/

#include "parrot/parrot.h"
#include "shared.h"

/* HEADERIZER HFILE: none */

/* HEADERIZER BEGIN: static */
/* Don't modify between HEADERIZER BEGIN / HEADERIZER END.  Your changes will be lost. */

static UINTVAL utf16_iter_get(PARROT_INTERP,
    ARGIN(const STRING *str),
    ARGIN(const String_iter *i),
    INTVAL offset)
        __attribute__nonnull__(1)
        __attribute__nonnull__(2)
        __attribute__nonnull__(3);

PARROT_WARN_UNUSED_RESULT
static UINTVAL utf16_iter_get_and_advance(PARROT_INTERP,
    ARGIN(const STRING *str),
    ARGMOD(String_iter *i))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2)
        __attribute__nonnull__(3)
        FUNC_MODIFIES(*i);

static void utf16_iter_set_and_advance(PARROT_INTERP,
    ARGMOD(STRING *str),
    ARGMOD(String_iter *i),
    UINTVAL c)
        __attribute__nonnull__(1)
        __attribute__nonnull__(2)
        __attribute__nonnull__(3)
        FUNC_MODIFIES(*str)
        FUNC_MODIFIES(*i);

static void utf16_iter_set_position(PARROT_INTERP,
    ARGIN(const STRING *str),
    ARGMOD(String_iter *i),
    UINTVAL n)
        __attribute__nonnull__(1)
        __attribute__nonnull__(2)
        __attribute__nonnull__(3)
        FUNC_MODIFIES(*i);

static void utf16_iter_skip(PARROT_INTERP,
    ARGIN(const STRING *str),
    ARGMOD(String_iter *i),
    INTVAL skip)
        __attribute__nonnull__(1)
        __attribute__nonnull__(2)
        __attribute__nonnull__(3)
        FUNC_MODIFIES(*i);

static UINTVAL utf16_ord(PARROT_INTERP,
    ARGIN(const STRING *src),
    INTVAL idx)
        __attribute__nonnull__(1)
        __attribute__nonnull__(2);

PARROT_WARN_UNUSED_RESULT
static UINTVAL utf16_scan(PARROT_INTERP, ARGIN(const STRING *src))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2);

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static STRING * utf16_substr(PARROT_INTERP,
    ARGIN(const STRING *src),
    INTVAL offset,
    INTVAL length)
        __attribute__nonnull__(1)
        __attribute__nonnull__(2);

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static STRING * utf16_to_encoding(PARROT_INTERP, ARGIN(const STRING *src))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2);

#define ASSERT_ARGS_utf16_iter_get __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(str) \
    , PARROT_ASSERT_ARG(i))
#define ASSERT_ARGS_utf16_iter_get_and_advance __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(str) \
    , PARROT_ASSERT_ARG(i))
#define ASSERT_ARGS_utf16_iter_set_and_advance __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(str) \
    , PARROT_ASSERT_ARG(i))
#define ASSERT_ARGS_utf16_iter_set_position __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(str) \
    , PARROT_ASSERT_ARG(i))
#define ASSERT_ARGS_utf16_iter_skip __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(str) \
    , PARROT_ASSERT_ARG(i))
#define ASSERT_ARGS_utf16_ord __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(src))
#define ASSERT_ARGS_utf16_scan __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(src))
#define ASSERT_ARGS_utf16_substr __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(src))
#define ASSERT_ARGS_utf16_to_encoding __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(src))
/* Don't modify between HEADERIZER BEGIN / HEADERIZER END.  Your changes will be lost. */
/* HEADERIZER END: static */

#if PARROT_HAS_ICU
#  include <unicode/utf16.h>
#  include <unicode/ustring.h>
#endif

#define UNIMPL Parrot_ex_throw_from_c_args(interp, NULL, EXCEPTION_UNIMPLEMENTED, \
    "unimpl utf16")


/*

=item C<static STRING * utf16_to_encoding(PARROT_INTERP, const STRING *src)>

Converts the string C<src> to this particular encoding.  If C<dest> is
provided, it will contain the result.  Otherwise this function operates in
place.


=cut

*/

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static STRING *
utf16_to_encoding(PARROT_INTERP, ARGIN(const STRING *src))
{
    ASSERT_ARGS(utf16_to_encoding)
    int           src_len, dest_len;
    Parrot_UInt2 *p;
    STRING       *result;

    if (src->encoding == Parrot_utf16_encoding_ptr
    ||  src->encoding == Parrot_ucs2_encoding_ptr)
        /* we have to use clone instead of copy because the Unicode upcase
         * and downcase functions assume to get an unshared buffer */
        return Parrot_str_clone(interp, src);

    result  = Parrot_gc_new_string_header(interp, 0);
    src_len = STRING_length(src);

    if (!src_len) {
        result->encoding = Parrot_ucs2_encoding_ptr;
        return result;
    }

    Parrot_gc_allocate_string_storage(interp, result, 2 * src_len);
    p = (Parrot_UInt2 *)result->strstart;

    if (STRING_max_bytes_per_codepoint(src) == 1) {
        for (dest_len = 0; dest_len < src_len; ++dest_len) {
            p[dest_len] = (unsigned char)src->strstart[dest_len];
        }
    }
    else if (src->encoding == Parrot_utf8_encoding_ptr) {
#if PARROT_HAS_ICU
        UErrorCode err = U_ZERO_ERROR;

        u_strFromUTF8(p, src_len, &dest_len, src->strstart, src->bufused, &err);

        if (!U_SUCCESS(err)) {
            /*
             * have to resize - required len in UChars is in dest_len
             */
            result->bufused = 2 * dest_len;
            Parrot_gc_reallocate_string_storage(interp, result, 2 * dest_len);

            p = (Parrot_UInt2 *)result->strstart;
            u_strFromUTF8(p, dest_len, &dest_len, src->strstart, src->bufused, &err);
            PARROT_ASSERT(U_SUCCESS(err));
        }
#else
        Parrot_ex_throw_from_c_args(interp, NULL, EXCEPTION_LIBRARY_ERROR,
            "no ICU lib loaded");
#endif
    }
    else {
        UNIMPL;
    }

    result->bufused  = 2 * dest_len;
    result->strlen   = src_len;

    /* downgrade if possible */
    if (dest_len == src_len)
        result->encoding = Parrot_ucs2_encoding_ptr;
    else
        result->encoding = Parrot_utf16_encoding_ptr;

    return result;
}

/*

=item C<static UINTVAL utf16_scan(PARROT_INTERP, const STRING *src)>

Returns the number of codepoints in string C<src> by scanning the whole
string.

=cut

*/

PARROT_WARN_UNUSED_RESULT
static UINTVAL
utf16_scan(PARROT_INTERP, ARGIN(const STRING *src))
{
    ASSERT_ARGS(utf16_scan)
#if PARROT_HAS_ICU
    const UChar * const s = (UChar*) src->strstart;
    UINTVAL pos = 0, charpos = 0;
    /*
     * this is used to initially calculate src->strlen,
     * therefore we must scan the whole string
     */
    while (pos * sizeof (UChar) < src->bufused) {
        U16_FWD_1_UNSAFE(s, pos);
        ++charpos;
    }
    return charpos;
#else
    UNUSED(src);

    Parrot_ex_throw_from_c_args(interp, NULL, EXCEPTION_LIBRARY_ERROR,
        "no ICU lib loaded");
#endif
}

/*

=item C<static UINTVAL utf16_ord(PARROT_INTERP, const STRING *src, INTVAL idx)>

Returns the codepoint in string C<src> at position C<offset>.

=cut

*/

static UINTVAL
utf16_ord(PARROT_INTERP, ARGIN(const STRING *src), INTVAL idx)
{
    ASSERT_ARGS(utf16_ord)
#if PARROT_HAS_ICU
    const UINTVAL len = STRING_length(src);
    const UChar  *s;
    UINTVAL       c, pos;

    if (idx < 0)
        idx += len;

    if ((UINTVAL)idx >= len)
        encoding_ord_error(interp, src, idx);

    s   = (UChar *)src->strstart;
    pos = 0;
    U16_FWD_N_UNSAFE(s, pos, idx);
    U16_GET_UNSAFE(s, pos, c);

    return c;
#else
    UNUSED(src);
    UNUSED(idx);

    Parrot_ex_throw_from_c_args(interp, NULL, EXCEPTION_LIBRARY_ERROR,
        "no ICU lib loaded");
#endif
}

/*

=item C<static STRING * utf16_substr(PARROT_INTERP, const STRING *src, INTVAL
offset, INTVAL length)>

Returns the codepoints in string C<src> at position C<offset> and length
C<count>.

=cut

*/

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static STRING *
utf16_substr(PARROT_INTERP, ARGIN(const STRING *src), INTVAL offset, INTVAL length)
{
    ASSERT_ARGS(utf16_substr)
#if PARROT_HAS_ICU
    const UChar * const s = (UChar*) src->strstart;
    const UINTVAL  strlen = STRING_length(src);
    STRING        *return_string;
    UINTVAL        pos = 0, start;

    if (offset < 0)
        offset += strlen;

    if ((UINTVAL)offset >= strlen || length <= 0) {
        /* Allow regexes to return $' easily for "aaa" =~ /aaa/ */
        if ((UINTVAL)offset == strlen || length <= 0)
            return Parrot_str_new_noinit(interp, 0);

        Parrot_ex_throw_from_c_args(interp, NULL,
            EXCEPTION_SUBSTR_OUT_OF_STRING,
            "Cannot take substr outside string");
    }

    return_string = Parrot_str_copy(interp, src);

    if (offset == 0 && (UINTVAL)length >= strlen)
        return return_string;

    U16_FWD_N_UNSAFE(s, pos, offset);

    start = pos * sizeof (UChar);
    return_string->strstart += start;

    if ((UINTVAL)length >= strlen - (UINTVAL)offset) {
        return_string->bufused -= start;
        return_string->strlen  -= offset;
    }
    else {
        U16_FWD_N_UNSAFE(s, pos, length);
        return_string->bufused = pos * sizeof (UChar) - start;
        return_string->strlen  = length;
    }

    return_string->hashval = 0;

    return return_string;
#else
    UNUSED(src);
    UNUSED(offset);
    UNUSED(length);

    Parrot_ex_throw_from_c_args(interp, NULL, EXCEPTION_LIBRARY_ERROR,
        "no ICU lib loaded");
#endif
}

/*

=item C<static UINTVAL utf16_iter_get(PARROT_INTERP, const STRING *str, const
String_iter *i, INTVAL offset)>

Get the character at C<i> plus C<offset>.

=cut

*/

static UINTVAL
utf16_iter_get(PARROT_INTERP,
    ARGIN(const STRING *str), ARGIN(const String_iter *i), INTVAL offset)
{
    ASSERT_ARGS(utf16_iter_get)
#if PARROT_HAS_ICU
    const UChar * const s = (UChar*) str->strstart;
    UINTVAL c, pos;

    pos = i->bytepos / sizeof (UChar);
    if (offset > 0) {
        U16_FWD_N_UNSAFE(s, pos, offset);
    }
    else if (offset < 0) {
        U16_BACK_N_UNSAFE(s, pos, -offset);
    }
    U16_GET_UNSAFE(s, pos, c);

    return c;
#else
    UNUSED(str);
    UNUSED(i);
    UNUSED(offset);

    Parrot_ex_throw_from_c_args(interp, NULL, EXCEPTION_LIBRARY_ERROR,
        "no ICU lib loaded");
#endif
}

/*

=item C<static void utf16_iter_skip(PARROT_INTERP, const STRING *str,
String_iter *i, INTVAL skip)>

Moves the string iterator C<i> by C<skip> characters.

=cut

*/

static void
utf16_iter_skip(PARROT_INTERP,
    ARGIN(const STRING *str), ARGMOD(String_iter *i), INTVAL skip)
{
    ASSERT_ARGS(utf16_iter_skip)
#if PARROT_HAS_ICU
    const UChar * const s = (UChar*) str->strstart;
    UINTVAL pos = i->bytepos / sizeof (UChar);

    if (skip > 0) {
        U16_FWD_N_UNSAFE(s, pos, skip);
    }
    else if (skip < 0) {
        U16_BACK_N_UNSAFE(s, pos, -skip);
    }

    i->charpos += skip;
    i->bytepos = pos * sizeof (UChar);
#else
    UNUSED(str);
    UNUSED(i);
    UNUSED(skip);

    Parrot_ex_throw_from_c_args(interp, NULL, EXCEPTION_LIBRARY_ERROR,
        "no ICU lib loaded");
#endif
}

/*

=item C<static UINTVAL utf16_iter_get_and_advance(PARROT_INTERP, const STRING
*str, String_iter *i)>

Moves the string iterator C<i> to the next UTF-16 codepoint.

=cut

*/

PARROT_WARN_UNUSED_RESULT
static UINTVAL
utf16_iter_get_and_advance(PARROT_INTERP,
    ARGIN(const STRING *str), ARGMOD(String_iter *i))
{
    ASSERT_ARGS(utf16_iter_get_and_advance)
#if PARROT_HAS_ICU
    const UChar * const s = (UChar*) str->strstart;
    UINTVAL c, pos;
    pos = i->bytepos / sizeof (UChar);
    /* TODO either make sure that we don't go past end or use SAFE
     *      iter versions
     */
    U16_NEXT_UNSAFE(s, pos, c);
    i->charpos++;
    i->bytepos = pos * sizeof (UChar);
    return c;
#else
    UNUSED(str);
    UNUSED(i);

    Parrot_ex_throw_from_c_args(interp, NULL, EXCEPTION_LIBRARY_ERROR,
        "no ICU lib loaded");
#endif
}

/*

=item C<static void utf16_iter_set_and_advance(PARROT_INTERP, STRING *str,
String_iter *i, UINTVAL c)>

With the string iterator C<i>, appends the codepoint C<c> and advances to the
next position in the string.

=cut

*/

static void
utf16_iter_set_and_advance(PARROT_INTERP,
    ARGMOD(STRING *str), ARGMOD(String_iter *i), UINTVAL c)
{
    ASSERT_ARGS(utf16_iter_set_and_advance)
#if PARROT_HAS_ICU
    UChar * const s = (UChar*) str->strstart;
    UINTVAL pos;
    pos = i->bytepos / sizeof (UChar);
    U16_APPEND_UNSAFE(s, pos, c);
    i->charpos++;
    i->bytepos = pos * sizeof (UChar);
#else
    UNUSED(str);
    UNUSED(i);
    UNUSED(c);

    Parrot_ex_throw_from_c_args(interp, NULL, EXCEPTION_LIBRARY_ERROR,
        "no ICU lib loaded");
#endif
}

/*

=item C<static void utf16_iter_set_position(PARROT_INTERP, const STRING *str,
String_iter *i, UINTVAL n)>

Moves the string iterator C<i> to the position C<n> in the string.

=cut

*/

static void
utf16_iter_set_position(PARROT_INTERP,
    ARGIN(const STRING *str), ARGMOD(String_iter *i), UINTVAL n)
{
    ASSERT_ARGS(utf16_iter_set_position)
#if PARROT_HAS_ICU
    UChar * const s = (UChar*) str->strstart;
    UINTVAL pos;
    pos = 0;
    U16_FWD_N_UNSAFE(s, pos, n);
    i->charpos = n;
    i->bytepos = pos * sizeof (UChar);
#else
    UNUSED(str);
    UNUSED(i);
    UNUSED(n);

    Parrot_ex_throw_from_c_args(interp, NULL, EXCEPTION_LIBRARY_ERROR,
        "no ICU lib loaded");
#endif
}


static STR_VTABLE Parrot_utf16_encoding = {
    0,
    "utf16",
    NULL,
    4, /* Max bytes per codepoint */

    utf16_to_encoding,
    unicode_chr,

    encoding_equal,
    encoding_compare,
    encoding_index,
    encoding_rindex,
    encoding_hash,
    unicode_validate,

    utf16_scan,
    utf16_ord,
    utf16_substr,

    encoding_is_cclass,
    encoding_find_cclass,
    encoding_find_not_cclass,

    encoding_get_graphemes,
    unicode_compose,
    encoding_decompose,

    unicode_upcase,
    unicode_downcase,
    unicode_titlecase,
    unicode_upcase_first,
    unicode_downcase_first,
    unicode_titlecase_first,

    utf16_iter_get,
    utf16_iter_skip,
    utf16_iter_get_and_advance,
    utf16_iter_set_and_advance,
    utf16_iter_set_position
};

STR_VTABLE *Parrot_utf16_encoding_ptr = &Parrot_utf16_encoding;


/*

=back

=head1 SEE ALSO

F<src/string/encoding/fixed_8.c>,
F<src/string/encoding/utf8.c>,
F<src/string.c>,
F<include/parrot/string.h>,
F<docs/string.pod>.

=cut

*/


/*
 * Local variables:
 *   c-file-style: "parrot"
 * End:
 * vim: expandtab shiftwidth=4:
 */
