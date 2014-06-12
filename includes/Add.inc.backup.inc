<?php

function islandora_relationship_editor_add_relationships(array $form, array &$form_state, $object_pid) {
    if (strpos(':', $object_pid) === FALSE) {
      $object_pid = urldecode($object_pid);
     }
$options_relationship = get_possible_relationships($object_pid);
$value_relationship = !empty($form_state['values']['relationship']) ? $form_state['values']['relationship'] : "";
  $form = array();
  $form['this'] = array(
    '#title' => t('This Object'),
    '#type' => 'textfield',
    '#value' => $object_pid,
    '#disabled' => TRUE,
  );

  $form['rels'] = array(
	  '#type' => 'fieldset',
	  '#title' => 'Relationship',
  );

  $form['rels']['relationship'] = array(
    '#type' => 'select',
    '#title' => t('Relationship'),
    '#options' => $options_relationship,
    '#required' => TRUE,
    '#ajax' => array(
      	'method' => 'replace',
      	'effect' => 'fade',
        'event' => 'change',
        'callback' => 'relationship_ajax_callback',
     ),
  );


  $form['rels']['description'] = array(
    	'#markup' => textfield_description_value($value_relationship),
        '#prefix' => '<div id="textfield_description_replace">',
        '#suffix' => '</div>',
  );

  $form['ns'] = array(
	  '#type' => 'fieldset',
	  '#title' => 'Namespace',
        '#prefix' => '<div id="fieldset_ns_replace">',
        '#suffix' => '</div>',
  );

  $form['ns']['namespace'] = array(
    	'#type' => 'textfield',
    	'#title' => t('Namespace'),
	'#disabled' => TRUE,
	'#value' => textfield_namespace_value($value_relationship),
  );

 $form['ns']['curr_alias'] = array(
    	'#type' => 'textfield',
    	'#title' => t('Namespace alias'),
	'#disabled' => TRUE,
	'#value' => isset($form_state['values']['relationship']) ? ns_has_alias($value_relationship, $object_pid) : "",
  );

  $form['ns']['add_new_alias'] = array(
	  '#type' => 'checkbox',
	  '#disabled' => TRUE,
	  '#title' => t('Add NEW alias'),
	  '#default_value' => FALSE,
	  '#value' => (isset($form_state['values']['relationship']) && !(ns_has_alias($value_relationship, $object_pid))) ? TRUE : FALSE,
  );

 $form['ns']['alias'] = array(
    	'#type' => 'textfield',
    	'#title' => t('NEW Namespace alias (blank for inline namespace)'),
	'#disabled' => (isset($form_state['values']['relationship']) && !(ns_has_alias($value_relationship, $object_pid))) ? FALSE : TRUE,
  );

  $form['obj'] = array(
	  '#type' => 'fieldset',
	  '#title' => 'Object',
  );

  $form['obj']['target'] = array(
    '#type' => 'textfield',
    '#title' => t('Target object'),
    '#required' => TRUE,
	'#autocomplete_path' => 'islandora/relationships/autocomplete/target/',
	//Agregar autocomplete acá con los objetos
  );

  $form['symmetric'] = array(
	  '#type' => 'fieldset',
	  '#title' => 'Symmetry',
         '#prefix' => '<div id="fieldset_symm_replace">',
         '#suffix' => '</div>',
  );

  $form['symmetric']['inv-relationship'] = array(
    	'#type' => 'textfield',
    	'#title' => t('Symmetric relationship'),
	'#disabled' => TRUE,
	'#value' => textfield_inverseof_value($value_relationship),
  );

 $form['symmetric']['inv-relationship_ns'] = array(
    	'#type' => 'textfield',
    	'#title' => t('Symmetric namespace'),
	'#disabled' => TRUE,
	'#value' => textfield_inverseof_ns($value_relationship),
  );

  $form['symmetric']['sym-relationship'] = array(
    	'#markup' => description_inverseof_value($value_relationship),
  );

 $form['symmetric']['is-symmetric'] = array(
	  '#type' => 'checkbox',
	  '#disabled' => !(has_inverseof_value($value_relationship)),
	  '#title' => t('Add the corresponding relationship?'),
  );

$form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('submit'),
  );
  return $form;

}


function relationship_ajax_callback(&$form, $form_state) {

  return array(
    '#type' => 'ajax',
    '#commands' => array(
      	ajax_command_replace("#textfield_description_replace", render($form['rels']['description'])),
	ajax_command_replace("#fieldset_symm_replace", render($form['symmetric'])),
	ajax_command_replace("#fieldset_ns_replace", render($form['ns']))
    )
  );
}

function ns_has_alias($value_relationship, $object_pid) {

  if (empty($value_relationship)) {
	return FALSE;
  }
  list($value_ns, $value_prop) = rdfx_split_uri($value_relationship);

  $object = islandora_object_load($object_pid);
  if (!$object) {
    	drupal_set_message("Fedora object doesn't exist.");
    	drupal_not_found();
    	return FALSE;
  }
  $rels = $object->relationships->get();
  foreach ($rels as $rel){
	if ($rel['predicate']['namespace'] == $value_ns) {
		if (!empty($rel['predicate']['alias'])){
			return $rel['predicate']['alias'];
		}
	}
  }
  return FALSE;
}

function alias_exists($alias, $object_pid) {

  if (empty($alias)) {
	return TRUE;
  }

  $object = islandora_object_load($object_pid);
  if (!$object) {
    	drupal_set_message("Fedora object doesn't exist.");
    	drupal_not_found();
    	return TRUE;
  }
  $rels = $object->relationships->get();
  foreach ($rels as $rel){
	if ($rel['predicate']['alias'] == $alias) {
		return TRUE;
	}
  }
  return FALSE;
}



