#ifndef _EXTERNAL_CV_H_
#define _EXTERNAL_CV_H_

#define  CV_MALLOC_ALIGN    32

typedef void* (*CvAllocFunc)(size_t size, void* userdata);
typedef int (*CvFreeFunc)(void* pptr, void* userdata);
typedef void (*CvYieldFunc)(void);

static inline void* cvAlignPtr( const void* ptr, int align )
{
    return (void*)( ((size_t)ptr + align - 1) & ~(size_t)(align-1) );
}


/****************************************************************************************\
*                       Multi-dimensional dense array (CvMatND)                          *
\****************************************************************************************/

typedef unsigned char uchar;
typedef void CvArr;

#define CV_MAX_DIM            32

typedef struct CvMatND
{
    int type;
    int dims;

    int* refcount;
    int hdr_refcount;

    union
    {
        uchar* ptr;
        float* fl;
        double* db;
        int* i;
        short* s;
    } data;

    struct
    {
        int size;
        int step;
    }
    dim[CV_MAX_DIM];
}
CvMatND;


/****************************************************************************************\
*                                         Histogram                                      *
\****************************************************************************************/

typedef struct CvHistogram
{
    int     type;
    CvArr*  bins;
    float   thresh[CV_MAX_DIM][2]; /* for uniform histograms */
    float** thresh2; /* for non-uniform histograms */
    CvMatND mat; /* embedded matrix header for array histograms */
}
CvHistogram;


/****************************************************************************************\
*                                    Image Processing                                    *
\****************************************************************************************/

#define CV_BILATERAL 4

/* Constants for color conversion */
#define  CV_YYCbCr2GRAY     62
#define  CV_GRAY2YYCbCr     63
#define  CV_GRAY2YY8080     64
#define  CV_YYCbCr2YCbCr    65
#define  CV_YCbCr2YYCbCr    66
#define  CV_YYCbCr2RGB      67
#define  CV_RGB2YYCbCr      68


/****************************************************************************************\
*                                  Image type (IplImage)                                 *
\****************************************************************************************/

typedef struct _IplImage
{
    int  nSize;         /* sizeof(IplImage) */
    int  ID;            /* version (=0)*/
    int  nChannels;     /* Most of OpenCV functions support 1,2,3 or 4 channels */
    int  alphaChannel;  /* ignored by OpenCV */
    int  depth;         /* pixel depth in bits: IPL_DEPTH_8U, IPL_DEPTH_8S, IPL_DEPTH_16S,
                           IPL_DEPTH_32S, IPL_DEPTH_32F and IPL_DEPTH_64F are supported */
    char colorModel[4]; /* ignored by OpenCV */
    char channelSeq[4]; /* ditto */
    int  dataOrder;     /* 0 - interleaved color channels, 1 - separate color channels.
                           cvCreateImage can only create interleaved images */
    int  origin;        /* 0 - top-left origin,
                           1 - bottom-left origin (Windows bitmaps style) */
    int  align;         /* Alignment of image rows (4 or 8).
                           OpenCV ignores it and uses widthStep instead */
    int  width;         /* image width in pixels */
    int  height;        /* image height in pixels */
    struct _IplROI *roi;/* image ROI. if NULL, the whole image is selected */
    struct _IplImage *maskROI; /* must be NULL */
    void  *imageId;     /* ditto */
    struct _IplTileInfo *tileInfo; /* ditto */
    int  imageSize;     /* image data size in bytes
                           (==image->height*image->widthStep
                           in case of interleaved data)*/
    char *imageData;  /* pointer to aligned image data */
    int  widthStep;   /* size of aligned image row in bytes */
    int  BorderMode[4]; /* ignored by OpenCV */
    int  BorderConst[4]; /* ditto */
    char *imageDataOrigin; /* pointer to very origin of image data
                              (not necessarily aligned) -
                              needed for correct deallocation */
}
IplImage;


/****************************************************************************************\
*                                  Matrix type (CvMat)                                   *
\****************************************************************************************/

typedef struct CvMat
{
    int type;
    int step;

    /* for internal use only */
    int* refcount;
    int hdr_refcount;

    union
    {
        unsigned char* ptr;
        short* s;
        int* i;
        float* fl;
        double* db;
    } data;

    union
    {
        int rows;
        int height;
    };

    union
    {
        int cols;
        int width;
    };

}
CvMat;


/****************************************************************************************\
*                      Other supplementary data type definitions                         *
\****************************************************************************************/

/************************************** CvPoint *****************************************/

typedef struct CvPoint
{
    int x;
    int y;
}
CvPoint;

