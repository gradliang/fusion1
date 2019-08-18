#ifndef __LINUX_NODEMASK_H
#define __LINUX_NODEMASK_H

/*
 * Nodemasks provide a bitmap suitable for representing the
 * set of Node's in a system, one bit position per Node number.
 *
 * See detailed comments in the file linux/bitmap.h describing the
 * data type on which these nodemasks are based.
 *
 * For details of nodemask_scnprintf() and nodemask_parse_user(),
 * see bitmap_scnprintf() and bitmap_parse_user() in lib/bitmap.c.
 * For details of nodelist_scnprintf() and nodelist_parse(), see
 * bitmap_scnlistprintf() and bitmap_parselist(), also in bitmap.c.
 * For details of node_remap(), see bitmap_bitremap in lib/bitmap.c.
 * For details of nodes_remap(), see bitmap_remap in lib/bitmap.c.
 * For details of nodes_onto(), see bitmap_onto in lib/bitmap.c.
 * For details of nodes_fold(), see bitmap_fold in lib/bitmap.c.
 *
 * The available nodemask operations are:
 *
 * void node_set(node, mask)		turn on bit 'node' in mask
 * void node_clear(node, mask)		turn off bit 'node' in mask
 * void nodes_setall(mask)		set all bits
 * void nodes_clear(mask)		clear all bits
 * int node_isset(node, mask)		true iff bit 'node' set in mask
 * int node_test_and_set(node, mask)	test and set bit 'node' in mask
 *
 * void nodes_and(dst, src1, src2)	dst = src1 & src2  [intersection]
 * void nodes_or(dst, src1, src2)	dst = src1 | src2  [union]
 * void nodes_xor(dst, src1, src2)	dst = src1 ^ src2
 * void nodes_andnot(dst, src1, src2)	dst = src1 & ~src2
 * void nodes_complement(dst, src)	dst = ~src
 *
 * int nodes_equal(mask1, mask2)	Does mask1 == mask2?
 * int nodes_intersects(mask1, mask2)	Do mask1 and mask2 intersect?
 * int nodes_subset(mask1, mask2)	Is mask1 a subset of mask2?
 * int nodes_empty(mask)		Is mask empty (no bits sets)?
 * int nodes_full(mask)			Is mask full (all bits sets)?
 * int nodes_weight(mask)		Hamming weight - number of set bits
 *
 * void nodes_shift_right(dst, src, n)	Shift right
 * void nodes_shift_left(dst, src, n)	Shift left
 *
 * int first_node(mask)			Number lowest set bit, or MAX_NUMNODES
 * int next_node(node, mask)		Next node past 'node', or MAX_NUMNODES
 * int first_unset_node(mask)		First node not set in mask, or 
 *					MAX_NUMNODES.
 *
 * nodemask_t nodemask_of_node(node)	Return nodemask with bit 'node' set
 * NODE_MASK_ALL			Initializer - all bits set
 * NODE_MASK_NONE			Initializer - no bits set
 * unsigned long *nodes_addr(mask)	Array of unsigned long's in mask
 *
 * int nodemask_scnprintf(buf, len, mask) Format nodemask for printing
 * int nodemask_parse_user(ubuf, ulen, mask)	Parse ascii string as nodemask
 * int nodelist_scnprintf(buf, len, mask) Format nodemask as list for printing
 * int nodelist_parse(buf, map)		Parse ascii string as nodelist
 * int node_remap(oldbit, old, new)	newbit = map(old, new)(oldbit)
 * void nodes_remap(dst, src, old, new)	*dst = map(old, new)(src)
 * void nodes_onto(dst, orig, relmap)	*dst = orig relative to relmap
 * void nodes_fold(dst, orig, sz)	dst bits = orig bits mod sz
 *
 * for_each_node_mask(node, mask)	for-loop node over mask
 *
 * int num_online_nodes()		Number of online Nodes
 * int num_possible_nodes()		Number of all possible Nodes
 *
 * int node_online(node)		Is some node online?
 * int node_possible(node)		Is some node possible?
 *
 * int any_online_node(mask)		First online node in mask
 *
 * node_set_online(node)		set bit 'node' in node_online_map
 * node_set_offline(node)		clear bit 'node' in node_online_map
 *
 * for_each_node(node)			for-loop node over node_possible_map
 * for_each_online_node(node)		for-loop node over node_online_map
 *
 * Subtlety:
 * 1) The 'type-checked' form of node_isset() causes gcc (3.3.2, anyway)
 *    to generate slightly worse code.  So use a simple one-line #define
 *    for node_isset(), instead of wrapping an inline inside a macro, the
 *    way we do the other calls.
 */

#include <linux/kernel.h>



#endif /* __LINUX_NODEMASK_H */
