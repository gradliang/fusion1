/* ZD1211 USB-WLAN driver for Linux
 *
 * Copyright (C) 2005-2007 Ulrich Kunitz <kune@deine-taler.de>
 * Copyright (C) 2006-2007 Daniel Drake <dsd@gentoo.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
 
#define LOCAL_DEBUG_ENABLE 0

#include <linux/types.h>
#include <linux/kernel.h>

#include "zd_rf.h"
#include "zd_usb.h"
#include "zd_chip.h"
#include "ndebug.h"

/* This RF programming code is based upon the code found in v2.16.0.0 of the
 * ZyDAS vendor driver. Unlike other RF's, Ubec publish full technical specs
 * for this RF on their website, so we're able to understand more than
 * usual as to what is going on. Thumbs up for Ubec for doing that. */

/* The 3-wire serial interface provides access to 8 write-only registers.
 * The data format is a 4 bit register address followed by a 20 bit value. */
#define AR2124_REGWRITE(reg, val) ((((reg) & 0xf) << 20) | ((val) & 0xfffff))

/* For channel tuning, we have to configure registers 1 (synthesizer), 2 (synth
 * fractional divide ratio) and 3 (VCO config).
 *
 * We configure the RF to produce an interrupt when the PLL is locked onto
 * the configured frequency. During initialization, we run through a variety
 * of different VCO configurations on channel 1 until we detect a PLL lock.
 * When this happens, we remember which VCO configuration produced the lock
 * and use it later. Actually, we use the configuration *after* the one that
 * produced the lock, which seems odd, but it works.
 *
 * If we do not see a PLL lock on any standard VCO config, we fall back on an
 * autocal configuration, which has a fixed (as opposed to per-channel) VCO
 * config and different synth values from the standard set (divide ratio
 * is still shared with the standard set). */





/* The per-channel synth values for all standard VCO configurations. These get
 * written to register 1. */

/* This table stores the synthesizer fractional divide ratio for *all* VCO
 * configurations (both standard and autocal). These get written to register 2. */

/* Here is the data for all the standard VCO configurations. We shrink our
 * table a little by observing that both channels in a consecutive pair share
 * the same value. We also observe that the high 4 bits ([0:3] in the specs)
 * are all 'Reserved' and are always set to 0x4 - we chop them off in the data
 * below. */
 
