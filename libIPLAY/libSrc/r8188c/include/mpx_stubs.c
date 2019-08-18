
struct tkip_ctx {
	u32 iv32;
	u16 iv16;
	u16 p1k[5];
	int initialized;
};

enum ieee80211_key_alg {
	ALG_WEP,
	ALG_TKIP,
	ALG_CCMP,
};
struct ieee80211_key_conf {
	enum ieee80211_key_alg alg;
	u8 hw_key_idx;
	u8 flags;
	s8 keyidx;
	u8 keylen;
	u8 key[0];
};
#define NUM_RX_DATA_QUEUES	17
struct ieee80211_key {
	struct ieee80211_local *local;
	struct ieee80211_sub_if_data *sdata;
	struct sta_info *sta;

	/* for sdata list */
	struct list_head list;
	/* for todo list */
	struct list_head todo;

	/* protected by todo lock! */
	unsigned int flags;

	union {
		struct {
			/* last used TSC */
			struct tkip_ctx tx;

			/* last received RSC */
			struct tkip_ctx rx[NUM_RX_DATA_QUEUES];
		} tkip;
		struct {
			u8 tx_pn[6];
			u8 rx_pn[NUM_RX_DATA_QUEUES][6];
			struct crypto_cipher *tfm;
			u32 replays; /* dot11RSNAStatsCCMPReplays */
			/* scratch buffers for virt_to_page() (crypto API) */
#ifndef AES_BLOCK_LEN
#define AES_BLOCK_LEN 16
#endif
			u8 tx_crypto_buf[6 * AES_BLOCK_LEN];
			u8 rx_crypto_buf[6 * AES_BLOCK_LEN];
		} ccmp;
	} u;

	/* number of times this key has been used */
	int tx_rx_count;

#ifdef CONFIG_MAC80211_DEBUGFS
	struct {
		struct dentry *stalink;
		struct dentry *dir;
		struct dentry *keylen;
		struct dentry *flags;
		struct dentry *keyidx;
		struct dentry *hw_key_idx;
		struct dentry *tx_rx_count;
		struct dentry *algorithm;
		struct dentry *tx_spec;
		struct dentry *rx_spec;
		struct dentry *replays;
		struct dentry *key;
		struct dentry *ifindex;
		int cnt;
	} debugfs;
#endif

	/*
	 * key config, must be last because it contains key
	 * material as variable length member
	 */
	struct ieee80211_key_conf conf;
};

#define PHASE1_LOOP_COUNT 8
#define NL80211_TKIP_DATA_OFFSET_ENCR_KEY	0
struct crypto_blkcipher;

