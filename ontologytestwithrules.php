<?php

include_once("arc2/ARC2.php");
include_once("Graphite/Graphite.php");
use Ruler\RuleBuilder;
use Ruler\Context;

function test(){

	$fedora_url = 'http://localhost:8080/fedora';
	$xmlfilelink = $fedora_url."/objects/ontologies:f60e3156-0ff9-478e-8c2a-3c8f53533a16/datastreams/ONTOLOGY/content";	
	$graph = new Graphite();
	$graph->load($xmlfilelink);
	foreach($graph->allOfType('owl:Ontology') as $ontology)
	{
		echo "ontology uri".$ontology->uri;
	}
	foreach($graph->allOfType('owl:ObjectProperty')->sort( "rdfs:label") as $property)
	{
		echo $property->dumpText()."\n\r";
		echo $property->get("-owl:onProperty")->dumpText()."\n\r";
		echo $property->get("-owl:onProperty")->get("-rdfs:subClassOf")->uri."\n\r";//Si el URI es el $subject entonces ese modelo de contenidos posee esa restriccion
		echo "relaciones\n\r";
		//echo $property->relations()->dumpText();
		echo "\n\r";
	}
		get_restrictions_onClasses($graph);
			
	
	
}

function get_restrictions_onClasses($resource_list)
{		
	spl_autoload_register(function($c){
		
		@include preg_replace('#\\\|_(?!.+\\\)#','/',$c).'.php';});
	

    $rb = new RuleBuilder;
 	$context = new Context;
	$rels=array();
	$rels_cmodel=array();
	//Test rels
	$rels[0]= array('predicate' => array(
	                           'value' => 'identifiedAs',
	                           'alias' => 'rdb',
	                           'namespace' => 'http://www.semanticweb.org/dpino/ontologies/2014/3/untitled-ontology-3#',
	                       ),	     
	                  'object' => array
	                      (
	                           'literal' => 'false',
	                           'value' => 'islandora:1',
	               			),		
					);	
		$rels[1]=array(					                  'predicate' => array(
					                           'value' => 'identifiedAs',
					                           'alias' => 'rdb',
					                           'namespace' => 'http://www.semanticweb.org/dpino/ontologies/2014/3/untitled-ontology-3#',
					                       ),
	     
					                  'object' => array
					                      (
					                           'literal' => 'false',
					                           'value' => 'islandora:2',
	     
					               			)		
									);	
		//Test cmodels for every member of rels(object)
		$rels_cmodel[0]='info:fedora/biodiversity:TaxonCModel';
		$rels_cmodel[1]='info:fedora/biodiversity:MaterialSampleCModel';
		
		
		
		$subject="info:fedora/biodiversity:IdentificationCModel";
		//Now define our construction blocks for our rules Aka as propositions
		$cardinality = $rb->create(
		    $rb->logicalAnd(
		        $rb['minNumRel']->lessThanOrEqualTo($rb['matchedNumRels']),
		        $rb['maxNumRel']->greaterThanOrEqualTo($rb['matchedNumRels'])
		    ),
		    function() use ($context) {
		        echo 'cardinalidad Cumple para '.$context['OnProperty']."\n\r";
		    }
		);
		
		$cardinalitynotmet=$rb->create(
					$rb->logicalNot($cardinality),
						function() use ($context) {
							echo "Cardinalidad No cumple para ".$context['OnProperty']."\n\r";
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
		
		
		$otherrelationssameontology= $rb->create(
					$rb['actualOnClass']->Complement(
					$rb['OnClass'])->NotEqualTo(NULL),
					function()
						{
							echo "hay otras\n\r";
						}
				
				);
					
				
						
		
		
		$propertyonlyClass = $rb->create(
		$rb->logicalAnd($propertymetsClass,$rb->logicalNot($otherrelationssameontology))
		);
		
		
		$allvaluesFrom=$rb->create(
			$rb->logicalOr(
				$rb['actualNumRel']->EqualTo(0),//Proposition 1, there are no relations
				$propertyonlyClass
				),
				function() use ($context)
					{
						echo "allvaluesfrom \r\n";
					}
			);
			
		$somevaluesFrom=$rb->create(
				 $rb['matchedNumRels']->greaterThanOrEqualTo(1)
				,function() use ($context)
					{
					echo "somevaluesfrom\r\n";
					}											 	 
			 );
			 //!importante dejar siempre una regla para el caso de que no haya ninguna relación, que permita agregar una claro, aunque la ontologia exija una
				
		foreach($resource_list->allOfType('owl:Class') as $class)
		{
			if ($class->uri==$subject)
			{	
			
				foreach($class->all('rdfs:subClassOf') as $thing)
				{
					if ($thing->isType('owl:Restriction'))
					{
						
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
							
								//Now Look if there are cardinality and existance restricctions and infere some simple rules
								//In OWL Lite Cardinality can only be 0 or 1, we could add also owl2 owl:qualifiedCardinality for compatibility if ontology was generated using i.e Protege
								if ($thing->has("owl:cardinality"))
								{
									if ((int)$thing->getLiteral("owl:cardinality")==0)
									{
										$context['minNumRel']=0;
										$context['maxNumRel']=$context['minNumRel'];
									
									
									
									}
									elseif ((int)$thing->getLiteral("owl:cardinality")==1)
									{
										$context['minNumRel']=1;
										$context['maxNumRel']=1;
									
									}
									else
									{
										//"OWL LITE only allows 0 or 1 as values for owl:maxCardinality";
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
										$context['maxNumRel']=(int)$thing->getLiteral("owl:maxCardinality");
										
									}
								
								}
								
							
							
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
									
					
								}
								else
								{
									//Nothing to restrain, jump to the next
									echo "no to class restriction at all";
									continue; 
					
						
								}	
							}
							
							//Rule Context and Propositions definition
							//We must check two different propositions here:
							//First if we have space for adding a new allowed relation to this object. This works on this loop. We have a onPropery, onClass pair.
							//Check for incorrect relations already in place: we must filter out relations that are not defined in this ontology.This is a must because we are working on a local scope here.
							//This check should be done a Class basis, not in this loop?
							//Are Both propositions utually exclusive? a) We can allow the adding of new relations(allowed ones) even if there are wrong ones in place or b) we can consider 
							//waiting for them to be fixed before we continue adding new ones.
							//I like b), no new ones until everything is fixed. So we mantain our Repository consistent with the given ontology
							
							//Context Variable, holds and array with indexes to matching OnProperty, OnClass existing relations
							
							
							$context['matchedRels']=function() use ($rels,$rels_cmodel,$context)							
								{
									$match=array();
									foreach($rels as $relation_key=>$rel)
										{	
											if (($rel['predicate']['namespace'].$rel['predicate']['value'] == $context['OnProperty']) && ($rels_cmodel[$relation_key] == $context['OnClass']))
												{	
												$match[]=$relation_key;//Hash of matched rels	
												}
										}
										var_dump( $match,TRUE);
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
							$context['actualOnClass'] = function() use ($rels,$rels_cmodel,$context)
											{
												$classes=array();
												foreach($rels as $relation_key=>$class)
													{	
														if ($class['predicate']['namespace'].$class['predicate']['value'] == $context['OnProperty'])
															{	
															$classes[]=$rels_cmodel[$relation_key];		
															}
													}
													//var_dump($props,TRUE);
													return $classes;
											};		
						//End of Context definition
								
									
						$otherrelationssameontology->execute($context);
							
						$cardinalitynotmet->execute($context);
						$cardinality->execute($context);
						$notsetCardinality->execute($context);
						
						//$existanceOfProperty->execute($context);
						$propertymetsClass->execute($context);
						$allvaluesFrom->execute($context);
						$somevaluesFrom->execute($context);	
							
						
					
						
						
						
						}		
						else
							{ 
								//Nothing to restrain, jump to the next
								echo "no hay onPropery";
								continue;	
							}			
				
						}
					else
						{
							
							echo "no hay restriccion";
							continue;	
							
						}						
					
						
				}
			}

		
		}	
	

	echo "\r\n";

	
	
	
}
test();

?>  