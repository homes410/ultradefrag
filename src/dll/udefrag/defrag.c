/*
 *  UltraDefrag - powerful defragmentation tool for Windows NT.
 *  Copyright (c) 2007-2010 by Dmitri Arkhangelski (dmitriar@gmail.com).
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
 * @file defrag.c
 * @brief Volume defragmentation.
 * @addtogroup Defrag
 * @{
 */

#include "udefrag-internals.h"

/*
* Set UD_DRY_RUN environment variable
* to avoid actual data moving in tests.
*/
int dry_run = 0;

/* forward declarations */
static int move_file_helper(HANDLE hFile,winx_file_info *f,
	ULONGLONG target,udefrag_job_parameters *jp,WINX_FILE *fVolume);
static int move_file_blocks_helper(HANDLE hFile,winx_file_info *f,
	winx_blockmap *first_block,ULONGLONG n_blocks,ULONGLONG target,
	udefrag_job_parameters *jp,WINX_FILE *fVolume);
static void redraw_freed_space(udefrag_job_parameters *jp,
		ULONGLONG lcn, ULONGLONG length, int old_color);

/**
 * @brief Performs a volume defragmentation.
 * @return Zero for success, negative value otherwise.
 */
int defragment(udefrag_job_parameters *jp)
{
	udefrag_fragmented_file *f, *f_largest;
	winx_volume_region *rgn, *rgn_largest;
	ULONGLONG length;
	ULONGLONG time, time2, seconds;
	ULONGLONG moved_clusters;
	ULONGLONG defragmented_files;
	ULONGLONG joined_fragments;
	winx_blockmap *block, *first_block, *longest_sequence;
	ULONGLONG n_blocks, max_n_blocks;
	ULONGLONG remaining_clusters;
	WINX_FILE *fVolume = NULL;
	char buffer[32];
	short env_buffer[32];
	int result;
	
	/* analyze volume */
	result = analyze(jp);
	if(result < 0)
		return result;
	
	/* check for %UD_DRY_RUN% existence */
	if(winx_query_env_variable(L"UD_DRY_RUN",env_buffer,sizeof(env_buffer)/sizeof(short)) >= 0){
		DebugPrint("%UD_DRY_RUN% environment variable exists,\n");
		DebugPrint("therefore no actual data moves will be performed on disk\n");
		dry_run = 1;
	} else {
		dry_run = 0;
	}
	
	/* open the volume */
	fVolume = winx_vopen(winx_toupper(jp->volume_letter));
	if(fVolume == NULL)
		return (-1);
	
	/* reset statistics */
	jp->pi.clusters_to_process = 0;
	jp->pi.processed_clusters = 0;
	jp->pi.moved_clusters = 0;
	jp->pi.current_operation = 'D';
	for(f = jp->fragmented_files; f; f = f->next){
		if(jp->termination_router((void *)jp)) goto done;
		if(f->f->disp.blockmap){
			if(!is_directory(f->f) || jp->actions.allow_dir_defrag)
				jp->pi.clusters_to_process += f->f->disp.clusters;
		}
		if(f->next == jp->fragmented_files) break;
	}
	
	/* defragment all small files */
	winx_dbg_print_header(0,0,"defragmentation of %c: started",jp->volume_letter);
	time = winx_xtime();

	/* fill free regions in the beginning of the volume */
	defragmented_files = 0;
	for(rgn = jp->free_regions; rgn; rgn = rgn->next){
		if(jp->termination_router((void *)jp)) goto done;
		
		/* skip micro regions */
		if(rgn->length < 2) goto next_rgn;

		/* find largest fragmented file that fits in the current region */
		do {
			if(jp->termination_router((void *)jp)) goto done;
			f_largest = NULL, length = 0;
			for(f = jp->fragmented_files; f; f = f->next){
				if(f->f->disp.clusters > length && f->f->disp.clusters <= rgn->length && f->f->disp.blockmap){
					if(!is_directory(f->f) || jp->actions.allow_dir_defrag){
						f_largest = f;
						length = f->f->disp.clusters;
					}
				}
				if(f->next == jp->fragmented_files) break;
			}
			if(f_largest == NULL) goto next_rgn;
			
			/* move the file */
			if(move_file(f_largest->f,rgn->lcn,jp,fVolume) >= 0){
				DebugPrint("Defrag success for %ws\n",f_largest->f->path);
				defragmented_files ++;
			} else {
				DebugPrint("Defrag failure for %ws\n",f_largest->f->path);
			}

			/* skip locked files here to prevent skipping the current free region */
		} while(is_locked(f_largest->f));
		
		/* truncate list of fragmented files */
		update_fragmented_files_list(jp);
		
		/* after file moving continue from the first free region */
		rgn = jp->free_regions->prev;
		continue;
		
	next_rgn:
		if(rgn->next == jp->free_regions) break;
	}

	/* display amount of moved data and number of defragmented files */
	moved_clusters = jp->pi.moved_clusters;
	DebugPrint("%I64u files defragmented\n",defragmented_files);
	DebugPrint("%I64u clusters moved\n",moved_clusters);
	winx_fbsize(moved_clusters * jp->v_info.bytes_per_cluster,1,buffer,sizeof(buffer));
	DebugPrint("%s moved\n",buffer);
	/* display time needed for defragmentation */
	time2 = winx_xtime() - time;
	seconds = time2 / 1000;
	winx_time2str(seconds,buffer,sizeof(buffer));
	time2 -= seconds * 1000;
	winx_dbg_print_header(0,0,"defragmentation of %c: completed in %s %I64ums",
		jp->volume_letter,buffer,time2);
	
	/* defragment all large files partially */
	/* UD_FILE_TOO_LARGE flag should not be set for success of the operation */
	winx_dbg_print_header(0,0,"partial defragmentation of %c: started",jp->volume_letter);
	time = winx_xtime();
	
	/* fill largest free region first */
	/* TODO: add more termination points */
	defragmented_files = 0;
	do {
		joined_fragments = 0;
		
		/* find largest free space region */
		rgn_largest = NULL, length = 0;
		for(rgn = jp->free_regions; rgn; rgn = rgn->next){
			if(jp->termination_router((void *)jp)) goto done;
			if(rgn->length > length){
				rgn_largest = rgn;
				length = rgn->length;
			}
			if(rgn->next == jp->free_regions) break;
		}
		if(rgn_largest == NULL || rgn_largest->length < 2) break;
		
		/* find largest fragmented file which fragments can be joined there */
		do {
			f_largest = NULL, length = 0;
			for(f = jp->fragmented_files; f; f = f->next){
				if(f->f->disp.clusters > length && f->f->disp.blockmap && !is_too_large(f->f)){
					if(!is_directory(f->f) || jp->actions.allow_dir_defrag){
						f_largest = f;
						length = f->f->disp.clusters;
					}
				}
				if(f->next == jp->fragmented_files) break;
			}
			if(f_largest == NULL) goto part_defrag_done;
			
			/* find longest sequence of file blocks which fits in the current free region */
			longest_sequence = NULL, max_n_blocks = 0;
			for(first_block = f_largest->f->disp.blockmap; first_block; first_block = first_block->next){
				n_blocks = 0, remaining_clusters = rgn_largest->length;
				for(block = first_block; block; block = block->next){
					if(block->length <= remaining_clusters){
						n_blocks ++;
						remaining_clusters -= block->length;
					} else {
						break;
					}
					if(block->next == f_largest->f->disp.blockmap) break;
				}
				if(n_blocks > 1 && n_blocks > max_n_blocks){
					longest_sequence = first_block;
					max_n_blocks = n_blocks;
				}
				if(first_block->next == f_largest->f->disp.blockmap) break;
			}
			
			if(longest_sequence == NULL){
				/* fragments cannot be joined */
				f_largest->f->user_defined_flags |= UD_FILE_TOO_LARGE;
			} else {
				/* join fragments */
				if(move_file_blocks(f_largest->f,longest_sequence,max_n_blocks,rgn_largest->lcn,jp,fVolume) >= 0){
					DebugPrint("Partial defrag success for %ws\n",f_largest->f->path);
					joined_fragments += max_n_blocks;
					defragmented_files ++;
				} else {
					DebugPrint("Partial defrag failure for %ws\n",f_largest->f->path);
				}
			}
		} while(is_too_large(f_largest->f));
	} while(joined_fragments);

part_defrag_done:
	/* display amount of moved data and number of partially defragmented files */
	moved_clusters = jp->pi.moved_clusters - moved_clusters;
	DebugPrint("%I64u files partially defragmented\n",defragmented_files);
	DebugPrint("%I64u clusters moved\n",moved_clusters);
	winx_fbsize(moved_clusters * jp->v_info.bytes_per_cluster,1,buffer,sizeof(buffer));
	DebugPrint("%s moved\n",buffer);
	/* display time needed for partial defragmentation */
	time2 = winx_xtime() - time;
	seconds = time2 / 1000;
	winx_time2str(seconds,buffer,sizeof(buffer));
	time2 -= seconds * 1000;
	winx_dbg_print_header(0,0,"partial defragmentation of %c: completed in %s %I64ums",
		jp->volume_letter,buffer,time2);
	
	/* mark all files not processed yet as too large */
	for(f = jp->fragmented_files; f; f = f->next){
		if(jp->termination_router((void *)jp)) goto done;
		if(f->f->disp.blockmap){
			if(!is_directory(f->f) || jp->actions.allow_dir_defrag)
				f->f->user_defined_flags |= UD_FILE_TOO_LARGE;
		}
		if(f->next == jp->fragmented_files) break;
	}
	
	/* redraw all temporary system space as free */
	redraw_all_temporary_system_space_as_free(jp);

done:
	if(fVolume)
		winx_fclose(fVolume);
	return 0;
}