static const u16 tkip_sbox[256] =
{
	0xC6A5, 0xF884, 0xEE99, 0xF68D, 0xFF0D, 0xD6BD, 0xDEB1, 0x9154,
	0x6050, 0x0203, 0xCEA9, 0x567D, 0xE719, 0xB562, 0x4DE6, 0xEC9A,
	0x8F45, 0x1F9D, 0x8940, 0xFA87, 0xEF15, 0xB2EB, 0x8EC9, 0xFB0B,
	0x41EC, 0xB367, 0x5FFD, 0x45EA, 0x23BF, 0x53F7, 0xE496, 0x9B5B,
	0x75C2, 0xE11C, 0x3DAE, 0x4C6A, 0x6C5A, 0x7E41, 0xF502, 0x834F,
	0x685C, 0x51F4, 0xD134, 0xF908, 0xE293, 0xAB73, 0x6253, 0x2A3F,
	0x080C, 0x9552, 0x4665, 0x9D5E, 0x3028, 0x37A1, 0x0A0F, 0x2FB5,
	0x0E09, 0x2436, 0x1B9B, 0xDF3D, 0xCD26, 0x4E69, 0x7FCD, 0xEA9F,
	0x121B, 0x1D9E, 0x5874, 0x342E, 0x362D, 0xDCB2, 0xB4EE, 0x5BFB,
	0xA4F6, 0x764D, 0xB761, 0x7DCE, 0x527B, 0xDD3E, 0x5E71, 0x1397,
	0xA6F5, 0xB968, 0x0000, 0xC12C, 0x4060, 0xE31F, 0x79C8, 0xB6ED,
	0xD4BE, 0x8D46, 0x67D9, 0x724B, 0x94DE, 0x98D4, 0xB0E8, 0x854A,
	0xBB6B, 0xC52A, 0x4FE5, 0xED16, 0x86C5, 0x9AD7, 0x6655, 0x1194,
	0x8ACF, 0xE910, 0x0406, 0xFE81, 0xA0F0, 0x7844, 0x25BA, 0x4BE3,
	0xA2F3, 0x5DFE, 0x80C0, 0x058A, 0x3FAD, 0x21BC, 0x7048, 0xF104,
	0x63DF, 0x77C1, 0xAF75, 0x4263, 0x2030, 0xE51A, 0xFD0E, 0xBF6D,
	0x814C, 0x1814, 0x2635, 0xC32F, 0xBEE1, 0x35A2, 0x88CC, 0x2E39,
	0x9357, 0x55F2, 0xFC82, 0x7A47, 0xC8AC, 0xBAE7, 0x322B, 0xE695,
	0xC0A0, 0x1998, 0x9ED1, 0xA37F, 0x4466, 0x547E, 0x3BAB, 0x0B83,
	0x8CCA, 0xC729, 0x6BD3, 0x283C, 0xA779, 0xBCE2, 0x161D, 0xAD76,
	0xDB3B, 0x6456, 0x744E, 0x141E, 0x92DB, 0x0C0A, 0x486C, 0xB8E4,
	0x9F5D, 0xBD6E, 0x43EF, 0xC4A6, 0x39A8, 0x31A4, 0xD337, 0xF28B,
	0xD532, 0x8B43, 0x6E59, 0xDAB7, 0x018C, 0xB164, 0x9CD2, 0x49E0,
	0xD8B4, 0xACFA, 0xF307, 0xCF25, 0xCAAF, 0xF48E, 0x47E9, 0x1018,
	0x6FD5, 0xF088, 0x4A6F, 0x5C72, 0x3824, 0x57F1, 0x73C7, 0x9751,
	0xCB23, 0xA17C, 0xE89C, 0x3E21, 0x96DD, 0x61DC, 0x0D86, 0x0F85,
	0xE090, 0x7C42, 0x71C4, 0xCCAA, 0x90D8, 0x0605, 0xF701, 0x1C12,
	0xC2A3, 0x6A5F, 0xAEF9, 0x69D0, 0x1791, 0x9958, 0x3A27, 0x27B9,
	0xD938, 0xEB13, 0x2BB3, 0x2233, 0xD2BB, 0xA970, 0x0789, 0x33A7,
	0x2DB6, 0x3C22, 0x1592, 0xC920, 0x8749, 0xAAFF, 0x5078, 0xA57A,
	0x038F, 0x59F8, 0x0980, 0x1A17, 0x65DA, 0xD731, 0x84C6, 0xD0B8,
	0x82C3, 0x29B0, 0x5A77, 0x1E11, 0x7BCB, 0xA8FC, 0x6DD6, 0x2C3A,
};

static u16 tkipS(u16 val)
{
	return tkip_sbox[val & 0xff] ^ swab16(tkip_sbox[val >> 8]);
}

static u8 *write_tkip_iv(u8 *pos, u16 iv16);
const unsigned int crc32_tab[] = {
       0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
       0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
       0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
       0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
       0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
       0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
       0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
       0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
       0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
       0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
       0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
       0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
       0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
       0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
       0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
       0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
       0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
       0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
       0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
       0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
       0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
       0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
       0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
       0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
       0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
       0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
       0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
       0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
       0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
       0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
       0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
       0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
       0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
       0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
       0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
       0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
       0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
       0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
       0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
       0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
       0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
       0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
       0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
       0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
       0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
       0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
       0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
       0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
       0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
       0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
       0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
       0x2d02ef8dL
};
static inline void put_unaligned_le16(u16 val, void *p)
{
	__le16 v;
	v = cpu_to_le16(val);
	memcpy(p, &v, sizeof v);

}
static inline void put_unaligned_le32(u32 val, void *p)
{
	__le32 v;
	v = cpu_to_le32(val);
	memcpy(p, &v, sizeof v);
}
static inline u16 get_unaligned_le16(const void *p)
{
	__le16 val;
	memcpy(&val, p, sizeof val);

	return le16_to_cpup(&val);
}

