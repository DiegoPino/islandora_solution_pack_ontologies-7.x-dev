<?php
/*
* @file
* Holds  functions to manage ontologies to contentmodels relations for islandora_ontologies.
*/
$module_path = drupal_get_path('module', 'islandora_ontologies');
include_once($module_path.'/arc2/ARC2.php');
include_once($module_path.'/Graphite/Graphite.php');

/**
* This function will read the ONTOLOGY Stream from an object and display a table to add relations to matching CMODEL objects PIDS to owl:classes
*
*/

function islandora_ontologies_parsed_vocabulary_view_form(array $form, array &$form_state, $islandora_object_pid){
	module_load_include('inc', 'islandora', 'includes/utilities');
	
	
	
	$islandora_object = islandora_object_load($islandora_object_pid);
	$datastream = $islandora_object['ONTOLOGY'];
	if (!$datastream)
	{
		drupal_set_message(t("Fedora Ontoloy object doesn't have an ONTOLOGY stream."));
		drupal_not_found();
		exit;
	}
	$existing_content_models = islandora_get_content_models();
 
	$PID=$islandora_object->id;
	
	$fedora_url = variable_get('islandora_base_url', 'http://localhost:8080/fedora');
	$xmlfilelink = $fedora_url."/objects/".$PID."/datastreams/ONTOLOGY/content";
	$graph = new Graphite();
	$graph->load($xmlfilelink);
	$existing_classes=array();
	foreach($graph->allOfType('owl:Class') as $resource_list)
	{
		$parts = ARC2::splitURI($resource_list->uri);//We need don´t need in our keys the info:fedora part, but we must check if it´s there
		if ($parts[0]!='info:fedora/')
			{
			continue;	
			}
		$existing_classes[$parts[1]]=$resource_list->uri;
		//@TODO check if ontology is correct before adding associate option
		//get_restrictions_onClasses($classes);
	}
	$rows = array();
	//print_r($existing_classes);
	$matchedcontenmodels=array();
	$matchedcontenmodels=array_intersect_key($existing_content_models,$existing_classes);//Here we get those owl classes that are present as content models in our repository
	foreach ($matchedcontenmodels as $contentmodelpid => $content_model) {
		try {
			$contentmodel_object =islandora_object_load($contentmodelpid);
		}
		catch (Exception $e) {
			continue;
		}
		
		if ($contentmodel_object)
		{   
			$label = $contentmodel_object['label'];
			
			$rels = $contentmodel_object->relationships->get(ISLANDORA_RELS_EXT_URI,'hasLocalOntology',$PID,RELS_TYPE_URI);//We get a haslocalOntology relation for this ontologyPID
			//print_r($rels);
			$existingassociation=count($rels)>0?1:0;
			$rows[$contentmodelpid] = array(
				'selected' => array(
					'#type' => 'checkbox',
					'#default_value' => $existingassociation,
				),
				'title' => array(
					'#markup' => l(t('@label (@pid)', array('@label' => $label, '@pid' => $contentmodelpid)), "islandora/object/{$contentmodelpid}"),
				),
				'allowed_relations' => array(
				        '#markup' => t('hola'),
					),	
			);
		}
		
  
  
  
  
	}
	//print_r($rows);
		
	//$form['#action'] = request_uri() . '#association_management';
	$form['help'] = array(
			'#type' => 'item',
			'#markup' => l(t('About Ontology'), '', array()),
		);
	$form['table'] = array(
			'#tree' => TRUE,
			'#header' => array(
				array('class' => array('select-all')), t('PID'), 'Namespace',
			),
			'#theme' => 'islandora_ontologies_ontology_management_table',
			'rows' => $rows,
		);
		
	$form['submit'] = array(
			'#type' => 'submit',
			'#value' => t('Update Ontology Content Models associations'),
		);
				
	
 
	$form_state['ontologyobject_pid'] =$PID;
	return $form;
 
 
 
 
	// foreach ($content_models as $pid => $content_model) {
    
		//	}
    
		/* error_log($xmlfilelink,1);
		$parser->parse($xmlfilelink);
		$triples = $parser->getTriples();
		$ns=$parser->parser->nsp;
		$index = ARC2::getSimpleIndex($triples, false) ;
		$triples = ARC2::getTriplesFromIndex($index);
		//$terms = islandora_ontologies_rdfx_fetch_terms($triples);
		//foreach ($parser->parser->nsp as $uri => $prefix) {
		//     $namespaces[$prefix] = $uri;
		//}
  
		// $vocabulary = array(
		//    'terms' => $terms,
		//    'namespaces' => $namespaces,
		//);*/
 
		//return (array('triples'=>$triples,'ns'=>$ns,'index'=> $index));
	}

/**
 * Submit handler for ontology content models association management form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */


function islandora_ontologies_parsed_vocabulary_view_form_submit(array $form, array &$form_state){	
	$ontologyobject=islandora_object_load($form_state['ontologyobject_pid']);
	$ontologyobject_pid=$ontologyobject->id;
	dpm($ontologyobject_pid);
	foreach ($form_state['values']['table']['rows'] as $pid => $properties) {
		unset($subject);
		unset($rels);
		
		try {
			$subject = islandora_object_load($pid);	
		}
		catch (Exception $e) {
			continue;
		}
		
		if ($subject) 
		{  
			dpm($properties);
	
			//@TODO allow multiple haslocalOntology relations on tha same contentmodel to different ontology objects.
		    //This means we must check if ontologies does not have the same internal predicate_uri, if so this could lead to erroneous interpretation of applied relations between objects.	
			switch($properties['selected'])
				{
				case 1:	
				//$subject->relationships->remove(ISLANDORA_RELS_EXT_URI, 'hasLocalOntology', NULL, RELS_TYPE_URI);	
				$rels = $subject->relationships->get(ISLANDORA_RELS_EXT_URI,'hasLocalOntology',$ontologyobject_pid,RELS_TYPE_URI);//We get a haslocalOntology relation for this ontologyPID
				$existingassociation=count($rels) > 0 ? 1 : 0;
				if (!$existingassociation)
					{
					$subject->relationships->add(ISLANDORA_RELS_EXT_URI, 'hasLocalOntology', $ontologyobject_pid, RELS_TYPE_URI);
			 		drupal_set_message(t('Ontology Class to existing Content Model @label association done', array('@label' => $subject['label'])));	
					}
				break;
				default:
				//$subject->relationships->remove(ISLANDORA_RELS_EXT_URI, 'hasLocalOntology', NULL, RELS_TYPE_URI);	 with NULL removes every relation that matches the name
				$subject->relationships->remove(ISLANDORA_RELS_EXT_URI, 'hasLocalOntology', $ontologyobject_pid, RELS_TYPE_URI);		
			 	drupal_set_message(t('Ontology Class association for Object @ontologyobject_pid from Content Model @label removed', array('@ontologyobject_pid'=>$ontologyobject_pid,'@label' => $subject['label'])));	
				}
		//First we remove any existing relation. 
		//$subject->relationships->remove(ISLANDORA_RELS_EXT_URI, 'hasLocalOntology', NULL, RELS_TYPE_URI);
		//$relationships->add($predicate_uri, $predicate, $object, $literal);
		//
		}
	}	
	 $form_state['redirect'] = NULL; 
	}
	/**
	 * Manage Object Relations local task.
	 *
	 * Defines the actions to appear in the Relations section of the Manage tab.
	 *
	 * @param AbstractObject $object
	 *   The collection object.
	 *
	 * @return array
	 *   A renderable array.
	 */
function islandora_ontologies_manage_relations(AbstractObject $object) {
  	module_load_include('inc', 'islandora_ontologies', 'includes/utilities');
	module_load_include('inc', 'islandora_ontologies', 'includes/add_relation');
	module_load_include('inc', 'islandora_ontologies', 'includes/del_relation');
	
	  $return_form = array(
	    'manage_relations' => array(),
	  );
	  $data = islandora_ontologies_manage_relations_form($return_form, $object);
	  
	  $data['manage_relations']['#type'] = 'vertical_tabs';
	  return $data;
	}
	/**
	 * Implements islandora_basic_collection_build_manage_object_hook().
	 *
	 * @param array $form_state
	 *   The Form state of the form.
	 * @param AbstractObject $object
	 *   The Islandora object being processed.
	 *
	 * @return array
	 *   Returning the built form.
	 */
function islandora_ontologies_manage_relations_form(array $form_state, AbstractObject $object) {
	  // Defines accessible elements.
  	
	  $access = array(
	    'add_relationships' => islandora_object_access_callback(ISLANDORA_MANAGE_PROPERTIES, $object),
	    'del_relationships' => islandora_object_access_callback(ISLANDORA_MANAGE_PROPERTIES, $object),
	  );
	 
	  // Build forms for only the accessible elements.
	  foreach ($access as $element => $accessible) {
	    $form = "islandora_ontologies_{$element}_form";
		
	    $forms[$element] = $accessible ? drupal_get_form($form, $object) : NULL;
	  }
	  
	  $form_state['manage_relations']['add_relationships'] = array(
	    '#id' => 'add-relations',
	    '#group' => 'manage_relations',
	    '#access' => $access['add_relationships'],
	    '#type' => 'fieldset',
	    '#title' => t('Add Relations'),
	    'form' => $forms['add_relationships'],
	  );
	  $form_state['manage_relations']['del_relationships'] = array(
	    '#id' => 'delete-relations',
	    '#group' => 'manage_relations',
	    '#access' => $access['del_relationships'],
	    '#type' => 'fieldset',
	    '#title' => t('Delete existing relationships'),
	    'form' => $forms['del_relationships'],
	  );
	  return $form_state;
	}	
	