static inline CvPoint cvPoint( int x, int y )
{
    CvPoint p;

    p.x = x;
    p.y = y;

    return p;
}

/*************************************** CvRect *****************************************/

typedef struct CvRect
{
    int x;
    int y;
    int width;
    int height;
}
CvRect;

static inline CvRect cvRect( int x, int y, int width, int height )
{
    CvRect r;

    r.x = x;
    r.y = y;
    r.width = width;
    r.height = height;

    return r;
}

/*************************************** CvSize *****************************************/

typedef struct
{
    int width;
    int height;
}
CvSize;

static inline CvSize cvSize( int width, int height )
{
    CvSize s;

    s.width = width;
    s.height = height;

    return s;
}


/****************************************************************************************\
*                                   Dynamic Data structures                              *
\****************************************************************************************/

/******************************** Memory storage ****************************************/

typedef struct CvMemBlock
{
    struct CvMemBlock*  prev;
    struct CvMemBlock*  next;
}
CvMemBlock;

typedef struct CvMemStorage
{
    int signature;
    CvMemBlock* bottom;/* first allocated block */
    CvMemBlock* top;   /* current memory block - top of the stack */
    struct  CvMemStorage* parent; /* borrows new blocks from */
    int block_size;  /* block size */
    int free_space;  /* free space in the current block */
}
CvMemStorage;


/*********************************** Sequence *******************************************/

typedef struct CvSeqBlock
{
    struct CvSeqBlock*  prev; /* previous sequence block */
    struct CvSeqBlock*  next; /* next sequence block */
    int    start_index;       /* index of the first element in the block +
                                 sequence->first->start_index */
    int    count;             /* number of elements in the block */
    char*  data;              /* pointer to the first element of the block */
}
CvSeqBlock;


#define CV_TREE_NODE_FIELDS(node_type)                          \
    int       flags;         /* micsellaneous flags */          \
    int       header_size;   /* size of sequence header */      \
    struct    node_type* h_prev; /* previous sequence */        \
    struct    node_type* h_next; /* next sequence */            \
    struct    node_type* v_prev; /* 2nd previous sequence */    \
    struct    node_type* v_next  /* 2nd next sequence */

/*
   Read/Write sequence.
   Elements can be dynamically inserted to or deleted from the sequence.
*/
#define CV_SEQUENCE_FIELDS()                                            \
    CV_TREE_NODE_FIELDS(CvSeq);                                         \
    int       total;          /* total number of elements */            \
    int       elem_size;      /* size of sequence element in bytes */   \
    char*     block_max;      /* maximal bound of the last block */     \
    char*     ptr;            /* current write pointer */               \
    int       delta_elems;    /* how many elements allocated when the seq grows */  \
    CvMemStorage* storage;    /* where the seq is stored */             \
    CvSeqBlock* free_blocks;  /* free blocks list */                    \
    CvSeqBlock* first; /* pointer to the first sequence block */

typedef struct CvSeq
{
    CV_SEQUENCE_FIELDS()
}
CvSeq;


/************************ Run-length encoding for binary images *************************/

static inline void prefetch(const void *x) {;}

struct list_head {
	struct list_head *next, *prev;
};

#define LIST_HEAD_DECLARE(name) struct list_head name

#define list_entry(ptr, type, member) ({			\
        const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
        (type *)( (char *)__mptr - offsetof(type,member) );})

#define list_for_each_entry(pos, head, member)				\
	for (pos = list_entry((head)->next, typeof(*pos), member);	\
	     prefetch(pos->member.next), &pos->member != (head); 	\
	     pos = list_entry(pos->member.next, typeof(*pos), member))

#define list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = list_entry((head)->next, typeof(*pos), member),	\
		n = list_entry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (head); 					\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))

#define  CV_SHAPE_RECT      0
#define  CV_SHAPE_CROSS     1
#define  CV_SHAPE_ELLIPSE   2
#define  CV_SHAPE_CUSTOM    100

typedef struct CvRun
{
    int xL;
    int xR;
    int y;
    int index;      /* used by erode algorithm */
    LIST_HEAD_DECLARE(run_node);
}
CvRun;

typedef struct CvRleRegion
{
    int left;       /* bounding box of the region */
    int top;
    int right;
    int bottom;
    int area;
    int index;      /* used by erode algorithm */
    int nRuns;
    LIST_HEAD_DECLARE(run_list);
    LIST_HEAD_DECLARE(region_node);
}
CvRleRegion;