/******************************************************/
/*                  common routines                   */
/******************************************************/

/**
 * @brief Moves the file entirely.
 * @param[in] f the file to be moved.
 * @param[in] target the LCN 
 * of the target free region.
 * @param[in] jp job parameters.
 * @param[in] fVolume the volume descriptor.
 * @return Zero for success, negative value otherwise.
 */
int move_file(winx_file_info *f,ULONGLONG target,udefrag_job_parameters *jp,WINX_FILE *fVolume)
{
	NTSTATUS Status;
	HANDLE hFile;
	int result;
	winx_blockmap *block;
	
	/* open the file */
	Status = udefrag_fopen(f,&hFile);
	if(Status != STATUS_SUCCESS){
		DebugPrintEx(Status,"Cannot open %ws",f->path);
		/* redraw space */
		colorize_file_as_system(jp,f);
		/* file is locked by other application, so its state is unknown */
		/* don't reset its statistics though! */
		/*f->disp.clusters = 0;
		f->disp.fragments = 0;
		f->disp.flags = 0;*/
		winx_list_destroy((list_entry **)(void *)&f->disp.blockmap);
		f->user_defined_flags |= UD_FILE_LOCKED;
		jp->pi.processed_clusters += f->disp.clusters;
		if(jp->progress_router)
			jp->progress_router(jp); /* redraw progress */
		return (-1);
	}
	
	/* move the file */
	result = move_file_helper(hFile,f,target,jp,fVolume);
	if(result < 0)
		f->user_defined_flags |= UD_FILE_MOVING_FAILED;
	NtCloseSafe(hFile);
	
	/* update statistics */
	if(result >= 0){
		jp->pi.fragmented --;
		jp->pi.fragments -= f->disp.fragments;
		f->disp.flags &= ~WINX_FILE_DISP_FRAGMENTED;
	}
	
	/* redraw target space */
	colorize_map_region(jp,target,f->disp.clusters,
			get_file_color(jp,f),FREE_SPACE);
	if(jp->progress_router)
		jp->progress_router(jp); /* redraw map */
			
	/* remove target space from free space pool */
	jp->free_regions = winx_sub_volume_region(jp->free_regions,target,f->disp.clusters);
	
	/* redraw source space; after winx_sub_volume_region()! */
	if(result >= 0){
		for(block = f->disp.blockmap; block; block = block->next){
			redraw_freed_space(jp,block->lcn,block->length,FRAGM_SPACE);
			if(block->next == f->disp.blockmap) break;
		}
		if(jp->progress_router)
			jp->progress_router(jp); /* redraw map */
	}
	
	winx_list_destroy((list_entry **)(void *)&f->disp.blockmap);
	return result;
}

