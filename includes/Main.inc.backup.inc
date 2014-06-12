<?php


function islandora_relationship_editor_display_relationships(array $form, array &$form_state, $object_pid) {
  $object = islandora_object_load($object_pid);
  if (!$object) {
    drupal_set_message("Fedora object doesn't exist.");
    drupal_not_found();
    exit;
  }
  $rels = $object->relationships->get();
  $create_table_rows = function($o) use ($object_pid) {
    $o['namespace'] = $o['predicate']['namespace']. " (xmlns:".$o['predicate']['alias'].")";
    $o['relationship'] = $o['predicate']['value'];
    $o['object'] = $o['object']['value'] ? $o['object']['value'] : ($o['object']['literal'] ? $o['object']['literal'] : '');
    $o['delete'] = l(t('Delete'), islandora_relationship_editor_get_delete_path($object_pid, urlencode($o['predicate']['namespace']), $o['predicate']['value'], $o['object']));
    if ($o['predicate']['namespace'] == 'info:fedora/fedora-system:def/model#') {
      $o['delete'] = t('Can\'t delete fedora model.');
    }
    if ($o['predicate']['namespace'] == 'info:fedora/fedora-system:def/relations-external#' and $o['predicate']['value'] == 'isMemberOfCollection') {
      $o['delete'] = t('Manage from collection object');
    }
    return $o;
  };
  $form = array(
    'list' => array(
      '#type' => 'fieldset',
      '#title' => t('List relationships'),
      '#value' => theme('islandora_ontologies_editor_table', array(
                       'relationships' => array_map($create_table_rows, $rels)
		       )),
    ),
  );
  return $form;
}

