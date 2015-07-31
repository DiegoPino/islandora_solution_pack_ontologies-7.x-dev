<?php

function islandora_ontologies_del_relationships_form(array $form, array &$form_state, $islandora_object) {
	module_load_include('inc', 'islandora_ontologies', 'includes/utilities');
  form_load_include($form_state, 'inc', 'islandora_ontologies', 'includes/del_relation');
	
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
  $rows_data=array();
  $inverse=array();
  

  //Let's read all inverses so we can, on delete, select them by predicate
  foreach ($posible_relastionships as $cmodel => $localrels) {
    foreach ($localrels['posiblerels_complete'] as $ontologies=>$rels) {
      foreach ($rels as $relkey=>$relinfo) {
        $inverse[$relkey]=array();
        if (count((array) $relinfo['inverse'])==2) {
         $inverse[$relkey]['predicate']=$relinfo['inverse']['proplocal'];
         $inverse[$relkey]['namespace']=$relinfo['inverse']['ns'];
        }
        elseif(count((array) $relinfo['inverseof'])==2) {
          $inverse[$relkey]['predicate']=$relinfo['inverseof']['proplocal'];
          $inverse[$relkey]['namespace']=$relinfo['inverseof']['ns'];
        } 
        
        
      }
    }  
  }
  
  
  
  
	foreach ($currentrelations as $o) {
			
		$target_object_link="";
		$row=array();
		$row['selected'] = array(
			'#type' => 'checkbox',
			'#default_value' => 0,
		);
		$row['namespace']['#markup'] = $o['predicate']['namespace']. " (xmlns:".$o['predicate']['alias'].")";
		$row['relationship']['#markup'] = $o['predicate']['value'];
    if (($o['object']['value']) && !($o['object']['literal']))
		{
			try {
				$target_object =islandora_object_load($o['object']['value']);
				$target_object_pid=$o['object']['value'];
				if ($target_object) {
				//Target exists, so lets look for an inverse or inverseof in the possible array
				//look for an inverseof or inverse first. This object can have relations outside our current ontologies, so we must check...it´s far from optimal, too much loops.
				$target_object_link=l(t('@label (@pid)', array('@label' =>$target_object->label, '@pid' =>$target_object_pid)), "islandora/object/{$target_object_pid}/manage/relationships");
				$row['object']['#markup'] = $target_object_link;
				}
				else {
					$target_object_link=t('Wrong relation, Target Object (@pid) does not longer exist!',array('@pid'=>$target_object_pid));	
					$row['object']['#markup'] = $target_object_link;
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
		}
		
		
		
		$rows[]=$row;
    $inverse_predicate = isset( $inverse[ $o['predicate']['namespace'].$o['predicate']['value'] ]['predicate']) ? $inverse[$o['predicate']['namespace'].$o['predicate']['value']]['predicate'] : NULL;
    $inverse_namespace = isset( $inverse[$o['predicate']['namespace'].$o['predicate']['value']]['namespace']) ? $inverse[$o['predicate']['namespace'].$o['predicate']['value']]['namespace'] : NULL;
    $rows_data[]=array(
      'predicate'=>$o['predicate']['value'],
      'namespace'=>$o['predicate']['namespace'],
      'target_object'=>$o['object']['value'],
      'inverse' => $inverse_predicate,
      'inverse_namespace'=> $inverse_namespace,
      );

    }
    
	
  
  
	$form['table'] = array(
			'#tree' => TRUE,
			'#title' => t('List relationships'),
			'#header' => array( array('class' => array('select-all')),
        t('Namespace'),t('Property'), t('Object')
			),
			'#theme' => 'islandora_ontologies_ontology_management_table',
			'rows' =>$rows,
		);
  if (count($rows)) {
   $form['operation']=array(
     '#type' => 'select',
     '#title' => t('Operate on'),
     '#options' => array('delete_current' => t('selected relations'),'delete_both' => t('selected relations and inverse ones in target object, if present')), 
     '#required' => TRUE,
     '#default_value'=>'delete_current',
     '#prefix' => '<div id="relationship_select_delete">',
     '#suffix' => '</div>',
   ); 
  $form['reldata']=array(
      '#type' => 'value',
      '#value'=> $rows_data,
    );
  $form_state['islandora_object_pid']=$islandora_object_pid;  
 	$form['submit'] = array(
 			'#type' => 'submit',
 			'#value' => t('Delete relations'),
 		);
    //@TODO we could implement any other type of operations, like move to other target, etc.
  }
   return $form;
	}



function islandora_ontologies_del_relationships_form_submit(array $form, array &$form_state)
{
	 $islandora_object_pid=$form_state['islandora_object_pid'];
   $rows_data=$form_state['values']['reldata'];
  try {
		$islandora_object=islandora_object_load($islandora_object_pid);
	}
	catch (Exception $e) 
	{
		return FALSE;
	}
  $success=0;
  $success_inverse=0;
  $failed_inverse=0;
  foreach ($form_state['values']['table']['rows'] as $rowkey => $properties) {
      if ($properties['selected']) {
        switch($form_state['values']['operation']) {
          case 'delete_current':   
            if ($islandora_object->relationships->remove($rows_data[$rowkey]['namespace'], $rows_data[$rowkey]['predicate'], $rows_data[$rowkey]['target_object'])){
              $success++;
            }
            break;
            case 'delete_both':
            if ($islandora_object->relationships->remove($rows_data[$rowkey]['namespace'], $rows_data[$rowkey]['predicate'], $rows_data[$rowkey]['target_object'])){
              $success++;
            }
            try {
              $target_object=islandora_object_load($rows_data[$rowkey]['target_object']);
              if ($target_object) {
                if ($target_object->relationships->remove($rows_data[$rowkey]['inverse_namespace'], $rows_data[$rowkey]['inverse'], $islandora_object_pid)) {
                  $success_inverse++;
                }
              }
            }
            catch (Exception $e) { 
              $failed_inverse++;
              continue;
            
            }
          break;
         }
     }
   }
   drupal_set_message(format_plural($success, '@count relation for this object deleted', '@count relations for this object deleted'), 'status');
   drupal_set_message(format_plural($success_inverse, '@count inverse relation to this object deleted', '@count inverse relations to this object deleted'), 'status');
   if ($failed_inverse>0)
     {
     drupal_set_message(format_plural($failed_inverse, '@count inverse relation could not be delete because the object was not present', '@count inverse relation could not be delete because the object was not present'), 'status');
     }
   $form_state['redirect'] = array("islandora/object/{$islandora_object_pid}/manage/relationships", array('fragment' => 'delete-relations'));
  
 
}

/**
* @see forked from islandora_relationship_editor, replaced by islandora_ontologies_del_relationships_form_submit
* Not longer used in this form, left here for future needs
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
foreach ($form_state['values']['table']['rows'] as $pid => $properties) {}


	  
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