/**
 * @brief Moves blocks of the file.
 * @param[in] f the file to be moved.
 * @param[in] first_block the first block.
 * @param[in] n_blocks number of blocks to move.
 * @param[in] target the LCN of the target free region.
 * @param[in] jp job parameters.
 * @param[in] fVolume the volume descriptor.
 * @return Zero for success, negative value otherwise.
 */
int move_file_blocks(winx_file_info *f,winx_blockmap *first_block,
	ULONGLONG n_blocks,ULONGLONG target,udefrag_job_parameters *jp,WINX_FILE *fVolume)
{
	winx_blockmap *block, *next_block;
	ULONGLONG n, length;
	NTSTATUS Status;
	HANDLE hFile;
	int result;
	
	length = 0;
	for(block = first_block, n = 0; block; block = block->next, n++){
		length += block->length;
		if(block->next == f->disp.blockmap || n == (n_blocks - 1)) break;
	}
	
	/* open the file */
	Status = udefrag_fopen(f,&hFile);
	if(Status != STATUS_SUCCESS){
		DebugPrintEx(Status,"Cannot open %ws",f->path);
		/* redraw space */
		colorize_file_as_system(jp,f);
		/* file is locked by other application, so its state is unknown */
		/* don't reset its statistics though! */
		/*f->disp.clusters = 0;
		f->disp.fragments = 0;
		f->disp.flags = 0;*/
		winx_list_destroy((list_entry **)(void *)&f->disp.blockmap);
		f->user_defined_flags |= UD_FILE_LOCKED;
		jp->pi.processed_clusters += length;
		if(jp->progress_router)
			jp->progress_router(jp); /* redraw progress */
		return (-1);
	}
	
	/* TODO: move the file */
	result = move_file_blocks_helper(hFile,f,first_block,n_blocks,target,jp,fVolume);
	/* don't set UD_FILE_MOVING_FAILED here */
	NtCloseSafe(hFile);
	
	/* update statistics */
	if(result >= 0){
		jp->pi.fragments -= n_blocks;
	}
	
	/* redraw target space */
	colorize_map_region(jp,target,length,
			get_file_color(jp,f),FREE_SPACE);
	if(jp->progress_router)
		jp->progress_router(jp); /* redraw map */
			
	/* remove target space from free space pool */
	jp->free_regions = winx_sub_volume_region(jp->free_regions,target,length);
	
	/* redraw source space; after winx_sub_volume_region()! */
	if(result >= 0){
		for(block = first_block, n = 0; block; block = block->next, n++){
			redraw_freed_space(jp,block->lcn,block->length,FRAGM_SPACE);
			if(block->next == f->disp.blockmap || n == (n_blocks - 1)) break;
		}
		if(jp->progress_router)
			jp->progress_router(jp); /* redraw map */
	}
	
	/* adjust blockmap */
	first_block->lcn = target;
	first_block->length = length;
	for(block = first_block->next, n = 1; block; n++){
		next_block = block->next;
		winx_list_remove_item((list_entry **)(void *)&f->disp.blockmap,(list_entry *)(void *)block);
		if(next_block == f->disp.blockmap || n == (n_blocks - 1)) break;
		block = next_block;
	}
	f->disp.fragments -= (n_blocks - 1);
	return result;
}

