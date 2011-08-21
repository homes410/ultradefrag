/*
 *  UltraDefrag - powerful defragmentation tool for Windows NT.
 *  Copyright (c) 2007-2011 by Dmitri Arkhangelski (dmitriar@gmail.com).
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/**
 * @file search.c
 * @brief File blocks and free space regions searching.
 * @details We use binary search trees whenever possible
 * to speed things up. If memory allocation fails for
 * some operation on a binary tree, we destroy it and
 * use alternative searching in double linked lists.
 * @addtogroup Search
 * @{
 */

#include "udefrag-internals.h"

/************************************************************/
/*          Free space region searching routines            */
/************************************************************/

/**
 * @brief Searches for free space region starting at the beginning of the volume.
 * @param[in] jp job parameters structure.
 * @param[in] min_length minimum length of region, in clusters.
 * @note In case of termination request returns NULL immediately.
 */
winx_volume_region *find_first_free_region(udefrag_job_parameters *jp,ULONGLONG min_length)
{
	winx_volume_region *rgn;
	ULONGLONG time = winx_xtime();

	for(rgn = jp->free_regions; rgn; rgn = rgn->next){
		if(jp->termination_router((void *)jp)) break;
		if(rgn->length >= min_length){
			jp->p_counters.searching_time += winx_xtime() - time;
			return rgn;
		}
		if(rgn->next == jp->free_regions) break;
	}
	jp->p_counters.searching_time += winx_xtime() - time;
	return NULL;
}

/**
 * @brief Searches for free space region starting at the end of the volume.
 * @param[in] jp job parameters structure.
 * @param[in] min_length minimum length of region, in clusters.
 * @note In case of termination request returns NULL immediately.
 */
winx_volume_region *find_last_free_region(udefrag_job_parameters *jp,ULONGLONG min_length)
{
	winx_volume_region *rgn;
	ULONGLONG time = winx_xtime();

	if(jp->free_regions){
		for(rgn = jp->free_regions->prev; rgn; rgn = rgn->prev){
			if(jp->termination_router((void *)jp)) break;
			if(rgn->length >= min_length){
				jp->p_counters.searching_time += winx_xtime() - time;
				return rgn;
			}
			if(rgn->prev == jp->free_regions->prev) break;
		}
	}
	jp->p_counters.searching_time += winx_xtime() - time;
	return NULL;
}

/**
 * @brief Searches for best matching free space region.
 * @param[in] jp job parameters structure.
 * @param[in] start_lcn a point which divides disk into two parts (see below).
 * @param[in] min_length minimal accepted length of the region, in clusters.
 * @param[in] preferred_position one of the FIND_MATCHING_RGN_xxx constants:
 * FIND_MATCHING_RGN_FORWARD - region after the start_lcn preferred
 * FIND_MATCHING_RGN_BACKWARD - region before the start_lcn preferred
 * FIND_MATCHING_RGN_ANY - any region accepted
 * @note In case of termination request returns
 * NULL immediately.
 */
winx_volume_region *find_matching_free_region(udefrag_job_parameters *jp,
    ULONGLONG start_lcn, ULONGLONG min_length, int preferred_position)
{
	winx_volume_region *rgn, *rgn_matching;
	ULONGLONG length;
	ULONGLONG time = winx_xtime();
	
	rgn_matching = NULL, length = 0;
	for(rgn = jp->free_regions; rgn; rgn = rgn->next){
		if(jp->termination_router((void *)jp)){
			jp->p_counters.searching_time += winx_xtime() - time;
			return NULL;
		}
		if(preferred_position == FIND_MATCHING_RGN_BACKWARD \
		  && rgn->lcn > start_lcn)
			if(rgn_matching != NULL)
				break;
		if(rgn->length >= min_length){
            if(length == 0 || rgn->length < length){
                rgn_matching = rgn;
                length = rgn->length;
				if(length == min_length \
				  && preferred_position != FIND_MATCHING_RGN_FORWARD)
					break;
            }
		}
		if(rgn->next == jp->free_regions) break;
	}
	jp->p_counters.searching_time += winx_xtime() - time;
	return rgn_matching;
}

/**
 * @brief Searches for largest free space region.
 * @note In case of termination request returns
 * NULL immediately.
 */
