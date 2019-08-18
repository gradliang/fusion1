
#ifndef _GLIB_H_
#define _GLIB_H_

#include "mpTrace_copy.h"
#include "ndebug_copy.h"

#define G_BYTE_ORDER 4321
#define G_MAXSIZE   0x100000

#define G_LITTLE_ENDIAN 1234
#define G_BIG_ENDIAN    4321
#define G_PDP_ENDIAN    3412		/* unused, need specific PDP check */	

#ifndef	FALSE
#define	FALSE	(0)
#endif

#ifndef	TRUE
#define	TRUE	(!FALSE)
#endif

/* Define min and max constants for the fixed size numerical types */
#define G_MININT8	((gint8)  0x80)
#define G_MAXINT8	((gint8)  0x7f)
#define G_MAXUINT8	((guint8) 0xff)

#define G_MININT16	((gint16)  0x8000)
#define G_MAXINT16	((gint16)  0x7fff)
#define G_MAXUINT16	((guint16) 0xffff)

#define G_MININT32	((gint32)  0x80000000)
#define G_MAXINT32	((gint32)  0x7fffffff)
#define G_MAXUINT32	((guint32) 0xffffffff)

#define G_MAXULONG	((guint32) 0xffffffff)

#define G_GINT64_CONSTANT	
#define G_MININT64	((gint64) G_GINT64_CONSTANT(0x8000000000000000))
#define G_MAXINT64	G_GINT64_CONSTANT(0x7fffffffffffffff)
#define G_MAXUINT64	G_GINT64_CONSTANT(0xffffffffffffffffU)
typedef int gint;
typedef long   glong;
typedef char gchar;
typedef unsigned char guchar;
typedef void* gpointer;
typedef gint gboolean;
typedef unsigned int guint;
typedef unsigned char guint8;
typedef short gint16;
typedef unsigned short guint16;
typedef unsigned long gulong;
typedef unsigned int gsize;
typedef const void *gconstpointer;
typedef signed long long gint64;
typedef unsigned long long guint64;
typedef double  gdouble;
typedef unsigned long guint32;
typedef guint32 GQuark;
typedef gint gssize;
typedef long gint32;

typedef guint32 gunichar;
typedef guint16 gunichar2;

typedef struct _GList GList;
struct _GList
{
  gpointer data;
  GList *next;
  GList *prev;
};

typedef struct _GArray		GArray;
typedef struct _GByteArray	GByteArray;
typedef struct _GPtrArray	GPtrArray;

struct _GArray
{
  gchar *data;
  guint len;
};

struct _GByteArray
{
  guint8 *data;
  guint	  len;
};

struct _GPtrArray
{
  gpointer *pdata;
  guint	    len;
};

typedef struct _GSList GSList;

struct _GSList {
    gpointer data;
    GSList *next;
};

//typedef struct _GHashNode      GHashNode;
typedef struct _GHashTable GHashTable;

typedef enum
{
  G_OPTION_ARG_NONE,
  G_OPTION_ARG_STRING,
  G_OPTION_ARG_INT,
  G_OPTION_ARG_CALLBACK,
  G_OPTION_ARG_FILENAME,
  G_OPTION_ARG_STRING_ARRAY,
  G_OPTION_ARG_FILENAME_ARRAY,
  G_OPTION_ARG_DOUBLE,
  G_OPTION_ARG_INT64
} GOptionArg;
guint g_direct_hash(gconstpointer v);
gboolean g_direct_equal(gconstpointer v1, gconstpointer v2);

extern void g_free(gpointer mem);

typedef struct _GString GString;

struct _GString {
    gchar *str;
    gsize len;
    gsize allocated_len;
};

typedef struct _GQueue		GQueue;

struct _GQueue
{
  GList *head;
  GList *tail;
  guint  length;
};

#define G_QUEUE_INIT { NULL, NULL, 0 }

typedef struct _GMutex          GMutex;
typedef struct _GCond           GCond;

typedef gint            (*GCompareFunc)         (gconstpointer  a,
                                                 gconstpointer  b);
typedef gint            (*GCompareDataFunc)     (gconstpointer  a,
                                                 gconstpointer  b,
						 gpointer       user_data);
typedef gboolean        (*GEqualFunc)           (gconstpointer  a,
                                                 gconstpointer  b);
typedef void            (*GDestroyNotify)       (gpointer       data);
typedef void            (*GFunc)                (gpointer       data,
                                                 gpointer       user_data);