/**
 * @brief Moves part of the file.
 * @return Zero for success,
 * negative value otherwise.
 * @note On NT4 this function can move
 * no more than 256 kilobytes once.
 */
static int move_file_part(HANDLE hFile,ULONGLONG startVcn,
	ULONGLONG targetLcn,ULONGLONG n_clusters,udefrag_job_parameters *jp,WINX_FILE *fVolume)
{
	NTSTATUS Status;
	IO_STATUS_BLOCK iosb;
	MOVEFILE_DESCRIPTOR mfd;

	DebugPrint("sVcn: %I64u,tLcn: %I64u,n: %u\n",
		 startVcn,targetLcn,n_clusters);

	if(jp->termination_router((void *)jp)) return (-1);
	
	if(dry_run){
		jp->pi.moved_clusters += n_clusters;
		return 0;
	}

	/* setup movefile descriptor and make the call */
	mfd.FileHandle = hFile;
	mfd.StartVcn.QuadPart = startVcn;
	mfd.TargetLcn.QuadPart = targetLcn;
#ifdef _WIN64
	mfd.NumVcns = n_clusters;
#else
	mfd.NumVcns = (ULONG)n_clusters;
#endif
	Status = NtFsControlFile(winx_fileno(fVolume),NULL,NULL,0,&iosb,
						FSCTL_MOVE_FILE,&mfd,sizeof(MOVEFILE_DESCRIPTOR),
						NULL,0);
	if(NT_SUCCESS(Status)){
		NtWaitForSingleObject(winx_fileno(fVolume),FALSE,NULL);
		Status = iosb.Status;
	}
	if(!NT_SUCCESS(Status)){
		DebugPrintEx(Status,"Cannot move file clusters");
		return (-1);
	}

	/*
	* Actually moving result is unknown here,
	* because API may return success, while
	* actually clusters may be moved partially.
	*/
	jp->pi.moved_clusters += n_clusters;
	return 0;
}

