/*
 * PROJECT: GEMMapper
 * FILE: archive_search.c
 * DATE: 06/06/2012
 * AUTHOR(S): Santiago Marco-Sola <santiagomsola@gmail.com>
 */

#include "archive_search.h"
#include "archive_select.h"
#include "archive_score.h"
#include "matches_classify.h"

/*
 * Debug
 */
#define DEBUG_ARCHIVE_SEARCH_READ_NAME GEM_DEEP_DEBUG

/*
 * Setup
 */
GEM_INLINE archive_search_t* archive_search_new(
    archive_t* const archive,search_parameters_t* const search_parameters,
    select_parameters_t* const select_parameters) {
  ARCHIVE_CHECK(archive);
  // Allocate handler
  archive_search_t* const archive_search = mm_alloc(archive_search_t);
  // Archive
  archive_search->archive = archive;
  // Sequence
  sequence_init(&archive_search->sequence);
  sequence_init(&archive_search->rc_sequence);
  // Approximate Search
  archive_search->as_parameters.search_parameters = search_parameters;
  archive_search->select_parameters = select_parameters;
  approximate_search_init(
      &archive_search->forward_search_state,archive,
      &archive_search->as_parameters,false);
  approximate_search_init(
      &archive_search->reverse_search_state,archive,
      &archive_search->as_parameters,true);
  // Archive search control (Flow control) [DEFAULTS]
  archive_search->probe_strand = true;
  archive_search->emulate_rc_search = !archive->indexed_complement;
  // Return
  return archive_search;
}
GEM_INLINE void archive_search_configure(
    archive_search_t* const archive_search,const sequence_end_t sequence_end,
    mm_search_t* const mm_search) {
  // Configure F/R search states
  switch (sequence_end) {
    case single_end:
    case paired_end1:
      approximate_search_configure(
          &archive_search->forward_search_state,&mm_search->filtering_candidates_forward_end1,
          &mm_search->text_collection,&mm_search->interval_set,mm_search->mm_stack);
      approximate_search_configure(
          &archive_search->reverse_search_state,&mm_search->filtering_candidates_reverse_end1,
          &mm_search->text_collection,&mm_search->interval_set,mm_search->mm_stack);
      break;
    case paired_end2:
      approximate_search_configure(
          &archive_search->forward_search_state,&mm_search->filtering_candidates_forward_end2,
          &mm_search->text_collection,&mm_search->interval_set,mm_search->mm_stack);
      approximate_search_configure(
          &archive_search->reverse_search_state,&mm_search->filtering_candidates_reverse_end2,
          &mm_search->text_collection,&mm_search->interval_set,mm_search->mm_stack);
      break;
    default:
      GEM_INVALID_CASE();
      break;
  }
  // Text-Collection
  archive_search->text_collection = &mm_search->text_collection;
  // Stats
  archive_search->mapper_stats = mm_search->mapper_stats;
  // MM
  archive_search->mm_stack = mm_search->mm_stack;
}
GEM_INLINE void archive_search_prepare_sequence(archive_search_t* const archive_search) {
  PROF_START(GP_ARCHIVE_SEARCH_PREPARE_SEQUENCE);
  // Check the index characteristics & generate reverse-complement (if needed)
  if (archive_search->archive->indexed_complement) {
    archive_search->emulate_rc_search = false;
  } else {
    if (archive_search->archive->filter_type == Iupac_dna) {
      sequence_generate_reverse_complement(&archive_search->sequence,&archive_search->rc_sequence);
    } else {
      sequence_generate_reverse(&archive_search->sequence,&archive_search->rc_sequence);
    }
    archive_search->emulate_rc_search = !sequence_equals(&archive_search->sequence,&archive_search->rc_sequence);
  }
  // Generate the pattern(s)
  const bool prepare_rl_pattern = archive_search->archive->text->run_length;
  approximate_search_t* const forward_search_state = &archive_search->forward_search_state;
  pattern_prepare(&archive_search->sequence,&forward_search_state->pattern,
      &forward_search_state->region_profile,forward_search_state->as_parameters,
      prepare_rl_pattern,&forward_search_state->do_quality_search,archive_search->mm_stack);
  if (archive_search->emulate_rc_search) {
    approximate_search_t* const reverse_search_state = &archive_search->reverse_search_state;
    pattern_prepare(&archive_search->rc_sequence,&reverse_search_state->pattern,
        &reverse_search_state->region_profile,reverse_search_state->as_parameters,
        prepare_rl_pattern,&reverse_search_state->do_quality_search,archive_search->mm_stack);
  } else {
    pattern_clear(&archive_search->reverse_search_state.pattern);
  }
  PROF_STOP(GP_ARCHIVE_SEARCH_PREPARE_SEQUENCE);
}
GEM_INLINE void archive_search_reset(archive_search_t* const archive_search) {
  PROF_START(GP_ARCHIVE_SEARCH_SE_INIT);
  // Instantiate parameters actual-values
  const uint64_t sequence_length = sequence_get_length(&archive_search->sequence);
  search_instantiate_values(&archive_search->as_parameters,sequence_length);
  // Prepare for sequence
  archive_search_prepare_sequence(archive_search);
  // Clear F/R search states
  approximate_search_reset(&archive_search->forward_search_state);
  if (archive_search->emulate_rc_search) {
    approximate_search_reset(&archive_search->reverse_search_state);
  }
  PROF_STOP(GP_ARCHIVE_SEARCH_SE_INIT);
}
GEM_INLINE void archive_search_delete(archive_search_t* const archive_search) {
  // Delete Sequence
  sequence_destroy(&archive_search->sequence);
  sequence_destroy(&archive_search->rc_sequence);
  // Destroy search states
  approximate_search_destroy(&archive_search->forward_search_state);
  approximate_search_destroy(&archive_search->reverse_search_state);
  // Free handler
  mm_free(archive_search);
}
/*
 * Accessors
 */
GEM_INLINE sequence_t* archive_search_get_sequence(const archive_search_t* const archive_search) {
  return (sequence_t*)&archive_search->sequence;
}
GEM_INLINE uint64_t archive_search_get_search_canditates(const archive_search_t* const archive_search) {
  if (archive_search->archive->indexed_complement) {
    return approximate_search_get_num_filtering_candidates(&archive_search->forward_search_state);
  } else {
    return approximate_search_get_num_filtering_candidates(&archive_search->forward_search_state) +
           approximate_search_get_num_filtering_candidates(&archive_search->reverse_search_state);
  }
}
GEM_INLINE uint64_t archive_search_get_search_exact_matches(const archive_search_t* const archive_search) {
  if (archive_search->archive->indexed_complement) {
    return approximate_search_get_num_exact_filtering_candidates(&archive_search->forward_search_state);
  } else {
    return approximate_search_get_num_exact_filtering_candidates(&archive_search->forward_search_state) +
           approximate_search_get_num_exact_filtering_candidates(&archive_search->reverse_search_state);
  }
}
GEM_INLINE uint64_t archive_search_get_max_region_length(const archive_search_t* const archive_search) {
  if (archive_search->archive->indexed_complement) {
    return archive_search->forward_search_state.region_profile.max_region_length;
  } else {
    return MAX(archive_search->forward_search_state.region_profile.max_region_length,
               archive_search->reverse_search_state.region_profile.max_region_length);
  }
}
GEM_INLINE uint64_t archive_search_get_num_zero_regions(const archive_search_t* const archive_search) {
  if (archive_search->archive->indexed_complement) {
    return archive_search->forward_search_state.region_profile.num_zero_regions;
  } else {
    return MAX(archive_search->forward_search_state.region_profile.num_zero_regions,
               archive_search->reverse_search_state.region_profile.num_zero_regions);
  }
}
GEM_INLINE bool archive_search_finished(const archive_search_t* const archive_search) {
  if (archive_search->archive->indexed_complement) {
    return archive_search->forward_search_state.search_state == asearch_end;
  } else {
    return archive_search->forward_search_state.search_state == asearch_end &&
           archive_search->reverse_search_state.search_state == asearch_end;
  }
}
/*
 * Utils
 */
GEM_INLINE void archive_search_hold_verification_candidates(archive_search_t* const archive_search) {
  approximate_search_hold_verification_candidates(&archive_search->forward_search_state);
  if (archive_search->emulate_rc_search) {
    approximate_search_hold_verification_candidates(&archive_search->reverse_search_state);
  }
}
GEM_INLINE void archive_search_release_verification_candidates(archive_search_t* const archive_search) {
  approximate_search_release_verification_candidates(&archive_search->forward_search_state);
  if (archive_search->emulate_rc_search) {
    approximate_search_release_verification_candidates(&archive_search->reverse_search_state);
  }
}