typedef guint           (*GHashFunc)            (gconstpointer  key);
typedef void            (*GHFunc)               (gpointer       key,
                                                 gpointer       value,
                                                 gpointer       user_data);

typedef struct _GThreadPool     GThreadPool;
typedef struct _GRand           GRand;

/* Thread Pools
 */


typedef struct _GError GError;

struct _GError
{
  GQuark       domain;
  gint         code;
  gchar       *message;
};
typedef void (*MThreadAlloc) (gpointer data, int max); 

struct _GThreadPool
{
  GFunc func;
  gpointer user_data;
  gboolean exclusive;
};

typedef struct _GOptionEntry   GOptionEntry;
struct _GOptionEntry
{
  const gchar *long_name;
  gchar        short_name;
  gint         flags;

  GOptionArg   arg;
  gpointer     arg_data;
  
  const gchar *description;
  const gchar *arg_description;
};
#define G_BEGIN_DECLS
#define G_END_DECLS

#define G_GNUC_PRINTF(a,b) __attribute__ ((format (printf, (a), (b))))
#define ATTR_PACKED __attribute__ ((packed))

/* assertion API */
#define g_assert_cmpint(n1, cmp, n2)    do { gint64 __n1 = (n1), __n2 = (n2); \
                                             if (__n1 cmp __n2) ; else \
                                               g_assertion_message_cmpnum (G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, \
                                                 #n1 " " #cmp " " #n2, __n1, #cmp, __n2, 'i'); } while (0)
#ifndef G_LOG_DOMAIN
#define G_LOG_DOMAIN    ((gchar*) 0)
#endif  /* G_LOG_DOMAIN */
#  define G_STRFUNC     ((const char*) (__PRETTY_FUNCTION__))

#define G_GNUC_MALLOC
#define G_GNUC_ALLOC_SIZE(x)
#  define G_STRUCT_OFFSET(struct_type, member)	\
      ((glong) ((guint8*) &((struct_type*) 0)->member))

/* slices - fast allocation/release of small memory blocks
 */
gpointer g_slice_alloc          	(gsize	       block_size) G_GNUC_MALLOC G_GNUC_ALLOC_SIZE(1);
gpointer g_slice_alloc0         	(gsize         block_size) G_GNUC_MALLOC G_GNUC_ALLOC_SIZE(1);
gpointer g_slice_copy                   (gsize         block_size,
                                         gconstpointer mem_block) G_GNUC_MALLOC G_GNUC_ALLOC_SIZE(1);
void     g_slice_free1          	(gsize         block_size,
					 gpointer      mem_block);
void     g_slice_free_chain_with_offset (gsize         block_size,
					 gpointer      mem_chain,
					 gsize         next_offset);
#define  g_slice_new(type)      ((type*) g_slice_alloc (sizeof (type)))
#define  g_slice_new0(type)     ((type*) g_slice_alloc0 (sizeof (type)))
#define g_slice_dup(type, mem)                                  \
  (1 ? (type*) g_slice_copy (sizeof (type), (mem))              \
     : ((void) ((type*) 0 == (mem)), (type*) 0))
#define g_slice_free(type, mem)				do {	\
  if (1) g_slice_free1 (sizeof (type), (mem));			\
  else   (void) ((type*) 0 == (mem)); 				\
} while (0)

#define g_slice_free_chain(type, mem_chain, next)	do {	\
  if (1) g_slice_free_chain_with_offset (sizeof (type),		\
                 (mem_chain), G_STRUCT_OFFSET (type, next)); 	\
  else   (void) ((type*) 0 == (mem_chain));			\
} while (0)


#ifdef LINUX
#  define _G_NEW(struct_type, n_structs, func) \
	(struct_type *) (__extension__ ({			\
	  gsize __n = (gsize) (n_structs);			\
	  gsize __s = sizeof (struct_type);			\
	  gpointer __p;						\
	  if (__s == 1)						\
	    __p = g_##func (__n);				\
	  else if (__builtin_constant_p (__n) &&		\
	           (__s == 0 || __n <= G_MAXSIZE / __s))	\
	    __p = g_##func (__n * __s);				\
	  else							\
	    __p = g_##func##_n (__n, __s);			\
	  __p;							\
	}))
#else
#define malloc0(s) mm_zalloc(s)
#define malloc(s) mm_malloc(s)
#  define _G_NEW(struct_type, n_structs, func) \
	(struct_type *) (({			\
	  gsize __n = (gsize) (n_structs);			\
	  gsize __s = sizeof (struct_type);			\
	  gpointer __p;						\
	    __p = func (__n * __s);			\
	  __p;							\
	}))
