=====================
Realtek 8188c
=====================
:Authors:   Bill Wang
:Date:      11 Mar 2012

1. Hotplug is not supported.
2. 16-hour stress test (with Picasa slideshow) runs without problems.
3. Ad-hoc mode was tested.  But disassociation has not been tested with 
   ad-hoc mode.
4. Build for infrastructure mode (or station mode)
   - include/corelib.h

      #define NETWARE_ENABLE			1 
      #define Make_USB	 			    7
      #undef SKB_OPT
      #define Make_ADHOC 				0

5. Build for ad-hoc mode
   - include/corelib.h

      #define NETWARE_ENABLE			1 
      #define Make_USB	 			    7
      #undef SKB_OPT
      #define Make_ADHOC 				1

.. # vim: tw=78 sw=2 fo+=n fo-=l ft=rst:
