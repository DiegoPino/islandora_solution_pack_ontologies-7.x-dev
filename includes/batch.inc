<?php

/**
 * @file
 * This file contains all the batch operations for Objects that have ontologies associated.
 */

require_once dirname(__FILE__) . '/utilities.inc';

/**
 * Starts the batch operation.
 *
 * @param array $operands
 *   All the objects to operate on.
 * @param array $context
 *   The context of this batch operation.
 */
function  islandora_ontologies_start_operation(array $operands, array &$context) {
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['total'] = count($operands);
  }
  return @$operands[$context['sandbox']['progress']];
}

/**
 * Ends the batch operation.
 *
 * @param array $context
 *   The context of this batch operation.
 *
 * @return bool
 *   TRUE or FALSE depending on the context of the batch.
 */
function  islandora_ontologies_end_operation(array &$context) {
  $context['sandbox']['progress']++;
  if ($context['sandbox']['progress'] < $context['sandbox']['total']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['total'];
    return FALSE;
  }
  return TRUE;
}

/**
 * The batch 'finished' callback.
 */
function  islandora_ontologies_batch_finished($success, $results, $operations) {
  if (!$success) {
    // Generic Error Message.
    $error_operation = reset($operations);
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array(
      '%error_operation' => $error_operation[0],
      '@arguments' => print_r($error_operation[1], TRUE)));
    drupal_set_message($message, 'error');
  }
}

/**
 * Delete the given child objects within the collection.
 *
 * @param string $pid
 *   The pid of the object to purge the children from.
 *
 * @return array
 *   The batch definition.
 */
function  islandora_ontologies_removeto_relations_batch($pid) {
  $batch = array(
    'finished' => ' islandora_ontologies_batch_finished',
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'islandora_ontologies') . '/includes/batch.inc',
  );


  $message_parameters = array('@object' => $pid);
  $batch += array(
    'operations' => array(
      array(
        'islandora_ontologies_removeto_relations_batch_operation',
        array($pid),
      ),
    ),
    'title' => t('Deleting all predicates pointing to @object.', $message_parameters),
    'init_message' => t('Preparing to delete all relations pointing to @object.', $message_parameters),
  );

  return $batch;
}

/**
 * Deletes all children from the given object.
 *
 * @param string $pid
 *   The parent object to purge the children from.
 * @param array $context
 *   The batch context.
 */
function  islandora_ontologies_removeto_relations_batch_operation($pid, array &$context) {
  if (!isset($context['sandbox']['total_count'])) {
    $context['sandbox']['deleted'] = 0;
    $context['sandbox']['total_count'] = count(islandora_ontologies_relatedto_objects($pid));
    if ($context['sandbox']['total_count'] === 0) {
      $context['finished'] = 1;
      return;
    }
    $context['sandbox']['current'] = 0.0;
  }

  
  $procesed_pids = array();

  // Each time through the batch, get first 10 objects.
  $query_results =  islandora_ontologies_relatedto_objects($pid, 0, 5);
  $count = count($query_results);
  foreach ($query_results as $queryresult) {
    $object = islandora_object_load($queryresult['object']['value']);
    $this_pid = $object->id;
    $this_label = $object->label;
    if (islandora_object_access(ISLANDORA_MANAGE_PROPERTIES, $object)) {
      if ($object->relationships->remove(NULL, NULL, $pid,RELS_TYPE_URI)) {
        $procesed_pids[] = $this_pid;
        watchdog(' islandora_ontologies', 'Deleted relations in object "@label" (PID=@pid) poiting to @subjectpid', array('@label' => $this_label, '@pid' => $this_pid,'@subjectpid'=>$pid), WATCHDOG_NOTICE);
        }
    }
    else {
      watchdog(' islandora_ontologies', 'Permission denied to delete relations in "@label" (PID=@pid).', array('@label' => $this_label, '@pid' => $this_pid), WATCHDOG_ERROR);
    }

  }

  
  $context['sandbox']['current'] += $count;
  $context['sandbox']['deleted'] += count( $procesed_pids);
  if ($count === 0) {
    $context['finished'] = 1;
  }
  else {
    $context['finished'] = $context['sandbox']['current'] / $context['sandbox']['total_count'];
  }

  if (!$query_results || $context['finished'] == 1) {
    drupal_set_message(format_plural($context['sandbox']['deleted'],
      'Deleted relations from 1 object poiting to @pid.',
      'Deleted relations from @count objects poiting to @pid.',
      array(
        '@pid' => $pid
      )
    ));
  }
}