#endif
#define g_new0(struct_type, n_structs)			_G_NEW (struct_type, n_structs, malloc0)
#define g_new(struct_type, n_structs)			_G_NEW (struct_type, n_structs, malloc)

#define G_LIKELY(expr) (expr)
#define G_UNLIKELY(expr) (expr)

#  define G_STMT_START  do
#  define G_STMT_END    while (0)

#define g_return_if_fail(expr)			G_STMT_START{ (void)0; }G_STMT_END
#define g_return_val_if_fail(expr,val)		G_STMT_START{ (void)0; }G_STMT_END
#define g_return_if_reached()			G_STMT_START{ return; }G_STMT_END
#define g_return_val_if_reached(val)		G_STMT_START{ return (val); }G_STMT_END

gboolean g_str_equal (gconstpointer v1, gconstpointer v2);
guint g_str_hash (gconstpointer v);

#define GPOINTER_TO_INT(p)	((gint)   (p))
#define GPOINTER_TO_UINT(p)	((guint)  (p))

#define GINT_TO_POINTER(i)	((gpointer)  (glong)(i))
#define GUINT_TO_POINTER(u)	((gpointer)  (gulong)(u))

#define g_thread_supported()    1

#define G_THREAD_UF(op, arglist)					\
    (*g_thread_functions_for_glib_use . op) arglist
#define G_THREAD_CF(op, fail, arg)					\
    (g_thread_supported () ? G_THREAD_UF (op, arg) : (fail))
#define G_THREAD_ECF(op, fail, mutex, type)				\
    (g_thread_supported () ? 						\
      ((type(*)(GMutex*, const gulong, gchar const*))			\
      (*g_thread_functions_for_glib_use . op))				\
     (mutex, G_MUTEX_DEBUG_MAGIC, G_STRLOC) : (fail))

/* A random number to recognize debug calls to g_mutex_... */
#define G_MUTEX_DEBUG_MAGIC 0xf8e18ad7

#define g_mutex_new()            G_THREAD_UF (mutex_new,      ())
#define g_cond_new()             G_THREAD_UF (cond_new,       ())
#define g_cond_signal(cond)      G_THREAD_CF (cond_signal,    (void)0, (cond))
#define g_cond_broadcast(cond)   G_THREAD_CF (cond_broadcast, (void)0, (cond))
#define g_cond_free(cond)        G_THREAD_CF (cond_free,      (void)0, (cond))
#define g_private_new(destructor) G_THREAD_UF (private_new, (destructor))
#define g_private_get(private_key) G_THREAD_CF (private_get, \
                                                ((gpointer)private_key), \
                                                (private_key))

# define g_mutex_lock(mutex)						\
    G_THREAD_ECF (mutex_lock,    (void)0, (mutex), void)
# define g_mutex_trylock(mutex)						\
    G_THREAD_ECF (mutex_trylock, TRUE,    (mutex), gboolean)
# define g_mutex_unlock(mutex)						\
    G_THREAD_ECF (mutex_unlock,  (void)0, (mutex), void)
# define g_mutex_free(mutex)						\
    G_THREAD_CF (mutex_free,     (void)0, (mutex))
# define g_cond_wait(cond, mutex)					\
    G_THREAD_CF (cond_wait,      (void)0, (cond, mutex))
# define g_cond_timed_wait(cond, mutex, abs_time)			\
    G_THREAD_CF (cond_timed_wait, TRUE,   (cond, mutex, abs_time))


#define g_assert_not_reached()          do { (void) 0; } while (0)
#define g_assert(expr)                  do { (void) 0; } while (0)

#define GLIB_VAR extern
#define G_STRINGIFY(macro_or_string)	G_STRINGIFY_ARG (macro_or_string)
#define	G_STRINGIFY_ARG(contents)	#contents

#define g_array_elt_len(array,i) ((array)->elt_size * (i))
#define g_array_elt_pos(array,i) ((array)->data + g_array_elt_len((array),(i)))
#define g_array_elt_zero(array, pos, len) 				\
  (memset (g_array_elt_pos ((array), pos), 0,  g_array_elt_len ((array), len)))
#define g_array_zero_terminate(array) G_STMT_START{			\
  if ((array)->zero_terminated)						\
    g_array_elt_zero ((array), (array)->len, 1);			\
}G_STMT_END

#  define G_STRLOC	__FILE__ ":" G_STRINGIFY (__LINE__)