winx_volume_region *find_largest_free_region(udefrag_job_parameters *jp)
{
	winx_volume_region *rgn, *rgn_largest;
	ULONGLONG length;
	ULONGLONG time = winx_xtime();
	
	rgn_largest = NULL, length = 0;
	for(rgn = jp->free_regions; rgn; rgn = rgn->next){
		if(jp->termination_router((void *)jp)){
			jp->p_counters.searching_time += winx_xtime() - time;
			return NULL;
		}
		if(rgn->length > length){
			rgn_largest = rgn;
			length = rgn->length;
		}
		if(rgn->next == jp->free_regions) break;
	}
	jp->p_counters.searching_time += winx_xtime() - time;
	return rgn_largest;
}

/************************************************************/
/*                    Auxiliary routines                    */
/************************************************************/

/**
 * @brief Auxiliary structure used to save file blocks in binary tree.
 */
struct file_block {
	winx_file_info *file;
	winx_blockmap *block;
};

/**
 * @brief Auxiliary routine used to sort file blocks in binary tree.
 */
static int blocks_compare(const void *prb_a, const void *prb_b, void *prb_param)
{
	struct file_block *a, *b;
	
	a = (struct file_block *)prb_a;
	b = (struct file_block *)prb_b;
	
	if(a->block->lcn < b->block->lcn)
		return (-1);
	if(a->block->lcn == b->block->lcn)
		return 0;
	return 1;
}

/**
 * @brief Auxiliary routine used to free memory allocated for tree items.
 */
static void free_item (void *prb_item, void *prb_param)
{
	struct file_block *item = (struct file_block *)prb_item;
	winx_heap_free(item);
}

/**
 * @brief Creates and initializes
 * the binary tree of all file blocks.
 * @return Zero for success, 
 * negative value otherwise.
 * @note jp->file_blocks must be
 * initialized by NULL or contain
 * pointer to a valid tree before 
 * this call.
 */
int create_file_blocks_tree(udefrag_job_parameters *jp)
{
	DebugPrint("create_file_blocks_tree called");
	
	if(jp == NULL)
		return (-1);
	
	if(jp->file_blocks)
		destroy_file_blocks_tree(jp);
	
	jp->file_blocks = prb_create(blocks_compare,NULL,NULL);
	if(jp->file_blocks == NULL){
		DebugPrint("create_file_blocks_tree: tree creation failed");
		return (-1);
	}
	return 0;
}

/**
 * @brief Adds a file block to
 * the binary tree of all file blocks.
 * @return Zero for success, 
 * negative value otherwise.
 * @note Destroys the tree in case of errors.
 */
int add_block_to_file_blocks_tree(udefrag_job_parameters *jp, winx_file_info *file, winx_blockmap *block)
{
	struct file_block *fb;
	
	if(jp == NULL || file == NULL || block == NULL)
		return (-1);
	
	if(jp->file_blocks == NULL)
		return (-1);

	fb = winx_heap_alloc(sizeof *fb);
	if(fb == NULL){
		destroy_file_blocks_tree(jp);
		return (-1);
	}
	
	fb->file = file;
	fb->block = block;
	/* FIXME: what if a duplicate item exists */
	if(prb_probe(jp->file_blocks,(void *)fb) == NULL){
		winx_heap_free(fb);
		destroy_file_blocks_tree(jp);
		return (-1);
	}
	return 0;
}

/**
 * @brief Removes a file block from
 * the binary tree of all file blocks.
 * @return Zero for success, 
 * negative value otherwise.
 * @note Destroys the tree in case of errors.
 */
int remove_block_from_file_blocks_tree(udefrag_job_parameters *jp, winx_blockmap *block)
{
	struct file_block *fb;
	struct file_block b;
	
	if(jp == NULL || block == NULL)
		return (-1);
	
	if(jp->file_blocks == NULL)
		return (-1);

	b.file = NULL;
	b.block = block;
	fb = prb_delete(jp->file_blocks,&b);
	if(fb == NULL){
		destroy_file_blocks_tree(jp);
		return (-1);
	}
	winx_heap_free(fb);
	return 0;
}

/**
 * @brief Destroys the binary
 * tree of all file blocks.
 */
void destroy_file_blocks_tree(udefrag_job_parameters *jp)
{
	DebugPrint("destroy_file_blocks_tree called");
	if(jp){
		if(jp->file_blocks){
			prb_destroy(jp->file_blocks,free_item);
			jp->file_blocks = NULL;
		}
	}
}

