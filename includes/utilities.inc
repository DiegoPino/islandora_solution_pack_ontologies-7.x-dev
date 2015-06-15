<?php
/*
* @file
* Holds helper functions for islandora_red_biodiversidad.
*/

/**
* This function will add an DS composite datastream to an object.
*
* This is a workaround of a known issue with temp files being lost in Tuque in
* batches because of object destruction during the serialization process.
*
* @param AbstractObject $islandora_object
*   The object to add the DS composite datastream to.
* @see Entities solution pack
*/
use Ruler\RuleBuilder;
use Ruler\Context;
use Ruler\RuleSet;
$module_path = drupal_get_path('module', 'islandora_ontologies');
define('CLASS_DIR', $module_path);

function islandora_ontologies_ingest__ontologies_ds_composite_datastream(AbstractObject $islandora_object) {
	$module_path = drupal_get_path('module', 'islandora_ontologies');

	$ds_composite_datastream = $islandora_object->constructDatastream('DS-COMPOSITE-MODEL', 'X');
	$ds_composite_datastream->label = 'DS-COMPOSITE-MODEL';
	$ds_composite_datastream->mimetype = 'text/xml';
	$ds_composite_datastream->setContentFromFile("$module_path/xml/islandora_ontologies_ds_composite_model.xml", FALSE);
	$islandora_object->ingestDatastream($ds_composite_datastream);
}
/**
* This function will read the ONTOLOGY Stream from an object and add relations to matching CMODEL objects PIDS to owl:classes
* Work in progress...not used nor implemented
*
*/
function islandora_ontologies_mapowlclasses_to_cmodels(AbstractObject $islandora_object)
{
	$module_path = drupal_get_path('module', 'islandora_ontologies');
	
	if (empty($islandora_object['ONTOLOGY']) || !islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $islandora_object['ONTOLOGY'])) {
		return array();
	}
	//islandora_ontologies_parse_vocabulary($islandora_object);
	return array();
	
	
	
}
/** 
 * Gets delete menu path for a relationship
 */
function islandora_ontologies_get_delete_path($objectPid, $namespace, $relationship, $targetPid) {
  $namespace = $namespace ? $namespace : 'NULL';
  $relationship = $relationship ? $relationship : 'NULL';
  $targetPid = $targetPid ? $targetPid : 'NULL';

  return 'islandora/object/' . $objectPid . '/manage/relationships/delete/' . $namespace . '/' . $relationship . '/' . $targetPid;
}

/**
* This function will return all CMODELS, classes, in an ontology
*
* If there are ontologie objects associated it returns the ontologie objects
*
* @param AbstractObject $islandora_object
*   The object to check for it´s content models and associated ontology objects.
* @return Array $associated_ontologies
* An array with the Ontology Object PIDS
*/
function islandora_ontologies_get_clasesin_ontologies(AbstractObject $ontology_object)
{

	$PID=$ontology_object->id;
	
	$fedora_url = variable_get('islandora_base_url', 'http://localhost:8080/fedora');
	$xmlfilelink = $fedora_url."/objects/".$PID."/datastreams/ONTOLOGY/content";
	$graph = new Graphite();
	$graph->load($xmlfilelink);
	$existing_classes=array();
	
	foreach($graph->allOfType('owl:Class') as $resource_list)
	{
		$parts = ARC2::splitURI($resource_list->uri);//We need don´t need in our keys the info:fedora part, but we must check if it´s there
		if ($parts[0]!='info:fedora/')
			{
			continue;	
			}
		$existing_classes[$parts[1]]=$resource_list->uri;
		//@TODO check if ontology is correct before adding associate option? This would be almost a reasoner. Not sure if neccesary.
		
	}
  return $existing_classes;

}


/**
 * Get data about objects associated with this pid through any rels-ext.
 *
 * @TODO: Add exclusive option - only return objects whose only relation
 * is to this object.
 *
 * @param int $pid
 *   The parent object's pid.
 *
 * @param int $page_number
 *   Used to calculate offset
 *
 * @param int $page_size
 *   If > 0, limit count of results to this number
 *
 * @return array
 *   An array containing pids of child objects
 */
function islandora_ontologies_relatedto_objects($pid, $page_number = 0, $page_size = 0) {
  $connection = islandora_get_tuque_connection();
  $query = <<<EOQ
SELECT ?object ?predicate
FROM <#ri>
WHERE {
  ?object <fedora-model:label> ?title ;
  ?predicate <info:fedora/{$pid}> .
  !optionals
  !filters
}

EOQ;
  //XACML permissions
  $collection_predicate_filters="";
  $query_optionals = (array) module_invoke('islandora_xacml_api', 'islandora_basic_collection_get_query_optionals', 'manage');
  $query_filters = (array) module_invoke('islandora_xacml_api', 'islandora_basic_collection_get_query_filters');
  //Exclude collection predicates here
  $collection_predicate_filters = " !sameTerm(?predicate, <fedora-rels-ext:isMemberOfCollection>) && !sameTerm(?predicate, <fedora-rels-ext:isMemberOf>) && !sameTerm(?predicate, <fedora-model:hasModel>)";
  $query_filters[] = $collection_predicate_filters;
  $filter_map = function ($filter) {
    return "FILTER($filter)";
  };
 
 
  $query = format_string($query, array(
    '!optionals' => !empty($query_optionals) ? ('OPTIONAL {{' . implode('} UNION {', $query_optionals) . '}}') : '',
    '!filters' => implode(' ', array_map($filter_map, $query_filters)),
  ));
  
 
  // Include paging parameters if provided.
  if ($page_number > 0 && $page_size > 0) {
    // Add in the offset.
    $offset = $page_number * $page_size;
    $query .= " offset $offset";
  }
  if ($page_size > 0) {
    // Add in the limit.
    $query .= " limit $page_size";
  }
 
  $queryresults = $connection->repository->ri->sparqlQuery($query);

  return $queryresults;
}



