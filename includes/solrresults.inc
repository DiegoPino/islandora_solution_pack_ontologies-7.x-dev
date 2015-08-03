<?php

/**
 * @file
 * Contains methods to create rendered Solr displays from raw Solr results.
 * Depends on Apache_Solr_Php client.
 */
class IslandoraOntologiesSolrResults extends IslandoraSolrResults{

 
  public function displayFacets($islandora_solr_query) {
    IslandoraOntologiesSolrFacets::init($islandora_solr_query);
    $output = '';
    $facet_order = $this->facetFieldArray;
   
    foreach ($facet_order as $facet_key => $facet_label) {
      $facet_obj = new IslandoraOntologiesSolrFacets($facet_key);
      $output .= $facet_obj->getFacet();
    }

    // As we add additional facets, we're repeatedly URL-encoding old facet
    // strings. when we double-encode quotation marks they're incomprehensible
    // to Solr.
    //$output = str_replace('%2B', '%252B', $output);

    return $output;
  }

}

class IslandoraOntologiesSolrFacets extends IslandoraSolrFacets {
  
  
  public static function init($islandora_solr_query) {
    error_log('init Solr facets Ontologies');
    parent::$islandoraSolrQuery = $islandora_solr_query;
    
   parent::$facet_fields = isset($islandora_solr_query->islandoraSolrResult) ? $islandora_solr_query->islandoraSolrResult['facet_counts']['facet_fields'] : array();
    parent::$facet_dates = isset($islandora_solr_query->islandoraSolrResult) ? $islandora_solr_query->islandoraSolrResult['facet_counts']['facet_dates'] : array();
    error_log(var_export(static::$facet_dates,true));
    // Not in place yet.
    // XXX: isset() checking, as older Solrs (before 3.1) won't return a value.
    static::$facet_ranges = isset($islandora_solr_query->islandoraSolrResult['facet_counts']['facet_ranges']) ?
      $islandora_solr_query->islandoraSolrResult['facet_counts']['facet_ranges'] :
      array();

    // Filtered, not simplified and fields as keys.
    parent::$facet_fields_settings = islandora_solr_get_fields('facet_fields', TRUE, FALSE, TRUE);
    parent::$facet_fields_settings_simple = _islandora_solr_simplify_fields(self::$facet_fields_settings);
    parent::$range_facets = islandora_solr_get_range_facets();
    parent::$soft_limit = variable_get('islandora_solr_facet_soft_limit', '10');
   parent::$exclude_range_values = array(
      'gap',
      'start',
      'end',
      'other',
      'hardend',
      'include',
    );

    // Calculate variable date gap.
    // @XXX move elsewhere?
  self::variableDateGap();
  }

