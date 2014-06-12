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
* This function will reads the content models of an Objects and searches if this contentmodels are associated to an Ontology Object
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
	
	//$contentmodelpid=$islandora_cmodel_object->id;
	
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
						exit;//or continue? we are getting only one for now.@todo add all of them, requires to modify also add_relation.inc and del_relation.inc	
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
	$not_system_rels= function($rel) 
	{
		//Filter out info:fedora/fedora-system:def/model
		if (strpos($rel['predicate']['namespace'], 'info:fedora/fedora-system:def/')===FALSE)
			{
			return TRUE;
			}	
			else
			{return FALSE;}	
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
		$graph = new Graphite();//@TODO Chek if it´s posible to combine all ontologies in one Graphite object and parse the whole thing 
		$graph->load($xmlfilelink);
		foreach($graph->allOfType('owl:ObjectProperty')->sort("rdfs:label") as $property)
		{
			
				
			if (!$property->get("-owl:onProperty")->get("-rdfs:subClassOf")->isNull())
			{
			if ($property->get("-owl:onProperty")->get("-rdfs:subClassOf")->uri == "info:fedora/".$subject)   //If this URI equals the subject we found an restricted $property for this class. If its a null node or inexistant it will give and GraphiteNull Object, no need to check for it.
			{
				//We won´t deal with the nature of the restriction here. We just wan´t a list of possible/defined/restricted properties for this particular class
				$inverse=$property->get('-owl:inverseOf')->isNull() ? '' : $property->get('-owl:inverseOf')->uri;
				$inverseof=$property->get('owl:inverseOf')->isNull() ? '' : $property->get('owl:inverseOf')->uri;
				//we must deal with inverse and inversof differently. If this property has an inverseof then, when the validation is made, we must invert subject/object
				//But when we have just an inverse, it means we can give our object part from the tripple this propery without question. assuming the ontology is well (logical)formed. 
				$uri_parts = ARC2::splitURI($property->uri);
				$posiblerels_complete[$PID][$property->uri]=array(
					'label'=> $property->label()->isNull() ? $uri_parts[1] : $property->label()->toString(),//If we don´t have a label display the short value of the property
					'inverse'=>$inverse,
					'inverseof'=>$inverseof,
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
				$rel_object=islandora_object_load($rel['object']['value']);		
				$rel['object']['cmodels']=array_map(function($cm){return "info:fedora/".$cm;},$rel_object->models);//We need the info:fedora/ part to compare againts de ontology definitions
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
* Optional: if onClass and/or onProperty is give it limits it return values to this matches
* @param $ontology_objectpid. The pid of the object that contains the Ontology 
* @param $subject. The Class that is being restricted (ContentmodelPID) without the info:fedora/ part
* @param $rels. Current Relations array for this object. We already discarded those properties who are not defined in this ontology.
* @param $onClass. only those restrictions that apply to a givenClass (otherContentmodelPID) without the info:fedora/ part.Default Null
* @param @onProperty. only those restrictions for a specific property. Full with ns.
*   The object to check for it´s content models and associated ontology objects.Default Null
* @return $restrictionsOnproperties with the properties,onClass and cardinality/existence restrictions and some human readable messages
*/
function islandora_ontologies_get_restricted_properties($ontology_objectpid,$subject,$rels,$onClass=NULL,$onProperty=NULL)
{
	$module_path = drupal_get_path('module', 'islandora_ontologies');
	define('CLASS_DIR', $module_path);
	include_once($module_path.'/arc2/ARC2.php');
	include_once($module_path.'/Graphite/Graphite.php');
    echo CLASS_DIR;
	$classloader=function($c) use ($module_path)
		{
			@include $module_path."/".preg_replace('#\\\|_(?!.+\\\)#','/',$c).'.php';
		};
	spl_autoload_register($classloader);
	
	$rb = new RuleBuilder;//We create our megarule first
	//Now we define our construction blocks
	$ruleset_currentrelations_compliant = new RuleSet;
	$context = new Context;
	
	$subject="info:fedora/".$subject;
	$onClass=isset($onClass)?"info:fedora/".$onClass: NULL;	
	
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
				);
			
	    }
	);
	//$cardinalityallowmore=$rb->create(
		
	$cardinalitynotmet=$rb->create(
				$rb->logicalNot($cardinality),
					function() use ($context,&$restrictionsOnproperties) {
						$restrictionsOnproperties[$context['OnProperty']]=
							array(
								'msg'=> array('Cardinality on '.$context['OnProperty'].' not complaint with ontology'),
								'onClass'=>$context['OnClass'],
							);
						
					} 
				);
	$notsetCardinality = $rb->create(
	    $rb->logicalOr(
	        $rb['minNumRel']->EqualTo(NULL),
	        $rb['maxNumRel']->EqualTo(NULL)
	    ),
	    function() use ($context) {
	        echo "No hay minimo y/o maximo\r\n";
	    }
	);
		
				
	
	$propertymetsClass = $rb->create($rb['actualOnClass']->setContains($rb['OnClass']),
	    function() use ($context) {
	        echo 'Existe clase '.$context['OnClass'].' para '.$context['OnProperty']."\n\r";
			
	    }
	);
	
	//We must deal with the fact that every object can have multiple contentmodels. If so in the case of allvaluesfrom we will have always problems
	//
	$otherrelationssameontology= $rb->create(
				$rb['actualOnClass']->Complement(
				$rb['OnClass'])->NotEqualTo(NULL),
				function() use ($context)
					{
						print_r($context['actualOnClass']);
						print_r($context['OnClass']);
						echo "hay otras\n\r";
					}
			
			);
				
			
					
	
	
	$propertyonlyClass = $rb->create(
	$rb->logicalAnd($propertymetsClass,$rb->logicalNot($otherrelationssameontology)),
	function() use ($context)
		{
			echo "allvaluesfrom para:\r\n".$context['OnProperty']." ".$context['OnClass'];
		}
	);
	
	
	$allvaluesFrom=$rb->create(
		$rb->logicalOr(
			$rb['actualNumRel']->EqualTo(0),//Proposition 1, there are no relations
			$propertyonlyClass
			),
			function() use ($context)
				{
					echo "allvaluesfrom para:\r\n".$context['OnProperty']." ".$context['OnClass'];
				}
		);
		
	$somevaluesFrom=$rb->create(
			 $rb['matchedNumRels']->greaterThanOrEqualTo(1),
			 function() use ($context)
				{
				echo "somevaluesfrom para:\r\n".$context['OnProperty']." ".$context['OnClass'];
				}											 	 
		 );
	
	
	//construction blocks end
	
	
	
	foreach($graph->allOfType('owl:Class') as $class)
	{
		if ($class->uri==$subject)
		{	
			
			foreach($class->all('rdfs:subClassOf') as $thing)
			{
				if ($thing->isType('owl:Restriction'))
				{
				$context['OnClass']=NULL;//In case we don´t have any	
					//It's a local restriction 
					//First check if there is an owl:onProperty and the property is defined in this document
					if (($thing->has("owl:onProperty")) && ($thing->get("owl:onProperty")->has('rdf:type')))
					{
						if (!isset($onProperty))//If we have this variable set get only restrictions that match
						{
							
							$context['OnProperty']=$thing->get("owl:onProperty")->uri;
						}
						else
						{
							$context['OnProperty']=$onProperty==$thing->get("owl:onProperty")->uri ? $thing->get("owl:onProperty")->uri : NULL;
						}
						//Now we check if there is a onClass definition and the class is also defined in this document		
						//Now Look if there are cardinality and existance restricctions and infere some simple rules
						//In OWL Lite Cardinality can only be 0 or 1, we could add also owl2 owl:qualifiedCardinality for compatibility if ontology was generated using i.e Protege
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
							//Only if there is no cardinality we fetch max first and the min					
							if (($thing->has("owl:minCardinality")) && ((int)$thing->getLiteral("owl:minCardinality")<=1))
							{					
								
								$context['minNumRel']=(int)$thing->getLiteral("owl:minCardinality");
							
							}
							if (($thing->has("owl:maxCardinality")) && ((int)$thing->getLiteral("owl:maxCardinality")<=1))
							{
								$context['minNumRel']=isset($context['minNumRel'])?$context['minNumRel']:0;//If min not set assume it´s 0
								$context['maxNumRel']=(int)$thing->getLiteral("owl:maxCardinality");
								
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
								echo "no onclass restriction, just treat as cardinality without onClass definition restriction";
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
	return $restrictionsOnproperties;		
	}
function islandora_ontologies_get_all_for_cmodel($cmodel)
	{
		 	
			$islandora_object_pid = $islandora_object->id;		
			if (!$islandora_object)
					{
						drupal_set_message(t("Islandora Object not found"));
						drupal_not_found();
						exit;	
					}	  
  	
		//We don´t want to reprocess every bit every time we re-render or sumbit the form. So store the processed ontology values on form_state		


		if (!isset($form_state['islandora_ontologies']))
			{
				$restrictionsOnOntologies=array();
		$currentontologies= islandora_ontologies_get_associated_ontologies($islandora_object);//@see utilities.inc
		$currentrelations=islandora_ontologies_get_current_localrelations($islandora_object_pid);//@see utilities.inc
		$posible_relastionships=islandora_ontologies_get_possible_localrelations($currentontologies);
		$currentrelationsfiltered=islandora_ontologies_filter_existing_relations($currentontologies,$currentrelations);
		foreach ($currentontologies as $contentmodelpid=>$ontology_objectpid)
			{
		
				$restrictionsOnOntologies[$ontology_objectpid]=islandora_ontologies_get_restricted_properties($ontology_objectpid,$contentmodelpid,$currentrelationsfiltered[$ontology_objectpid]);
			}
			dpm($restrictionsOnOntologies);
		$form_state['islandora_ontologies']['current']=$currentrelations;
		$form_state['islandora_ontologies']['restrictions']=$restrictionsOnOntologies;
		$form_state['islandora_ontologies']['posible']=$posible_relastionships['posiblerels_complete'];
		$form_state['islandora_ontologies']['options']=$posible_relastionships['posiblerels_asformoptions'];
		}	
		
		
		
		
		
		
	}
	