typedef enum
{
  G_THREAD_ERROR_AGAIN /* Resource temporarily unavailable */
} GThreadError;

typedef gpointer (*GThreadFunc) (gpointer data);
typedef struct _GPrivate        GPrivate;
typedef struct _GTimeVal                GTimeVal;
typedef enum
{
  G_THREAD_PRIORITY_LOW,
  G_THREAD_PRIORITY_NORMAL,
  G_THREAD_PRIORITY_HIGH,
  G_THREAD_PRIORITY_URGENT
} GThreadPriority;

struct _GTimeVal
{
  glong tv_sec;
  glong tv_usec;
};
typedef struct _GThreadFunctions GThreadFunctions;
struct _GThreadFunctions
{
  GMutex*  (*mutex_new)           (void);
  void     (*mutex_lock)          (GMutex               *mutex);
  gboolean (*mutex_trylock)       (GMutex               *mutex);
  void     (*mutex_unlock)        (GMutex               *mutex);
  void     (*mutex_free)          (GMutex               *mutex);
  GCond*   (*cond_new)            (void);
  void     (*cond_signal)         (GCond                *cond);
  void     (*cond_broadcast)      (GCond                *cond);
  void     (*cond_wait)           (GCond                *cond,
                                   GMutex               *mutex);
  gboolean (*cond_timed_wait)     (GCond                *cond,
                                   GMutex               *mutex,
                                   GTimeVal             *end_time);
  void      (*cond_free)          (GCond                *cond);
  GPrivate* (*private_new)        (GDestroyNotify        destructor);
  gpointer  (*private_get)        (GPrivate             *private_key);
  void      (*private_set)        (GPrivate             *private_key,
                                   gpointer              data);
  void      (*thread_create)      (GThreadFunc           func,
                                   gpointer              data,
                                   gulong                stack_size,
                                   gboolean              joinable,
                                   gboolean              bound,
                                   GThreadPriority       priority,
                                   gpointer              thread,
                                   GError              **error);
  void      (*thread_yield)       (void);
  void      (*thread_join)        (gpointer              thread);
  void      (*thread_exit)        (void);
  void      (*thread_set_priority)(gpointer              thread,
                                   GThreadPriority       priority);
  void      (*thread_self)        (gpointer              thread);
  gboolean  (*thread_equal)       (gpointer              thread1,
				   gpointer              thread2);
};
GLIB_VAR GThreadFunctions       g_thread_functions_for_glib_use;

extern gboolean g_mem_gc_friendly;

#ifndef NULL
#define NULL    ((void*)0)
#endif

#include <stdarg.h>
/* Glib log levels and flags.
 */
typedef enum
{
  /* log flags */
  G_LOG_FLAG_RECURSION          = 1 << 0,
  G_LOG_FLAG_FATAL              = 1 << 1,

  /* GLib log levels */
  G_LOG_LEVEL_ERROR             = 1 << 2,       /* always fatal */
  G_LOG_LEVEL_WARNING           = 1 << 4,
  G_LOG_LEVEL_MESSAGE           = 1 << 5,
  G_LOG_LEVEL_INFO              = 1 << 6,
  G_LOG_LEVEL_DEBUG             = 1 << 7,

  G_LOG_LEVEL_MASK              = ~(G_LOG_FLAG_RECURSION | G_LOG_FLAG_FATAL)
} GLogLevelFlags;
void g_logv (const gchar   *log_domain, GLogLevelFlags log_level, const gchar   *format, va_list	       args1);
static void
g_error (const gchar *format,
         ...)
{
  va_list args;
  va_start (args, format);
  g_logv (G_LOG_DOMAIN, G_LOG_LEVEL_ERROR, format, args);
  va_end (args);

  for(;;) ;
}

extern int errno;

typedef enum
{
  G_OPTION_FLAG_HIDDEN		= 1 << 0,
  G_OPTION_FLAG_IN_MAIN		= 1 << 1,
  G_OPTION_FLAG_REVERSE		= 1 << 2,
  G_OPTION_FLAG_NO_ARG		= 1 << 3,
  G_OPTION_FLAG_FILENAME	= 1 << 4,
  G_OPTION_FLAG_OPTIONAL_ARG    = 1 << 5,
  G_OPTION_FLAG_NOALIAS	        = 1 << 6
} GOptionFlags;

typedef struct _GOptionContext GOptionContext;

#ifndef MIN
  #define MIN(a,b)              ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
  #define MAX(a,b)              ((a) > (b) ? (a) : (b))
#endif