/**
 * @brief Prints list of file blocks.
 */
static void DbgPrintBlocksOfFile(winx_blockmap *blockmap)
{
	winx_blockmap *block;
	
	for(block = blockmap; block; block = block->next){
		DebugPrint("VCN: %I64u, LCN: %I64u, LENGTH: %u\n",
			block->vcn,block->lcn,block->length);
		if(block->next == blockmap) break;
	}
}

static int __stdcall dump_terminator(void *user_defined_data)
{
	udefrag_job_parameters *jp = (udefrag_job_parameters *)user_defined_data;

	return jp->termination_router((void *)jp);
}

/**
 * @brief move_file helper.
 */
static int move_file_helper(HANDLE hFile,winx_file_info *f,
	ULONGLONG target,udefrag_job_parameters *jp,WINX_FILE *fVolume)
{
	winx_blockmap *block;
	ULONGLONG curr_target, j, n, r;
	ULONGLONG clusters_to_process;
	winx_file_info f_cp;
	int result;
	
	DebugPrint("%ws\n",f->path);
	DebugPrint("t: %I64u n: %I64u\n",target,f->disp.clusters);

	clusters_to_process = f->disp.clusters;
	curr_target = target;
	for(block = f->disp.blockmap; block; block = block->next){
		/* try to move current block */
		n = block->length / jp->clusters_per_256k;
		for(j = 0; j < n; j++){
			result = move_file_part(hFile,block->vcn + j * jp->clusters_per_256k,
					curr_target,jp->clusters_per_256k,jp,fVolume);
			if(result < 0){
				jp->pi.processed_clusters += clusters_to_process;
				return result;
			}
			jp->pi.processed_clusters += jp->clusters_per_256k;
			clusters_to_process -= jp->clusters_per_256k;
			curr_target += jp->clusters_per_256k;
		}
		/* try to move rest of block */
		r = block->length % jp->clusters_per_256k;
		if(r){
			result = move_file_part(hFile,block->vcn + j * jp->clusters_per_256k,
					curr_target,r,jp,fVolume);
			if(result < 0){
				jp->pi.processed_clusters += clusters_to_process;
				return result;
			}
			jp->pi.processed_clusters += r;
			clusters_to_process -= r;
			curr_target += r;
		}
		if(block->next == f->disp.blockmap) break;
	}

	if(dry_run)
		return 0;

	/* check whether the file was actually moved or not */
	memcpy(&f_cp,f,sizeof(winx_file_info));
	f_cp.disp.blockmap = NULL;
	if(winx_ftw_dump_file(&f_cp,dump_terminator,(void *)jp) >= 0){
		if(is_fragmented(&f_cp)){
			DebugPrint("File moving failed: file is still fragmented\n");
			DbgPrintBlocksOfFile(f_cp.disp.blockmap);
			winx_list_destroy((list_entry **)(void *)&f_cp.disp.blockmap);
			return (-1);
		} else if(f_cp.disp.blockmap){
			if(f_cp.disp.blockmap->lcn != target){
				DebugPrint("File moving failed: file is not found on target space\n");
				DbgPrintBlocksOfFile(f_cp.disp.blockmap);
				winx_list_destroy((list_entry **)(void *)&f_cp.disp.blockmap);
				return (-1);
			}
		}
		winx_list_destroy((list_entry **)(void *)&f_cp.disp.blockmap);
	}

	return 0;
}

/**
 * @brief move_file_blocks helper.
 */