/************************************************************/
/*              File block searching routines               */
/************************************************************/

/**
 * @brief Searches for the first file block
 * after a specified cluster on the volume.
 * @param[in] jp job parameters.
 * @param[in,out] min_lcn pointer to variable containing
 * minimum LCN - file blocks below it will be ignored.
 * @param[in] flags one of MOVE_xxx flags defined in udefrag.h
 * @param[out] first_file pointer to variable receiving information
 * about the file the first block belongs to.
 * @return Pointer to the first block. NULL indicates failure.
 */
winx_blockmap *find_first_block(udefrag_job_parameters *jp, ULONGLONG *min_lcn, int flags, winx_file_info **first_file)
{
	winx_file_info *found_file, *file;
	winx_blockmap *first_block, *block;
	winx_blockmap b;
	struct file_block fb, *item;
	struct prb_traverser t;
	ULONGLONG lcn;
	ULONGLONG tm = winx_xtime();
	
	if(jp == NULL || min_lcn == NULL || first_file == NULL)
		return NULL;
	
	/* try to use fast binary tree search strategy first */
	if(jp->file_blocks != NULL){
		found_file = NULL; first_block = NULL;
		b.lcn = *min_lcn; fb.block = &b;
		prb_t_init(&t,jp->file_blocks);
		item = prb_t_insert(&t,jp->file_blocks,&fb);
		if(item == NULL){
			/* insertion failed, let's go to the slow search */
			goto slow_search;
		}
		if(item == &fb){
			/* block at min_lcn not found */
			item = prb_t_next(&t);
			if(prb_delete(jp->file_blocks,&fb) == NULL){
				/* removing failed, let's go to the slow search */
				goto slow_search;
			}
		}
		if(item){
			found_file = item->file;
			first_block = item->block;
		}
		while(!jp->termination_router((void *)jp)){
			if(found_file == NULL) break;
			if(!can_move(found_file,jp) || is_mft(found_file,jp)){
			} else if((flags == MOVE_FRAGMENTED) && !is_fragmented(found_file)){
			} else if((flags == MOVE_NOT_FRAGMENTED) && is_fragmented(found_file)){
			} else if(is_file_locked(found_file,jp)){
				jp->pi.processed_clusters += first_block->length;
			} else {
				/* desired block found */
				*min_lcn = first_block->lcn + 1; /* the current block will be skipped later anyway in this case */
				*first_file = found_file;
				jp->p_counters.searching_time += winx_xtime() - tm;
				return first_block;
			}
			
			/* skip current block */
			*min_lcn = *min_lcn + 1;
			/* and go to the next one */
			item = prb_t_next(&t);
			if(item == NULL) break;
			found_file = item->file;
			first_block = item->block;
		}
		*first_file = NULL;
		jp->p_counters.searching_time += winx_xtime() - tm;
		return NULL;
	}

slow_search:
	while(!jp->termination_router((void *)jp)){
		found_file = NULL; first_block = NULL; lcn = jp->v_info.total_clusters;
		for(file = jp->filelist; file; file = file->next){
			if(can_move(file,jp) && !is_mft(file,jp)){
				if((flags == MOVE_FRAGMENTED) && !is_fragmented(file)){
				} else if((flags == MOVE_NOT_FRAGMENTED) && is_fragmented(file)){
				} else {
					for(block = file->disp.blockmap; block; block = block->next){
						if(block->lcn >= *min_lcn && block->lcn < lcn && block->length){
							found_file = file;
							first_block = block;
							lcn = block->lcn;
						}
						if(block->next == file->disp.blockmap) break;
					}
				}
			}
			if(file->next == jp->filelist) break;
		}
		if(found_file == NULL) break;
		if(is_file_locked(found_file,jp)){
			jp->pi.processed_clusters += found_file->disp.clusters;
			continue;
		}
		/* desired block found */
		*min_lcn = first_block->lcn + 1; /* the current block will be skipped later anyway in this case */
		*first_file = found_file;
		jp->p_counters.searching_time += winx_xtime() - tm;
		return first_block;
	}
	*first_file = NULL;
	jp->p_counters.searching_time += winx_xtime() - tm;
	return NULL;
}

/** @} */