static const u32 ar2124_table[][45] = {
	//table 1                                                    
	{                                                            
		0xe20008, 0x999004, 0xb2662c,   //;NULL                 
		0xe20008, 0x999004, 0xb2662c,   //;Ch 1		 2412
		0xe20008, 0xd99004, 0xb2662c,   //;Ch 2		 2417
		0xe60008, 0x199004, 0xb2662c,   //;Ch 3		 2422
		0xe60008, 0x599004, 0xb2662c,   //;Ch 4		 2427
		0xe60008, 0x999004, 0xae662c,   //;Ch 5		 2432
		0xe60008, 0xd99004, 0xae662c,   //;Ch 6		 2437
		0xea0008, 0x199004, 0xae662c,   //;Ch 7		 2442
		0xea0008, 0x599004, 0xae662c,   //;Ch 8		 2447
		0xea0008, 0x999004, 0xaa662c,   //;Ch 9		 2452
		0xea0008, 0xd99004, 0xaa662c,   //;Ch 10		 2457
		0xee0008, 0x199004, 0xaa662c,   //;Ch 11		 2462
		0xee0008, 0x599004, 0xaa662c,   //;Ch 12		 2467
		0xee0008, 0x999004, 0xa6662c,   //;Ch 13		 2472
		0xf20008, 0x333004, 0xa6662c    //;Ch 14		 2484
	},                                                           
	//table 2                                                    
	{                                                            
		0xe20008, 0x999004, 0xb6662c,   //;NULL                 
		0xe20008, 0x999004, 0xb6662c,   //;Ch 1		 2412
		0xe20008, 0xd99004, 0xb6662c,   //;Ch 2		 2417
		0xe60008, 0x199004, 0xb6662c,   //;Ch 3		 2422
		0xe60008, 0x599004, 0xb6662c,   //;Ch 4		 2427
		0xe60008, 0x999004, 0xb2662c,   //;Ch 5		 2432
		0xe60008, 0xd99004, 0xb2662c,   //;Ch 6		 2437
		0xea0008, 0x199004, 0xb2662c,   //;Ch 7		 2442
		0xea0008, 0x599004, 0xb2662c,   //;Ch 8		 2447
		0xea0008, 0x999004, 0xae662c,   //;Ch 9		 2452
		0xea0008, 0xd99004, 0xae662c,   //;Ch 10		 2457
		0xee0008, 0x199004, 0xae662c,   //;Ch 11		 2462
		0xee0008, 0x599004, 0xae662c,   //;Ch 12		 2467
		0xee0008, 0x999004, 0xaa662c,   //;Ch 13		 2472
		0xf20008, 0x333004, 0xaa662c    //;Ch 14		 2484
	},                                                           
	//table 3                                                    
	{                                                            
		0xe20008, 0x999004, 0xba662c,   //;NULL                 
		0xe20008, 0x999004, 0xba662c,   //;Ch 1		 2412
		0xe20008, 0xd99004, 0xba662c,   //;Ch 2		 2417
		0xe60008, 0x199004, 0xba662c,   //;Ch 3		 2422
		0xe60008, 0x599004, 0xba662c,   //;Ch 4		 2427
		0xe60008, 0x999004, 0xb6662c,   //;Ch 5		 2432
		0xe60008, 0xd99004, 0xb6662c,   //;Ch 6		 2437
		0xea0008, 0x199004, 0xb6662c,   //;Ch 7		 2442
		0xea0008, 0x599004, 0xb6662c,   //;Ch 8		 2447
		0xea0008, 0x999004, 0xb2662c,   //;Ch 9		 2452
		0xea0008, 0xd99004, 0xb2662c,   //;Ch 10		 2457
		0xee0008, 0x199004, 0xb2662c,   //;Ch 11		 2462
		0xee0008, 0x599004, 0xb2662c,   //;Ch 12		 2467
		0xee0008, 0x999004, 0xae662c,   //;Ch 13		 2472
		0xf20008, 0x333004, 0xae662c    //;Ch 14		 2484
	},                                                           
	//table 4                                                    
	{                                                            
		0xe20008, 0x999004, 0xbe662c,   //;NULL                 
		0xe20008, 0x999004, 0xbe662c,   //;Ch 1		 2412
		0xe20008, 0xd99004, 0xbe662c,   //;Ch 2		 2417
		0xe60008, 0x199004, 0xbe662c,   //;Ch 3		 2422
		0xe60008, 0x599004, 0xbe662c,   //;Ch 4		 2427
		0xe60008, 0x999004, 0xba662c,   //;Ch 5		 2432
		0xe60008, 0xd99004, 0xba662c,   //;Ch 6		 2437
		0xea0008, 0x199004, 0xba662c,   //;Ch 7		 2442
		0xea0008, 0x599004, 0xba662c,   //;Ch 8		 2447
		0xea0008, 0x999004, 0xb6662c,   //;Ch 9		 2452
		0xea0008, 0xd99004, 0xb6662c,   //;Ch 10		 2457
		0xee0008, 0x199004, 0xb6662c,   //;Ch 11		 2462
		0xee0008, 0x599004, 0xb6662c,   //;Ch 12		 2467
		0xee0008, 0x999004, 0xb2662c,   //;Ch 13		 2472
		0xf20008, 0x333004, 0xb2662c    //;Ch 14		 2484
	},                                                           
	//table 5                                                    
	{                                                            
		0xe20008, 0x999004, 0xc2662c,   //;NULL                 
		0xe20008, 0x999004, 0xc2662c,   //;Ch 1		 2412
		0xe20008, 0xd99004, 0xc2662c,   //;Ch 2		 2417
		0xe60008, 0x199004, 0xc2662c,   //;Ch 3		 2422
		0xe60008, 0x599004, 0xc2662c,   //;Ch 4		 2427
		0xe60008, 0x999004, 0xbe662c,   //;Ch 5		 2432
		0xe60008, 0xd99004, 0xbe662c,   //;Ch 6		 2437
		0xea0008, 0x199004, 0xbe662c,   //;Ch 7		 2442
		0xea0008, 0x599004, 0xbe662c,   //;Ch 8		 2447
		0xea0008, 0x999004, 0xba662c,   //;Ch 9		 2452
		0xea0008, 0xd99004, 0xba662c,   //;Ch 10		 2457
		0xee0008, 0x199004, 0xba662c,   //;Ch 11		 2462
		0xee0008, 0x599004, 0xba662c,   //;Ch 12		 2467
		0xee0008, 0x999004, 0xb6662c,   //;Ch 13		 2472
		0xf20008, 0x333004, 0xb6662c    //;Ch 14		 2484
	},                                                           
	//table 6                                                                                                                                                                                                                                          
	{                                                                                                                                                                                                                                                  
		0xe20008, 0x999004, 0xc6662c,   //;NULL                                                                                                                                                                                                       
		0xe20008, 0x999004, 0xc6662c,   //;Ch 1		 2412                                                                                                                                                                                      
		0xe20008, 0xd99004, 0xc6662c,   //;Ch 2		 2417                                                                                                                                                                                      
		0xe60008, 0x199004, 0xc6662c,   //;Ch 3		 2422                                                                                                                                                                                      
		0xe60008, 0x599004, 0xc6662c,   //;Ch 4		 2427                                                                                                                                                                                      
		0xe60008, 0x999004, 0xc2662c,   //;Ch 5		 2432                                                                                                                                                                                      
		0xe60008, 0xd99004, 0xc2662c,   //;Ch 6		 2437                                                                                                                                                                                      
		0xea0008, 0x199004, 0xc2662c,   //;Ch 7		 2442                                                                                                                                                                                      
		0xea0008, 0x599004, 0xc2662c,   //;Ch 8		 2447                                                                                                                                                                                      
		0xea0008, 0x999004, 0xbe662c,   //;Ch 9		 2452                                                                                                                                                                                      
		0xea0008, 0xd99004, 0xbe662c,   //;Ch 10		 2457                                                                                                                                                                                      
		0xee0008, 0x199004, 0xbe662c,   //;Ch 11		 2462                                                                                                                                                                                      
		0xee0008, 0x599004, 0xbe662c,   //;Ch 12		 2467                                                                                                                                                                                      
		0xee0008, 0x999004, 0xba662c,   //;Ch 13		 2472                                                                                                                                                                                      
		0xf20008, 0x333004, 0xba662c    //;Ch 14		 2484                                                                                                                                                                                      
	},                                                                                                                                                                                                                                                 
	//table 7                                                                                                                                                                                                                                          
	{                                                                                                                                                                                                                                                  
		0xe20008, 0x999004, 0xca662c,   //;NULL                                                                                                                                                                                                       
		0xe20008, 0x999004, 0xca662c,   //;Ch 1		 2412                                                                                                                                                                                      
		0xe20008, 0xd99004, 0xca662c,   //;Ch 2		 2417                                                                                                                                                                                      
		0xe60008, 0x199004, 0xca662c,   //;Ch 3		 2422                                                                                                                                                                                      
		0xe60008, 0x599004, 0xca662c,   //;Ch 4		 2427                                                                                                                                                                                      
		0xe60008, 0x999004, 0xc6662c,   //;Ch 5		 2432                                                                                                                                                                                      
		0xe60008, 0xd99004, 0xc6662c,   //;Ch 6		 2437                                                                                                                                                                                      
		0xea0008, 0x199004, 0xc6662c,   //;Ch 7		 2442                                                                                                                                                                                      
		0xea0008, 0x599004, 0xc6662c,   //;Ch 8		 2447                                                                                                                                                                                      
		0xea0008, 0x999004, 0xc2662c,   //;Ch 9		 2452                                                                                                                                                                                      
		0xea0008, 0xd99004, 0xc2662c,   //;Ch 10		 2457                                                                                                                                                                                      
		0xee0008, 0x199004, 0xc2662c,   //;Ch 11		 2462                                                                                                                                                                                      
		0xee0008, 0x599004, 0xc2662c,   //;Ch 12		 2467                                                                                                                                                                                      
		0xee0008, 0x999004, 0xbe662c,   //;Ch 13		 2472                                                                                                                                                                                      
		0xf20008, 0x333004, 0xbe662c    //;Ch 14		 2484                                                                                                                                                                                      
	},                                                                                                                                                                                                                                                 
	//table 8                                                                                                                                                                                                                                          
	{                                                                                                                                                                                                                                                  
		0xe20008, 0x999004, 0xce662c,   //;NULL                                                                                                                                                                                                       
		0xe20008, 0x999004, 0xce662c,   //;Ch 1		 2412                                                                                                                                                                                      
		0xe20008, 0xd99004, 0xce662c,   //;Ch 2		 2417                                                                                                                                                                                      
		0xe60008, 0x199004, 0xce662c,   //;Ch 3		 2422                                                                                                                                                                                      
		0xe60008, 0x599004, 0xce662c,   //;Ch 4		 2427                                                                                                                                                                                      
		0xe60008, 0x999004, 0xca662c,   //;Ch 5		 2432                                                                                                                                                                                      
		0xe60008, 0xd99004, 0xca662c,   //;Ch 6		 2437                                                                                                                                                                                      
		0xea0008, 0x199004, 0xca662c,   //;Ch 7		 2442                                                                                                                                                                                      
		0xea0008, 0x599004, 0xca662c,   //;Ch 8		 2447                                                                                                                                                                                      
		0xea0008, 0x999004, 0xc6662c,   //;Ch 9		 2452                                                                                                                                                                                      
		0xea0008, 0xd99004, 0xc6662c,   //;Ch 10		 2457                                                                                                                                                                                      
		0xee0008, 0x199004, 0xc6662c,   //;Ch 11		 2462                                                                                                                                                                                      
		0xee0008, 0x599004, 0xc6662c,   //;Ch 12		 2467                                                                                                                                                                                      
		0xee0008, 0x999004, 0xc2662c,   //;Ch 13		 2472                                                                                                                                                                                      
		0xf20008, 0x333004, 0xc2662c    //;Ch 14		 2484                                                                                                                                                                                      
	},                                                                                                                                                                                                                                                 
	//table 9                                                                                                                                                                                                                                          
	{                                                                                                                                                                                                                                                  
		0xe20008, 0x999004, 0xd2662c,   //;NULL                                                                                                                                                                                                       
		0xe20008, 0x999004, 0xd2662c,   //;Ch 1		 2412                                                                                                                                                                                      
		0xe20008, 0xd99004, 0xd2662c,   //;Ch 2		 2417                                                                                                                                                                                      
		0xe60008, 0x199004, 0xd2662c,   //;Ch 3		 2422                                                                                                                                                                                      
		0xe60008, 0x599004, 0xd2662c,   //;Ch 4		 2427                                                                                                                                                                                      
		0xe60008, 0x999004, 0xce662c,   //;Ch 5		 2432                                                                                                                                                                                      
		0xe60008, 0xd99004, 0xce662c,   //;Ch 6		 2437                                                                                                                                                                                      
		0xea0008, 0x199004, 0xce662c,   //;Ch 7		 2442                                                                                                                                                                                      
		0xea0008, 0x599004, 0xce662c,   //;Ch 8		 2447                                                                                                                                                                                      
		0xea0008, 0x999004, 0xca662c,   //;Ch 9		 2452                                                                                                                                                                                      
		0xea0008, 0xd99004, 0xca662c,   //;Ch 10		 2457                                                                                                                                                                                      
		0xee0008, 0x199004, 0xca662c,   //;Ch 11		 2462                                                                                                                                                                                      
		0xee0008, 0x599004, 0xca662c,   //;Ch 12		 2467                                                                                                                                                                                      
		0xee0008, 0x999004, 0xc6662c,   //;Ch 13		 2472                                                                                                                                                                                      
		0xf20008, 0x333004, 0xc6662c    //;Ch 14		 2484                                                                                                                                                                                      
	},                                                                                                                                                                                                                                                 
	//table 10                                                    
	{                                                             
		0xe20008, 0x999004, 0xd6662c,   //;NULL                  
		0xe20008, 0x999004, 0xd6662c,   //;Ch 1		 2412 
		0xe20008, 0xd99004, 0xd6662c,   //;Ch 2		 2417 
		0xe60008, 0x199004, 0xd6662c,   //;Ch 3		 2422 
		0xe60008, 0x599004, 0xd6662c,   //;Ch 4		 2427 
		0xe60008, 0x999004, 0xd2662c,   //;Ch 5		 2432 
		0xe60008, 0xd99004, 0xd2662c,   //;Ch 6		 2437 
		0xea0008, 0x199004, 0xd2662c,   //;Ch 7		 2442 
		0xea0008, 0x599004, 0xd2662c,   //;Ch 8		 2447 
		0xea0008, 0x999004, 0xce662c,   //;Ch 9		 2452 
		0xea0008, 0xd99004, 0xce662c,   //;Ch 10		 2457 
		0xee0008, 0x199004, 0xce662c,   //;Ch 11		 2462 
		0xee0008, 0x599004, 0xce662c,   //;Ch 12		 2467 
		0xee0008, 0x999004, 0xca662c,   //;Ch 13		 2472 
		0xf20008, 0x333004, 0xca662c    //;Ch 14		 2484 
	},                                                            
	//table 11                                                    
	{                                                             
		0xe20008, 0x999004, 0xda662c,   //;NULL                  
		0xe20008, 0x999004, 0xda662c,   //;Ch 1		 2412 
		0xe20008, 0xd99004, 0xda662c,   //;Ch 2		 2417 
		0xe60008, 0x199004, 0xda662c,   //;Ch 3		 2422 
		0xe60008, 0x599004, 0xda662c,   //;Ch 4		 2427 
		0xe60008, 0x999004, 0xd6662c,   //;Ch 5		 2432 
		0xe60008, 0xd99004, 0xd6662c,   //;Ch 6		 2437 
		0xea0008, 0x199004, 0xd6662c,   //;Ch 7		 2442 
		0xea0008, 0x599004, 0xd6662c,   //;Ch 8		 2447 
		0xea0008, 0x999004, 0xd2662c,   //;Ch 9		 2452 
		0xea0008, 0xd99004, 0xd2662c,   //;Ch 10		 2457 
		0xee0008, 0x199004, 0xd2662c,   //;Ch 11		 2462 
		0xee0008, 0x599004, 0xd2662c,   //;Ch 12		 2467 
		0xee0008, 0x999004, 0xce662c,   //;Ch 13		 2472 
		0xf20008, 0x333004, 0xce662c    //;Ch 14		 2484 
	},                                                            
	//table AUTOCAL                                                   
	{                                                             
		0xe21608, 0x999004, 0x46662c,   //;NULL                  
		0xe21608, 0x999004, 0x46662c,   //;Ch 1		 2412 
		0xe21608, 0xd99004, 0x46662c,   //;Ch 2		 2417 
		0xe61608, 0x199004, 0x46662c,   //;Ch 3		 2422 
		0xe61608, 0x599004, 0x46662c,   //;Ch 4		 2427 
		0xe61608, 0x999004, 0x46662c,   //;Ch 5		 2432 
		0xe61608, 0xd99004, 0x46662c,   //;Ch 6		 2437 
		0xea1608, 0x199004, 0x46662c,   //;Ch 7		 2442 
		0xea1608, 0x599004, 0x46662c,   //;Ch 8		 2447 
		0xea1608, 0x999004, 0x46662c,   //;Ch 9		 2452 
		0xea1608, 0xd99004, 0x46662c,   //;Ch 10		 2457 
		0xee1608, 0x199004, 0x46662c,   //;Ch 11		 2462 
		0xee1608, 0x599004, 0x46662c,   //;Ch 12		 2467 
		0xee1608, 0x999004, 0x46662c,   //;Ch 13		 2472 
		0xf21608, 0x333004, 0x46662c    //;Ch 14		 2484 
	}                                                                   
};
 



