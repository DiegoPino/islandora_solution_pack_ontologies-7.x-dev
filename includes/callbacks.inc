<?php

/**
 * @file
 * Holds the ajax callbacks for the islandora_ontologies.
 */

/**
 * Returns array of items to autocomplete forms.
 *
 * @param string $type
 *   Selects object PIDS by contentModel
 *
 * @return array
 *   array of all candidates
 */
function islandora_ontologies_targetobject_autocomplete($cmodel,$rel_name) {
  module_load_include('inc', 'islandora_solr_search', 'includes/utilities');
  $mappings = array(
    'scientificName' => array(
      'cmodel' => 'biodiversity:biodiversityCModel',
      'title' => 'dwc.scientificName_mt',
    ),
    'genus' => array(
      'cmodel' => 'biodiversity:biodiversityCModel',
      'title' => 'dwc.genus_mt',
    ),
  );
 
  $results = array();

  $content_model_field = variable_get(
      'islandora_solr_content_model_field', 'RELS_EXT_hasModel_uri_ms'
  );
   
  $biodiversidad_query_field = $mappings[$type]['title'];
  $stub="";
  if (isset($_GET['q'])) {
    $stub = end(explode('/', $_GET['q']));
}
  $query_processor = new IslandoraSolrQueryProcessor();
  $query_processor->solrQuery = format_string(
      '!biodiversidad_query_field:(!biodiversidad_query)', array(
        '!biodiversidad_query_field' => $biodiversidad_query_field,
        '!biodiversidad_query' => $stub.'*',
      )
  );
  $query_processor->solrStart = 0;
  $query_processor->solrLimit = 100;
  $query_processor->solrParams = array(
    'fl' => "$biodiversidad_query_field,PID",
    'fq' => format_string(
        '!content_model_field:"info:fedora/!content_model"', array(
          '!content_model_field' => $content_model_field,
          '!content_model' => $mappings[$type]['cmodel'],
        )
    ),
  );
  $url = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr'));
  $solr = new Apache_Solr_Service($url['host'], $url['port'], $url['path'] . '/');
  $solr->setCreateDocuments(FALSE);
  try {
    $results = $solr->search($query_processor->solrQuery, $query_processor->solrStart, $query_processor->solrLimit, $query_processor->solrParams, 'GET');
    $json = json_decode($results->getRawResponse(), TRUE);
  }
  catch (Exception $e) {
    watchdog_exception('Islandora Ontologies', $e, 'Got an exception while searching Object belonging to CMODEL in Solr for callback.', array(), WATCHDOG_ERROR);
  }
  $list = array();
  
  foreach ($json['response']['docs'] as $choice) {
	
    if (isset($choice[$mappings[$type]['title']])) {
      $list[$choice[$mappings[$type]['title']][0]] = $choice[$mappings[$type]['title']][0];//Solr devuelve un arreglo. Verificar schema.xml
    }
  }
 
  drupal_json_output($list);
}
function islandora_ontologies_get_content_models($ignore_system_namespace = TRUE) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $tuque = islandora_get_tuque_connection();
  $query = "PREFIX fm: <" . FEDORA_MODEL_URI . ">
            PREFIX fr: <" . FEDORA_RELS_EXT_URI . ">
            SELECT ?object ?label
            FROM <#ri>
            WHERE {
              {?object fm:hasModel <info:fedora/fedora-system:ContentModel-3.0>;
                       fm:state fm:Active
              }
            UNION{
                ?object fr:isMemberOfCollection <info:fedora/islandora:ContentModelsCollection>;
                fm:state fm:Active
              }
            OPTIONAL{
                 ?object fm:label ?label
            }
           }";
  $content_models = array();
  $results = $tuque->repository->ri->sparqlQuery($query, 'unlimited');
  foreach ($results as $result) {
    $content_model = $result['object']['value'];
    $label = $result['label']['value'];
    $namespace = islandora_get_namespace($content_model);
    $ignore = $ignore_system_namespace && $namespace == 'fedora-system';
    $ignore |= !islandora_namespace_accessible($namespace);
    if (!$ignore) {
      $content_models[$content_model] = array('pid' => $content_model, 'label' => $label);
    }
  }
  return $content_models;
}
/**
* Filters current displayed objects in relations graph by facet filters
*
* @see islandora_ontologies_block_view()
*
* @param $js, string, if called via ajax or not
* @param $filters, fields to filter current query
*
* @return json
*/
function islandora_ontologies_ajax_facetfilter_callback($js,$filters) {
  module_load_include('inc', 'islandora_ontologies', 'includes/blocks');
  if (!isset($_SESSION['islandora_ontologies_facet_filters'])) {
    return ; //means no other function initialized the basic query 
   
  }
  else
  {
    if (isset($_SESSION['islandora_ontologies_facet_filters']['f']) && in_array((string)$filters, $_SESSION['islandora_ontologies_facet_filters']['f'], true)) {
      return; //Filter was already in session, no need to re-filter
    }
    else
    {
      $_SESSION['islandora_ontologies_facet_filters']['f'][]=$filters;
    }
  }
   //Now filter our set:
  $islandora_ontologies_solr_queryclass = new IslandoraSolrQueryProcessor();
  $query = "*:*";
  $solr_cloud_prefix=variable_get('islandora_solrcloud_setting_uniqueid_prefix','');
  function addcloudprefix($pid,$prefix)
  {
      return("\"".$prefix.$pid."\"");
  }
  $params=array();
  $params['f']=$_SESSION['islandora_ontologies_facet_filters']['f'];
  $islandora_ontologies_solr_queryclass->buildQuery($query, $params);
  $islandora_ontologies_solr_queryclass->solrParams['fq'][]=$_SESSION['islandora_ontologies_facet_filters']['fq'];
  $islandora_ontologies_solr_queryclass->solrParams['hl'] = 'false';
  $islandora_ontologies_solr_queryclass->solrLimit=$_SESSION['islandora_ontologies_facet_filters']['numFound'];
  $islandora_ontologies_solr_queryclass->executeQuery(TRUE,TRUE);

  $vlotnodes = array();
  if (count($islandora_ontologies_solr_queryclass->islandoraSolrResult['response']['objects'])>0)
    {
      foreach($islandora_ontologies_solr_queryclass->islandoraSolrResult['response']['objects'] as $solr_objects) {
      $vlotnodes[]=$solr_objects['PID'];
      }
      
    }
 
  
  
  

   
   if ($js == 'ajax') {
     $commands = array();
     //Look Kids, i'm going to unbind previous set behaviours by Islandora Solr.
     $javascript = drupal_add_js();
     foreach ($javascript['settings']['data'] as $key => $item) {
         if (isset($item['islandoraSolrRangeSlider']))
           {
             foreach($item['islandoraSolrRangeSlider'] as $slider) {
               $commands[] = ajax_command_invoke('#date-range-slider-canvas-'.$slider['form_key'], 'unbind'); 
             }
           
           }
       }
    
     $commands[] = ajax_command_invoke(NULL, 'islandoraOntologiesVlotfaceting', array($vlotnodes));
     if (count($islandora_ontologies_solr_queryclass->islandoraSolrResult['response']['objects'])>0)
     {
  
     $commands[] = ajax_command_html('#block-islandora-ontologies-ontologies-solr-facets', islandora_ontologies_block_content($islandora_ontologies_solr_queryclass));
     }
     // Menu 'page callback' and #ajax['callback'] functions are supposed to
     // return render arrays. If returning an Ajax commands array, it must be
     // encapsulated in a render array structure.
     ajax_deliver(array('#type' => 'ajax', '#commands' => $commands));
   }
   else {
     // No ajax, just display the time on a page using renderable array.
   
   }
}



