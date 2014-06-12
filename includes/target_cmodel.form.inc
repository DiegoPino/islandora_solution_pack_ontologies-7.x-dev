<?php

/**
 * @file
 * Handles a form for choosing a target CMODEL for our ontology Object, used for ontologies:onoltogyCMODEL as the first ingest step
 */

/**
 *  Creates the form with the target CMODEL list
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 *
 * @return array
 *   The drupal form definition.
 */
function islandora_ontologies_target_cmodel_form(array $form, array &$form_state) {
    module_load_include('inc', 'islandora_basic_collection', 'includes/utilities');
	module_load_include('inc','islandora','includes/utilities');
    $content_models = islandora_get_content_models();
	$form = array();
	$form['target_cmodel'] = array(
      'contentmodel' => array(
        '#title' => t('Target Content Model for this Ontology'),
        '#description' => t('Defines to which existing Content Model we are going to apply this Ontology'),
        '#type' => 'select',
        '#options' => islandora_basic_collection_get_content_models_as_form_options($content_models),
		'#required' =>'TRUE'
      ),
  );
  return $form;

}