/* TX gain settings. The array index corresponds to the TX power integration
 * values found in the EEPROM. The values get written to register 7. */
static u32 ar2124_txgain[] = {
	[0x00] = 0x07FDF,
	[0x01] = 0x07FFF,
	[0x02] = 0x17E9F,
	[0x03] = 0x17F9F,
	[0x04] = 0x17FDF,
	[0x05] = 0x17FFF,
	[0x06] = 0x0FE9F,
	[0x07] = 0x0FF9F,
	[0x08] = 0x0FFDF,
	[0x09] = 0x0FFFF,
	[0x0a] = 0x27F9F,
	[0x0b] = 0x27FDF,
	[0x0c] = 0x27FFF,
	[0x0d] = 0x37FDF,
	[0x0e] = 0x37FFF,
	[0x0f] = 0x2FFDF,
	[0x10] = 0x2FFFF,
	[0x11] = 0x3FFDF,
	[0x12] = 0x3FFFF,
};

/* RF-specific structure */
struct ar2124_priv {
	/* index into synth/VCO config tables where PLL lock was found
	 * -1 means autocal */
	int config;
};

#define AR2124_PRIV(rf) ((struct ar2124_priv *) (rf)->priv)

//static int ar2124_synth_set_channel(struct zd_chip *chip, int channel,bool autocal)
static int ar2124_synth_set_channel(struct zd_chip *chip, struct zd_rf *rf, int channel)
{
	int r;
	int config = AR2124_PRIV(rf)->config;
	
	r = zd_rfwrite_cr_locked(chip, ar2124_table[config][channel*3]);  // TY, 090122
	if (r)
		return r;
	return zd_rfwrite_cr_locked(chip, ar2124_table[config][channel*3+1]);  // TY, 090122
}

