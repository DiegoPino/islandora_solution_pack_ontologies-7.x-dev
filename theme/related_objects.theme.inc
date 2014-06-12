<?php

function islandora_relationship_editor_preprocess_islandora_relationship_editor_related_objects(&$variables) {
  $islandora_object = $variables['islandora_object'];
  $repository = $islandora_object->repository;
  $rels = $islandora_object->relationships->get();
  $related = array();
  foreach ($rels as $key => $rel) {
    if (($rel['predicate']['alias'] != 'fedora') and ($rel['predicate']['alias'] != 'fedora-model')) {
       watchdog('rosie', 'on object ' . $rel['object']['value']);
       try {
         $object = $islandora_object->repository->getObject($rel['object']['value']);
       } catch (Exception $e) {
         continue;
       }
       if ($object) {
         $related[] = $object;
       } 
    }
  }
  $variables['related'] = $related;
}
