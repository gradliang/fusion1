//---------------------------------------------------------------------------

#ifndef XPG_SPRITE_FRAME_H
#define XPG_SPRITE_FRAME_H

/* define for sprite animation, movie->page->sprite->frame */
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

enum SpriteFrameActions
{
	SET_VISIBLE,   // 0
	SET_ROLE,      // 1
	SET_POS_X,     // 2
	SET_POS_Y,     // 3
	SET_SIZE_W,    // 4
	SET_SIZE_H,    // 5
	SET_BLEND,     // 6
	SET_ANGLE,     // 7
	SET_FRAME_TAG, // 8
	SET_GOTO,      // 9

	/* for total action count */
	ACT_COUNT
};

#define ACT_VISIBLE   (1<<SET_VISIBLE)              // 0x001
#define ACT_ROLE      (1<<SET_ROLE)                 // 0x002
#define ACT_POS_X     (1<<SET_POS_X)                // 0x004
#define ACT_POS_Y     (1<<SET_POS_Y)                // 0x008
#define ACT_SIZE_W    (1<<SET_SIZE_W)               // 0x010
#define ACT_SIZE_H    (1<<SET_SIZE_H)               // 0x020
#define ACT_BLEND     (1<<SET_BLEND)                // 0x040
#define ACT_ANGLE     (1<<SET_ANGLE)                // 0x080
#define ACT_FRAME_TAG (1<<SET_FRAME_TAG)            // 0x100
#define ACT_GOTO      (1<<SET_GOTO)                 // 0x200
//#define ACT_INCLUDE_VALUE (1 << 16)               // set this for
#define ACT_POS       (ACT_POS_X | ACT_POS_Y)       // 0x030
#define ACT_SIZE      (ACT_SIZE_W | ACT_SIZE_H)     // 0x0C0

typedef struct
{
    short m_iFrame;
	short m_iAction;
	long  *m_alValue;
} xpg_sprite_frame_t;

//---------------------------------------------------------------------------
typedef struct
{
    short m_iAction;
	char m_boVisible;
	char m_iBlend;     /* 0 : not transperant, 100: unvisible */
	short m_iFrame;
	short m_iRole;
	short m_iPx, m_iPy;
	short m_iWidth, m_iHeight;
	short m_iAngle;

	/* flow and timing control */
	char *m_FrameTag;   /* tag could match with event - for example : mouse down, key down, active, selected */
	char *m_GotoTag;

	int m_iGotoRepeatTimes;  /* 0 : forever, 1-n : repeat n times */
	int m_iGotoRepeatedCount; /* for run time record how much times repeated, only work when play, should reset when start to play */
} xpg_sprite_ani_t;

//---------------------------------------------------------------------------
 #endif
