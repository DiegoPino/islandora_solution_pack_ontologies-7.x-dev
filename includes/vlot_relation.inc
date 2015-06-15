<?php

function islandora_ontologies_vlot_relationships_form(array $form, array &$form_state, $islandora_object) {
	module_load_include('inc', 'islandora_ontologies', 'includes/utilities');
	form_load_include($form_state, 'inc', 'islandora_ontologies', 'includes/vlot_relation');//Needed because we have to ways of calling this form and inside the wrapper 
   drupal_add_library('islandora_ontologies', 'islandora_ontologies_manage_vlot');
  //function islandora_ontologies_manage_relations the link to the embeded and needed functions in this file get`s lost.
	//islandora_ontologies_get_all_for_cmodel('biodiversity:OccurrenceCModel');
	
	
	
	if (!$islandora_object)
	{
		drupal_set_message(t("Islandora Object not found"));
		drupal_not_found();
		exit;	
	}	  
	$islandora_object_pid = $islandora_object->id;		
	//We don´t want to reprocess every bit every time we re-render or sumbit the form. So store the processed ontology values on form_state		
	$select_field="";

	if (!isset($form_state['islandora_ontologies_vlot']))
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
		$form_state['islandora_ontologies_vlot']['current']=$currentrelations;
		$form_state['islandora_ontologies_vlot']['restrictions']=$restrictionsOnOntologies;
		$form_state['islandora_ontologies_vlot']['posible_relations']=$posible_relastionships;
		$form_state['islandora_ontologies_vlot']['options_subjectcmodel']=$options_subjectCmodel;
    $models = $options_subjectCmodel_tmp;
    }
    $result=array();
      foreach($models  as $contentmodelpid)
      {
    		
      	if ($cache = cache_get('islandora_ontologies_adjacency'.$contentmodelpid))
      	{	
      		$result=$cache->data;
        }		
      	else
      	{
      		//If no cache present we get the full graph paths for this ContentModel
      		$fullgraph=islandora_ontologies_get_graph_path($contentmodelpid);
     		if (count($fullgraph)>0)
      		{
      			foreach ($fullgraph as $ontologiepid=>$localgraph)
      			{
      				if (count($localgraph)>0)
      				{
      					//Having the paths for every associated ontologies, we traverse this structure using getdepthfirst. This way we get an array with every "road" or consecutive "edges" a.k.a relations 
      					//that connect this type of Content Model (origin) to every other one in the full Graph. 
      					$g = new islandora_ontologies_adjacency_graph($localgraph);
      					$result[$ontologiepid]=$g->getdepthFirst("info:fedora/".$contentmodelpid);
      				}
      			}
      				//lastly we set the cache. This is necesarry because we don´t wan´t to recalculate every time the same structure. Only if a Ontology changes, or new ones are added, the cache is cleaned.		
      			cache_set('islandora_ontologies_adjacency'.$contentmodelpid, $result, 'cache');	
      		}
      	}	

      }
      $vlot_settings=array();
      //Now build an vlot vis settings array based on the cmodels, that will serve as containers for this workflow. What we are building here is a container 
      
      foreach($result as $ontologie_paths) {
        foreach($ontologie_paths as $individual_path) {
          $cmodel_nodes_keys = array_keys($individual_path); //get the keys, our graphs are handled as acyclic, so we won't loose any state
         
          foreach($individual_path as $cmodel_node=>$cmodel_node_relation)
          {
           
            $next_cmodel = next($cmodel_nodes_keys);
            if ($next_cmodel!==false) {
              $model_label=$cmodel_node;
              $model_label=preg_match('/^(.*[\/\#])([^\/\#]+)$/', $model_label, $m)? $m[2]: $model_label;
              if (!isset($vlot_settings[$cmodel_node])) {
                 $vlot_settings[$cmodel_node]=array('relates_to'=>array(),'label'=>$model_label,'node_cmodel'=>$model_label,'node_real_cmodel'=>$cmodel_node,'nodetype'=>'placeholder','link'=>url("islandora/object/{$cmodel_node}",array('absolute'=>TRUE)));
              }
              
              $relation=$cmodel_node_relation[0];
              $rel_label=preg_match('/^(.*[\/\#])([^\/\#]+)$/',$relation, $m)? $m[2]: $relation;
						  //Now add this object as adjacent to the previous one, if not already done
               $rel_label=preg_match('/^(.*[\:\/])([^\:\/]+)$/', $rel_label, $m)? $m[2]: $rel_label;
              
              if (!isset($vlot_settings[$cmodel_node]['relates_to'][$next_cmodel])) {
                $vlot_settings[$cmodel_node]['relates_to'][$next_cmodel]=array($cmodel_node_relation[0]=>$rel_label);
              }
              if (!isset($vlot_settings[$cmodel_node]['relates_to'][$next_cmodel][$cmodel_node_relation[0]])) {
                  $vlot_settings[$cmodel_node]['relates_to'][$next_cmodel][$cmodel_node_relation[0]]=$rel_label;
                }
            }
          } 
        }
      }
      dpm($form_state['islandora_ontologies_vlot']);
      
      
      

	$value_relationship=!empty($form_state['values']['relationship']) ? $form_state['values']['relationship'] : "";	
	$value_subjectCmodel=!empty($form_state['values']['subjectCmodel_select']) ? $form_state['values']['subjectCmodel_select']:"";
	
	$form = array();
	$form['this_vlot_object'] = array(
		'#title' => t('This Object'),
		'#type' => 'textfield',
		'#value' => $islandora_object_pid,
		'#disabled' => TRUE,
  );
  if (count($vlot_settings)>1) //only one means the current object (starting object of the graph)
  				{
  				$vlot_graph_style=islandora_ontologies_vlot_get_info('islandora_red_biodiversidad'); //@see hook_islandora_ontologies_vlot_info and islandora_ontologies_vlot_get_info
  				$vlot =islandora_ontologies_build_manage_vlot($vlot_graph_style, $vlot_settings, '840','800');
  				
          $form['this_vlot_graph']= array (
          '#markup'=>drupal_render($vlot),
        );
            
  }
  
  
  //dpm($result);
  //dpm($form_state['islandora_ontologies_vlot']);
return $form;

}







