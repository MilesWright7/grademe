/* Weight-setting and scoring implementation for Naive-Bayes classification */

/* Copyright (C) 1997 Andrew McCallum

   Written by:  Andrew Kachites McCallum <mccallum@cs.cmu.edu>

   This file is part of the Bag-Of-Words Library, `libbow'.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License
   as published by the Free Software Foundation, version 2.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA */

#include <bow/libbow.h>
#include <math.h>

/* Function to assign `Naive Bayes'-style weights to each element of
   each document vector. */
void
bow_naivebayes_set_weights (bow_barrel *barrel)
{
  int ci;
  bow_cdoc *cdoc;
  int wi;			/* a "word index" into WI2DVF */
  int max_wi;			/* the highest "word index" in WI2DVF. */
  bow_dv *dv;			/* the "document vector" at index WI */
  int dvi;			/* an index into the DV */
  int weight_setting_num_words = 0;

  /* We assume that we have already called BOW_BARREL_NEW_VPC() on
     BARREL, so BARREL already has one-document-per-class. */
    
  assert (!strcmp (barrel->method->name, "naivebayes")
	  || !strcmp (barrel->method->name, "crossentropy"));
  max_wi = MIN (barrel->wi2dvf->size, bow_num_words());

  /* The CDOC->PRIOR should have been set in bow_barrel_new_vpc();
     verify it. */
  for (ci = 0; ci < barrel->cdocs->length; ci++)
    {
      cdoc = bow_array_entry_at_index (barrel->cdocs, ci);
      assert (cdoc->prior >= 0);
    }

#if 0
  /* For Shumeet, make all counts either 0 or 1. */
  for (wi = 0; wi < max_wi; wi++) 
    {
      dv = bow_wi2dvf_dv (barrel->wi2dvf, wi);
      if (dv == NULL)
	continue;
      for (dvi = 0; dvi < dv->length; dvi++) 
	{
	  assert (dv->entry[dvi].count);
	  dv->entry[dvi].count = 1;
	}
    }  
  /* And set uniform priors */
  for (ci = 0; ci < barrel->cdocs->length; ci++)
    {
      cdoc = bow_array_entry_at_index (barrel->cdocs, ci);
      cdoc->prior = 1.0;
    }
#endif

  /* Get the total number of terms in each class; store this in
     CDOC->WORD_COUNT. */
  for (ci = 0; ci < barrel->cdocs->length; ci++)
    {
      cdoc = bow_array_entry_at_index (barrel->cdocs, ci);
      cdoc->word_count = 0;
    }
  for (wi = 0; wi < max_wi; wi++) 
    {
      dv = bow_wi2dvf_dv (barrel->wi2dvf, wi);
      if (dv == NULL)
	continue;
      for (dvi = 0; dvi < dv->length; dvi++) 
	{
	  cdoc = bow_array_entry_at_index (barrel->cdocs, 
					   dv->entry[dvi].di);
	  cdoc->word_count += dv->entry[dvi].count;
	}
    }

  /* Set the weights in the BARREL's WI2DVF so that they are
     equal to P(w|C), the probability of a word given a class. */
  for (wi = 0; wi < max_wi; wi++) 
    {
      dv = bow_wi2dvf_dv (barrel->wi2dvf, wi);

      /* If the model doesn't know about this word, skip it. */
      if (dv == NULL)
	continue;

      /* Now loop through all the elements, setting their weights */
      for (dvi = 0; dvi < dv->length; dvi++) 
	{
	  cdoc = bow_array_entry_at_index (barrel->cdocs, 
					   dv->entry[dvi].di);
	  /* Here CDOC->WORD_COUNT is the total number of words in the class */
	  /* We use Laplace Estimation. */
	  dv->entry[dvi].weight = ((float)
				   (1 + dv->entry[dvi].count)
				   / (barrel->wi2dvf->num_words
				      + cdoc->word_count));
	  assert (dv->entry[dvi].weight > 0);
	}
      weight_setting_num_words++;
      /* Set the IDF.  NaiveBayes doesn't use it; make it have no effect */
      dv->idf = 1.0;
    }
#if 0
  fprintf (stderr, "wi2dvf num_words %d, weight-setting num_words %d\n",
	   barrel->wi2dvf->num_words, weight_setting_num_words);
#endif
}