static int ar2124_write_vco_cfg(struct zd_chip *chip, u16 value)
{
	return zd_rfwrite_cr_locked(chip, value);  // TY, 090122
}

static int ar2124_init_mode(struct zd_chip *chip)
{
	MP_DEBUG("====ar2124_init_mode====");

	static const u32 rv[] = {
		0x19fa40, // enter IDLE mode 
		0x59fa40, // enter CAL_VCO mode 
		0x29fa40, // enter RX/TX mode 
		0x2bfe40, // power down RSSI circuit 
	};
	zd_rfwritev_cr_locked(chip, rv, ARRAY_SIZE(rv));  
	return zd_iowrite16_locked(chip, 0x80, CR240);  // Turn off HW synthesiz control.
}

static int ar2124_set_tx_gain_level(struct zd_chip *chip, int channel)
{
	u8 int_value = chip->pwr_int_values[channel - 1];
	u32 value = AR2124_REGWRITE(7, ar2124_txgain[int_value]);
	
	if (int_value >= ARRAY_SIZE(ar2124_txgain)) {
		dev_dbg_f(zd_chip_dev(chip), "can't configure TX gain for "
			  "int value %x on channel %d\n", int_value, channel);
		return 0;
	}
	
	MP_DEBUG2("=== ar2124_set_tx_gain_level [%d]= 0x%x ===", int_value, value);
	return zd_rfwrite_cr_locked(chip, value);  
}