/* Functions like the ones in <ctype.h> that are not affected by locale. */
typedef enum {
  G_ASCII_ALNUM  = 1 << 0,
  G_ASCII_ALPHA  = 1 << 1,
  G_ASCII_CNTRL  = 1 << 2,
  G_ASCII_DIGIT  = 1 << 3,
  G_ASCII_GRAPH  = 1 << 4,
  G_ASCII_LOWER  = 1 << 5,
  G_ASCII_PRINT  = 1 << 6,
  G_ASCII_PUNCT  = 1 << 7,
  G_ASCII_SPACE  = 1 << 8,
  G_ASCII_UPPER  = 1 << 9,
  G_ASCII_XDIGIT = 1 << 10
} GAsciiType;
GLIB_VAR const guint16 * const g_ascii_table;
#define g_ascii_isalnum(c) \
  ((g_ascii_table[(guchar) (c)] & G_ASCII_ALNUM) != 0)

#define g_ascii_isalpha(c) \
  ((g_ascii_table[(guchar) (c)] & G_ASCII_ALPHA) != 0)

#define g_ascii_iscntrl(c) \
  ((g_ascii_table[(guchar) (c)] & G_ASCII_CNTRL) != 0)

#define g_ascii_isdigit(c) \
  ((g_ascii_table[(guchar) (c)] & G_ASCII_DIGIT) != 0)

#define g_ascii_isgraph(c) \
  ((g_ascii_table[(guchar) (c)] & G_ASCII_GRAPH) != 0)

#define g_ascii_islower(c) \
  ((g_ascii_table[(guchar) (c)] & G_ASCII_LOWER) != 0)

#define g_ascii_isprint(c) \
  ((g_ascii_table[(guchar) (c)] & G_ASCII_PRINT) != 0)

#define g_ascii_ispunct(c) \
  ((g_ascii_table[(guchar) (c)] & G_ASCII_PUNCT) != 0)

#define g_ascii_isspace(c) \
  ((g_ascii_table[(guchar) (c)] & G_ASCII_SPACE) != 0)

#define g_ascii_isupper(c) \
  ((g_ascii_table[(guchar) (c)] & G_ASCII_UPPER) != 0)

#define g_ascii_isxdigit(c) \
  ((g_ascii_table[(guchar) (c)] & G_ASCII_XDIGIT) != 0)

#define G_STATIC_MUTEX_INIT 32

typedef int GStaticMutex;
#define G_GSIZE_FORMAT "%d"

#define G_DIR_SEPARATOR '/'
#define G_DIR_SEPARATOR_S "/"
#define G_IS_DIR_SEPARATOR(c) ((c) == G_DIR_SEPARATOR)
#define G_SEARCHPATH_SEPARATOR ':'
#define G_SEARCHPATH_SEPARATOR_S ":"

#define G_LOCK_NAME(name)               g__ ## name ## _lock
#define G_LOCK_DEFINE_STATIC(name)    static G_LOCK_DEFINE (name)
#  define G_LOCK_DEFINE(name)           \
    GStaticMutex G_LOCK_NAME (name) = G_STATIC_MUTEX_INIT

/* GStaticMutexes can be statically initialized with the value
 * G_STATIC_MUTEX_INIT, and then they can directly be used, that is
 * much easier, than having to explicitly allocate the mutex before
 * use
 */
#define g_static_mutex_lock(mutex) \
    g_mutex_lock (g_static_mutex_get_mutex (mutex))
#define g_static_mutex_trylock(mutex) \
    g_mutex_trylock (g_static_mutex_get_mutex (mutex))
#define g_static_mutex_unlock(mutex) \
    g_mutex_unlock (g_static_mutex_get_mutex (mutex))
void g_static_mutex_init (GStaticMutex *mutex);
void g_static_mutex_free (GStaticMutex *mutex);
#    define G_LOCK(name) g_static_mutex_lock       (&G_LOCK_NAME (name))
#    define G_UNLOCK(name) g_static_mutex_unlock   (&G_LOCK_NAME (name))
#    define G_TRYLOCK(name) g_static_mutex_trylock (&G_LOCK_NAME (name))

#ifdef LINUX
#define g_assert_cmpuint(n1, cmp, n2)   do { guint64 __n1 = (n1), __n2 = (n2); \
                                             if (__n1 cmp __n2) ; else \
                                               g_assertion_message_cmpnum (G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, \
                                                 #n1 " " #cmp " " #n2, __n1, #cmp, __n2, 'i'); } while (0)
#else
#define g_assert_cmpuint(n1, cmp, n2)   do { guint64 __n1 = (n1), __n2 = (n2); \
                                             if (__n1 cmp __n2) ; else \
                                               MP_ASSERT (0); } while (0)