u8 *ieee80211_tkip_add_iv(u8 *pos, struct ieee80211_key *key, u16 iv16)
{
	pos = write_tkip_iv(pos, iv16);
	*pos++ = (key->conf.keyidx << 6) | (1 << 5) /* Ext IV */;
	put_unaligned_le32(key->u.tkip.tx.iv32, pos);
	return pos + 4;
}
static u8 *write_tkip_iv(u8 *pos, u16 iv16)
{
	*pos++ = iv16 >> 8;
	*pos++ = ((iv16 >> 8) | 0x20) & 0x7f;
	*pos++ = iv16 & 0xFF;
	return pos;
}
void zd_EncryptData (
	u8		Wep_Key_Len,
    u8*		Wep_Key,
    u8*		Wep_Iv,
    u16		Num_Bytes,
    u8*		Inbuf,
    u8*		Outbuf,
    u32*	Icv);
static void tkip_mixing_phase1(const u8 *tk, struct tkip_ctx *ctx,
			       const u8 *ta, u32 tsc_IV32)
{
	int i, j;
	u16 *p1k = ctx->p1k;

	p1k[0] = tsc_IV32 & 0xFFFF;
	p1k[1] = tsc_IV32 >> 16;
	p1k[2] = get_unaligned_le16(ta + 0);
	p1k[3] = get_unaligned_le16(ta + 2);
	p1k[4] = get_unaligned_le16(ta + 4);

	for (i = 0; i < PHASE1_LOOP_COUNT; i++) {
		j = 2 * (i & 1);
		p1k[0] += tkipS(p1k[4] ^ get_unaligned_le16(tk + 0 + j));
		p1k[1] += tkipS(p1k[0] ^ get_unaligned_le16(tk + 4 + j));
		p1k[2] += tkipS(p1k[1] ^ get_unaligned_le16(tk + 8 + j));
		p1k[3] += tkipS(p1k[2] ^ get_unaligned_le16(tk + 12 + j));
		p1k[4] += tkipS(p1k[3] ^ get_unaligned_le16(tk + 0 + j)) + i;
	}
	ctx->initialized = 1;
}

static void tkip_mixing_phase2(const u8 *tk, struct tkip_ctx *ctx,
			       u16 tsc_IV16, u8 *rc4key)
{
	u16 ppk[6];
	const u16 *p1k = ctx->p1k;
	int i;

	ppk[0] = p1k[0];
	ppk[1] = p1k[1];
	ppk[2] = p1k[2];
	ppk[3] = p1k[3];
	ppk[4] = p1k[4];
	ppk[5] = p1k[4] + tsc_IV16;

	ppk[0] += tkipS(ppk[5] ^ get_unaligned_le16(tk + 0));
	ppk[1] += tkipS(ppk[0] ^ get_unaligned_le16(tk + 2));
	ppk[2] += tkipS(ppk[1] ^ get_unaligned_le16(tk + 4));
	ppk[3] += tkipS(ppk[2] ^ get_unaligned_le16(tk + 6));
	ppk[4] += tkipS(ppk[3] ^ get_unaligned_le16(tk + 8));
	ppk[5] += tkipS(ppk[4] ^ get_unaligned_le16(tk + 10));
	ppk[0] += ror16(ppk[5] ^ get_unaligned_le16(tk + 12), 1);
	ppk[1] += ror16(ppk[0] ^ get_unaligned_le16(tk + 14), 1);
	ppk[2] += ror16(ppk[1], 1);
	ppk[3] += ror16(ppk[2], 1);
	ppk[4] += ror16(ppk[3], 1);
	ppk[5] += ror16(ppk[4], 1);

	rc4key = write_tkip_iv(rc4key, tsc_IV16);
	*rc4key++ = ((ppk[5] ^ get_unaligned_le16(tk)) >> 1) & 0xFF;

	for (i = 0; i < 6; i++)
		put_unaligned_le16(ppk[i], rc4key + 2 * i);
}