/**
* This function will read the content models of an Objects and searches if this contentmodels are associated to an Ontology Object
*
* If there are ontologie objects associated it returns the ontologie objects
*
* @param AbstractObject $islandora_object
*   The object to check for it´s content models and associated ontology objects.
* @return Array $associated_ontologies
* An array with the Ontology Object PIDS
*/
function islandora_ontologies_get_associated_ontologies(AbstractObject $islandora_object)
{
	$associated_ontologies=array();
	$target_object_models=$islandora_object->models;
	
	
	foreach($target_object_models as $contentmodelpid)
	{
	
		if ($contentmodelpid!="fedora-system:FedoraObject-3.0")
		{
			
			unset($ontologies);
			unset($contentmodel_object);
			try {
				
				$contentmodel_object =islandora_object_load($contentmodelpid);
				}
			catch (Exception $e) 
			{
				drupal_set_message(t("Content Model does not exist!"));
				drupal_not_found();	
				continue;
			}
			if ($contentmodel_object)
			{
			
				$ontologies=islandora_ontologies_get_associated_ontologies_for_cmodel($contentmodel_object);
				
				if (sizeof($ontologies)>0)
					{
					$associated_ontologies[$contentmodelpid]=$ontologies;		
					}
						
				}	
			}
		}
		
	return $associated_ontologies;
}
/**
* This function will read a content model Objects and search if there are associated Ontology Objects
*
* If found, the ontologie objects are returned
*
* @param AbstractObject $islandora_object
*   The object to check for it´s content models and associated ontology objects.
* @return Array $associated_ontologies
* An array with the Ontology Object PIDS
*/
function islandora_ontologies_get_associated_ontologies_for_cmodel(AbstractObject $islandora_cmodel_object)
{
	$associated_ontology=array();
	
	
			if ($islandora_cmodel_object)
			{
				
				$rels=$islandora_cmodel_object->relationships->get(ISLANDORA_RELS_EXT_URI,'hasLocalOntology',NULL,RELS_TYPE_URI);	
				
				foreach($rels as $existingrelations)
				{
					try {
						$ontology_object=islandora_object_load($existingrelations['object']['value']);
					}
					catch (Exception $e) 
					{
						continue;
					}
					if ($ontology_object && isset($ontology_object['ONTOLOGY']))//we have an associated Ontology object and it has an ONTOLOGY Datastream
					{
						$associated_ontology[$ontology_object->id]=$ontology_object->label;
								
					}
					else
					{
						exit;//or continue? we are getting only one for now.@todo add all of them. Must check, but all other functions are aware of multiple ontologies.
					}
						
				}	
			}	
			
	return $associated_ontology;
}
/**
* This function gets all exisiting relations for a Subject, Islandora Object, leaving Fedoras System Relations out
* @param string $subject_pid, the PID of the Object who holds relations
*   The object to check for it´s content models and associated ontology objects.
* @return Array $relslocal
* An array with the the local relations
*/
function islandora_ontologies_get_current_localrelations($subject_pid)
{
	$subject= islandora_object_load($subject_pid);
	if (!$subject) {
		drupal_set_message("Fedora object doesn't exist.");
		drupal_not_found();
		exit;
	}
	$not_system_rels= function($rel) {
    //Filter out info:fedora/fedora-system:def/model and xacml/other islandora ones
    if ( strpos($rel['predicate']['namespace'], 'info:fedora/fedora-system:def/')===FALSE && strpos($rel['predicate']['namespace'],'http://islandora.ca/ontology/relsext#')===FALSE) {
      return TRUE;
    }
    else {
      return FALSE;
    }
      
	};
	
	$rels = $subject->relationships->get();
	$relslocal=array_filter($rels, $not_system_rels);
	return $relslocal;
}
/**
* This function gets all posible relations for a Subject based on their associated Ontologie
* @param array $subject_associated_ontologies_pids, subject + PIDs of the Ontologies
*   The object to check for it´s content models and associated ontology objects.
* 1 Subject, multiple ontologies
* @return Array 
* An array with the the relations and preprocessed form options as plus.
*/
function islandora_ontologies_get_possible_localrelations($subject,$subject_associated_ontologies_pids)
{
	$module_path = drupal_get_path('module', 'islandora_ontologies');
	include_once($module_path.'/arc2/ARC2.php');
	include_once($module_path.'/Graphite/Graphite.php');	
	
	
	$posiblerels_complete=array();
	$posiblerels_asformoptions=array();
	$uri_parts_inverse=array();
	$uri_parts_inverse=array();
	$inverse_array=array();
	$inverseof_array=array();
	foreach($subject_associated_ontologies_pids as $PID=>$label)
	{
		
		unset($ontology_object);
		$ontology_object=islandora_object_load($PID);
		if (!$ontology_object) {
			drupal_set_message("Ontology object doesn't exist.");
			drupal_not_found();
			continue;
		}	
		$ontology_label=$ontology_object->label;//We already have this as $label, but it does not hurt to recheck
		
		$fedora_url = variable_get('islandora_base_url', 'http://localhost:8080/fedora');
		$xmlfilelink = $fedora_url."/objects/".$PID."/datastreams/ONTOLOGY/content";
		$graph = new Graphite();//@TODO Check if it´s posible to combine all ontologies in one Graphite object and parse the whole thing 
		$graph->load($xmlfilelink);
		foreach($graph->allOfType('owl:ObjectProperty')->sort("rdfs:label") as $property)
		{
			
			
			$inverse="";
			$inverseof="";	
			$inverse_array=array();
			$inverseof_array=array();
			if (!$property->get("-owl:onProperty")->get("-rdfs:subClassOf")->isNull())
			{
			foreach($property->all("-owl:onProperty") as $prop)//Could be multiple, if the same property is used across mult. class restrictions. Fixes previous false asumption!
				{
				
				
			if ($prop->get("-rdfs:subClassOf")->uri == "info:fedora/".$subject)   //If this URI equals the subject we found an restricted $property for this class. If its a null node or inexistant it will give and GraphiteNull Object, no need to check for it.
			{
				//We won´t deal with the nature of the restriction here. We just wan´t a list of possible/defined/restricted properties for this particular class
				$inverse=$property->get('-owl:inverseOf')->isNull() ? '' : $property->get('-owl:inverseOf')->uri;
				$inverseof=$property->get('owl:inverseOf')->isNull() ? '' : $property->get('owl:inverseOf')->uri;
				//we must deal with inverse and inverseof differently. If this property has an inverseof then, when the validation is made, we must invert subject/object
				//But when we have just an inverse, it means we can give our object part from the tripple this propery without question. assuming the ontology is well (logical)formed. 
				$uri_parts = ARC2::splitURI($property->uri);
				if (!empty($inverse))
					{
					$uri_parts_inverse=ARC2::splitURI($inverse);
					$inverse_array=array('ns'=>$uri_parts_inverse[0],'proplocal'=>$uri_parts_inverse[1]);	
					}
				if (!empty($inverseof))
						{
						$uri_parts_inverse=ARC2::splitURI($inverseof);
						$inverseof_array=array('ns'=>$uri_parts_inverse[0],'proplocal'=>$uri_parts_inverse[1]);	
						}	
				$posiblerels_complete[$PID][$property->uri]=array(
					'label'=> $property->label()->isNull() ? $uri_parts[1] : $property->label()->toString(),//If we don´t have a label display the short value of the property
					'inverse'=> sizeof($inverse_array)>0?$inverse_array:'',
					'inverseof'=> sizeof($inverseof_array)>0?$inverseof_array:'',
					'ns'=>$uri_parts[0],//Not entirely correct. If there are namespace alias, we must check this in the outerloop.
					'comment'=>$property->getString('rdfs:comment'),
					'proplocal'=>$uri_parts[1],	
					'alias'=>'',//@todo read the complete namespace from the ontology and check for namespaces alias	
				);
				$posiblerels_asformoptions[$ontology_label][$PID." ".$property->uri]=$posiblerels_complete[$PID][$property->uri]['label'];//To avoid other loops elsewhere we process the needed form options here. 
				//Note: our option value is composed of the PID of the ontology object and the property URL. this way we can later discriminated from which ontology object we are going to evaluate our restrictions		
			}
			
		    }
			
		}
		}
			
	}
	
	return array('posiblerels_complete'=>$posiblerels_complete,'posiblerels_asformoptions'=>$posiblerels_asformoptions);
}
/**
* This function Filters existing relations and returns only those defined in the ontologies associated. This is necessary because we don´t wan´t to limit or restrict properties out of our scope.
* @param string $subject_pid, the PID of the Object who holds relations
*   The object to check for it´s content models and associated ontology objects.
* @return Array $relsfiltered
* An array with the the relations defined in "key" ontology_pid 
*/
function islandora_ontologies_filter_existing_relations($subject, array $subject_associated_ontologies_pids, array $currentlocalrelations)
{
	$module_path = drupal_get_path('module', 'islandora_ontologies');
	include_once($module_path.'/arc2/ARC2.php');
	include_once($module_path.'/Graphite/Graphite.php');	
	$existing_properties_in_ontology=array();	
	$relsfiltered=array();
	$local_defined_in_ontology = function($rel) use ($existing_properties_in_ontology)
	{
		
	
		return (in_array($rel['predicate']['namespace'].$rel['predicate']['value'],$existing_properties_in_ontology));//Only true if the current predicate exists in this ontology
	};
	
	$content_models_for_relation = function($rel) 
		{
			if ($rel['object']['value'])
				{
					try{
					$rel_object=islandora_object_load($rel['object']['value']);			
					}
					catch (Exception $e) 
					{
					drupal_drupal_set_message("Error. existing relation to a not existing Object");
					drupal_not_found();
					}
				if ($rel_object)
					{
					$rel['object']['cmodels']=array_map(function($cm){return "info:fedora/".$cm;},$rel_object->models);//We need the info:fedora/ part to compare againts de ontology definitions	
          }
					else{
					$rel['object']['cmodels']=array('error');	
					}
						
						
				}
				
			return $rel;	
		
		};
	foreach($subject_associated_ontologies_pids as $PID=>$label)
	{
		unset($ontology_object);
		unset($existing_properties_in_ontology);
		$ontology_object=islandora_object_load($PID);
		if (!$ontology_object) {
			drupal_set_message("Ontology object doesn't exist.");
			drupal_not_found();
			exit;
		}	
		$ontology_label=$label;
		
		$fedora_url = variable_get('islandora_base_url', 'http://localhost:8080/fedora');
		$xmlfilelink = $fedora_url."/objects/".$PID."/datastreams/ONTOLOGY/content";
		$graph = new Graphite();
		$graph->load($xmlfilelink);
		foreach($graph->allOfType('owl:ObjectProperty')->sort("rdfs:label") as $property)
		{
			
			$existing_properties_in_ontology[]=$property->uri;
		
		}
		
		$relsfiltered[$PID]=array_filter($currentlocalrelations,function($rel) use ($existing_properties_in_ontology)
	{
		
	
		return (in_array($rel['predicate']['namespace'].$rel['predicate']['value'],$existing_properties_in_ontology));//Only true if the current predicate exists in this ontology
	});
		
		$relsfiltered[$PID]=array_map($content_models_for_relation,$relsfiltered[$PID]);	
	}
	
	return $relsfiltered;
}




/**
* This function gets restrictions for a Class ($subject)
* Optional: if onClass and/or onProperty is given, it limits it return values to this matches
* @param $ontology_objectpid. The pid of the object that contains the Ontology 
* @param $subject. The Class that is being restricted (ContentmodelPID) with or without the info:fedora/ part
* @param $rels. Current Relations array for this object. Accept Only those properties who are not defined in this ontology. 
* you can use islandora_ontologies_filter_existing_relations($subject,$ontologies,$currentrelations) to filter an existing set;If empty should be of type array
* @param $onClass. only those restrictions that apply to a givenClass (otherContentmodelPID) with or without the info:fedora/ part. Default Null
* @param $onProperty. only those restrictions for a specific property. Full with ns.
* @return $restrictionsOnproperties with the properties,onClass and cardinality/existence restrictions and some human readable messages
*/
function islandora_ontologies_get_restricted_properties($ontology_objectpid,$subject,array $rels,$onClass = NULL,$onProperty = NULL)
{
	$module_path = drupal_get_path('module', 'islandora_ontologies');
	include_once($module_path.'/arc2/ARC2.php');
	include_once($module_path.'/Graphite/Graphite.php');
	$classloader=function($c) use ($module_path)
		{
			@include $module_path."/".preg_replace('#\\\|_(?!.+\\\)#','/',$c).'.php';
		};
	spl_autoload_register($classloader);
	
	$rb = new RuleBuilder;//We create our megarule first
	//Now we define our construction blocks
	$ruleset_currentrelations_compliant = new RuleSet;
	$context = new Context;
	//I need some flexibility, let´s allow both forms for $subject and $onclass.
	if (strpos($subject, 'info:fedora/')===FALSE)
		{
		$subject="info:fedora/".$subject;
		}
		
	$onClass=(isset($onClass) && (strpos($subject, 'info:fedora/')===FALSE))? "info:fedora/".$onClass: $onClass;	
		
	$restrictionsOnproperties=array();//our return array
	
	
	$PID=$ontology_objectpid;	
	$fedora_url = variable_get('islandora_base_url', 'http://localhost:8080/fedora');
	$xmlfilelink = $fedora_url."/objects/".$PID."/datastreams/ONTOLOGY/content";
	$graph = new Graphite();
	$graph->load($xmlfilelink);
	$existing_classes=array();
	
	$cardinality = $rb->create(
	    $rb->logicalAnd(
	        $rb['minNumRel']->lessThanOrEqualTo($rb['matchedNumRels']),
	        $rb['maxNumRel']->greaterThanOrEqualTo($rb['matchedNumRels'])
	    ),
	    function() use ($context,&$restrictionsOnproperties) {
	       // if ($context['maxNumRel']-$context['matchedNumRels'])>0)//Means we have some space for another
			
			$restrictionsOnproperties[$context['OnProperty']]=array(
					'msg'=> array('Cardinality on '.$context['OnProperty'].' complaint with ontology'),
					'onClass'=>$context['OnClass'],
					'inverse'=>$context['inverse'],
					'inverseof'=>$context['inverseof'],
				);
			
	    }
	);
	
	$cardinalityallowsmore = $rb->create(
	    $rb->logicalOr(
			 $rb->logicalAnd(
	        $rb['matchedNumRels']->lessThan($rb['maxNumRel']),
	        $rb['maxNumRel']->greaterThan(0)
			),$rb['maxNumRel']->EqualTo(NULL)		
		),
		function() use ($context,&$restrictionsOnproperties) {
			$restrictionsOnproperties[$context['OnProperty']]['allowsmore']=TRUE;
      
      //We should denote how many we can add to extend our functionallity.We will return max and current. We won't calculate the 
      //difference here because we could need at some point the whole picture, not only how many more we can add.
      $restrictionsOnproperties[$context['OnProperty']]['currentnum']=$context['matchedNumRels'];
      if (!isset($context['maxNumRel'])||($context['maxNumRel'] === NULL)) { //MEANS unlimited
        //We could use INF constant, but i think this does not make sense, better a human parsable string.
        $restrictionsOnproperties[$context['OnProperty']]['currentmax']="unlimited";
      }
      else {
       $restrictionsOnproperties[$context['OnProperty']]['currentmax']=$context['maxNumRel'];
      }
    }
	);
	$cardinalitydoesnotallowmore = $rb->create(
    $rb->logicalNot($cardinalityallowsmore),
      function() use ($context,&$restrictionsOnproperties) {
        $restrictionsOnproperties[$context['OnProperty']]['allowsmore']=FALSE;
         $restrictionsOnproperties[$context['OnProperty']]['currentnum']=$context['matchedNumRels'];
         $restrictionsOnproperties[$context['OnProperty']]['currentmax']=$context['maxNumRel'];
       }
  );
  
  
  
  	
	$cardinalitynotmet=$rb->create(
				$rb->logicalNot($cardinality),
					function() use ($context,&$restrictionsOnproperties) {
						$restrictionsOnproperties[$context['OnProperty']]=
							array(
								'msg'=> array('Cardinality on '.$context['OnProperty'].' not complaint with ontology'),
								'onClass'=>$context['OnClass'],
								'inverse'=>$context['inverse'],
								'inverseof'=>$context['inverseof'],
							);
						
					} 
				);
	$notsetCardinality = $rb->create(
	    $rb->logicalOr(
	        $rb['minNumRel']->EqualTo(NULL),
	        $rb['maxNumRel']->EqualTo(NULL)
	    ),
	    function() use ($context) {
	       // echo "No hay minimo y/o maximo\r\n";
	    }
	);
		
				
	
	$propertymetsClass = $rb->create($rb['actualOnClass']->setContains($rb['OnClass']),
	    function() use ($context) {
	       //echo 'Existe clase '.$context['OnClass'].' para '.$context['OnProperty']."\n\r";
			
	    }
	);
	
	//We must deal with the fact that every object can have multiple contentmodels. If so in the case of allvaluesfrom we will have always problems
	//
	$otherrelationssameontology= $rb->create(
				$rb['actualOnClass']->Complement(
				$rb['OnClass'])->NotEqualTo(NULL),
				function() use ($context)
					{
					}
			);

	
	$propertyonlyClass = $rb->create(
	$rb->logicalAnd($propertymetsClass,$rb->logicalNot($otherrelationssameontology)),
	function() use ($context)
		{
			//echo "allvaluesfrom para:\r\n".$context['OnProperty']." ".$context['OnClass'];
		}
	);
	
	
	$allvaluesFrom=$rb->create(
		$rb->logicalOr(
			$rb['actualNumRel']->EqualTo(0),//Proposition 1, there are no relations
			$propertyonlyClass
			),
			function() use ($context)
				{
					//echo "allvaluesfrom para:\r\n".$context['OnProperty']." ".$context['OnClass'];
				}
		);
		
	$somevaluesFrom=$rb->create(
			 $rb['matchedNumRels']->greaterThanOrEqualTo(1),
			 function() use ($context)
				{
				//echo "somevaluesfrom para:\r\n".$context['OnProperty']." ".$context['OnClass'];
				}											 	 
		 );
	
	
	//construction blocks end
	
	$localcontext=array();//max and min local accumulator.
	
  
  
  
	foreach($graph->allOfType('owl:Class') as $class)
	{
		if ($class->uri == $subject)
		{	
			foreach($class->all('rdfs:subClassOf') as $thing)
			{
				if ($thing->isType('owl:Restriction'))
				{
				$context['OnClass']=NULL;//In case we don´t have any	
				$context['inverse']=NULL;
				$context['inverseof']=NULL;
        //reset this limits so we don't get stuck on them on every foreach

					//It's a local restriction 
					//First check if there is an owl:onProperty and the property is defined in this document
					if (($thing->has("owl:onProperty")) && ($thing->get("owl:onProperty")->has('rdf:type')))
					{
						if (!isset($onProperty))//If we have this variable set get only restrictions that match
						{
							
							$context['OnProperty'] = $thing->get("owl:onProperty")->uri;
						}
						else
						{
							$context['OnProperty'] = $onProperty == $thing->get("owl:onProperty")->uri ? $thing->get("owl:onProperty")->uri : NULL; //Only null if the property does not exist.
						}
          	//April 2015
            //There is something odd on how protege writes restrictions. max and min are split inside separate subclassof ->Restriction for the same onProperty. Assuming Protege will do the same for other types of
            //Combined restrictions. That means that i can't assume that i will get all, maximums and minimums by evaluating only one restricted property N. We will deal with this by creating
            //a context based on a function. Love rulebuilder. 
            if (!isset($localcontext[$context['OnProperty']])){$localcontext[$context['OnProperty']]=array();}
            
						
						//Also get inverse and inverseof
						$property=$thing->get("owl:onProperty");
						
						
							$inverse=$property->get('-owl:inverseOf')->isNull() ? '' : $property->get('-owl:inverseOf')->uri;
							$inverseof=$property->get('owl:inverseOf')->isNull() ? '' : $property->get('owl:inverseOf')->uri;
							//we must deal with inverse and inverseof differently. If this property has an inverseof then, when the validation is made, we must invert subject/object
							//But when we have just an inverse, it means we can give our object part from the tripple this propery without question. assuming the ontology is well (logical)formed. 
							
							if (!empty($inverse))
								{
								$context['inverse']=$inverse;
								}
							if (!empty($inverseof))
									{
									$context['inverseof']=$inverseof;
									}	
									//Now we check if there is a onClass definition and the class is also defined in this document		
									//Now Look if there are cardinality and existance restricctions and infere some simple rules
									//In OWL Lite Cardinality can only be 0 or 1, owl 2 allows more int. for owl:qualifiedCardinality, added for compatibility if ontology was generated using i.e Protege
						if ( ($thing->has("owl:cardinality")) || ($thing->has("owl:qualifiedCardinality")) )
						{
							if ($thing->has("owl:cardinality"))
							{
							
							if ((int)$thing->getLiteral("owl:cardinality")==0)
							{
								$localcontext[$context['OnProperty']]['minNumRel']=0;
                $localcontext[$context['OnProperty']]['maxNumRel']=$localcontext[$context['OnProperty']]['minNumRel'];
								$ruleset_currentrelations_compliant->addRule($cardinality);
								$ruleset_currentrelations_compliant->addRule($cardinalitynotmet);
							
							}
							elseif ((int)$thing->getLiteral("owl:cardinality")==1)
							{
								$localcontext[$context['OnProperty']]['minNumRel']=1;
								$localcontext[$context['OnProperty']]['maxNumRel']=$localcontext[$context['OnProperty']]['minNumRel'];
								$ruleset_currentrelations_compliant->addRule($cardinality);
								$ruleset_currentrelations_compliant->addRule($cardinalitynotmet);
							
							}
							else
							{
								//"OWL LITE only allows 0 or 1 as values for owl:maxCardinality";
								$ruleset_currentrelations_compliant->addRule($cardinalitynotset);
								continue;	
							}
							}
							else
							{
								if ((int)$thing->getLiteral("owl:qualifiedCardinality")==0)
								{
  								$localcontext[$context['OnProperty']]['minNumRel']=0;
                  $localcontext[$context['OnProperty']]['maxNumRel']=$localcontext[$context['OnProperty']]['minNumRel'];
									$ruleset_currentrelations_compliant->addRule($cardinality);
									$ruleset_currentrelations_compliant->addRule($cardinalitynotmet);
							
								}
								elseif (ctype_digit((string) $thing->getLiteral("owl:qualifiedCardinality"))) //Now OWL 2 Compliant
								{
  								$localcontext[$context['OnProperty']]['minNumRel']=$thing->getLiteral("owl:qualifiedCardinality");
                  $localcontext[$context['OnProperty']]['maxNumRel']=$localcontext[$context['OnProperty']]['minNumRel'];
									$ruleset_currentrelations_compliant->addRule($cardinality);
									$ruleset_currentrelations_compliant->addRule($cardinalitynotmet);
							
								}
								else
								{
									//"OWL LITE only allows 0 or 1 as values for owl:maxCardinality, owl2 has more options.;
									$ruleset_currentrelations_compliant->addRule($cardinalitynotset);
									continue;	
								}
							}
						}
						else
						{
						if (($thing->has("owl:minCardinality")) || ($thing->has("owl:minQualifiedCardinality"))) {
							//Only if there is no cardinality we fetch max first and the min					
							if (($thing->has("owl:minCardinality")) && ((int)$thing->getLiteral("owl:minCardinality")<=1))
							{					
								
								$localcontext[$context['OnProperty']]['minNumRel']=(int)$thing->getLiteral("owl:minCardinality");
							
							}
							elseif (($thing->has("owl:minQualifiedCardinality")) && (ctype_digit((string) $thing->getLiteral("owl:minQualifiedCardinality")))) //Now OWL 2 compliant, can be any 0 or any positive integer
							{					
								
								$localcontext[$context['OnProperty']]['minNumRel']=(int)$thing->getLiteral("owl:minQualifiedCardinality");
							
							}
							
						}
						if (($thing->has("owl:maxCardinality")) || ($thing->has("owl:maxQualifiedCardinality"))) {
							if (($thing->has("owl:maxCardinality")) && ((int)$thing->getLiteral("owl:maxCardinality")<=1))
							{
								$localcontext[$context['OnProperty']]['minNumRel']=isset($localcontext[$context['OnProperty']]['minNumRel'])?$localcontext[$context['OnProperty']]['minNumRel']:0;//If min not set assume it´s 0
								$localcontext[$context['OnProperty']]['maxNumRel']=(int)$thing->getLiteral("owl:maxCardinality");
								
							}
							elseif (($thing->has("owl:maxQualifiedCardinality")) && (ctype_digit((string) $thing->getLiteral("owl:maxQualifiedCardinality"))))//Now OWL 2 compliant, can be any 0 or any positive integer
								{
									$localcontext[$context['OnProperty']]['minNumRel']=isset($localcontext[$context['OnProperty']]['minNumRel'])?$localcontext[$context['OnProperty']]['minNumRel']:0;//If min not set assume it´s 0
									$localcontext[$context['OnProperty']]['maxNumRel']=(int)$thing->getLiteral("owl:maxQualifiedCardinality");
								
								}
							}
							$ruleset_currentrelations_compliant->addRule($cardinality);
							$ruleset_currentrelations_compliant->addRule($cardinalitynotmet);
						
						}
						
						
						
						if (($thing->has("owl:onClass")) && ($thing->get("owl:onClass")->has('rdf:type')))	
						{
							if (!isset($onClass))//If we have this variable set get only restrictions that match
							{	
								$context['OnClass']=$thing->get("owl:onClass")->uri;
							}
							else
							{
								$context['OnClass']=$onClass==$thing->get("owl:onClass")->uri ? $thing->get("owl:onClass")->uri  : NULL;
							}	
							$ruleset_currentrelations_compliant->addRule($propertymetsClass);
							
							
						
						
						}
						else //Check from allvaluesfrom or somevaluesfrom
						{
							//owl:onClass is "disjoint" with owl:allValuesFrom and owl:someValuesFrom
							if (($thing->has("owl:allValuesFrom")) && ($thing->get("owl:allValuesFrom")->has('rdf:type')))		
							{
								if (!isset($onClass))//If we have this variable set get only restrictions that match
								{	
									$context['OnClass']=$thing->get("owl:allValuesFrom")->uri;
								}
								else
								{
									$context['OnClass']=$onClass==$thing->get("owl:allValuesFrom")->uri ? $thing->get("owl:allValuesFrom")->uri : NULL;
								}
								$ruleset_currentrelations_compliant->addRule($allvaluesFrom);
								
								 
							}
							elseif (($thing->has("owl:someValuesFrom")) && ($thing->get("owl:someValuesFrom")->has('rdf:type')))		
							{
								if (!isset($onClass))//If we have this variable set get only restrictions that match
								{	
									$context['OnClass']=$thing->get("owl:someValuesFrom")->uri;
								}
								else
								{
									$context['OnClass']=$onClass==$thing->get("owl:someValuesFrom")->uri ? $thing->get("owl:someValuesFrom")->uri : NULL;
								}
								$ruleset_currentrelations_compliant->addRule($somevaluesFrom);
				
							}
							else
							{
								//Nothing to restrain, jump to the next
								//echo "no onclass restriction, just treat as cardinality without onClass definition restriction";
								//continue; 
				
					
							}	
						}
						
						//Rule Context and Propositions definition
						//We must check two different propositions here:
							//First if we have space for adding a new allowed relation to this object. This works on this loop. We have a onPropery, onClass pair.
							//Check for incorrect relations already in place: we must filter out relations that are not defined in this ontology.This is a must because we are working on a local scope here.
							//This check should be done a Class basis, not in this loop?
							//Are Both propositions mutually exclusive? a) We can allow the adding of new relations(allowed ones) even if there are wrong ones in place or b) we can consider 
							//waiting for them to be fixed before we continue adding new ones.
							//I like b), no new ones until everything is fixed. So we mantain our Repository consistent with the given ontology
						
							//Context Variable, holds and array with indexes to matching OnProperty, OnClass existing relations
						
						
							$context['matchedRels']=function() use ($rels,$context)							
							{
								$match=array();
								foreach($rels as $relation_key=>$rel)
								{	
									if (($rel['predicate']['namespace'].$rel['predicate']['value'] == $context['OnProperty']) && (in_array($context['OnClass'],$rel['object']['cmodels'])))
									{	
										$match[]=$relation_key;//Hash of matched rels	
									}
								}
								
								return $match;
							};	
							//Context Variable, (int) holds the number of existing matched relations
							$context['matchedNumRels']=	sizeof($context['matchedRels']);
							
							
						
						
						
							$context['actualNumRel'] = function() use ($rels, $context)
							{
								$num=sizeof($rels);																	
								return $num;
							};
								
											
							$context['actualOnProperty'] = function() use ($rels, $context)
							{
								$props=array();
								foreach($rels as $relation=>$prop)
								{
									$props[]=$prop['predicate']['namespace'].$prop['predicate']['value'];
								}
								//var_dump($props,TRUE);
								return $props;
							};	
							$context['actualOnClass'] = function() use ($rels,$context)
							{
								$classes=array();
								foreach($rels as $relation_key=>$class)
								{	
									if ($class['predicate']['namespace'].$class['predicate']['value'] == $context['OnProperty'])
									{	
										//We can´t add the whole cmodels array here. Every object of a exisiting relation can have multiple content models.
										//we need to discriminate which relation is true or false. If we just pass the whole cmodels array we will only know on a global basis if the group of rels for this option validates.
										foreach($class['object']['cmodels'] as $cmodel)
											{
											$classes[]=$cmodel;		
											}
									}
								}
								
								//var_dump($props,TRUE);
								return array_unique($classes);
							};		
							//End of Context definition															
							//$otherrelationssameontology->execute($context);						
							//$cardinalitynotmet->execute($context);
							//$cardinality->execute($context);
							//$notsetCardinality->execute($context);					
							//$propertymetsClass->execute($context);
							//$allvaluesFrom->execute($context);
							//$somevaluesFrom->execute($context);	
              $context['minNumRel']=function() use ($context,$localcontext){return isset($localcontext[$context['OnProperty']]['minNumRel']) ? $localcontext[$context['OnProperty']]['minNumRel'] : NULL ; };
              $context['maxNumRel']=function() use ($context,$localcontext){return isset($localcontext[$context['OnProperty']]['maxNumRel']) ? $localcontext[$context['OnProperty']]['maxNumRel'] : NULL ; };  
 
              
							$ruleset_currentrelations_compliant->addRule($cardinalityallowsmore);//check if with the given relations we can add something more...
						  $ruleset_currentrelations_compliant->addRule($cardinalitydoesnotallowmore);
              $ruleset_currentrelations_compliant->executeRules($context);
							
					
					
					
						}		
						else
						{ 
							//Nothing to restrain, no onProperty jump to the next
							continue;	
						}			
            
					}
					else
					{
						//Nothing to restrain, no Restriction jump to the next
						
						continue;	
						
					}						
				
					
				}
			}

		
		}
	foreach ($restrictionsOnproperties as $prop => $info)//Add namespace and local at last
		{
			
			//can't be both...
			if (isset($info['inverse'])||isset($info['inverseof']))
			{
			if (sizeof($info['inverse'])>0)
				{
				$uri_parts_inverse = ARC2::splitURI($info['inverse']);	
				$restrictionsOnproperties[$prop]['inverse']=array('ns'=>$uri_parts_inverse[0],'localname'=>$uri_parts_inverse[1]);
				}
			elseif (sizeof($info['inverseof'])>0)
				{
					
					$uri_parts_inverse = ARC2::splitURI($info['inverseof']);	
					$restrictionsOnproperties[$prop]['inverseof']=array('ns'=>$uri_parts_inverse[0],'localname'=>$uri_parts_inverse[1]);
						
				}
			}		
			$uri_parts = ARC2::splitURI($prop);
			
			$restrictionsOnproperties[$prop]['ns']=$uri_parts[0];
			$restrictionsOnproperties[$prop]['localname']=$uri_parts[1];
		}	
	return $restrictionsOnproperties;		
	}
