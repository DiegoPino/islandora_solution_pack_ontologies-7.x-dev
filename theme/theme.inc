<?php

function islandora_ontologies_preprocess_islandora_ontologies_parsed(array &$variables) {
 $module_path = drupal_get_path('module', 'islandora_ontologies');
 module_load_include('inc', 'islandora_ontologies', 'includes/utilities');
 $islandora_object = $variables['islandora_object'];
 $parsed_ontology=islandora_ontologies_parse_vocabulary($islandora_object);
 $variables['triples'] = $parsed_ontology['triples'];
 $variables['ns'] = $parsed_ontology['ns'];
 $variables['index'] = $parsed_ontology['index'];
 
}
/**
 * Theme the ontology management table.
 *
 * @param array $variables
 *   Variables passed to this theme function.
 *
 * @return string
 *   Markup representing the table for rendering.
 */
function theme_islandora_ontologies_ontology_management_table(array $variables) {
  // Manually add the table select javascript as we are using a custom table.
  drupal_add_js('misc/tableselect.js');
   
  $table = $variables['table'];
  $row_elements = $table['rows'];
  $rows = array();
  foreach (element_children($row_elements) as $key) {
    $columns = array();
    $row_element = $row_elements[$key];
    foreach (element_children($row_element) as $key) {
      $column_element = $row_element[$key];
      $columns[] = array(
        'data' => drupal_render($column_element),
        'class' => isset($cell_element['#attributes']['class']) ? $column_element['#attributes']['class'] : NULL,
      );
    }
    $rows[] = $columns;
  }
  $variables = array(
    'header' => $table['#header'],
    'rows' => $rows,
    'attributes' => $table['#attributes'],
    'caption' => NULL,
    'colgroups' => NULL,
    'sticky' => NULL,
    'empty' => t("No Relations found"),
  );
  return theme('table', $variables);
}
