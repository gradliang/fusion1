#ifndef _SUB_COMMON_H_
#define _SUB_COMMON_H_

#if (DISPLAY_VIDEO_SUBTITLE || LYRIC_ENABLE)


#define SUB_LINE_LEN 1000

// subtitle formats
#define SUB_INVALID   -1
#define SUB_MICRODVD  0
#define SUB_SUBRIP    1
#define SUB_SUBVIEWER 2
#define SUB_SAMI      3
#define SUB_VPLAYER   4
#define SUB_RT        5
#define SUB_SSA       6
#define SUB_PJS       7
#define SUB_MPSUB     8
#define SUB_AQTITLE   9
#define SUB_SUBVIEWER2 10
#define SUB_SUBRIP09 11
#define SUB_JACOSUB  12
#define SUB_MPL2     13
#define SUB_MP3_LYRIC    14

#define MAX_SUBTITLE_FILES 128

#define SUB_MAX_TEXT 12
#define SUB_ALIGNMENT_BOTTOMLEFT       1
#define SUB_ALIGNMENT_BOTTOMCENTER     2
#define SUB_ALIGNMENT_BOTTOMRIGHT      3
#define SUB_ALIGNMENT_MIDDLELEFT       4
#define SUB_ALIGNMENT_MIDDLECENTER     5
#define SUB_ALIGNMENT_MIDDLERIGHT      6
#define SUB_ALIGNMENT_TOPLEFT          7
#define SUB_ALIGNMENT_TOPCENTER        8
#define SUB_ALIGNMENT_TOPRIGHT         9

#define LANGUAGE_UNKNOWN -1

typedef struct
{
   short lang_index;
}S_SUBTITLE_INFO_T;

typedef struct 
{
    int lines;
    int start;
    int end;
    unsigned char* text[SUB_MAX_TEXT];
	unsigned char alignment;
	BOOL displaying;   
} S_SUBTITLE_T;

typedef struct _S_SUB_NODE_T
{
    S_SUBTITLE_T s_sub;
	struct _S_SUB_NODE_T* p_prev; 
    struct _S_SUB_NODE_T* p_next; 	
}S_SUB_NODE_T;

typedef struct {
    S_SUBTITLE_T *subtitles;
    char *filename;
    int sub_uses_time;
    int sub_num;          // number of subtitle structs
    int sub_errs;
} sub_data;

extern char *fribidi_charset;
extern int flip_hebrew;
extern int fribidi_flip_commas;

// Unicode
#define SIG_MAX_LEN 5

#ifndef AV_RB16
#   define AV_RB16(x)                           \
    ((((const uint8_t*)(x))[0] << 8) |          \
      ((const uint8_t*)(x))[1])
#endif

#ifndef AV_WB16
#   define AV_WB16(p, d) do {                   \
        ((uint8_t*)(p))[1] = (d);               \
        ((uint8_t*)(p))[0] = (d)>>8;            \
    } while(0)
#endif

#ifndef AV_RL16
#   define AV_RL16(x)                           \
    ((((const uint8_t*)(x))[1] << 8) |          \
      ((const uint8_t*)(x))[0])
#endif

#ifndef AV_WL16
#   define AV_WL16(p, d) do {                   \
        ((uint8_t*)(p))[0] = (d);               \
        ((uint8_t*)(p))[1] = (d)>>8;            \
    } while(0)
#endif

#define av_log2(x) (31 - sub_clz((x)|1))

/*!
 * \def GET_UTF8(val, GET_BYTE, ERROR)
 * Converts a UTF-8 character (up to 4 bytes long) to its 32-bit UCS-4 encoded form
 * \param val is the output and should be of type uint32_t. It holds the converted
 * UCS-4 character and should be a left value.
 * \param GET_BYTE gets UTF-8 encoded bytes from any proper source. It can be
 * a function or a statement whose return value or evaluated value is of type
 * uint8_t. It will be executed up to 4 times for values in the valid UTF-8 range,
 * and up to 7 times in the general case.
 * \param ERROR action that should be taken when an invalid UTF-8 byte is returned
 * from GET_BYTE. It should be a statement that jumps out of the macro,
 * like exit(), goto, return, break, or continue.
 */