/**
* This functions returns every existing restricted relation for a CMODEL subject, from all their associated ontologies.	
*
*/
		
function islandora_ontologies_get_all_for_cmodel($contentmodelpid)
	{
		$currentrels=array();//we don't wan't the current relations here. Just assume a brand new object.
		$associated_ontologies=array(); 
		$return_rels=array();	
		try {
			$contentmodel_object =islandora_object_load($contentmodelpid);
			}
			catch (Exception $e) 
			{
			return;	
			}	
		if ($contentmodel_object)	
			{
			$associated_ontologies=islandora_ontologies_get_associated_ontologies_for_cmodel($contentmodel_object);//returns $associated_ontology[$ontology_object->id]=$ontology_object->label;
			if (sizeof($associated_ontologies)>0)
				{
				foreach($associated_ontologies as $pid =>$label)
				{
					$return_rels[$pid]=islandora_ontologies_get_restricted_properties($pid,$contentmodelpid,$currentrels,NULL,NULL);
				
				}
			
			}
		}
	return 	$return_rels;	
	}
/**
* Let´s traverse this graph and find where how long the distance is!	
* This function traverses the OWL graph in search for paths between objects belonging to classes and builds and adjacency list. 
* Fedoras implementation/Mulgara does not allow arbritrary paths to be traversed(al leats not optimal), so we must know how to get from point 1(object) to any other object->of class in an ontology to ask for a 
* local graph. Even more, be can cache the returning structure and only update the cache in the rare case the attached ontologies are updated.		
* Update : we also store now the "direction" of the properties. We will set 1 for normal properties, 0 for inverse ones. This allows us further process everything by checking how many times we
*  go from one to many and one to one, to filter by level of strong connection of our graph/diverging interpretations on traversal...    
* @param $subject any contentmodel pid
* @return adjacency list of neightborns and their relations plus the direction/nature of the property, if it's a versed one or an inverse.
* @todo make this a class				
*/	
function islandora_ontologies_get_graph_path($subject)
	{
		
		//This is going to be cool
		//We are going to build an adjacency list! It`s quicker than traversing the whole stuff everytime. 
        /* This is the idea
<?php
$graph = array(
  'A' => array('B' => 3, 'D' => 3, 'F' => 6),
  'B' => array('A' => 3, 'D' => 1, 'E' => 3),
  'C' => array('E' => 2, 'F' => 3),
  'D' => array('A' => 3, 'B' => 1, 'E' => 1, 'F' => 2),
  'E' => array('B' => 3, 'C' => 2, 'D' => 1, 'F' => 5),
  'F' => array('A' => 6, 'C' => 3, 'D' => 2, 'E' => 5),
);*/
		$adjacency = array();
		$adjacencyparsed = array();
		$adjacencyfull = array();
    $verse_prop = 1; //If the property is a normal one 
		$inverse_prop = 0; //If it's the inverse of a normal one.
		try {
			$contentmodel_object =islandora_object_load($subject);
			}
			catch (Exception $e) 
			{
			return;	
			}	
		if ($contentmodel_object)	
			{
			$tmgraph=islandora_ontologies_get_all_for_cmodel($subject);
			if(count($tmgraph)>0)
				{
				foreach ($tmgraph as $ontology => $relations) {
					$adjacency[$ontology]=islandora_ontologies_get_all_restricted_properties($ontology);
					//Now we have an array with every $ontology as key and classes and their relations
					//Lets clean this up.
				
					
					$adjacencyparsed=array();
					foreach ($adjacency[$ontology] as $class=>$props) {
						$adjacencyparsed[$class]=array();	
            if (count($props[0]>0)) {
              foreach($props[0] as $propname=>$propinfo) {
               
                if ((array_key_exists('onClass',$propinfo)) && (!empty($propinfo['onClass']))) {
                   //Start by assuming that any property is a versed one in case i don't get an inverse or inverseof key for this.
                  $current_propdirection=$verse_prop;
                  if ((array_key_exists('inverse',$propinfo)) && (!empty($propinfo['inverse']))) {
                  //if this property has an "inverseo", then it's an verse  property  
                  $current_propdirection=$verse_prop;
                    //Lets try to assing the verse one to the parent class
                    if (array_key_exists($propinfo['onClass'],$adjacencyparsed)) {
                      $adjacencyparsed[$propinfo['onClass']][$class][]=array($propinfo['inverse'],$inverse_prop);
                      $adjacencyparsed[$propinfo['onClass']][$class] = array_map("unserialize", array_unique(array_map("serialize", $adjacencyparsed[$propinfo['onClass']][$class])));
                      
                    }
                    else {
                      $adjacencyparsed[$propinfo['onClass']] = array($class=>array($propinfo['inverse'],$inverse_prop));	
                      }
                  }
                  if ((array_key_exists('inverseof',$propinfo)) && (!empty($propinfo['inverseof']))) {
                    
                    $current_propdirection=$inverse_prop;
                    if (array_key_exists($propinfo['onClass'],$adjacencyparsed)) {
                      $adjacencyparsed[$propinfo['onClass']][$class][]=array($propinfo['inverseof'],$verse_prop);
                      $adjacencyparsed[$propinfo['onClass']][$class] = array_map("unserialize", array_unique(array_map("serialize", $adjacencyparsed[$propinfo['onClass']][$class])));
                     
                    }
                    else {
                      $adjacencyparsed[$propinfo['onClass']] = array($class=>array($propinfo['inverseof'],$verse_prop));	
                    }
                  }
                  //Now, knowing what type of property this is, verse or inverse, i finally add this to the current class
                  $adjacencyparsed[$class][$propinfo['onClass']][]=array($propname,$current_propdirection);
                }
             
                //Avoid duplicates
                $adjacencyparsed[$class][$propinfo['onClass']] = array_map("unserialize", array_unique(array_map("serialize", $adjacencyparsed[$class][$propinfo['onClass']])));
                
								  
              }//end foreach
            }
          }//end foreach
				 $adjacencyfull[$ontology]=$adjacencyparsed;
				}
      }
		}
		
		return($adjacencyfull);
}
/** Workhorse Function to islandora_ontologies_get_graph_path
Simililar to islandora_ontologies_get_restricted_properties, but it parses all clases and returns less data for every one. It´s computational intensive but more efficient than calling islandora_ontologies_get_restricted_properties for every class
*/
/**
* This function gets restrictions for all classes in an ontology
* @param $ontology_objectpid. The pid of the object that contains the Ontology 
* @return $graph
*/
function islandora_ontologies_get_all_restricted_properties($ontology_objectpid)
{
	$module_path = drupal_get_path('module', 'islandora_ontologies');
	include_once($module_path.'/arc2/ARC2.php');
	include_once($module_path.'/Graphite/Graphite.php');
	$classloader=function($c) use ($module_path)
		{
			@include $module_path."/".preg_replace('#\\\|_(?!.+\\\)#','/',$c).'.php';
		};
	spl_autoload_register($classloader);
	
	$rb = new RuleBuilder;
	$ruleset_currentrelations_compliant = new RuleSet;
	$context = new Context;
	$restrictionsOnproperties=array();
	$PID=$ontology_objectpid;	
	$fedora_url = variable_get('islandora_base_url', 'http://localhost:8080/fedora');
	$xmlfilelink = $fedora_url."/objects/".$PID."/datastreams/ONTOLOGY/content";
	$graph = new Graphite();
	$graph->load($xmlfilelink);
	$existing_classes=array();
	
	$cardinality = $rb->create(
	    $rb->logicalAnd(
	        $rb['minNumRel']->lessThanOrEqualTo($rb['matchedNumRels']),
	        $rb['maxNumRel']->greaterThanOrEqualTo($rb['matchedNumRels'])
	    ),
	    function() use ($context,&$restrictionsOnproperties) {
			$restrictionsOnproperties[$context['OnProperty']]=array(
					'onClass'=>$context['OnClass'],
					'inverse'=>$context['inverse'],
					'inverseof'=>$context['inverseof'],
				);
			
	    }
	);	
	$cardinalityallowsmore = $rb->create(
	    $rb->logicalOr(
			 $rb->logicalAnd(
	        $rb['matchedNumRels']->lessThan($rb['maxNumRel']),
	        $rb['maxNumRel']->greaterThan(0)
			),$rb['maxNumRel']->EqualTo(NULL)		
		),
		function() use ($context,&$restrictionsOnproperties) {
		$restrictionsOnproperties[$context['OnProperty']]['allowsmore']=TRUE;
    $restrictionsOnproperties[$context['OnProperty']]['currentnum']=$context['matchedNumRels'];
    
    if (!isset($context['maxNumRel'])||($context['maxNumRel'] === NULL)) { //MEANS unlimited
      //We could use INF constant, but i think this does not make sense, better a human parsable string.
      $restrictionsOnproperties[$context['OnProperty']]['currentmax']="unlimited";
      
    }
    else
    {
     $restrictionsOnproperties[$context['OnProperty']]['currentmax']=$context['maxNumRel'];
    }
	    }
	);
		
	$cardinalitynotmet=$rb->create(
				$rb->logicalNot($cardinality),
					function() use ($context,&$restrictionsOnproperties) {
						$restrictionsOnproperties[$context['OnProperty']]=
							array(
								'onClass'=>$context['OnClass'],
								'inverse'=>$context['inverse'],
								'inverseof'=>$context['inverseof'],
							);
						
					} 
				);
	$notsetCardinality = $rb->create(
	    $rb->logicalOr(
	        $rb['minNumRel']->EqualTo(NULL),
	        $rb['maxNumRel']->EqualTo(NULL)
			)
	);
						
	
	$propertymetsClass = $rb->create($rb['actualOnClass']->setContains($rb['OnClass'])
	);	
	//We must deal with the fact that every object can have multiple contentmodels. If so in the case of allvaluesfrom we will have always problems
	$otherrelationssameontology= $rb->create(
				$rb['actualOnClass']->Complement(
				$rb['OnClass'])->NotEqualTo(NULL)	
			);
	
	$propertyonlyClass = $rb->create(
	$rb->logicalAnd($propertymetsClass,$rb->logicalNot($otherrelationssameontology))	
	);
	
	
	$allvaluesFrom=$rb->create(
		$rb->logicalOr(
			$rb['actualNumRel']->EqualTo(0),//Proposition 1, there are no relations
			$propertyonlyClass
			)
		);
		
	$somevaluesFrom=$rb->create(
			 $rb['matchedNumRels']->greaterThanOrEqualTo(1)										 	 
		 );	
	
	//construction blocks end
	
	
	
	foreach($graph->allOfType('owl:Class') as $class)
	{		
		$classname=$class->uri;
		//$context = new Context;
		$graph_path[$classname]=array();
		
		$restrictionsOnproperties=array();
			foreach($class->all('rdfs:subClassOf') as $thing)
			{
				if ($thing->isType('owl:Restriction'))
				{
					$context['OnProperty']="";
					$context['inverse']="";
					$context['inverseof']="";
					$context['OnClass']="";//In case we don´t have any	
          //reset cardinality restrictions
          unset($context['minNumRel']);
          unset($context['maxNumRel']);  
					//It's a local restriction 
					//First check if there is an owl:onProperty and the property is defined in this document
					if (($thing->has("owl:onProperty")) && ($thing->get("owl:onProperty")->has('rdf:type')))
					{
							
						$context['OnProperty']=$thing->get("owl:onProperty")->uri;
						
						//Also get inverse and inverseof
						$property=$thing->get("owl:onProperty");												
						$inverse=$property->get('-owl:inverseOf')->isNull() ? '' : $property->get('-owl:inverseOf')->uri;
						$inverseof=$property->get('owl:inverseOf')->isNull() ? '' : $property->get('owl:inverseOf')->uri;
						//we must deal with inverse and inverseof differently. If this property has an inverseof then, when the validation is made, we must invert subject/object
						//But when we have just an inverse, it means we can give our object part from the tripple this propery without question. assuming the ontology is well (logical)formed. 
						if (!empty($inverse))
								{
								$context['inverse']=$inverse;
								}
							if (!empty($inverseof))
								{
								$context['inverseof']=$inverseof;
								}	
						if (($thing->has("owl:cardinality")) || ($thing->has("owl:qualifiedCardinality")))
							{							
							if ($thing->has("owl:cardinality"))
							{
							
							if ((int)$thing->getLiteral("owl:cardinality")==0)
							{
								$context['minNumRel']=0;
								$context['maxNumRel']=$context['minNumRel'];
								$ruleset_currentrelations_compliant->addRule($cardinality);
								$ruleset_currentrelations_compliant->addRule($cardinalitynotmet);
							
							}
							elseif ((int)$thing->getLiteral("owl:cardinality")==1)
							{
								$context['minNumRel']=1;
								$context['maxNumRel']=1;
								$ruleset_currentrelations_compliant->addRule($cardinality);
								$ruleset_currentrelations_compliant->addRule($cardinalitynotmet);
							
							}
							else
							{
								//"OWL LITE only allows 0 or 1 as values for owl:maxCardinality";
								$ruleset_currentrelations_compliant->addRule($cardinalitynotset);
								continue;	
							}
							}
							else
							{
								if ((int)$thing->getLiteral("owl:qualifiedCardinality")==0)
								{
									$context['minNumRel']=0;
									$context['maxNumRel']=$context['minNumRel'];
									$ruleset_currentrelations_compliant->addRule($cardinality);
									$ruleset_currentrelations_compliant->addRule($cardinalitynotmet);
							
								}
								elseif ((int)$thing->getLiteral("owl:qualifiedCardinality")==1)
								{
									$context['minNumRel']=1;
									$context['maxNumRel']=1;
									$ruleset_currentrelations_compliant->addRule($cardinality);
									$ruleset_currentrelations_compliant->addRule($cardinalitynotmet);
							
								}
								else
								{
									//"OWL LITE only allows 0 or 1 as values for owl:maxCardinality";
									$ruleset_currentrelations_compliant->addRule($cardinalitynotset);
									continue;	
								}
							}
						}
						else
						{
							
							if (($thing->has("owl:minCardinality")) || ($thing->has("owl:minQualifiedCardinality")))
								{
							//Only if there is no cardinality we fetch max first and the min					
							if (($thing->has("owl:minCardinality")) && ((int)$thing->getLiteral("owl:minCardinality")<=1))
							{					
								
								$context['minNumRel']=(int)$thing->getLiteral("owl:minCardinality");
							
							}
							elseif (($thing->has("owl:minQualifiedCardinality")) && ((int)$thing->getLiteral("owl:minQualifiedCardinality")<=1))
							{					
								
								$context['minNumRel']=(int)$thing->getLiteral("owl:minQualifiedCardinality");
							
							}
							
							}
							if (($thing->has("owl:maxCardinality")) || ($thing->has("owl:maxQualifiedCardinality")))
								{
							if (($thing->has("owl:maxCardinality")) && ((int)$thing->getLiteral("owl:maxCardinality")<=1))
							{
								$context['minNumRel']=isset($context['minNumRel'])?$context['minNumRel']:0;//If min not set assume it´s 0
								$context['maxNumRel']=(int)$thing->getLiteral("owl:maxCardinality");
								
							}
							elseif (($thing->has("owl:maxQualifiedCardinality")) && ((int)$thing->getLiteral("owl:maxQualifiedCardinality")<=1))
								{
									$context['minNumRel']=isset($context['minNumRel'])?$context['minNumRel']:0;//If min not set assume it´s 0
									$context['maxNumRel']=(int)$thing->getLiteral("owl:maxQualifiedCardinality");
								}
							}
							$ruleset_currentrelations_compliant->addRule($cardinality);
							$ruleset_currentrelations_compliant->addRule($cardinalitynotmet);
						
						}
					if (($thing->has("owl:onClass")) && ($thing->get("owl:onClass")->has('rdf:type')))	
						{
								
							$context['OnClass']=$thing->get("owl:onClass")->uri;
							$ruleset_currentrelations_compliant->addRule($propertymetsClass);
							
							
						
						
						}
						else //Check from allvaluesfrom or somevaluesfrom
						{
							//owl:onClass is "disjoint" with owl:allValuesFrom and owl:someValuesFrom
							if (($thing->has("owl:allValuesFrom")) && ($thing->get("owl:allValuesFrom")->has('rdf:type')))		
							{
									
								$context['OnClass']=$thing->get("owl:allValuesFrom")->uri;
								$ruleset_currentrelations_compliant->addRule($allvaluesFrom);
								
								 
							}
							elseif (($thing->has("owl:someValuesFrom")) && ($thing->get("owl:someValuesFrom")->has('rdf:type')))		
							{
									
								$context['OnClass']=$thing->get("owl:someValuesFrom")->uri;
							   $ruleset_currentrelations_compliant->addRule($somevaluesFrom);
				
							}
								
						}
						//Rule Context and Propositions definition
						//Same as other function, but empty. We don´t assume existing relations;
						$context['matchedRels']=array();							
						$context['matchedNumRels']=	0;
						$context['actualNumRel'] = 0;
						$context['actualOnProperty'] = array();								
						$context['actualOnClass'] = array();
						$ruleset_currentrelations_compliant->addRule($cardinalityallowsmore);//check if with the given relations we can add something more...
						$ruleset_currentrelations_compliant->executeRules($context);
						
						
						
						
						}		
						else
						{ 
							//Nothing to restrain, no onProperty jump to the next
							
							continue;	
						}			
			
					}
					else
					{
						//Nothing to restrain, no Restriction jump to the next
						continue;	
						
					}						
			}//End if has restriction
      
			foreach ($restrictionsOnproperties as $prop => $info)//Add namespace and local at last
				{
		      
					$uri_parts = ARC2::splitURI($prop);

					$restrictionsOnproperties[$prop]['ns']=$uri_parts[0];
					$restrictionsOnproperties[$prop]['localname']=$uri_parts[1];
				}
					
			array_push($graph_path[$classname],$restrictionsOnproperties);		
			
			
			}

		
		
			return ($graph_path);
		
	}