static int move_file_blocks_helper(HANDLE hFile,winx_file_info *f,
	winx_blockmap *first_block,ULONGLONG n_blocks,ULONGLONG target,
	udefrag_job_parameters *jp,WINX_FILE *fVolume)
{
	winx_blockmap *block;
	ULONGLONG curr_target, i, j, n, r;
	ULONGLONG clusters_to_process;
	winx_file_info f_cp;
	int result;
	int block_found;
	
	DebugPrint("%ws\n",f->path);
	DebugPrint("t: %I64u vcn: %I64u n_blocks: %I64u\n",target,first_block->vcn,n_blocks);

	clusters_to_process = 0;
	for(block = first_block, i = 0; block; block = block->next, i++){
		clusters_to_process += block->length;
		if(block->next == f->disp.blockmap || i == (n_blocks - 1)) break;
	}

	curr_target = target;
	for(block = first_block, i = 0; block; block = block->next, i++){
		/* try to move current block */
		n = block->length / jp->clusters_per_256k;
		for(j = 0; j < n; j++){
			result = move_file_part(hFile,block->vcn + j * jp->clusters_per_256k,
					curr_target,jp->clusters_per_256k,jp,fVolume);
			if(result < 0){
				jp->pi.processed_clusters += clusters_to_process;
				return result;
			}
			jp->pi.processed_clusters += jp->clusters_per_256k;
			clusters_to_process -= jp->clusters_per_256k;
			curr_target += jp->clusters_per_256k;
		}
		/* try to move rest of block */
		r = block->length % jp->clusters_per_256k;
		if(r){
			result = move_file_part(hFile,block->vcn + j * jp->clusters_per_256k,
					curr_target,r,jp,fVolume);
			if(result < 0){
				jp->pi.processed_clusters += clusters_to_process;
				return result;
			}
			jp->pi.processed_clusters += r;
			clusters_to_process -= r;
			curr_target += r;
		}
		if(block->next == f->disp.blockmap || i == (n_blocks - 1)) break;
	}

	if(dry_run)
		return 0;

	/* check whether the file was actually moved or not */
	memcpy(&f_cp,f,sizeof(winx_file_info));
	f_cp.disp.blockmap = NULL;
	if(winx_ftw_dump_file(&f_cp,dump_terminator,(void *)jp) >= 0){
		if(f_cp.disp.blockmap){
			block_found = 0;
			for(block = f_cp.disp.blockmap; block; block = block->next){
				if(block->lcn == target){
					block_found = 1;
					break;
				}
				if(block->next == f_cp.disp.blockmap) break;
			}
			if(!block_found){
				DebugPrint("File moving failed: file block is not found on target space\n");
				DbgPrintBlocksOfFile(f_cp.disp.blockmap);
				winx_list_destroy((list_entry **)(void *)&f_cp.disp.blockmap);
				return (-1);
			}
		}
		winx_list_destroy((list_entry **)(void *)&f_cp.disp.blockmap);
	}

	return 0;
}

/**
 * @brief Redraws freed space.
 */
static void redraw_freed_space(udefrag_job_parameters *jp,
		ULONGLONG lcn, ULONGLONG length, int old_color)
{
	/*
	* On FAT partitions after file moving filesystem driver marks
	* previously allocated clusters as free immediately.
	* But on NTFS they are always still allocated by system.
	* Only the next volume analysis frees them.
	*/
	if(jp->fs_type == FS_NTFS){
		/* mark clusters as temporarily allocated by system  */
		colorize_map_region(jp,lcn,length,TEMPORARY_SYSTEM_SPACE,old_color);
	} else {
		colorize_map_region(jp,lcn,length,FREE_SPACE,old_color);
		jp->free_regions = winx_add_volume_region(jp->free_regions,lcn,length);
	}
}

/**
 * @brief Removes unfragmented files
 * from the list of fragmented.
 */
void update_fragmented_files_list(udefrag_job_parameters *jp)
{
	udefrag_fragmented_file *f, *next, *head;
	
	f = head = jp->fragmented_files;
	while(f){
		next = f->next;
		if(!is_fragmented(f->f)){
			winx_list_remove_item((list_entry **)(void *)&jp->fragmented_files,(list_entry *)f);
			if(jp->fragmented_files == NULL)
				break; /* list is empty */
			if(jp->fragmented_files != head){
				head = jp->fragmented_files;
				f = next;
				continue;
			}
		}
		f = next;
		if(f == head) break;
	}
}

/** @} */