<?php

function islandora_ontologies_add_relationships_form(array $form, array &$form_state, $islandora_object) {
    module_load_include('inc', 'islandora_ontologies', 'includes/utilities');
	form_load_include($form_state, 'inc', 'islandora_ontologies', 'includes/add_relation');//Needed because we have to ways of calling this form and inside the wrapper function islandora_ontologies_manage_relations the link to the embeded and needed functions in this file get`s lost.
	
	$islandora_object_pid = $islandora_object->id;		
	if (!$islandora_object)
			{
				drupal_set_message(t("Islandora Object not found"));
				drupal_not_found();
				exit;	
			}	  
  	
//We don´t want to reprocess every bit every time we re-render or sumbit the form. So store the processed ontology values on form_state		
$select_field="";

if (!isset($form_state['islandora_ontologies']))
	{
	$restrictionsOnOntologies=array();
		$posible_relastionships=array();
		$currentrelationsfiltered=array();
		$options_subjectCmodel=array();
$currentontologies= islandora_ontologies_get_associated_ontologies($islandora_object);//@see utilities.inc
$currentrelations=islandora_ontologies_get_current_localrelations($islandora_object_pid);//@see utilities.inc

foreach($currentontologies as $subject=>$ontologies)
	{
	
	$posible_relastionships[$subject]=islandora_ontologies_get_possible_localrelations($subject,$ontologies);
	$currentrelationsfiltered[$subject]=islandora_ontologies_filter_existing_relations($subject,$ontologies,$currentrelations);	
	foreach($ontologies as $ontology_objectpid=>$label)
		{
		$restrictionsOnOntologies[$subject][$ontology_objectpid]=islandora_ontologies_get_restricted_properties($ontology_objectpid,$subject,$currentrelationsfiltered[$subject][$ontology_objectpid]);
		}
	$value_subjectCmodel=$subject;//Lets set any as default
	}
	$options_subjectCmodel_tmp=array_keys($currentontologies);
	if (sizeof($options_subjectCmodel_tmp)>0)
		{
		$options_subjectCmodel=array_combine($options_subjectCmodel_tmp,$options_subjectCmodel_tmp);//keys and values the same
		}
$form_state['islandora_ontologies']['current']=$currentrelations;
$form_state['islandora_ontologies']['restrictions']=$restrictionsOnOntologies;
$form_state['islandora_ontologies']['posible_relations']=$posible_relastionships;
$form_state['islandora_ontologies']['options_subjectcmodel']=$options_subjectCmodel;
}

$value_relationship=!empty($form_state['values']['relationship']) ? $form_state['values']['relationship'] : "";	
$value_subjectCmodel=!empty($form_state['values']['subjectCmodel_select']) ? $form_state['values']['subjectCmodel_select']: "";
	
  $form = array();
  $form['this'] = array(
    '#title' => t('This Object'),
    '#type' => 'textfield',
    '#value' => $islandora_object_pid,
    '#disabled' => TRUE,
  );



  $form['subject'] = array(
	  '#type' => 'fieldset',
	  '#title' => 'Subject Class(CMODEL)',
  );
  $form['subject']['subjectCmodel'] =array(
    '#type' => 'hidden',
    '#value'=> $value_subjectCmodel,
	'#default_value'=> $value_subjectCmodel,
);

 if (sizeof($form_state['islandora_ontologies']['posible_relations'])>0)  //means we have more than one CMODEL as subject, so me must deal with restrictions based on that...
 	{
    $form['subject']['subjectCmodel_select'] = array(
      '#type' => 'select',
      '#title' => t('Content Models for this Object'),
      '#options' => $form_state['islandora_ontologies']['options_subjectcmodel'],
      '#required' => TRUE,
      '#ajax' => array(
          'method' => 'replace',
          'effect' => 'fade',
          'event' => 'change',
          'callback' => 'relationship_subject_ajax_callback',//I woud really love ajax callbacks could use parameters... 
       ),
    );	
 	}
    $form['rels'] = array(
  	  '#type' => 'fieldset',
  	  '#title' => 'Relationship',
	  '#prefix' => '<div id="relationship_replace">',
	  '#suffix' => '</div>',
    );
   	   
  $form['rels']['relationship'] = array(
    '#type' => 'select',
    '#title' => t('Relationship'),
    '#options' => select_relationsship_values($value_subjectCmodel,$form_state['islandora_ontologies']['posible_relations']),
    '#required' => TRUE,
    '#ajax' => array(
      	'method' => 'replace',
      	'effect' => 'fade',
        'event' => 'change',
        'callback' => 'relationship_ajax_callback',//I woud really love ajax callbacks could use parameters... 
     ),
     '#prefix' => '<div id="relationship_select_replace">',
     '#suffix' => '</div>',
  );
	

  $form['rels']['description'] = array(
    	'#markup' => textfield_description_value($value_relationship,$form_state['islandora_ontologies']['posible_relations'],$value_subjectCmodel),
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
	'#value' => textfield_namespace_value($value_relationship,$form_state['islandora_ontologies']['posible_relations'],$value_subjectCmodel),
  );

 /*$form['ns']['curr_alias'] = array(
    	'#type' => 'textfield',
    	'#title' => t('Namespace alias'),
	'#disabled' => TRUE,
	'#value' => "",//isset($form_state['values']['relationship']) ? ns_has_alias($value_relationship, $object_pid) : "",
  );*/

  /*$form['ns']['add_new_alias'] = array(
	  '#type' => 'checkbox',
	  '#disabled' => TRUE,
	  '#title' => t('Add NEW alias'),
	  '#default_value' => FALSE,
	  '#value' => "",//(isset($form_state['values']['relationship']) && !(ns_has_alias($value_relationship, $object_pid))) ? TRUE : FALSE,
  );*/

 /*$form['ns']['alias'] = array(
    	'#type' => 'textfield',
    	'#title' => t('NEW Namespace alias (blank for inline namespace)'),
	'#disabled' => "",//(isset($form_state['values']['relationship']) && !(ns_has_alias($value_relationship, $object_pid))) ? FALSE : TRUE,
  );*/

  $form['obj'] = array(
	  '#type' => 'fieldset',
	  '#title' => 'Object',
      '#prefix' => '<div id="fieldset_objtarget_replace">',
      '#suffix' => '</div>',
	  
  );
  $form['obj']['msg'] = array(
	  '#markup'=>t('<b>Select a relationship first</b>'),
	  '#access'=>empty($value_relationship),
  );
  $form['obj']['target'] = array(
    '#type' => 'textfield',
    '#title' => t('Target object'),
    '#required' => TRUE,
	'#autocomplete_path' => textfield_autocomplete_value($value_relationship,$form_state['islandora_ontologies']['restrictions'],$value_subjectCmodel),
	'#access'=>!empty($value_relationship),
	//Autocomplete_path, must be dinamic added to allow only objects of restricted class for this property 
  );
  $form['obj']['restriction']=array(
	  '#items' => items_restrictions_value($value_relationship,$form_state['islandora_ontologies']['restrictions'],$value_subjectCmodel),
	  '#theme' => 'item_list',
	  
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


function relationship_subject_ajax_callback(&$form, $form_state) {
  $ajax = array( '#type' => 'ajax', '#commands' =>'');
 
  if (!empty($form_state['values']['subjectCmodel_select'])) 
  	{
			 
 	$ajax['#commands']=array(
    ajax_command_replace("#relationship_replace", render($form['rels'])),
	//ajax_command_replace("#fieldset_symm_replace", render($form['symmetric'])),
	ajax_command_replace("#fieldset_ns_replace", render($form['ns'])),
	ajax_command_replace("#fieldset_objtarget_replace", render($form['obj'])),
	//ajax_command_invoke("#edit-obj",'show'),
	
  );
}
else
	{
	$ajax['#commands']=array(
	ajax_command_replace("#fieldset_objtarget_replace", render($form['obj'])),
	ajax_command_replace("#relationship_replace", render($form['rels'])),
);
	}
return $ajax;
}
function relationship_ajax_callback(&$form, $form_state) {
  $ajax = array( '#type' => 'ajax', '#commands' =>'');
  
  if (!empty($form_state['values']['relationship'])) 
  	{
		$form['obj']['target']['#value']="";//lets clear this;	 
 	$ajax['#commands']=array(
    ajax_command_replace("#textfield_description_replace", render($form['rels']['description'])),
	//ajax_command_replace("#fieldset_symm_replace", render($form['symmetric'])),
	ajax_command_replace("#fieldset_ns_replace", render($form['ns'])),
	ajax_command_replace("#fieldset_objtarget_replace", render($form['obj'])),
	//ajax_command_invoke("#edit-obj",'show'),
	
  );
}
else
	{
	$form['obj']['target']['#value']="";	
		$ajax['#commands']=array(
	ajax_command_replace("#fieldset_objtarget_replace", render($form['obj'])),
);
	}
return $ajax;
}
/*function ns_has_alias($value_relationship, $object_pid) {

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
}*/

/*function alias_exists($alias, $object_pid) {

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
*/
function select_relationsship_values($value_subjectCmodel,$possible_relations) {
	
  if (!empty($value_subjectCmodel)) {
	  $options=$possible_relations[$value_subjectCmodel]['posiblerels_asformoptions'];
  
  }
  return empty($options) ? array() : $options;
};

function textfield_namespace_value($value_relationship,$possible_relations,$subject) {
  if (!empty($value_relationship)) {
	  list($ontology,$property)=explode(" ",$value_relationship);
	  dpm($possible_relations[$subject]['posiblerels_complete']);
	$value_ns = $possible_relations[$subject]['posiblerels_complete'][$ontology][$property]['ns'];
  }
  return empty($value_ns) ? "No namespace" : $value_ns;
};

function textfield_description_value($value_relationship,$possible_relations,$subject) {
  
  if (!empty($value_relationship)) {
	  list($ontology,$property)=explode(" ",$value_relationship);
	  $value_description=$possible_relations[$subject]['posiblerels_complete'][$ontology][$property]['comment'];
  }
 return empty($value_description) ? "No description" : $value_description;
}
function textfield_autocomplete_value($value_relationship,$restrictionson_properties,$subject) {
	$autocomplete_target='islandora/relationships/autocomplete/target/';
  if (!empty($value_relationship)) {
	  list($ontology,$property)=explode(" ",$value_relationship);
	  if ($restrictionson_properties[$subject][$ontology][$property]['onClass'])
	  	{
	   $autocomplete_target=$autocomplete_target.str_replace("info:fedora/","",$restrictionson_properties[$subject][$ontology][$property]['onClass'])."/";
   		}
	  else
		{
		$autocomplete_target=$autocomplete_target."//";	
		}  	
  }
 return $autocomplete_target;
}

function items_restrictions_value($value_relationship,$restrictionson_properties,$subject) {
  $value_restrictions=array();
  if (!empty($value_relationship)) {
	  list($ontology,$property)=explode(" ",$value_relationship);
	  $value_restrictions=$restrictionson_properties[$subject][$ontology][$property]['msg'];
	 
  }
  return $value_restrictions;
}

function textfield_inverseof_value($value_relationship) {
  if (!empty($value_relationship)) {
  	//$vocabulary = get_current_vocabulary();
  	$term = $vocabulary['terms'][$value_relationship];
	if (isset($term['inverse'][0])) {
		//list($value_ns, $value_inverse) = rdfx_split_uri($term['inverse'][0]);
	}
  }
  return empty($value_inverse) ? "No inverse" : $value_inverse;
}

function textfield_inverseof_ns($value_relationship) {
  if (!empty($value_relationship)) {
  	//$vocabulary = get_current_vocabulary();
  	$term = $vocabulary['terms'][$value_relationship];
	if (isset($term['inverse'][0])) {
		//list($value_ns, $value_inverse) = rdfx_split_uri($term['inverse'][0]);
	}
  }
  return empty($value_ns) ? "No inverse ns" : $value_ns;
}

function description_inverseof_value($value_relationship) {
  if (!empty($value_relationship)) {
  	//$vocabulary = get_current_vocabulary();
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
  	//$vocabulary = get_current_vocabulary();
  	$term = $vocabulary['terms'][$value_relationship];
	if (isset($term['inverse'][0])) {
		return TRUE;
	}
  }

  return FALSE;
}

function islandora_ontologies_add_relationships_form_submit($form, &$form_state) {
	
  $subject = islandora_object_load($form_state['values']['this']);//Subject in the sense of the actual relation. We used subject before what in the meaning of subject(class) in the ontologi
  $possible_relations=$form_state['islandora_ontologies']['posible_relations'];
  $current_relations=$form_state['islandora_ontologies']['current'];
 
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
  
 
  
  $subjectCMODEL=$form_state['values']['subjectCmodel_select'];
  list($ontology,$property)=explode(" ",$form_state['values']['relationship']);
 
	$rel_ns = $possible_relations[$subjectCMODEL]['posiblerels_complete'][$ontology][$property]['ns'];
    $rel_name =  $possible_relations[$subjectCMODEL]['posiblerels_complete'][$ontology][$property]['proplocal'];
	
	
	if (empty($rel_ns) || empty($rel_name))
		{
		  form_set_error('relationship', "Relationship error: No property or namespace absent");
		  $form_state['redirect'] = FALSE;
		  return ;	
		}
	  //Don´t allow duplicated relations please!	
	 foreach ($current_relations as $key=>$relation)
	    	{
	    	if (($relation['object']['value']==$form_state['values']['target'])&&($relation['predicate']['value']==$rel_name) && ($relation['predicate']['namespace']==$rel_ns))
				{
				form_set_error('relationship', t("Relationship error: this relation is already present"));
  		 	 	$form_state['redirect'] = FALSE;
  		  	    return ;
				}	
			}	
		
		
  $subject->relationships->add($rel_ns, $rel_name, $form_state['values']['target']);
  $form_state['redirect'] = "islandora/object/{$subject->id}/manage/relationships";

}