#define GET_UTF8(val, GET_BYTE, ERROR)\
    val= GET_BYTE;\
    {\
        int ones= 7 - av_log2(val ^ 255);\
        if(ones==1)\
            ERROR\
        val&= 127>>ones;\
        while(--ones > 0){\
            int tmp= GET_BYTE - 128;\
            if(tmp>>6)\
                ERROR\
            val= (val<<6) + tmp;\
        }\
    }

/*!
 * \def GET_UTF16(val, GET_16BIT, ERROR)
 * Converts a UTF-16 character (2 or 4 bytes) to its 32-bit UCS-4 encoded form
 * \param val is the output and should be of type uint32_t. It holds the converted
 * UCS-4 character and should be a left value.
 * \param GET_16BIT gets two bytes of UTF-16 encoded data converted to native endianness.
 * It can be a function or a statement whose return value or evaluated value is of type
 * uint16_t. It will be executed up to 2 times.
 * \param ERROR action that should be taken when an invalid UTF-16 surrogate is
 * returned from GET_BYTE. It should be a statement that jumps out of the macro,
 * like exit(), goto, return, break, or continue.
 */
#define GET_UTF16(val, GET_16BIT, ERROR)\
    val = GET_16BIT;\
    {\
        unsigned int hi = val - 0xD800;\
        if (hi < 0x800) {\
            val = GET_16BIT - 0xDC00;\
            if (val > 0x3FFU || hi > 0x3FFU)\
                ERROR\
            val += (hi<<10) + 0x10000;\
        }\
    }\

/*!
 * \def PUT_UTF8(val, tmp, PUT_BYTE)
 * Converts a 32-bit Unicode character to its UTF-8 encoded form (up to 4 bytes long).
 * \param val is an input-only argument and should be of type uint32_t. It holds
 * a UCS-4 encoded Unicode character that is to be converted to UTF-8. If
 * val is given as a function it is executed only once.
 * \param tmp is a temporary variable and should be of type uint8_t. It
 * represents an intermediate value during conversion that is to be
 * output by PUT_BYTE.
 * \param PUT_BYTE writes the converted UTF-8 bytes to any proper destination.
 * It could be a function or a statement, and uses tmp as the input byte.
 * For example, PUT_BYTE could be "*output++ = tmp;" PUT_BYTE will be
 * executed up to 4 times for values in the valid UTF-8 range and up to
 * 7 times in the general case, depending on the length of the converted
 * Unicode character.
 */
#define PUT_UTF8(val, tmp, PUT_BYTE)\
    {\
        int bytes, shift;\
        uint32_t in = val;\
        if (in < 0x80) {\
            tmp = in;\
            PUT_BYTE\
        } else {\
            bytes = (av_log2(in) + 4) / 5;\
            shift = (bytes - 1) * 6;\
            tmp = (256 - (256 >> bytes)) | (in >> shift);\
            PUT_BYTE\
            while (shift >= 6) {\
                shift -= 6;\
                tmp = 0x80 | ((in >> shift) & 0x3f);\
                PUT_BYTE\
            }\
        }\
    }

/*!
 * \def PUT_UTF16(val, tmp, PUT_16BIT)
 * Converts a 32-bit Unicode character to its UTF-16 encoded form (2 or 4 bytes).
 * \param val is an input-only argument and should be of type uint32_t. It holds
 * a UCS-4 encoded Unicode character that is to be converted to UTF-16. If
 * val is given as a function it is executed only once.
 * \param tmp is a temporary variable and should be of type uint16_t. It
 * represents an intermediate value during conversion that is to be
 * output by PUT_16BIT.
 * \param PUT_16BIT writes the converted UTF-16 data to any proper destination
 * in desired endianness. It could be a function or a statement, and uses tmp
 * as the input byte.  For example, PUT_BYTE could be "*output++ = tmp;"
 * PUT_BYTE will be executed 1 or 2 times depending on input character.
 */