// Traverse Graph implemention
// breadth first Based on http://www.sitepoint.com/data-structures-4/ by Ignatius Teo
// depthFirstSearch my own. 
class islandora_ontologies_adjacency_graph {
	  protected $graph;
	  protected $visited = array();
	  protected $pathdepth;
	  
	 
	  public function __construct($graph) {
	    $this->graph = $graph;
		
	    foreach ($this->graph as $vertex => $adj) {
		  $this->visited[$vertex] = false;
		  
	    }
	    
	   
		$this->pathdepth=array();
	   
		
	  }
 
	
	  public function breadthFirstSearch($origin, $destination) {
	    // mark all nodes as unvisited
	    foreach ($this->graph as $vertex => $adj) {
			
		  $this->visited[$vertex] = false;
		  
	    }
 
	    // create an empty queue
	    $q = new SplQueue();
 
	    // enqueue the origin vertex and mark as visited
	    $q->enqueue($origin);
	    $this->visited[$origin] = true;
 
	    // this is used to track the path back from each node
	    $path = array();
	    $path[$origin] = new SplDoublyLinkedList();
	    $path[$origin]->setIteratorMode(
	      SplDoublyLinkedList::IT_MODE_FIFO|SplDoublyLinkedList::IT_MODE_KEEP
	    );
 
	    $path[$origin]->push($origin);
         
	    $found = false;
		//dpm($this->visited);
		//dpm($path);
	    // while queue is not empty and destination not found
	   while (!$q->isEmpty() && $q->bottom() != $destination) {
	      $t = $q->dequeue();
         
	      if (!empty($this->graph[$t])) {
	        // for each adjacent neighbor
	        foreach ($this->graph[$t] as $vertex=>$props) {
				dpm($vertex);
	          if (!$this->visited[$vertex]) {
	            // if not yet visited, enqueue vertex and mark
	            // as visited
				//dpm($t);
	             $this->visited[$vertex] = true;
	            // add vertex to current path
	            $path[$vertex] = clone $path[$t];
				//dpm($path[$vertex]);
	            $path[$vertex]->push($vertex);
				
	          }
	        }
	      }
	    }
        
		if (isset($path[$destination])) {
	      echo "$origin to $destination in ", 
	        count($path[$destination]) - 1,
	        " hopsn";
	      $sep = '';
	      foreach ($path[$destination] as $vertex) {
	        echo $sep, $vertex;
	        $sep = '->';
	      }
	      echo "n";
		 return ($path[$destination]); 
		}
	    else {
	      echo "No route from $origin to $destination";
		  return;
	    }
	  }
	  public function getdepthFirst($vertex)
	  	{
	  	$this->depthFirstSearch($vertex);
		  $pathdepthtmp=$this->pathdepth;
		  $this->pathdepth=array();
		  return $pathdepthtmp;
			  	
	  	}
	 
	 
	  protected function depthFirstSearch($vertex, $path=array(), $localvisited=array()) {
	     $localvisited[$vertex] = true;
	    
	      if (!empty($this->graph[$vertex])) {
	        // for each adjacent neighbor
			     $tovisit=false;
			foreach ($this->graph[$vertex] as $vertex2=>$props) {
				
			  if (!array_key_exists($vertex2,$localvisited)||(!$localvisited[$vertex2])) {
	           
				$tovisit=true;      
				
				$path[$vertex]=$props;  
	           	$this->depthFirstSearch($vertex2,$path,$localvisited);
							
	          }
			
			  				  
	        }
		  if (!$tovisit)
		  	{
				$path[$vertex]=array();
			  $this->pathdepth[]=$path;
			 
			   return ;
		  }
	      }
		  else
		  {
			$path[$vertex]=array();
		 $this->pathdepth[]=$path;
	 
	   return ;
		  }
		  	  
  }
	  	  	  
	}
	