  public static function variableDateGap() {
    // 1: Gather info.
    // @todo Move this to separate functions.
    // Check settings.
    $facet_fields_settings = self::$facet_fields_settings;
    $variable_date_gap = array();
    foreach ($facet_fields_settings as $key => $settings) {
      if (isset($settings['solr_field_settings']['range_facet_variable_gap']) && $settings['solr_field_settings']['range_facet_variable_gap'] == 1) {
        $variable_date_gap[] = $settings['solr_field'];
      }
    }

    $islandora_solr_query = self::$islandoraSolrQuery;
    $fq = isset($islandora_solr_query->solrParams['fq']) ? $islandora_solr_query->solrParams['fq'] : array();
    $facet_dates = self::$facet_dates;
    // Populate with terms that needed a second solr call to update facets.
    error_log('date gap calc');
    error_log(var_export($facet_dates,true));
   
    $needs_solr_call = array();
    // Loop over all date facets.
    foreach ($facet_dates as $solr_field => $buckets) {
      $values = array();
      // Loop over all filters.
      foreach ($fq as $key => $filter) {
        // Check for enabled range filters.
        if (strpos($filter, $solr_field) === FALSE) {
          continue;
        }
        // Don't include excluded ranges for now, because it's complicated.
        // Maybe keep it like this for Solr 1.4. Solr 3.5 and up would
        // calculate this through the StatsComponent.
        if (substr($filter, 0, 1) == '-') {
          continue;
           error_log('continue2');
        }
        // @todo This could be done differently?
        // Check 'variable range gap' settings for this field.
        error_log($solr_field);
        error_log($filter);
        
        if (!in_array($solr_field, $variable_date_gap)) {
          continue;
        }
        // Split the filter into field and value.
        $filter_split = preg_split(ISLANDORA_SOLR_QUERY_FIELD_VALUE_SPLIT_REGEX, $filter, 2);
        // Trim brackets.
        $filter_split[1] = trim($filter_split[1], "\"");
        // Split range filter string to return formatted date values.
        $filter_str = $filter_split[1];
        $filter_str = trim($filter_str, '[');
        $filter_str = trim($filter_str, ']');
        $filter_array = explode(' TO ', $filter_str);
        // Collect values in array: timestamp => ISO8601.
        $from_str = trim($filter_array[0]);
        $to_str = trim($filter_array[1]);
        $field = trim($filter_split[0], ' -');
        // If a star is given, we need to perform an extra query to find out
        // what the minimum or maximum value is that 'star' would return.
        if (strpos($from_str, '*') !== FALSE) {
          $from_str = self::findMinMaxValue($field, 'asc');
        }
        if (strpos($to_str, '*') !== FALSE) {
          $to_str = self::findMinMaxValue($field, 'desc');
        }
        $values['from'][strtotime(strtolower($from_str))] = $from_str;
        $values['to'][strtotime(strtolower($to_str))] = $to_str;
      }

      // If the date facet field is found as at least one range filter,
      // calculate gap.
      if (empty($values)) {
        continue;
      }

      // From max value & to min value.
      $from_unix = max(array_keys($values['from']));
      $to_unix = min(array_keys($values['to']));

      // If the from date is bigger than the to date, abort.
      if ($from_unix >= $to_unix) {
        continue;
      }

      // Get ISO8601 values.
      $from = $values['from'][$from_unix];
      // If the hour is 00:00:00, subtract one second. If we always subtract
      // one second we keep eating away seconds after every filter.
      if (strpos($from, '00:00:00') !== FALSE) {
        $from .= '-1SECOND';
      }
      $to = $values['to'][$to_unix];

      // 2: Calculate and render range gap.
      // Calculate difference between from and to date.
      $diff = abs($from_unix - $to_unix);

      // Total difference in days.
      $total_days = floor($diff / 60 / 60 / 24);

      // @todo Make max buckets variable.
      // @todo Fine tune this: it's not very precise.
      // For maximum 15 buckets.
      if ($total_days <= 15) {
        $gap = '+1DAY';
      }
      elseif ($total_days <= 105) {
        $gap = '+7DAYS';
      }
      elseif ($total_days <= 450) {
        $gap = '+1MONTH';
      }
      elseif ($total_days <= 5475) {
        $gap = '+1YEAR';
      }
      elseif ($total_days <= 10950) {
        $gap = '+2YEARS';
      }
      elseif ($total_days <= 18250) {
        $gap = '+5YEARS';
      }
      elseif ($total_days <= 54750) {
        $gap = '+10YEARS';
      }
      elseif ($total_days <= 547500) {
        $gap = '+100YEARS';
      }
      elseif ($total_days <= 5475000) {
        $gap = '+1000YEARS';
      }

      // @todo Try to find a way to clone this object, because it's passed by
      // reference throughout the entire page call.
      // Update range facet values.
      $islandora_solr_query->solrParams["f.$solr_field.facet.date.start"] = $from;
      $islandora_solr_query->solrParams["f.$solr_field.facet.date.end"] = $to;
      $islandora_solr_query->solrParams["f.$solr_field.facet.date.gap"] = $gap;

      // Update variable.
      $needs_solr_call[] = $solr_field;
    }

    // 3: Execute.
    // If an extra solr call is necessary.
    if (!empty($needs_solr_call)) {
      // New query processor class.
      $range_query = new IslandoraSolrQueryProcessor();

      // Internal Solr query.
      $range_query->internalSolrQuery = $islandora_solr_query->internalSolrQuery;
      $range_query->solrLimit = 0;
      $range_query->solrStart = 0;
      $range_query->solrQuery = $islandora_solr_query->solrQuery;
      $range_query->solrParams = $islandora_solr_query->solrParams;
      // No need to include normal facets.
      unset($range_query->solrParams['facet.field']);

      // Excecute query.
      $range_query->executeQuery(TRUE,TRUE);

      // 4. Run query.
      $response_array = $range_query->islandoraSolrResult;

      // 5: Update date facets.
     
      
      self::$facet_dates = $response_array['facet_counts']['facet_dates'];
    }
    self::$needed_solr_call = $needs_solr_call;
  }

/*  public function findFacetType() {
    error_log(var_export(static::$facet_dates,true));
    error_log(var_export(self::$facet_dates,true));
    
    $facet_field = $this->facet_field;
    if (array_key_exists($facet_field, static::$facet_fields)) {
      $this->facet_type = 'facet_fields';
    }
    if (array_key_exists($facet_field, static::$facet_dates)) {
      $this->facet_type = 'facet_dates';
    }
    if (array_key_exists($facet_field, static::$facet_ranges)) {
      $this->facet_type = 'facet_ranges';
    }
  }
/**
   * Render text facets. Overrides IslandoraSolrFacets method
   *
   * Based on a prepared array of results, this method will process and render
   * a facet as normal text. It includes bucket value, count, include link and
   * exclude link. If configured it also adds a 'read more' link to expose more
   * results.
   *
   * @param array $results
   *   An array with the prepared facet results.
   */
  public function renderText($results) {
    $facet_field = $this->facet_field;
    $islandora_solr_query = self::$islandoraSolrQuery;
    $soft_limit = self::$soft_limit;
    $buckets = array();
 
    foreach ($results as $key => $values) {
      $bucket = $values['bucket'];
      $filter = $values['filter'];
      $count = $values['count'];
      // Current URL query.
      $fq = isset($islandora_solr_query->solrParams['fq']) ? $islandora_solr_query->solrParams['fq'] : array();
      // 1: Check minimum count.
      // 2: Check if the filter isn't active.
      if ($count < self::$minimum_count || array_search($filter, $fq) !== FALSE) {
        continue;
      }
      //Path to Ajax enabled Links
      $path = "islandora/relationships/facetfilter/nojs/";
      // Parameters set in URL.
      $params = $islandora_solr_query->internalSolrParams;
      // Set filter key if there are no filters included.
      if (!isset($params['f'])) {
        $params['f'] = array();
      }
      //We don't need acummulated filters, because those are already stored in a $_SESSION variable for AJAX use.
      $query_plus = array('f' => array($filter));
      $query_minus =array('f' => array('-' . $filter));
 
      // Set basic attributes.
      $attr_plus = $attr_minus = $attr = array('rel' => 'nofollow');
      $attr['href'] = $attr_plus['href'] = url($path.$query_plus['f'][0]);
      $attr['class']=array('use-ajax');
      $attr_minus['href'] = url($path, array('query' => $query_minus));
      $attr_plus['class'] = array('plus','use-ajax');
      $attr_minus['class'] = array('minus','use-ajax');

      // XXX: We are not using l() because of active classes:
      // @see http://drupal.org/node/41595
      // Create link.
      $link['link'] = '<a' . drupal_attributes($attr) . '>' . $bucket . '</a>';
      $link['count'] = $count;
      $link['link_plus'] = '<a' . drupal_attributes($attr_plus) . '>+</a>';
      $link['link_minus'] = '<a' . drupal_attributes($attr_minus) . '>-</a>';
      $buckets[] = $link;
    }

    // Show more link.
    if (count($buckets) > $soft_limit) {
      $buckets_visible = array_slice($buckets, 0, $soft_limit);
      $buckets_hidden = array_slice($buckets, $soft_limit);
      $this->content .= theme('islandora_solr_facet', array(
        'buckets' => $buckets_visible,
        'hidden' => FALSE,
        'pid' => $facet_field,
      ));
      $this->content .= theme('islandora_solr_facet', array(
        'buckets' => $buckets_hidden,
        'hidden' => TRUE,
        'pid' => $facet_field,
      ));
      $this->content .= $this->showMore();
    }
    elseif (!empty($buckets)) {
      $this->content .= theme('islandora_solr_facet', array(
        'buckets' => $buckets,
        'hidden' => FALSE,
        'pid' => $facet_field,
      ));
    }
  }

}