function textfield_namespace_value($value_relationship) {
  if (!empty($value_relationship)) {
	list($value_ns, $value_prop) = rdfx_split_uri($value_relationship);
  }
  return empty($value_ns) ? "No namespace" : $value_ns;
}

function textfield_description_value($value_relationship) {
  if (!empty($value_relationship)) {
  $vocabulary = get_current_vocabulary();
  $term = $vocabulary['terms'][$value_relationship];
	if (isset($term['comment'])) {
	  if (isset($term['comment']['en'])) {
		$servoc = $term['comment']['en'];
	  } else {
		$servoc = reset($term['comment']);
	  }
	}
	$value_description = $servoc;
  }
  return empty($value_description) ? "No description" : $value_description;

}

function textfield_inverseof_value($value_relationship) {
  if (!empty($value_relationship)) {
  	$vocabulary = get_current_vocabulary();
  	$term = $vocabulary['terms'][$value_relationship];
	if (isset($term['inverse'][0])) {
		list($value_ns, $value_inverse) = rdfx_split_uri($term['inverse'][0]);
	}
  }
  return empty($value_inverse) ? "No inverse" : $value_inverse;
}

function textfield_inverseof_ns($value_relationship) {
  if (!empty($value_relationship)) {
  	$vocabulary = get_current_vocabulary();
  	$term = $vocabulary['terms'][$value_relationship];
	if (isset($term['inverse'][0])) {
		list($value_ns, $value_inverse) = rdfx_split_uri($term['inverse'][0]);
	}
  }
  return empty($value_ns) ? "No inverse ns" : $value_ns;
}

function description_inverseof_value($value_relationship) {
  if (!empty($value_relationship)) {
  	$vocabulary = get_current_vocabulary();
  	$term = $vocabulary['terms'][$value_relationship];
	if (isset($term['inverse'][0])) {
		$term1 = $vocabulary['terms'][$term['inverse'][0]];
		if (isset($term1['label'])) {
	  		if (isset($term1['label']['en'])) {
				$servoc = $term1['label']['en'];
	  		} else {
				$servoc = reset($term1['label']);
	  		}
		}
		$value_inverse = "<B>".$servoc."</B>";
	}
  }

  return empty($value_inverse) ? "No inverse<HR>" : $value_inverse."<HR>";
}

function has_inverseof_value($value_relationship) {
  if (!empty($value_relationship)) {
  	$vocabulary = get_current_vocabulary();
  	$term = $vocabulary['terms'][$value_relationship];
	if (isset($term['inverse'][0])) {
		return TRUE;
	}
  }

  return FALSE;
}

function islandora_relationship_editor_add_relationships_submit($form, &$form_state) {
  $subject = islandora_object_load($form_state['values']['this']);
  if (!$subject) {
    form_set_error('target', "Subject error: Fedora object doesn't exist.");
    $form_state['redirect'] = FALSE;
    return ;
  }


  $object = islandora_object_load($form_state['values']['target']);
  if (!$object) {
    form_set_error('target', "Object error: Fedora object doesn't exist.");
    $form_state['redirect'] = FALSE;
    return ;
  }

  list($rel_ns, $rel_name) = rdfx_split_uri($form_state['values']['relationship']);  

  $relOk = 0;
  if ($form_state['values']['add_new_alias']){
	if (!(empty($form_state['values']['alias']))){
		if (alias_exists($form_state['values']['alias'], $form_state['values']['this'])) {
			form_set_error('alias', "Duplicated alias");
			$form_state['redirect'] = FALSE;
			return ;
		} else {
			$relOk = 1; // Add Alias and Insert Rel
		}
	} else {
		$relOk = 2; // Insert Rel with ns inline
	}
  } else {
	$relOk = 3; // Insert Rel with alias exists
  }

  $invOk = 0; 
  if ($form_state['values']['is-symmetric']) {
	if (ns_has_alias($form_state['values']['inv-relationship_ns'].$form_state['values']['inv-relationship'], $form_state['values']['target'])) {
		$invOk = 3; // Insert inv_rel with alias exists
	} else {
		if (($form_state['values']['inv-relationship_ns'] == $rel_ns) && (($relOk == 1) || ($relOk == 3))) {
			$invOk = 1; // Add Alias and Insert Inv_Rel
		} elseif (($form_state['values']['inv-relationship_ns'] == $rel_ns) && ($relOk == 2)) {
			$invOk = 2; // Insert Inv_Rel with ns inline
		} else {
			form_set_error('is-symmetric', "Inverse namespace alias error");
			$form_state['redirect'] = FALSE;
			return ;
		}
	}
  }



// Insert rel in RELS-EXT of subject
  if ($relOk == 1) {
	$subject->relationships->registerNamespace($form_state['values']['alias'], $rel_ns);
	$subject->relationships->add($rel_ns, $rel_name, $form_state['values']['target']);
  } elseif ($relOk > 1) {
	$subject->relationships->add($rel_ns, $rel_name, $form_state['values']['target']);
  }



// Insert inverse rel in RELS-EXT of object
  if ($invOk == 1) {
	$object->relationships->registerNamespace(empty($form_state['values']['curr_alias']) ? $form_state['values']['alias'] : $form_state['values']['curr_alias'], $form_state['values']['inv-relationship_ns']);
	$object->relationships->add($form_state['values']['inv-relationship_ns'], $form_state['values']['inv-relationship'], $form_state['values']['this']);
  } elseif ($invOk >1) {
	$object->relationships->add($form_state['values']['inv-relationship_ns'], $form_state['values']['inv-relationship'], $form_state['values']['this']);
  }


  $form_state['redirect'] = "islandora/object/{$subject->id}/manage/relationships";

}