typedef struct CvRleElement
{
    int nCols;
    int nRows;
    int anchorX;
    int anchorY;
    int *values;
    int nRuns;
    LIST_HEAD_DECLARE(run_list);
}
CvRleElement;

typedef struct CvRleBinaryImage
{
    int width;
    int height;
    int nRegions;
    LIST_HEAD_DECLARE(region_list);
}
CvRleBinaryImage;


/*********************** Haar-like Object Detection structures **************************/

#define CV_HAAR_FEATURE_MAX  3

#define SCALE_BITS 16
#define SCALE_PRECISION (1 << SCALE_BITS)
#define SCALE_CONST(A) (((A) >= 0) ? ((real_t)((A)*(SCALE_PRECISION)+0.5)) : ((real_t)((A)*(SCALE_PRECISION)-0.5)))
typedef int real_t;

typedef struct CvHaarFeature
{
    int  tilted;
    struct
    {
        CvRect r;
        real_t weight;
    } rect[CV_HAAR_FEATURE_MAX];
}
CvHaarFeature;

typedef struct CvHaarClassifier
{
    int count;
    CvHaarFeature* haar_feature;
    real_t* threshold;
    int* left;
    int* right;
    real_t* alpha;
}
CvHaarClassifier;

typedef struct CvHaarStageClassifier
{
    int  count;
    real_t threshold;
    CvHaarClassifier* classifier;

    int next;
    int child;
    int parent;
}
CvHaarStageClassifier;

typedef int sumtype;
typedef unsigned int sqsumtype;

typedef struct CvHidHaarFeature
{
    struct
    {
        sumtype *p0, *p1, *p2, *p3;
        real_t weight;
    }
    rect[CV_HAAR_FEATURE_MAX];
}
CvHidHaarFeature;

typedef struct CvHidHaarTreeNode
{
    CvHidHaarFeature feature;
    real_t threshold;
    int left;
    int right;
}
CvHidHaarTreeNode;

typedef struct CvHidHaarClassifier
{
    int count;
    //CvHaarFeature* orig_feature;
    CvHidHaarTreeNode* node;
    real_t* alpha;
}
CvHidHaarClassifier;

typedef struct CvHidHaarStageClassifier
{
    int  count;
    real_t threshold;
    CvHidHaarClassifier* classifier;
    int two_rects;
    
    struct CvHidHaarStageClassifier* next;
    struct CvHidHaarStageClassifier* child;
    struct CvHidHaarStageClassifier* parent;
}
CvHidHaarStageClassifier;

struct CvHidHaarClassifierCascade
{
    int  count;
    int  is_stump_based;
    int  has_tilted_features;
    int  is_tree;
    real_t inv_window_area;
    CvMat sum, sqsum, tilted;
    CvHidHaarStageClassifier* stage_classifier;
    sqsumtype *pq0, *pq1, *pq2, *pq3;
    sumtype *p0, *p1, *p2, *p3;

    void** ipp_stages;
};

typedef struct CvHidHaarClassifierCascade CvHidHaarClassifierCascade;

typedef struct CvHaarClassifierCascade
{
    int  flags;
    int  count;
    CvSize orig_window_size;
    CvSize real_window_size;
    real_t scale;
    CvHaarStageClassifier* stage_classifier;
    CvHidHaarClassifierCascade* hid_cascade;
}
CvHaarClassifierCascade;

typedef struct CvAvgComp
{
    CvRect rect;
    int neighbors;
}
CvAvgComp;


/*********************** Simple Floodfill structures **************************/

typedef struct CvFFillSegment
{
    int y;
    int l;
    int r;
    int prevl;
    int prevr;
    int dir;
}
CvFFillSegment;

#define CV_UP 1
#define CV_DOWN -1             

#define ICV_PUSH( Y, L, R, PREV_L, PREV_R, DIR )\
{                                               \
    tail->y = (int)(Y);                         \
    tail->l = (int)(L);                         \
    tail->r = (int)(R);                         \
    tail->prevl = (int)(PREV_L);                \
    tail->prevr = (int)(PREV_R);                \
    tail->dir = (int)(DIR);                     \
    if( ++tail >= buffer_end )                  \
        tail = buffer;                          \
}

#define ICV_POP( Y, L, R, PREV_L, PREV_R, DIR ) \
{                                               \
    Y = head->y;                                \
    L = head->l;                                \
    R = head->r;                                \
    PREV_L = head->prevl;                       \
    PREV_R = head->prevr;                       \
    DIR = head->dir;                            \
    if( ++head >= buffer_end )                  \
        head = buffer;                          \
}

#endif /*_EXTERNAL_CV_H_*/