/* For changing weight of unseen words.
   I really should implement `deleted interpolation' */
/* M_EST_P summed over all words in the vocabulary must sum to 1.0! */
#if 0
/* This is the special case of the M-estimate that is `Laplace smoothing' */
#define M_EST_M  (barrel->wi2dvf->num_words)
#define M_EST_P  (1.0 / barrel->wi2dvf->num_words)
#define WORD_PRIOR_COUNT 1.0
#else
#define M_EST_M  (cdoc->word_count \
		  ? (((float)barrel->wi2dvf->num_words) / cdoc->word_count) \
		  : 1.0)
#define M_EST_P  (1.0 / barrel->wi2dvf->num_words)
#endif

int
bow_naivebayes_score_loo (bow_barrel *barrel, bow_wv *query_wv, 
			  bow_score *bscores, int bscores_len,
			  int loo_class)
{
  double *scores;		/* will become prob(class), indexed over CI */
  int ci;			/* a "class index" (document index) */
  int wvi;			/* an index into the entries of QUERY_WV. */
  int dvi;			/* an index into a "document vector" */
  float pr_w_c;			/* P(w|C), prob a word is in a class */
  double pr_tf;			/* P(w|C)^TF, ditto, by occurr's in QUERY_WV */
  double log_pr_tf;		/* log(P(w|C)^TF), ditto, log() of it */
  double rescaler;		/* Rescale SCORES by this after each word */
  double new_score;		/* a temporary holder */
  int num_scores;		/* number of entries placed in SCORES */

  /* Allocate space to store scores for *all* classes (documents) */
  scores = alloca (barrel->cdocs->length * sizeof (double));

  /* Instead of multiplying probabilities, we will sum up
     log-probabilities, (so we don't loose floating point resolution),
     and then take the exponent of them to get probabilities back. */

  /* Initialize the SCORES to the class prior probabilities. */
  if (bow_print_word_scores)
    printf ("%s\n",
	    "(CLASS PRIOR PROBABILIES)");
  for (ci = 0; ci < barrel->cdocs->length; ci++)
    {
      bow_cdoc *cdoc;
      cdoc = bow_array_entry_at_index (barrel->cdocs, ci);
      if (bow_uniform_class_priors)
	scores[ci] = 1;
      else
	{
	  /* LOO_CLASS is not implemented for cases in which we are
	     not doing uniform class priors. */
	  assert (loo_class == -1);
	  assert (cdoc->prior > 0.0f && cdoc->prior <= 1.0f);
	  scores[ci] = log (cdoc->prior);
	  if (((bow_params_naivebayes*)(barrel->method->params))
	      ->score_with_log_probabilities == bow_yes)
	    scores[ci] = - scores[ci];
	}
      assert (scores[ci] > -FLT_MAX + 1.0e5);
      if (bow_print_word_scores)
	printf ("%16s %-40s  %10.9f\n", 
		"",
		(strrchr (cdoc->filename, '/') ? : cdoc->filename),
		scores[ci]);
    }

  /* Loop over each word in the word vector QUERY_WV, putting its
     contribution into SCORES. */
  for (wvi = 0; wvi < query_wv->num_entries; wvi++)
    {
      int wi;			/* the word index for the word at WVI */
      bow_dv *dv;		/* the "document vector" for the word WI */

      /* Get information about this word. */
      wi = query_wv->entry[wvi].wi;
      dv = bow_wi2dvf_dv (barrel->wi2dvf, wi);

      /* If the model doesn't know about this word, skip it. */
      if (!dv)
	continue;

      if (bow_print_word_scores)
	printf ("%-30s (queryweight=%.8f)\n",
		bow_int2word (wi), 
		query_wv->entry[wvi].weight * query_wv->normalizer);

      rescaler = DBL_MAX;

      /* Loop over all classes, putting this word's (WI's)
	 contribution into SCORES. */
      for (ci = 0, dvi = 0; ci < barrel->cdocs->length; ci++)
	{
	  /* Both these values are pretty arbitrary small numbers. */
	  static const double min_pr_tf = FLT_MIN * 1.0e5;
	  bow_cdoc *cdoc;

	  cdoc = bow_array_entry_at_index (barrel->cdocs, ci);
	  assert (cdoc->type == model);

	  /* Assign PR_W_C to P(w|C), either using a DV entry, or, if
	     there is no DV entry for this class, using M-estimate 
	     smoothing */
	  if (dv)
	    while (dvi < dv->length && dv->entry[dvi].di < ci)
	      dvi++;
	  if (dv && dvi < dv->length && dv->entry[dvi].di == ci)
	    {
	      if (loo_class == ci)
		{
		  /* xxx This is not exactly right, because 
		     BARREL->WI2DVF->NUM_WORDS might have changed with the
		     removal of QUERY_WV's document. */
		  pr_w_c = ((float)
			    ((M_EST_M * M_EST_P) + dv->entry[dvi].count 
			     - query_wv->entry[wvi].count)
			    / (M_EST_M + cdoc->word_count
			       - query_wv->entry[wvi].count));
		  assert (pr_w_c > 0 && pr_w_c <= 1);
		}
	      else
		{
		  pr_w_c = ((float)
			    ((M_EST_M * M_EST_P) + dv->entry[dvi].count)
			    / (M_EST_M + cdoc->word_count));
		  assert (pr_w_c > 0 && pr_w_c <= 1);
		}
	    }
	  else
	    {
	      if (loo_class == ci)
		{
		  /* xxx This is not exactly right, because 
		     BARREL->WI2DVF->NUM_WORDS might have changed with the
		     removal of QUERY_WV's document. */
		  pr_w_c = ((M_EST_M * M_EST_P)
			    / (M_EST_M + cdoc->word_count
			       - query_wv->entry[wvi].count));
		  assert (pr_w_c > 0 && pr_w_c <= 1);
		}
	      else
		{
		  pr_w_c = ((M_EST_M * M_EST_P)
			    / (M_EST_M + cdoc->word_count));
		  assert (pr_w_c > 0 && pr_w_c <= 1);
		}
	    }
	  assert (pr_w_c > 0 && pr_w_c <= 1);

	  /* Take into consideration the number of times it occurs in 
	     the query document */
	  pr_tf = pow (pr_w_c, query_wv->entry[wvi].count);
	  /* PR_TF can be zero due to round-off error, when PR_W_C is
	     very small and QUERY_WV->ENTRY[CURRENT_INDEX].COUNT is
	     very large.  Here we fudgingly avoid this by insisting
	     that PR_TF not go below some arbitrary small number. */
	  if (pr_tf < min_pr_tf)
	    pr_tf = min_pr_tf;

	  log_pr_tf = log (pr_tf);
	  assert (log_pr_tf > -FLT_MAX + 1.0e5);

	  if (((bow_params_naivebayes*)(barrel->method->params))
	      ->score_with_log_probabilities == bow_yes)
	    log_pr_tf = -log_pr_tf;

	  scores[ci] += log_pr_tf;

	  if (bow_print_word_scores)
	    printf (" %8.2e %7.2f %-40s  %10.9f\n", 
		    pr_w_c,
		    log_pr_tf, 
		    (strrchr (cdoc->filename, '/') ? : cdoc->filename),
		    scores[ci]);

	  /* Keep track of the minimum score updated for this word. */
	  if (rescaler > scores[ci])
	    rescaler = scores[ci];
	}

      if (((bow_params_naivebayes*)(barrel->method->params))
	  ->score_with_log_probabilities == bow_no)
	{
	  /* Loop over all classes, re-scaling SCORES so that they
	     don't get so small we loose floating point resolution.
	     This scaling always keeps all SCORES positive. */
	  if (rescaler < 0)
	    {
	      for (ci = 0; ci < barrel->cdocs->length; ci++)
		{
		  /* Add to SCORES to bring them close to zero.  RESCALER is
		     expected to often be less than zero here. */
		  /* xxx If this doesn't work, we could keep track of the min
		     and the max, and sum by their average. */
		  scores[ci] += -rescaler;
		  assert (scores[ci] > -DBL_MAX + 1.0e5
			  && scores[ci] < DBL_MAX - 1.0e5);
		}
	    }
	}
    }
  /* Now SCORES[] contains a (unnormalized) log-probability for each class. */

  if (((bow_params_naivebayes*)(barrel->method->params))
      ->score_with_log_probabilities == bow_no)
    {
      /* Rescale the SCORE one last time, this time making them all 0 or
     negative, so that exp() will work well, especially around the
     higher-probability classes. */
      {
	rescaler = -DBL_MAX;
	for (ci = 0; ci < barrel->cdocs->length; ci++)
	  if (scores[ci] > rescaler) 
	    rescaler = scores[ci];
	/* RESCALER is now the maximum of the SCORES. */
	for (ci = 0; ci < barrel->cdocs->length; ci++)
	  scores[ci] -= rescaler;
      }

      /* Use exp() on the SCORES to get probabilities from
         log-probabilities. */
      for (ci = 0; ci < barrel->cdocs->length; ci++)
	{
	  new_score = exp (scores[ci]);
	  /* assert (new_score > 0 && new_score < DBL_MAX - 1.0e5); */
	  scores[ci] = new_score;
	}
    }
  else
    {
      for (ci = 0; ci < barrel->cdocs->length; ci++)
	scores[ci] = 1.0 / scores[ci];
    }

  /* Normalize the SCORES so they all sum to one. */
  {
    double scores_sum = 0;
    for (ci = 0; ci < barrel->cdocs->length; ci++)
      scores_sum += scores[ci];
    for (ci = 0; ci < barrel->cdocs->length; ci++)
      {
	scores[ci] /= scores_sum;
	/* assert (scores[ci] > 0); */
      }
  }

  /* Return the SCORES by putting them (and the `class indices') into
     SCORES in sorted order. */
  {
    num_scores = 0;
    for (ci = 0; ci < barrel->cdocs->length; ci++)
      {
	if (num_scores < bscores_len
	    || bscores[num_scores-1].weight < scores[ci])
	  {
	    /* We are going to put this score and CI into SCORES
	       because either: (1) there is empty space in SCORES, or
	       (2) SCORES[CI] is larger than the smallest score there
	       currently. */
	    int dsi;		/* an index into SCORES */
	    if (num_scores < bscores_len)
	      num_scores++;
	    dsi = num_scores - 1;
	    /* Shift down all the entries that are smaller than SCORES[CI] */
	    for (; dsi > 0 && bscores[dsi-1].weight < scores[ci]; dsi--)
	      bscores[dsi] = bscores[dsi-1];
	    /* Insert the new score */
	    bscores[dsi].weight = scores[ci];
	    bscores[dsi].di = ci;
	  }
      }
  }

  return num_scores;
}

