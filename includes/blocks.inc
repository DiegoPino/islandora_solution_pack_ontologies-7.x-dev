<?php

/**
 * @file
 * Contains all blocks related functionality.
 */
/**
 * Implements hook_block_info().
 */
function islandora_ontologies_block_info() {
  $blocks = array();
  $blocks['ontologies_solr_facets'] = array(
      'info' => 'Facets for Related Objects - Ontologies ',
      'cache' => DRUPAL_NO_CACHE,
      'visibility' => BLOCK_VISIBILITY_LISTED,
      'pages' => 'islandora/object/*',
    );

  return $blocks;
}

/**
 * Implements hook_block_view().
 *
 * @global IslandoraSolrQueryProcessor $_islandora_ontologies_solr_queryclass
 *   The IslandoraSolrQueryProcessor object which includes the current query
 *   settings and the raw Solr results based on current relations objects graph
 */
function islandora_ontologies_block_view($block_name = '') {
  drupal_add_library('system', 'drupal.ajax');
  global $_islandora_ontologies_solr_queryclass;
  $block = array();
  // If the block that is called is defined, and we have permission to view it.
 if (!empty($block_name))
 {
   $block['subject']=t('Filter Objects in relations Graph');
   $block['content']=islandora_ontologies_block_content($_islandora_ontologies_solr_queryclass);
 }
 
  return $block;
}
/**
 * Islandora Ontologies Block Content Callback
 * Because we are using ajax on factes we need a function that allows us to reload the block
 * @param IslandoraSolrQueryProcessor $_islandora_ontologies_solr_queryclass
 * @return themed block content
 */
function islandora_ontologies_block_content($islandora_ontologies_solr_queryclass) {
  $blockcontent ='';
  if (user_access('search islandora solr') &&  ($islandora_ontologies_solr_queryclass instanceof IslandoraSolrQueryProcessor) ) {
    module_load_include('inc', 'islandora_solr', 'includes/results');
    module_load_include('inc', 'islandora_ontologies', 'includes/solrresults');
    $display_class = new IslandoraOntologiesSolrResults;
    $blockcontent = $display_class->displayFacets($islandora_ontologies_solr_queryclass);
    
  }
  return $blockcontent;
}