/*
 *  GEM-Mapper v3 (GEM3)
 *  Copyright (c) 2011-2017 by Santiago Marco-Sola  <santiagomsola@gmail.com>
 *
 *  This file is part of GEM-Mapper v3 (GEM3).
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * PROJECT: GEM-Mapper v3 (GEM3)
 * AUTHOR(S): Santiago Marco-Sola <santiagomsola@gmail.com>
 */

#ifndef MATCH_ALIGN_H_
#define MATCH_ALIGN_H_

#include "utils/essentials.h"
#include "matches/matches.h"
#include "matches/align/match_alignment.h"
#include "matches/scaffold/match_scaffold.h"

/*
 * Match Clipping
 */
void match_aling_add_clipping(
    match_trace_t* const match_trace,
    vector_t* const cigar_vector,
    const uint64_t sequence_clip_left,
    const uint64_t sequence_clip_right);

/*
 * Exact Alignment
 *   @align_input->key_length
 *   @align_input->text_position
 *   @align_input->text_trace_offset
 *   @align_input->text
 *   @align_input->text_offset_begin
 *   @align_input->text_offset_end
 *   @align_parameters->swg_penalties
 *   @match_trace->match_alignment.score
 */
void match_align_exact(
    matches_t* const matches,
    match_trace_t* const match_trace,
    match_align_input_t* const align_input,
    match_align_parameters_t* const align_parameters);

/*
 * Hamming Alignment (Only mismatches)
 *   @align_input->key
 *   @align_input->key_length
 *   @align_input->text_position
 *   @align_input->text_trace_offset
 *   @align_input->text
 *   @align_input->text_offset_begin
 *   @align_input->text_offset_end
 *   @align_parameters->allowed_enc
 */
void match_align_hamming(
    matches_t* const matches,
    match_trace_t* const match_trace,
    match_align_input_t* const align_input,
    match_align_parameters_t* const align_parameters);

/*
 * Levenshtein Alignment (edit distance)
 *   @align_input->key
 *   @align_input->bpm_pattern
 *   @align_input->text_trace_offset
 *   @align_input->text_position
 *   @align_input->text
 *   @align_input->text_offset_begin
 *   @align_input->text_offset_end
 *   @align_input->text_length
 *   @align_parameters->max_error
 *   @align_parameters->left_gap_alignment
 */
void match_align_levenshtein(
    matches_t* const matches,
    match_trace_t* const match_trace,
    match_align_input_t* const align_input,
    match_align_parameters_t* const align_parameters,
    mm_stack_t* const mm_stack);

/*
 * SWG Alignment (Gap-affine)
 *   @align_input->key
 *   @align_input->key_length
 *   @align_input->text_trace_offset
 *   @align_input->text_position
 *   @align_input->text
 *   @align_input->text_length
 *   @align_input->text_offset_begin
 *   @align_input->text_offset_end
 *   @align_parameters->swg_penalties
 *   @align_parameters->swg_threshold
 *   @align_parameters->min_identity
 *   @align_parameters->left_gap_alignment
 *   @align_parameters->allowed_enc
 *   @align_parameters->max_bandwidth
 *   @align_parameters->scaffolding
 *   @align_parameters->scaffolding_min_coverage
 *   @align_parameters->scaffolding_matching_min_length
 *   @align_parameters->scaffolding_homopolymer_min_context
 *   @align_parameters->cigar_curation
 *   @align_parameters->cigar_curation_min_end_context
 *   @match_scaffold->alignment_regions
 *   @match_scaffold->num_alignment_regions
 */
void match_align_smith_waterman_gotoh(
    matches_t* const matches,
    match_trace_t* const match_trace,
    match_align_input_t* const align_input,
    match_align_parameters_t* const align_parameters,
    match_scaffold_t* const match_scaffold,
    mm_stack_t* const mm_stack);

#endif /* MATCH_ALIGN_H_ */