void ieee80211_wep_encrypt_data(struct crypto_blkcipher *tfm, u8 *rc4key,
				size_t klen, u8 *data, size_t data_len)
{
#ifdef LINUX
	struct blkcipher_desc desc = { .tfm = tfm };
	struct scatterlist sg;
	__le32 *icv;

	icv = (__le32 *)(data + data_len);
	*icv = cpu_to_le32(~crc32_le(~0, data, data_len));

	crypto_blkcipher_setkey(tfm, rc4key, klen);
	sg_init_one(&sg, data, data_len + WEP_ICV_LEN);
	crypto_blkcipher_encrypt(&desc, &sg, &sg, sg.length);
#else
	u32 icv;
    MP_DEBUG("%s", __func__);
	zd_EncryptData(klen,rc4key, NULL, data_len,data,data, &icv);
#endif
}
void ieee80211_tkip_encrypt_data(struct crypto_blkcipher *tfm,
				 struct ieee80211_key *key,
				 u8 *pos, size_t payload_len, u8 *ta)
{
	u8 rc4key[16];
	struct tkip_ctx *ctx = &key->u.tkip.tx;
	const u8 *tk = &key->conf.key[NL80211_TKIP_DATA_OFFSET_ENCR_KEY];

	/* Calculate per-packet key */
	if (ctx->iv16 == 0 || !ctx->initialized)
		tkip_mixing_phase1(tk, ctx, ta, ctx->iv32);

	tkip_mixing_phase2(tk, ctx, ctx->iv16, rc4key);

	pos = ieee80211_tkip_add_iv(pos, key, key->u.tkip.tx.iv16);
	ieee80211_wep_encrypt_data(tfm, rc4key, 16, pos, payload_len);
}

static int S_init;
u8 S[256];
void initWepState(void)
{
	int i;
	
	for (i=0; i<256; i++){
		S[i] = i;
	}	
}	

void zd_EncryptData (
	u8		Wep_Key_Len,
    u8*		Wep_Key,
    u8*		Wep_Iv,
    u16		Num_Bytes,
    u8*		Inbuf,
    u8*		Outbuf,
    u32*	Icv)
{
	u8  S2[256], Se[256];
	register u16 ui;
	register u16 i;
	register u16 j;
	register u8  temp;
	u8  keylen = Wep_Key_Len;
	u8  K;
	u8  *In = Inbuf;
	u8  *Out = Outbuf;
	u32 ltemp;
    if (!S_init)
    {
        S_init = true;
        initWepState();
    }
    for (i=0; i<256; i++){
        S2[i] = Wep_Key[i & (keylen-1)];
    }
    
    memcpy(Se, S, 256);
    
    j = 0;
    for (i=0; i<256; i++){
	    j = (j + Se[i] + S2[i]) ;
        j &= 255 ;
        
        // Swap S[i] and S[j]
        temp = Se[i];
        Se[i] = Se[j];
        Se[j] = temp;
    }
    
    i = j = 0;
    *Icv = -1;
    for (ui=0; ui<Num_Bytes; ui++){
    	i++;
        i &= 255;
        j += Se[i];
        j &= 255;
        
        // Swap S[i] and S[j]
        temp = Se[i];
        Se[i] = Se[j];
        Se[j] = temp;
//          temp = (S[i] + temp) & 255;
        temp += Se[i];
        temp &= 255;
        K = Se[temp];        // Key used to Xor with input data

        *Icv =  (*Icv >> 8) ^ crc32_tab[(*Icv ^ *In) & 0xff];

        *Out = *In ^ K;                 // XOR
        In++;
        Out++;
    } //End of for (ui = 0; ui < Num_Bytes; ui++)

    *Icv = ~(*Icv);
    ltemp = *Icv;
    for (ui=0; ui<4; ui++){
        i ++;
        i &= 255;
        j += Se[i];
        j &= 255;
        
        // Swap S[i] and S[j]
        temp = Se[i];
        Se[i] = Se[j];
        Se[j] = temp;
        temp += Se[i];
        temp &= 255;
        K = Se[temp];        // Key used to Xor with input data

        *Out++ = (u8) (ltemp ^ K) & 0xff;
        ltemp >>= 8;
    }
}