static int ar2124_init_hw(struct zd_rf *rf)
{
	int i, r;
	int found_config = -1;
	u16 intr_status;
	struct zd_chip *chip = zd_rf_to_chip(rf);

	MP_DEBUG("============= ar2124_init_hw=============");

	
	zd_iowrite16_locked(chip, 0x57, CR240);  // Turn on HW synthesiz control.
	static const struct zd_ioreq16 ioreqs[] = {
		{ CR10,  0x89 }, { CR15,  0x20 },
		{ CR17,  0x28 }, // 6112 no change
		{ CR23,  0x38 }, { CR24,  0x20 }, { CR26,  0x93 },
		{ CR27,  0x15 }, { CR28,  0x3e }, { CR29,  0x00 },
		{ CR33,  0x28 }, { CR34,  0x30 },
		{ CR35,  0x43 }, // 6112 3e->43 
		{ CR41,  0x24 }, { CR44,  0x32 },
		{ CR46,  0x92 }, // 6112 96->92 
		//{ CR47,  0x1e },  // TY:2009/01/19
		{ CR48,  0x04 }, // 5602 Roger 
		{ CR49,  0xfa }, { CR79,  0x58 }, { CR80,  0x30 },
		{ CR81,  0x30 }, { CR87,  0x0a }, { CR89,  0x04 },
		{ CR91,  0x00 }, { CR92,  0x0a }, { CR98,  0x8d },
		{ CR99,  0x28 }, { CR100, 0x02 },
		{ CR101, 0x09 }, // 6112 13->1f 6220 1f->13 6407 13->9 
		{ CR102, 0x27 },
		{ CR106, 0x1c }, // 5d07 5112 1f->1c 6220 1c->1f 6221 1f->1c
		{ CR107, 0x1c }, // 6220 1c->1a 5221 1a->1c 
		{ CR109, 0x13 },
		{ CR110, 0x1f }, // 6112 13->1f 6221 1f->13 6407 13->0x09 
		{ CR111, 0x13 }, { CR112, 0x1f }, { CR113, 0x27 },
		{ CR114, 0x23 }, // 6221 27->23 
		{ CR115, 0x24 }, // 6112 24->1c 6220 1c->24 
		{ CR116, 0x24 }, // 6220 1c->24 
		{ CR117, 0xfa }, // 6112 fa->f8 6220 f8->f4 6220 f4->fa 
		{ CR118, 0xf0 }, // 5d07 6112 f0->f2 6220 f2->f0 
		{ CR119, 0x1a }, // 6112 1a->10 6220 10->14 6220 14->1a 
		{ CR120, 0x4f },
		{ CR121, 0x1f }, // 6220 4f->1f 
		{ CR122, 0xf0 }, { CR123, 0x57 }, { CR125, 0xad },
		{ CR126, 0x6c }, { CR127, 0x03 },
		{ CR128, 0x14 }, // 6302 12->11 
		{ CR129, 0x12 }, // 6301 10->0f 
		{ CR130, 0x10 }, { CR137, 0x50 }, { CR138, 0xa8 },
		{ CR144, 0xac }, { CR146, 0x20 }, { CR252, 0xff },
		{ CR253, 0xff },
	};
	

	static const u32 rv[] = {
		0xd40002,    // configure reciever gain 
		0xf2798a, // configure transmitter gain 
		0xb581f6, // enable RX/TX filter tuning 
		0x7fffce, // disable TX gain in test mode 

		// enter CAL_FIL mode, TX gain set by registers, RX gain set by pins,
		// RSSI circuit powered down, reduced RSSI range 
		0x39fa40, // 5d01 cal_fil 

		// synthesizer configuration for channel 1 
		0xe20008,
		0x999004,

		// disable manual VCO band selection 
		0x406e0c,

		// enable manual VCO band selection, configure current level 
		0xc6062c,
	};

	r = zd_iowrite16a_locked(chip, ioreqs, ARRAY_SIZE(ioreqs));
	if (r)
		return r;

	r = zd_rfwritev_cr_locked(chip, rv, ARRAY_SIZE(rv));
	if (r)
		return r;

	r = ar2124_init_mode(chip);
	if (r)
		return r;

	/* To match the vendor driver behaviour, we use the configuration after
	 * the one that produced a lock. */
	AR2124_PRIV(rf)->config = 11;
	
	return 0;   
	//return zd_iowrite16_locked(chip, 0x06, CR203);
}