int
bow_naivebayes_score (bow_barrel *barrel, bow_wv *query_wv, 
		      bow_score *bscores, int bscores_len)
{
  return bow_naivebayes_score_loo (barrel, query_wv, 
				   bscores, bscores_len,
				   -1);
}

bow_params_naivebayes bow_naivebayes_params =
{
  bow_no,			/* no uniform priors */
  bow_yes,			/* normalize_scores */
  bow_no			/* score with probabilities, not log-probs */
};

bow_method bow_method_naivebayes = 
{
  "naivebayes",
  bow_naivebayes_set_weights,
  0,				/* no weight scaling function */
  NULL, /* bow_barrel_normalize_weights_by_summing, */
  bow_barrel_new_vpc_merge_then_weight,
  bow_barrel_set_vpc_priors_by_counting,
  bow_naivebayes_score,
  bow_wv_set_weights_to_count,
  NULL,				/* no need for extra weight normalization */
  &bow_naivebayes_params
};

void _register_method_naivebayes () __attribute__ ((constructor));
void _register_method_naivebayes ()
{
  bow_method_register_with_name (&bow_method_naivebayes, "naivebayes");
}


bow_params_naivebayes bow_crossentropy_params =
{
  bow_no,			/* no uniform priors */
  bow_yes,			/* normalize_scores */
  bow_yes			/* score with probabilities, not log-probs */
};

bow_method bow_method_crossentropy = 
{
  "crossentropy",
  bow_naivebayes_set_weights,
  0,				/* no weight scaling function */
  NULL,				/* no weight normalizing function */
  bow_barrel_new_vpc_merge_then_weight,
  bow_barrel_set_vpc_priors_by_counting,
  bow_naivebayes_score,
  bow_wv_set_weights_to_count,
  NULL,				/* no need for extra weight normalization */
  &bow_crossentropy_params
};

void _register_method_crossentropy () __attribute__ ((constructor));
void _register_method_crossentropy ()
{
  bow_method_register_with_name (&bow_method_crossentropy, "crossentropy");
}