#endif
# define g_atomic_int_get(atomic) 		((gint)*(atomic))
# define g_atomic_int_set(atomic, newval) 	((void) (*(atomic) = (newval)))
# define g_atomic_pointer_get(atomic) 		((gpointer)*(atomic))
# define g_atomic_pointer_set(atomic, newval)	((void) (*(atomic) = (newval)))

/* Array of skip-bytes-per-initial character.
 */
GLIB_VAR const gchar * const g_utf8_skip;
#  define G_GNUC_MAY_ALIAS
#define _g_slist_alloc0()       g_slice_new0 (GSList)
#define _g_slist_alloc()        g_slice_new (GSList)
#define _g_slist_free1(slist)   g_slice_free (GSList, slist)

#define g_atomic_int_dec_and_test(atomic) \
  (g_atomic_int_exchange_and_add ((atomic), -1) == 1)

#define  g_slist_next(slist)	         ((slist) ? (((GSList *)(slist))->next) : NULL)

#define g_atomic_int_inc(atomic) (g_atomic_int_add ((atomic), 1))

struct m_thread {
  GFunc func;
  gboolean started;
  gboolean sleep;
  unsigned short task_id;
  GThreadPool *threadpool;
};

extern void g_thread_pool_push (GThreadPool  *pool, gpointer      data, GError      **error);
extern GThreadPool* g_thread_pool_new (GFunc            func, gpointer         user_data, gint             max_threads, gboolean         exclusive, GError         **error);

#include "mpTrace_copy.h"
#include "log.h"

#define G_ASCII_DTOSTR_BUF_SIZE (29 + 10)

/* String to/from double conversion functions */

gdouble	              g_strtod         (const gchar  *nptr,
					gchar	    **endptr);
gdouble	              g_ascii_strtod   (const gchar  *nptr,
					gchar	    **endptr);
guint64		      g_ascii_strtoull (const gchar *nptr,
					gchar      **endptr,
					guint        base);
gint64		      g_ascii_strtoll  (const gchar *nptr,
					gchar      **endptr,
					guint        base);

typedef struct _GAsyncQueue GAsyncQueue;

GAsyncQueue*  g_async_queue_new                 (void);

/* Push data into the async queue. Must not be NULL. */
void         g_async_queue_push                 (GAsyncQueue      *queue,
						 gpointer          data);
void         g_async_queue_push_unlocked        (GAsyncQueue      *queue,
						 gpointer          data);

gint         g_async_queue_length               (GAsyncQueue      *queue);
gint         g_async_queue_length_unlocked      (GAsyncQueue      *queue);

#define DEBUG_MSG(x)  
//#define DEBUG_MSG(args) mpDebugPrint args

typedef struct _GThread         GThread;
struct  _GThread
{
  /*< private >*/
  GThreadFunc func;
  gpointer data;
  gboolean joinable;
  GThreadPriority priority;
};

#define G_FILE_ERROR g_file_error_quark ()

typedef enum
{
  G_KEY_FILE_ERROR_UNKNOWN_ENCODING,
  G_KEY_FILE_ERROR_PARSE,
  G_KEY_FILE_ERROR_NOT_FOUND,
  G_KEY_FILE_ERROR_KEY_NOT_FOUND,
  G_KEY_FILE_ERROR_GROUP_NOT_FOUND,
  G_KEY_FILE_ERROR_INVALID_VALUE
} GKeyFileError;

#define G_KEY_FILE_ERROR g_key_file_error_quark()

typedef struct _GKeyFile GKeyFile;


typedef enum
{
  G_KEY_FILE_NONE              = 0,
  G_KEY_FILE_KEEP_COMMENTS     = 1 << 0,
  G_KEY_FILE_KEEP_TRANSLATIONS = 1 << 1
} GKeyFileFlags;

#define G_GINT64_FORMAT "%ll"
#define G_GUINT64_FORMAT "%ll"

#define g_thread_create(func, data, joinable, error)			\
  (g_thread_create_full (func, data, 0, joinable, FALSE, 		\
                         G_THREAD_PRIORITY_NORMAL, error))

#define g_list_previous(list)	        ((list) ? (((GList *)(list))->prev) : NULL)
#define g_list_next(list)	        ((list) ? (((GList *)(list))->next) : NULL)

#endif
