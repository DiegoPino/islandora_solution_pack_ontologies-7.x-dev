<?php

function islandora_ontologies_del_relationships_form(array $form, array &$form_state, $islandora_object) {
	module_load_include('inc', 'islandora_ontologies', 'includes/utilities');
	
	$islandora_object_pid = $islandora_object->id;			
	if (!$islandora_object)
	{
		drupal_set_message(t("Islandora Object not found"));
		drupal_not_found();
		exit;	
	}	  
	$currentrelations=islandora_ontologies_get_current_localrelations($islandora_object_pid);
	$rows=array();
	dpm($currentrelations);
	foreach ($currentrelations as $o){
		$row=array();
		$row['namespace']['#markup'] = $o['predicate']['namespace']. " (xmlns:".$o['predicate']['alias'].")";
		$row['relationship']['#markup'] = $o['predicate']['value'];
		if ($o['object']['value'])
		{
			try {
				$target_object =islandora_object_load($o['object']['value']);
				$target_object_pid=$o['object']['value'];
				if ($target_object)
					{
						
						
				$target_object_link=l(t('@label (@pid)', array('@label' =>$target_object->label, '@pid' =>$target_object_pid)), "islandora/object/{$target_object_pid}/manage/relationships");
					}
					
			}
			catch (Exception $e) {
				$target_object_link=t('Error, Object (@pid) does not longer exist!',array('@pid'=>$target_object_pid));
				continue;
				;
			}
		}
		else
		{
			$target_object_link=$o['object']['literal']? t('literal: (@lit)',array('@lit'=>$o['object']['literal'])): " ";
			
		}
		
		$row['object']['#markup'] = $target_object_link;
		$row['delete']['#markup'] = l(t('Delete'), islandora_ontologies_get_delete_path($islandora_object_pid, urlencode($o['predicate']['namespace']), $o['predicate']['value'], $o['object']['value']));
		if ($o['predicate']['namespace'] == 'info:fedora/fedora-system:def/model#') {
			$row['delete']['#markup'] = t('Can\'t delete fedora model.');
		}
		if ($o['predicate']['namespace'] == 'info:fedora/fedora-system:def/relations-external#' and $o['predicate']['value'] == 'isMemberOfCollection') {
			$row['delete']['#markup'] = t('Manage from collection object');
		}
		$rows[]=$row;
	};
	
	$form = array(
		'table' => array(
			'#tree' => TRUE,
			'#title' => t('List relationships'),
			'#header' => array(
				t('Namespace'),t('Property'), t('Object'),t('Delete')
			),
			'#theme' => 'islandora_ontologies_ontology_management_table',
			'rows' =>$rows,
		));
		return $form;
	}

	/**
	* @see forked from islandora_relationship_editor 
	*/
function islandora_ontologies_delete_relationships(array $form, array &$form_state, $objectPid, $namespace, $relationship, $targetPid) {
	  $subject = islandora_object_load($objectPid);
	  if (!$subject) {
	    drupal_set_message("Fedora object " . $objectPid . " not found.");
	    drupal_not_found();
	    exit;
	  }

	  if ($subject->relationships->remove(urldecode($namespace), $relationship, $targetPid)) {

	  return array(
	    'object_pid' => array(
	      '#type' => 'hidden',
	      '#value' => $objectPid,
	    ),
	    'description' => array(
	      '#type' => 'markup',
	      '#prefix' => '<div>',
	      '#markup' => t('Relationship deleted'),
	      '#suffix' => '</div>',
	    ),
	    'back' => array(
	      '#type' => 'submit',
	      '#value' => t('Go back'),
	      '#name' => 'back',
	    ),
	  );

	  } else {
	  return array(
	    'object_pid' => array(
	      '#type' => 'hidden',
	      '#value' => $objectPid,
	    ),
	    'description' => array(
	      '#type' => 'markup',
	      '#prefix' => '<div>',
	      '#markup' => t('Relationship delete ERROR'),
	      '#suffix' => '</div>',
	    ),
	    'back' => array(
	      '#type' => 'submit',
	      '#value' => t('Go back'),
	      '#name' => 'back',
	    ),
	  );
	  }

	}

	function islandora_ontologies_delete_relationships_submit(array $form, array &$form_state) {

	    $form_state['redirect'] = 'islandora/object/' . $form_state['values']['object_pid'] . '/manage/relationships';
	}