static int ar2124_set_channel(struct zd_rf *rf, u8 channel)
{
	int r;
	int config = AR2124_PRIV(rf)->config;
	struct zd_chip *chip = zd_rf_to_chip(rf);

	static const struct zd_ioreq16 ioreqs[] = {
		{ CR128,  0x14 }, { CR129,  0x12 }, { CR130,  0x10 },  
		{ CR80,  0x30 }, { CR81,  0x30 }, { CR79,  0x58 },
		{ CR12,  0xf0 }, { CR77,  0x1b }, { CR78,  0x58 },
	};
	
	MP_DEBUG1("============= ar2124_set_channel = %d=============", channel);
	zd_iowrite16_locked(chip, 0x57, CR240);  // Turn on HW synthesiz control.
	r = ar2124_synth_set_channel(chip, rf, channel);
	if (r)
		return r;

	r = ar2124_write_vco_cfg(chip, ar2124_table[config][3*channel+2]);
	if (r)
		return r;

	r = ar2124_init_mode(chip);
	if (r)
		return r;

	r = zd_iowrite16a_locked(chip, ioreqs, ARRAY_SIZE(ioreqs));
	if (r)
		return r;

	r = ar2124_set_tx_gain_level(chip, channel);
	if (r)
		return r;

	return zd_iowrite16_locked(chip, 0x06, CR203);
}

