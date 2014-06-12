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




