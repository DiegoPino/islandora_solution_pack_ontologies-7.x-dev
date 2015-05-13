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
	$currentontologies= islandora_ontologies_get_associated_ontologies($islandora_object);//@see utilities.inc
	foreach($currentontologies as $subject=>$ontologies)
	{

		$posible_relastionships[$subject]=islandora_ontologies_get_possible_localrelations($subject,$ontologies);//We are going to use this to also delete the inverse one if they exist, so we don't get orphan relations and a mess.
				
	}
	
	
	
	
	
	
	$rows=array();
	dpm($currentrelations);
	foreach ($currentrelations as $o){
			
		$target_object_link="";
		$row=array();
		$row['namespace']['#markup'] = $o['predicate']['namespace']. " (xmlns:".$o['predicate']['alias'].")";
		$row['relationship']['#markup'] = $o['predicate']['value'];
		if (($o['object']['value']) && !($o['object']['literal']))
		{
			try {
				$target_object =islandora_object_load($o['object']['value']);
				$target_object_pid=$o['object']['value'];
				if ($target_object)
					{
					//Target exists, so lets look for an inverse or inverseof in the possible array
					//look for an inverseof or inverse first. This object can have relations outside our current ontologies, so we must check...it´s far from optimal, too much loops.
					foreach ($posible_relastionships as $cmodel => $localrels)
						{
							foreach ($localrels['posiblerels_complete'] as $ontologies=>$rels)
								{
									if (array_key_exists($o['predicate']['namespace'].$o['predicate']['value'],$rels))
										{
										
											
										if (count((array)$rels[$o['predicate']['namespace'].$o['predicate']['value']]['inverse'])==2)
											{
												
												
											}
										elseif (count((array)$rels[$o['predicate']['namespace'].$o['predicate']['value']]['inverseof'])==2)
											{
												
												
											}
											break;
										}	
								}
				
						}	
						
				$target_object_link=l(t('@label (@pid)', array('@label' =>$target_object->label, '@pid' =>$target_object_pid)), "islandora/object/{$target_object_pid}/manage/relationships");
				$row['object']['#markup'] = $target_object_link;
				$row['delete']['#markup'] = l(t('Delete'), islandora_ontologies_get_delete_path($islandora_object_pid, urlencode($o['predicate']['namespace']), $o['predicate']['value'], $o['object']['value']));
			
					}
					else
					{
					$target_object_link=t('Wrong relation, Object (@pid) does not longer exist!',array('@pid'=>$target_object_pid));	
					$row['object']['#markup'] = $target_object_link;
					$row['delete']['#markup'] = l(t('Delete'), islandora_ontologies_get_delete_path($islandora_object_pid, urlencode($o['predicate']['namespace']), $o['predicate']['value'], $o['object']['value']));
			
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
			$target_object_link_lit=$o['object']['literal']? t('literal: (@lit)',array('@lit'=>$o['object']['value'])): " ";
			$row['object']['#markup'] = $target_object_link_lit;//@todo manage literals
			$row['delete']['#markup'] = "";
		}
		
		
		
		if ($o['predicate']['namespace'] == 'info:fedora/fedora-system:def/model#') {
			$row['delete']['#markup'] = t('Can\'t delete fedora model.');
		}
		elseif ($o['predicate']['namespace'] == 'info:fedora/fedora-system:def/relations-external#' and $o['predicate']['value'] == 'isMemberOfCollection') {
			$row['delete']['#markup'] = t('Manage from collection object');
		}
		elseif ($o['predicate']['namespace'] == 'http://islandora.ca/ontology/relsext#') {
			$row['delete']['#markup'] = t('Manage from object policy');
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
function islandora_ontologies_delete_relationships(array $form, array &$form_state, $islandora_object_pid, $namespace, $relationship, $targetPid=NULL) {
	
	try {
		$islandora_object=islandora_object_load($islandora_object_pid);
	}
	catch (Exception $e) 
	{
		return FALSE;
	}
	
	  
	  if (!$islandora_object) {
	    drupal_set_message("Fedora object not found.");
	    drupal_not_found();
	    return;
	  }
	  
	  if ($islandora_object->relationships->remove(urldecode($namespace), $relationship, $targetPid)) {
	  return array(
	    'object_pid' => array(
	      '#type' => 'hidden',
	      '#value' => $islandora_object->id,
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
	      '#value' => $islandora_object->id,
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