#define PUT_UTF16(val, tmp, PUT_16BIT)\
    {\
        uint32_t in = val;\
        if (in < 0x10000) {\
            tmp = in;\
            PUT_16BIT\
        } else {\
            tmp = 0xD800 | ((in - 0x10000) >> 10);\
            PUT_16BIT\
            tmp = 0xDC00 | ((in - 0x10000) & 0x3FF);\
            PUT_16BIT\
        }\
    }\

typedef enum UErrorCode {
    /* The ordering of U_ERROR_INFO_START Vs U_USING_FALLBACK_WARNING looks weird
     * and is that way because VC++ debugger displays first encountered constant,
     * which is not the what the code is used for
     */

    U_USING_FALLBACK_WARNING  = -128,   /**< A resource bundle lookup returned a fallback result (not an error) */

    U_ERROR_WARNING_START     = -128,   /**< Start of information results (semantically successful) */

    U_USING_DEFAULT_WARNING   = -127,   /**< A resource bundle lookup returned a result from the root locale (not an error) */

    U_SAFECLONE_ALLOCATED_WARNING = -126, /**< A SafeClone operation required allocating memory (informational only) */

    U_STATE_OLD_WARNING       = -125,   /**< ICU has to use compatibility layer to construct the service. Expect performance/memory usage degradation. Consider upgrading */

    U_STRING_NOT_TERMINATED_WARNING = -124,/**< An output string could not be NUL-terminated because output length==destCapacity. */

    U_SORT_KEY_TOO_SHORT_WARNING = -123, /**< Number of levels requested in getBound is higher than the number of levels in the sort key */

    U_AMBIGUOUS_ALIAS_WARNING = -122,   /**< This converter alias can go to different converter implementations */

    U_DIFFERENT_UCA_VERSION = -121,     /**< ucol_open encountered a mismatch between UCA version and collator image version, so the collator was constructed from rules. No impact to further function */
    
    U_PLUGIN_CHANGED_LEVEL_WARNING = -120, /**< A plugin caused a level change. May not be an error, but later plugins may not load. */

    U_ERROR_WARNING_LIMIT,              /**< This must always be the last warning value to indicate the limit for UErrorCode warnings (last warning code +1) */


    U_ZERO_ERROR              =  0,     /**< No error, no warning. */

    U_ILLEGAL_ARGUMENT_ERROR  =  1,     /**< Start of codes indicating failure */
    U_MISSING_RESOURCE_ERROR  =  2,     /**< The requested resource cannot be found */
    U_INVALID_FORMAT_ERROR    =  3,     /**< Data format is not what is expected */
    U_FILE_ACCESS_ERROR       =  4,     /**< The requested file cannot be found */
    U_INTERNAL_PROGRAM_ERROR  =  5,     /**< Indicates a bug in the library code */
    U_MESSAGE_PARSE_ERROR     =  6,     /**< Unable to parse a message (message format) */
    U_MEMORY_ALLOCATION_ERROR =  7,     /**< Memory allocation error */
    U_INDEX_OUTOFBOUNDS_ERROR =  8,     /**< Trying to access the index that is out of bounds */
    U_PARSE_ERROR             =  9,     /**< Equivalent to Java ParseException */
    U_INVALID_CHAR_FOUND      = 10,     /**< Character conversion: Unmappable input sequence. In other APIs: Invalid character. */
    U_TRUNCATED_CHAR_FOUND    = 11,     /**< Character conversion: Incomplete input sequence. */
    U_ILLEGAL_CHAR_FOUND      = 12,     /**< Character conversion: Illegal input sequence/combination of input units. */
    U_INVALID_TABLE_FORMAT    = 13,     /**< Conversion table file found, but corrupted */
    U_INVALID_TABLE_FILE      = 14,     /**< Conversion table file not found */
    U_BUFFER_OVERFLOW_ERROR   = 15,     /**< A result would not fit in the supplied buffer */
    U_UNSUPPORTED_ERROR       = 16,     /**< Requested operation not supported in current context */
    U_RESOURCE_TYPE_MISMATCH  = 17,     /**< an operation is requested over a resource that does not support it */
    U_ILLEGAL_ESCAPE_SEQUENCE = 18,     /**< ISO-2022 illlegal escape sequence */
    U_UNSUPPORTED_ESCAPE_SEQUENCE = 19, /**< ISO-2022 unsupported escape sequence */
    U_NO_SPACE_AVAILABLE      = 20,     /**< No space available for in-buffer expansion for Arabic shaping */
    U_CE_NOT_FOUND_ERROR      = 21,     /**< Currently used only while setting variable top, but can be used generally */
    U_PRIMARY_TOO_LONG_ERROR  = 22,     /**< User tried to set variable top to a primary that is longer than two bytes */
    U_STATE_TOO_OLD_ERROR     = 23,     /**< ICU cannot construct a service from this state, as it is no longer supported */
    U_TOO_MANY_ALIASES_ERROR  = 24,     /**< There are too many aliases in the path to the requested resource.
                                             It is very possible that a circular alias definition has occured */
    U_ENUM_OUT_OF_SYNC_ERROR  = 25,     /**< UEnumeration out of sync with underlying collection */
    U_INVARIANT_CONVERSION_ERROR = 26,  /**< Unable to convert a UChar* string to char* with the invariant converter. */
    U_INVALID_STATE_ERROR     = 27,     /**< Requested operation can not be completed with ICU in its current state */
    U_COLLATOR_VERSION_MISMATCH = 28,   /**< Collator version is not compatible with the base version */
    U_USELESS_COLLATOR_ERROR  = 29,     /**< Collator is options only and no base is specified */
    U_NO_WRITE_PERMISSION     = 30,     /**< Attempt to modify read-only or constant data. */

    U_STANDARD_ERROR_LIMIT,             /**< This must always be the last value to indicate the limit for standard errors */
    /*
     * the error code range 0x10000 0x10100 are reserved for Transliterator
     */
    U_BAD_VARIABLE_DEFINITION=0x10000,/**< Missing '$' or duplicate variable name */
    U_PARSE_ERROR_START = 0x10000,    /**< Start of Transliterator errors */
    U_MALFORMED_RULE,                 /**< Elements of a rule are misplaced */
    U_MALFORMED_SET,                  /**< A UnicodeSet pattern is invalid*/
    U_MALFORMED_SYMBOL_REFERENCE,     /**< UNUSED as of ICU 2.4 */
    U_MALFORMED_UNICODE_ESCAPE,       /**< A Unicode escape pattern is invalid*/
    U_MALFORMED_VARIABLE_DEFINITION,  /**< A variable definition is invalid */
    U_MALFORMED_VARIABLE_REFERENCE,   /**< A variable reference is invalid */
    U_MISMATCHED_SEGMENT_DELIMITERS,  /**< UNUSED as of ICU 2.4 */
    U_MISPLACED_ANCHOR_START,         /**< A start anchor appears at an illegal position */
    U_MISPLACED_CURSOR_OFFSET,        /**< A cursor offset occurs at an illegal position */
    U_MISPLACED_QUANTIFIER,           /**< A quantifier appears after a segment close delimiter */
    U_MISSING_OPERATOR,               /**< A rule contains no operator */
    U_MISSING_SEGMENT_CLOSE,          /**< UNUSED as of ICU 2.4 */
    U_MULTIPLE_ANTE_CONTEXTS,         /**< More than one ante context */
    U_MULTIPLE_CURSORS,               /**< More than one cursor */
    U_MULTIPLE_POST_CONTEXTS,         /**< More than one post context */
    U_TRAILING_BACKSLASH,             /**< A dangling backslash */
    U_UNDEFINED_SEGMENT_REFERENCE,    /**< A segment reference does not correspond to a defined segment */
    U_UNDEFINED_VARIABLE,             /**< A variable reference does not correspond to a defined variable */
    U_UNQUOTED_SPECIAL,               /**< A special character was not quoted or escaped */
    U_UNTERMINATED_QUOTE,             /**< A closing single quote is missing */
    U_RULE_MASK_ERROR,                /**< A rule is hidden by an earlier more general rule */
    U_MISPLACED_COMPOUND_FILTER,      /**< A compound filter is in an invalid location */
    U_MULTIPLE_COMPOUND_FILTERS,      /**< More than one compound filter */
    U_INVALID_RBT_SYNTAX,             /**< A "::id" rule was passed to the RuleBasedTransliterator parser */
    U_INVALID_PROPERTY_PATTERN,       /**< UNUSED as of ICU 2.4 */
    U_MALFORMED_PRAGMA,               /**< A 'use' pragma is invlalid */
    U_UNCLOSED_SEGMENT,               /**< A closing ')' is missing */
    U_ILLEGAL_CHAR_IN_SEGMENT,        /**< UNUSED as of ICU 2.4 */
    U_VARIABLE_RANGE_EXHAUSTED,       /**< Too many stand-ins generated for the given variable range */
    U_VARIABLE_RANGE_OVERLAP,         /**< The variable range overlaps characters used in rules */
    U_ILLEGAL_CHARACTER,              /**< A special character is outside its allowed context */
    U_INTERNAL_TRANSLITERATOR_ERROR,  /**< Internal transliterator system error */
    U_INVALID_ID,                     /**< A "::id" rule specifies an unknown transliterator */
    U_INVALID_FUNCTION,               /**< A "&fn()" rule specifies an unknown transliterator */
    U_PARSE_ERROR_LIMIT,              /**< The limit for Transliterator errors */

    /*
     * the error code range 0x10100 0x10200 are reserved for formatting API parsing error
     */
    U_UNEXPECTED_TOKEN=0x10100,       /**< Syntax error in format pattern */
    U_FMT_PARSE_ERROR_START=0x10100,  /**< Start of format library errors */
    U_MULTIPLE_DECIMAL_SEPARATORS,    /**< More than one decimal separator in number pattern */
    U_MULTIPLE_DECIMAL_SEPERATORS = U_MULTIPLE_DECIMAL_SEPARATORS, /**< Typo: kept for backward compatibility. Use U_MULTIPLE_DECIMAL_SEPARATORS */
    U_MULTIPLE_EXPONENTIAL_SYMBOLS,   /**< More than one exponent symbol in number pattern */
    U_MALFORMED_EXPONENTIAL_PATTERN,  /**< Grouping symbol in exponent pattern */
    U_MULTIPLE_PERCENT_SYMBOLS,       /**< More than one percent symbol in number pattern */
    U_MULTIPLE_PERMILL_SYMBOLS,       /**< More than one permill symbol in number pattern */
    U_MULTIPLE_PAD_SPECIFIERS,        /**< More than one pad symbol in number pattern */
    U_PATTERN_SYNTAX_ERROR,           /**< Syntax error in format pattern */
    U_ILLEGAL_PAD_POSITION,           /**< Pad symbol misplaced in number pattern */
    U_UNMATCHED_BRACES,               /**< Braces do not match in message pattern */
    U_UNSUPPORTED_PROPERTY,           /**< UNUSED as of ICU 2.4 */
    U_UNSUPPORTED_ATTRIBUTE,          /**< UNUSED as of ICU 2.4 */
    U_ARGUMENT_TYPE_MISMATCH,         /**< Argument name and argument index mismatch in MessageFormat functions */
    U_DUPLICATE_KEYWORD,              /**< Duplicate keyword in PluralFormat */
    U_UNDEFINED_KEYWORD,              /**< Undefined Plural keyword */
    U_DEFAULT_KEYWORD_MISSING,        /**< Missing DEFAULT rule in plural rules */
    U_DECIMAL_NUMBER_SYNTAX_ERROR,    /**< Decimal number syntax error */
    U_FMT_PARSE_ERROR_LIMIT,          /**< The limit for format library errors */

    /*
     * the error code range 0x10200 0x102ff are reserved for Break Iterator related error
     */
    U_BRK_INTERNAL_ERROR=0x10200,          /**< An internal error (bug) was detected.             */
    U_BRK_ERROR_START=0x10200,             /**< Start of codes indicating Break Iterator failures */
    U_BRK_HEX_DIGITS_EXPECTED,             /**< Hex digits expected as part of a escaped char in a rule. */
    U_BRK_SEMICOLON_EXPECTED,              /**< Missing ';' at the end of a RBBI rule.            */
    U_BRK_RULE_SYNTAX,                     /**< Syntax error in RBBI rule.                        */
    U_BRK_UNCLOSED_SET,                    /**< UnicodeSet witing an RBBI rule missing a closing ']'.  */
    U_BRK_ASSIGN_ERROR,                    /**< Syntax error in RBBI rule assignment statement.   */
    U_BRK_VARIABLE_REDFINITION,            /**< RBBI rule $Variable redefined.                    */
    U_BRK_MISMATCHED_PAREN,                /**< Mis-matched parentheses in an RBBI rule.          */
    U_BRK_NEW_LINE_IN_QUOTED_STRING,       /**< Missing closing quote in an RBBI rule.            */
    U_BRK_UNDEFINED_VARIABLE,              /**< Use of an undefined $Variable in an RBBI rule.    */
    U_BRK_INIT_ERROR,                      /**< Initialization failure.  Probable missing ICU Data. */
    U_BRK_RULE_EMPTY_SET,                  /**< Rule contains an empty Unicode Set.               */
    U_BRK_UNRECOGNIZED_OPTION,             /**< !!option in RBBI rules not recognized.            */
    U_BRK_MALFORMED_RULE_TAG,              /**< The {nnn} tag on a rule is mal formed             */
    U_BRK_ERROR_LIMIT,                     /**< This must always be the last value to indicate the limit for Break Iterator failures */

    /*
     * The error codes in the range 0x10300-0x103ff are reserved for regular expression related errrs
     */
    U_REGEX_INTERNAL_ERROR=0x10300,       /**< An internal error (bug) was detected.              */
    U_REGEX_ERROR_START=0x10300,          /**< Start of codes indicating Regexp failures          */
    U_REGEX_RULE_SYNTAX,                  /**< Syntax error in regexp pattern.                    */
    U_REGEX_INVALID_STATE,                /**< RegexMatcher in invalid state for requested operation */
    U_REGEX_BAD_ESCAPE_SEQUENCE,          /**< Unrecognized backslash escape sequence in pattern  */
    U_REGEX_PROPERTY_SYNTAX,              /**< Incorrect Unicode property                         */
    U_REGEX_UNIMPLEMENTED,                /**< Use of regexp feature that is not yet implemented. */
    U_REGEX_MISMATCHED_PAREN,             /**< Incorrectly nested parentheses in regexp pattern.  */
    U_REGEX_NUMBER_TOO_BIG,               /**< Decimal number is too large.                       */
    U_REGEX_BAD_INTERVAL,                 /**< Error in {min,max} interval                        */
    U_REGEX_MAX_LT_MIN,                   /**< In {min,max}, max is less than min.                */
    U_REGEX_INVALID_BACK_REF,             /**< Back-reference to a non-existent capture group.    */
    U_REGEX_INVALID_FLAG,                 /**< Invalid value for match mode flags.                */
    U_REGEX_LOOK_BEHIND_LIMIT,            /**< Look-Behind pattern matches must have a bounded maximum length.    */
    U_REGEX_SET_CONTAINS_STRING,          /**< Regexps cannot have UnicodeSets containing strings.*/
    U_REGEX_OCTAL_TOO_BIG,                /**< Octal character constants must be <= 0377.         */
    U_REGEX_MISSING_CLOSE_BRACKET,        /**< Missing closing bracket on a bracket expression.   */
    U_REGEX_INVALID_RANGE,                /**< In a character range [x-y], x is greater than y.   */
    U_REGEX_STACK_OVERFLOW,               /**< Regular expression backtrack stack overflow.       */
    U_REGEX_TIME_OUT,                     /**< Maximum allowed match time exceeded                */
    U_REGEX_STOPPED_BY_CALLER,            /**< Matching operation aborted by user callback fn.    */
    U_REGEX_ERROR_LIMIT,                  /**< This must always be the last value to indicate the limit for regexp errors */

    /*
     * The error code in the range 0x10400-0x104ff are reserved for IDNA related error codes
     */
    U_IDNA_PROHIBITED_ERROR=0x10400,
    U_IDNA_ERROR_START=0x10400,
    U_IDNA_UNASSIGNED_ERROR,
    U_IDNA_CHECK_BIDI_ERROR,
    U_IDNA_STD3_ASCII_RULES_ERROR,
    U_IDNA_ACE_PREFIX_ERROR,
    U_IDNA_VERIFICATION_ERROR,
    U_IDNA_LABEL_TOO_LONG_ERROR,
    U_IDNA_ZERO_LENGTH_LABEL_ERROR,
    U_IDNA_DOMAIN_NAME_TOO_LONG_ERROR,
    U_IDNA_ERROR_LIMIT,
    /*
     * Aliases for StringPrep
     */
    U_STRINGPREP_PROHIBITED_ERROR = U_IDNA_PROHIBITED_ERROR,
    U_STRINGPREP_UNASSIGNED_ERROR = U_IDNA_UNASSIGNED_ERROR,
    U_STRINGPREP_CHECK_BIDI_ERROR = U_IDNA_CHECK_BIDI_ERROR,
    
    /*
     * The error code in the range 0x10500-0x105ff are reserved for Plugin related error codes
     */
    U_PLUGIN_ERROR_START=0x10500,         /**< Start of codes indicating plugin failures */
    U_PLUGIN_TOO_HIGH=0x10500,            /**< The plugin's level is too high to be loaded right now. */
    U_PLUGIN_DIDNT_SET_LEVEL,             /**< The plugin didn't call uplug_setPlugLevel in response to a QUERY */
    U_PLUGIN_ERROR_LIMIT,                 /**< This must always be the last value to indicate the limit for plugin errors */

    U_ERROR_LIMIT=U_PLUGIN_ERROR_LIMIT      /**< This must always be the last value to indicate the limit for UErrorCode (last error code +1) */
} UErrorCode;

typedef enum
{
    CHAR_ENCODING_UTF_8 = 0,	
	CHAR_ENCODING_UTF_16_LE,
	CHAR_ENCODING_UTF_16_BE,
	CHAR_ENCODING_UTF_32_BE,
	CHAR_ENCODING_UTF_32_LE,
	CHAR_ENCODING_UTF_7,
	CHAR_ENCODING_UTF_1,
	CHAR_ENCODING_UTF_EBCDIC,
	CHAR_ENCODING_SCSU,
	CHAR_ENCODING_BOCU_1,
	CHAR_ENCODING_GB_18030,
	CHAR_ENCODING_BIG5,
	CHAR_ENCODING_GB_2312,
	CHAR_ENCODING_GBK
}E_CHAR_ENCODING;
#endif 


#endif