/** LCS Class Implementation 
* @link http://en.wikipedia.org/wiki/Longest_common_subsequence_problem
* This class processes every calculated path for a given Ontology /graph) and searches for common partial subpaths
*  
*/
class islandora_ontologies_LCS {
  protected $matrix;
  private $paths_array;
  /**
  * @param $paths is an associative array containing a list of arrays with CMODELS and outgoing predicates sequences for a particular ontology: 
  * Each numeric key holds an array with a calculated path. Note that the last CMODEL, associative key, does not has an empty array. This is the last node 
  * of a path and has no outgoing relations. This structure is identical to the return value of class islandora_ontologies_adjacency_graph, method getdepthFirst($vertex).
  * Example
  * array( 
  *   0 => array (
  *    'info:fedora/biodiversity:LocationCModel' => array(
  *      'http://www.semanticweb.org/dpino/ontologies/2014/3/untitled-ontology-3#locatesEvent'
  *      ),
  *    'info:fedora/biodiversity:EventCModel' => array(
  *      'http://www.semanticweb.org/dpino/ontologies/2014/3/untitled-ontology-3#eventOf'
  *      ),
  *    'info:fedora/biodiversity:OccurrenceCModel' => array(
  *      'http://xmlns.com/foaf/0.1/depicts'
  *      ),
  *    'info:fedora/islandora:sp_basic_image' => array(
  *      'http://xmlns.com/foaf/0.1/depiction'
  *      ),
  *    'info:fedora/biodiversity:TaxonCModel' => array(
  *      'http://xmlns.com/foaf/0.1/depiction'
  *      ),
  *    'info:fedora/biodiversity:IdentificationCModel' => array(
  *     ),
  *  ),
  *  1 => array (
  *    'info:fedora/biodiversity:LocationCModel' => array(
  *      'http://www.semanticweb.org/dpino/ontologies/2014/3/untitled-ontology-3#locatesEvent'
  *      ),
  *    'info:fedora/biodiversity:EventCModel' => array(
  *      'http://www.semanticweb.org/dpino/ontologies/2014/3/untitled-ontology-3#eventOf'
  *      ),
  *    'info:fedora/biodiversity:OccurrenceCModel' => array(
  *      'http://www.semanticweb.org/dpino/ontologies/2014/3/untitled-ontology-3#hasIdentification'
  *      ),
  *    'info:fedora/biodiversity:IdentificationCModel' => array(
  *      'http://www.semanticweb.org/dpino/ontologies/2014/3/untitled-ontology-3#identifiedAs'
  *      ),
  *    'info:fedora/biodiversity:TaxonCModel' => array(
  *      'http://xmlns.com/foaf/0.1/depicts'
  *      ),
  *    'info:fedora/islandora:sp_basic_image' => array(
  *     ),
  *  ),
  *)  
  */
  public function __construct($paths) {
    $this->$paths = $paths;
  }
  /**
  * This method takes two paths and searches for a common subpath(subquence). It also stores where this common subpath starts in every path. 
  * This is not really used, but handy to have in future, because in our current implementation every path is calculed as a Directed Acyclic graph (DAG), so nodes(CMODELS) should never repeat.
  */
  public function computeLCS() {
    $m = count($pathA);
    $n = count($pathB);
    $matrix = array();
    //Fill the initial matrix
    for ($i = $m - 1; $i >= 0; $i--) {
                for ($j = $n - 1; $j >= 0; $j--) {
                    if ($pathA[$i] === $pathB[$j]) {
                        $matrix[$i][$j] =
                            (isset($matrix[$i + 1][$j + 1]) ? $matrix[$i + 1][$j + 1] : 0) +
                            1;
                    } else {
                        $matrix[$i][$j] = max(
                            (isset($matrix[$i + 1][$j]) ? $matrix[$i + 1][$j] : 0),
                            (isset($matrix[$i][$j + 1]) ? $matrix[$i][$j + 1] : 0)
                        );
                    }
                }
            }
    //Now use this matrix to compute the real longest common subsequence
    $i = 0;
    $j = 0;
    $lcspattern = array();
    while ($i < $m && $j < $n) {
        if ($pathA[$i] === $pathB[$j]) {
          if (sizeof($lcspattern) == 0) { //store the key/position where the subsequence starts in every Path
             $lcspattern_pos[]=$i;
          }
          $lcspatterns[] = $pathA[$i];
            
          $i++;
          $j++;
        } 
        elseif ((isset($matrix[$i + 1][$j]) ? $matrix[$i + 1][$j] : 0) >= (isset($matrix[$i][$j + 1]) ? $matrix[$i][$j + 1] : 0)) {
          $i++;
        } 
        else {
          $j++;
        }
    }
    return $lcspattern;
  
  }
}
    