static int ar2124_switch_radio_on(struct zd_rf *rf)
{
	int r;
	struct zd_chip *chip = zd_rf_to_chip(rf);
	struct zd_ioreq16 ioreqs[] = {
		{ CR11,  0x00 }, { CR251, 0x3f },
	};

	/* enter RXTX mode */
	r = zd_rfwrite_locked(chip, AR2124_REGWRITE(0, 0x025f94), RF_RV_BITS);
	if (r)
		return r;

	if (zd_chip_is_zd1211b(chip))
		ioreqs[1].value = 0x7f;

	return zd_iowrite16a_locked(chip, ioreqs, ARRAY_SIZE(ioreqs));
}

static int ar2124_switch_radio_off(struct zd_rf *rf)
{
	int r;
	struct zd_chip *chip = zd_rf_to_chip(rf);
	static const struct zd_ioreq16 ioreqs[] = {
		{ CR11,  0x04 }, { CR251, 0x2f },
	};

	/* enter IDLE mode */
	/* FIXME: shouldn't we go to SLEEP? sent email to zydas */
	r = zd_rfwrite_locked(chip, AR2124_REGWRITE(0, 0x25f90), RF_RV_BITS);
	if (r)
		return r;

	return zd_iowrite16a_locked(chip, ioreqs, ARRAY_SIZE(ioreqs));
}

static void ar2124_clear(struct zd_rf *rf)
{
	kfree(rf->priv);
}

int zd_rf_init_ar2124(struct zd_rf *rf)
{
	rf->init_hw = ar2124_init_hw;
	rf->set_channel = ar2124_set_channel;
	rf->switch_radio_on = ar2124_switch_radio_on;
	rf->switch_radio_off = ar2124_switch_radio_off;
	rf->patch_6m_band_edge = zd_rf_generic_patch_6m;
	rf->clear = ar2124_clear;
	/* we have our own TX integration code */
	rf->update_channel_int = 0;

	rf->priv = kmalloc(sizeof(struct ar2124_priv), GFP_KERNEL);
	if (rf->priv == NULL)
		return -ENOMEM;

	return 0;
}

