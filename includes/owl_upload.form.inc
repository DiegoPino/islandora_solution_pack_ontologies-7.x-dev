<?php

/**
 * @file
 * Handles the uploading of a OWL File (.owl o RDF).
 */

/**
 * Defines a file upload form for upload a OWL or RDF file containing a local ontology.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */


function islandora_ontologies_owl_upload_form(array $form, array &$form_state) {
  // print_r($form_state);
   
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $extensions = array('rdf owl');
  return array(
    'file' => array(
      '#title' => t('Ontology'),
      '#type' => 'managed_file',
      '#required' => TRUE,
      '#description' => t('Select ONTOLGY file to upload.<br/>Files must be less than <b>@size MB.</b><br/>Allowed file types: <b>@ext.</b>', array('@size' => $upload_size, '@ext' => $extensions[0])),
      '#default_value' => isset($form_state['values']['files']) ? $form_state['values']['files'] : NULL,
      '#upload_location' => 'temporary://',
      '#upload_validators' => array(
        'file_validate_extensions' => $extensions,
        // Assume its specified in MB.
        'file_validate_size' => array($upload_size * 1024 * 1024),
      ),
    ),
  );
}

/**
 * Submit handler, adds uploaded ONTOLOGY file to ingest object.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_ontologies_owl_upload_form_submit(array $form, array &$form_state) {

    $object= isset($form_state['object_id']) ? islandora_object_load($form_state['object_id']): $form_state['islandora']['objects'][0];
    
    if (empty($object['ONTOLOGY'])) {
      $ds = $object->constructDatastream('ONTOLOGY', 'M');
      $object->ingestDatastream($ds);
    }
    else {
      $ds = $object['ONTOLOGY'];
    }
    $file = file_load($form_state['values']['file']);
    $path = drupal_realpath($file->uri);
    $ds->setContentFromFile($path, FALSE);
    $ds->label = $file->filename;
    $ds->mimetype = $file->filemime;
 
 
  /*$object= isset($form_state['object_id']) ? islandora_object_load($form_state['object_id']): $form_state['islandora']['objects'][0];//true means we are updating, this variable is set on the wrapper function islandora_ontologies_update_ontology, no 
  $dsid = 'ONTOLOGY';
  // Before we do anything, check if the datastream exists. If it does, load it; otherwise construct it.
  // The easiest way to do this, as opposed to a string of cases or if/then/elses, is the ternary operator, e.g.
  // $variable = isThisThingTrueOrFalse($thing) ? setToThisIfTrue() : setToThisIfFalse();
  $file = file_load($form_state['values']['file']);
  $path = drupal_realpath($file->uri);
   try {
  $datastream = isset($object[$dsid]) ? $object[$dsid] : $object->constructDatastream($dsid,'M');
  $datastream->label = $file->filename;
  $datastream->mimeType = $file->filemime;
  
  /*Only ingest if dsid doesn´t exist */
  //$object->ingestDatastream($datastream);
  /*if (!isset($object[$dsid]))
  {
	 
  	$object->ingestDatastream($datastream);
  }
  $datastream->setContentFromFile($path, FALSE);
   file_delete($file);
 }
 catch (exception $e) {
   drupal_set_message(t('@message', array('@message' => check_plain($e->getMessage()))), 'error');
   // Make sure to delete anyways.
   file_delete($file);
   return;
 }*/
 drupal_set_message(t("Successfully Uploaded Ontology Datastream!"));
}
/**
 * Update Ontology from manage page.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 * @param AbstractObject $object
 *   The active fedora object.
 *
 * @return array
 *   The drupal form definition.
 */
function islandora_ontologies_update_ontology(array $form, array &$form_state, AbstractObject $object) {
 

  $form_state['object_id'] = $object->id;
  drupal_set_message($form_state['object_id']);
  dpm($object);
  
  $form = islandora_ontologies_owl_upload_form($form,$form_state);
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Submit',
  );
  return $form;
}

/**
 * Update Ontology form submit handler.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 *
 * @return array
 *   The drupal form definition.
 */
function  islandora_ontologies_update_ontology_submit(array $form, array &$form_state) {
  islandora_ontologies_owl_upload_form_submit($form, $form_state);
 
  /*@TODO add callback to recheck ontology and add/remove existing relations to this object */
}