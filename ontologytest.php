<?php

include_once("arc2/ARC2.php");
include_once("Graphite/Graphite.php");
//include_once("Ruler/src/Ruler/RuleBuilder.php");

function test(){

	$fedora_url = 'http://localhost:8080/fedora';
	$xmlfilelink = $fedora_url."/objects/ontologies:f60e3156-0ff9-478e-8c2a-3c8f53533a16/datastreams/ONTOLOGY/content";	
	$graph = new Graphite();
	$graph->load($xmlfilelink);
	//$graph->resource("_:g1-arc461fb8")->__toString();
	/*

	foreach($graphite_graph->allOfType('owl:Class') as $class)
	{
	
		
	foreach($class->all('rdfs:subClassOf') as $thing)
	{
				
	print $thing->dumpText();
	echo "\r\n";
	print $thing->get('-rdfs:subClassOf')->__toString();
	print "Relations\n\r".$thing->relations()->dumpText()."\n\r";//Esto me devuelve todos los elementos dentro de Subclase
	echo "\r\n";
	if ($thing->has("owl:onProperty")){echo $thing->getString("owl:onProperty"); //echo $graph->allofType('owl:ObjectProperty')->get($thing->getString("owl:onProperty"))->dumpText();
	}
	if ($thing->has("owl:onClass")){echo "\r\nOn Class: ".$thing->get("owl:onClass")->uri;}
			
	}

	echo "\r\n";

	}





	*/
	//$all_classdefinitons = $graph->allOfType('owl:Class');
	//$all_onClassrelationes = $graph->allOfType('owl:Restriction')->get('owl:onClass')->all('rdf:type');//This way we get the list of existing classes that have restrictions. Restriction to non existing classes are ommited
	//print $all_onClassrelationes->dumpText();
	//$new_resourcelist = $all_onClassrelationes->intersection($all_onClassrelationes);
	//print $new_resourcelist->dumpText();
	foreach($graph->allOfType('owl:Class') as $classes)
	{

		get_restrictions_onClasses($classes);
	}

	/*foreach($graph->allOfType('owl:ObjectProperty') as $properties)
	{
	print $properties->dumpText();
	echo "\n\r";
	}*/
}

function get_restrictions_onClasses($resource_list)
{		
	
    //$rb = new RuleBuilder;
 	
	print "For Class: ".$resource_list->uri."\n\r";
	$classuri=$resource_list->uri;
	foreach($resource_list->all('rdfs:subClassOf') as $thing)
	{
		/* We are using an rule set to describe restrictions that must be satisfied because multiple dependant restrictions are posible on a given Property/class even in OWL Lite.
		i.e owl:someValuesFrom->Class implies in our grammar:
			1.- that there must first be satisfied the condition of having for Class X at least on relation on property Y on Class Z. It´s existential. there must be at leat one!
			2.- Now we can allow For Class X any onProperty any onClass relation
		/*This way we can chain rules, it´s not even close to a fully reasoner but simplistic and handy way of checking what relations the final user are allowed to apply in Fedora Commons context to conform to this local Ontology
		
		
		
		
		
		
		we are usign https://github.com/bobthecow/Ruler as Rule engine beacuse it allows us to make complex rules based on simpler ones
		Each rule consist of at one of the following operators
		$a->greaterThan($b);          // true if $a > $b
		$a->greaterThanOrEqualTo($b); // true if $a >= $b
		$a->lessThan($b);             // true if $a < $b
		$a->lessThanOrEqualTo($b);    // true if $a <= $b
		$a->equalTo($b);              // true if $a == $b
		$a->notEqualTo($b);           // true if $a != $b
		$a->contains($b);             // true if in_array($b, $a) || strpos($b, $a) !== false
		$a->doesNotContain($b);       // true if !in_array($b, $a) || strpos($b, $a) === false
		$a->sameAs($b);               // true if $a === $b
		$a->notSameAs($b);            // true if $a !== $b
		
		that can be combined in any way using logical operators
		$rb->logicalNot($aEqualsB);                  // The same as $aDoesNotEqualB :)
		$rb->logicalAnd($aEqualsB, $aDoesNotEqualB); // True if both conditions are true
		$rb->logicalOr($aEqualsB, $aDoesNotEqualB);  // True if either condition is true
		$rb->logicalXor($aEqualsB, $aDoesNotEqualB); // True if only one condition is true
		
		each operator recives a context(the set of variables or even functions we wan´t to check)
		In this case our context variables come from a sparql query to our repo and their relations, what we want is a list of:
				objects who have fedora-model:hasModel with value $classuri and their relations.
		
		
		
		
		transforming owl restrictions to rules:
		basics are:
		onPropery:property
			 and (onClass:Class or allvaluesFrom or SomeValuesFrom) -> this conditions to a local range with some cardinality and existentiality restrictions
		
		se what we need to do is a chain of rules:
		using the same names as owl:
		
		$rule_allvaluesFrom=$rb->create($existing_object_class->)
			
		
		
		
		
		
		
	
		for each class
			for each option
				get rule() return array(property,min,max,toClass,mandatory) mandatory=TRUE or FALSE, min=0,1 max=0,1,any
					if !check_repo_complies_to_rule(property,class,min,max,toClass) 
					if mandatory
						alert('bad objects')
						continue;
					else
						render_userchoice(rule())
					
				
		
		
		
		
		*/
		$infered_restriction=array();
				
		if ($thing->isType('owl:Restriction'))
		{
			//print $thing->dumpText();	
			$rule_mandatory=FALSE;//FLAG INITIAL THIS RULE AS OPTIONAL
			
			//It's a local restriction 
			//First check if there is an owl:onProperty and the property is defined in this document
					
			if (($thing->has("owl:onProperty")) && ($thing->get("owl:onProperty")->has('rdf:type')))
			{
				print "property ".$thing->get("owl:onProperty")->uri;
				
				$rule_toproperty=$thing->get("owl:onProperty")->uri;
				
			}
			else
			{ 
						
				echo "property not present or invalid";
				//Nothing to restrain, jump to the next
				continue;	
			}
			//Now check if there if  owl:onProperty and the property is defined in this document	
			if (($thing->has("owl:onClass")) && ($thing->get("owl:onClass")->has('rdf:type')))	
			{
				print "\r\non Class: ".$thing->get("owl:onClass")->uri;	
				$rule_toclass=$thing->get("owl:onClass")->uri;
				//Now Look if there are cardinality and existance restricctions and infere some simple rules
				//In OWL Lite Cardinality can only be 0 or 1, we could add also owl2 owl:qualifiedCardinality for compatibility if ontology was generated using i.e Protege
				if ($thing->has("owl:cardinality"))
				{
					
					print "\r\n\tcardinality: ".$thing->getLiteral("owl:Cardinality");
					if ($thing->getLiteral("owl:Cardinality")==0)
					{
					$rule_mandatory=TRUE;	
					$rule_min=0;
					$rule_max=$rule_min;
					}
					elseif ($thing->getLiteral("owl:Cardinality")==1)
					{
					$rule_mandatory=TRUE;
					$rule_min=1;
					$rule_max=$rule_min;	
					}
					else
					{
					echo "OWL LITE only allows 0 or 1 as values for owl:maxCardinality";
					continue;	
					}
				}
				else
				{
					//Only if there is no cardinality we fetch max first and the min
					
					if ($thing->has("owl:minCardinality"))
					{
					
						print "\r\n\tmincardinality: ".	$thing->getLiteral("owl:minCardinality");
						$rule_min=(int)$thing->getLiteral("owl:minCardinality");
						 
					}
					if ($thing->has("owl:maxCardinality"))
					{
						if (!$thing->has("owl:minCardinality")){$rule_min=0;}//if min not present we assume as 0
						print "\r\n\tmaxcardinality: ".	$thing->getLiteral("owl:maxCardinality");
						$rule_max=(int)$thing->getLiteral("owl:maxCardinality");
						
					}
					
					if ($rule_min===$rule_max){$rule_mandatory=TRUE;}
				}
			
			
			}
			else
			{
				//owl:onClass is "disjoint" with owl:allValuesFrom and owl:someValuesFrom
				if (($thing->has("owl:allValuesFrom")) && ($thing->get("owl:allValuesFrom")->has('rdf:type')))		
				{
					print "\r\nAll values from Class: ".$thing->get("owl:allValuesFrom")->uri;	
					$rule_toclass=$thing->get("owl:allValuesFrom")->uri;
					$rule_mandatory=TRUE; //MEANS we have to treat the whole rule as a condition
					$rule_min=0;
					$rule_max=NULL;
					 // if whe there is a relation, then all must be to a entity of given class
				}
				elseif (($thing->has("owl:someValuesFrom")) && ($thing->get("owl:someValuesFrom")->has('rdf:type')))		
				{
					print "\r\nSome values from Class: ".$thing->get("owl:someValuesFrom")->uri;  
					//Means there must be at least one relation to an entity of given class, But we are free to relate more to the same or to others
					//similar to minCardinality=1 without maxCardinality + onClass 
					$rule_toclass=$thing->get("owl:someValuesFrom")->uri;
					$rule_mandatory=TRUE; //MEANS we have to treat the whole rule as a condition
					$rule_min=1;
					$rule_max=NULL;
					
				}
				else
				{
					continue; //there 	
					
						
				}	
			}
			if ((isset($rule_toproperty))&&(isset($rule_toclass))&&(isset($rule_min)))
			$parsed_restrictions[$classuri][]=array('onproperty'=>$rule_toproperty,'onclass'=>$rule_toclass,'min'=>$rule_min,'max'=>$rule_max,'mandatory'=>$rule_mandatory);
			
			
		}
	}

	echo "\r\n";

	var_dump($parsed_restrictions,TRUE);
	
	
	
	/*para cada clase iteramos por las restricciones, pueden ser varias
	1. chequear si existe has("owl:onProperty"), 
	2. chequear si existe object the onProperty existe y leerlo
	Para este extraer:
	- Label
	-ID
	alias
	type:TransitiveProperty
	type:Inversof -> chequear si existe object para inverseof
	     
	3. Chequear de más restrictivo a menos
	3.1 cardinality: 1 -> debe existir una sola instancia de clase owl:onClass con objecto de Onproperty
	3.2 cardinality: 0 -> No debe existir una instancia de clase owl:onClass con objecto de Onproperty
	3.3 minCardinality:1 -> es igual a 3.1
	3.4 minCardinality:0 -> puede o no existir
	3.5 maxCardinality:0 -> igual a 3.2
	3.5 maxCardinality:1 -> puedo existir como máximo 1. Si se puede omitir depende de minCardinality
	3.6 allValuesFrom: Puede no existir, pero si existe tienen que ser instancias de clase owl:onClass con objecto de Onproperty 			
	3.7 someValuesFrom: Al menos una instancia de clase owl:onClass con objecto de Onproperty, el resto puede ser cualquiera*/
	
	
	
	
}
test();








//agregar rdf:ID= a todo?
//Note that, unlike any of the value constraints described in the section on class descriptions, rdfs:range restrictions are global. Value constraints such as owl:allValuesFrom are used in a class description 
//	and are only enforced on the property when applied to that class. In contrast, rdfs:range restrictions apply to the property irrespective of the class to which it is applied. Thus, rdfs:range should be used with care.
//parsing restrictions
//We are going to work with OWL LITE, so restrictions are going to be always inside a Subclass of a Class

//Notice that owl:FunctionalProperty and owl:InverseFunctionalProperty specify global cardinality constraints. That is, no matter which class the property is applied to, the cardinality constraints must hold. 
//	This is different from the cardinality constraints contained in property restrictions. The latter are class descriptions and are only enforced on the property when applied to that class.
/*
The difference between the two formulations is the difference between a universal and existential quantification.

Relation	Implications
allValuesFrom	For all wines, if they have makers, all the makers are wineries.
someValuesFrom  	For all wines, they have at least one maker that is a winery.

3.4.3. hasValue [OWL DL]

hasValue allows us to specify classes based on the existence of particular property values. Hence, an individual will be a member of such a class whenever at least one of its property values is equal to the hasValue resource.

<owl:Class rdf:ID="Burgundy">
...
<rdfs:subClassOf>
<owl:Restriction>
<owl:onProperty rdf:resource="#hasSugar" />
<owl:hasValue rdf:resource="#Dry" />
</owl:Restriction>
</rdfs:subClassOf>
</owl:Class>
Here we declare that all Burgundy wines are dry. That is, their hasSugar property must have at least one value that is equal to Dry.

As for allValuesFrom and someValuesFrom, this is a local restriction. It holds for hasSugar as applied to Burgundy.

<rdf:type rdf:resource="&owl;TransitiveProperty"/>
y SymmetricProperty

*/
//1.- Get all Restriction->relations() -> Array or nodes
//My rules
//the subject of rdfs:subClassOf triples be class names and the object of rdfs:subClassOf triples be class names or restrictions;
//onProperty ->
//onClass ->resource
//cardinality
//maxCardinality
//minCardinality
//allValuesFrom ->resource the object of owl:allValuesFrom and owl:someValuesFrom triples be class names or datatype names;
//someValuesFrom ->resource	 ""
//the object of rdf:type triples be class names or restrictions;
//the object of rdfs:domain triples be class names; and
//the object of rdfs:range triples be class names or datatype names.
//NOTE: OWL Lite does not allow the use of owl:disjointWith.

/*	<owl:SymmetricProperty rdf:ID="friendOf">
<rdfs:domain rdf:resource="#Human"/>
<rdfs:range  rdf:resource="#Human"/>
</owl:SymmetricProperty>*/

//parse_ontology_fromObject(Abstract $islandora_object, $dsid ) /*Funcion principal en el objecto mismo de ontologia */
// get_current_ontologies_fromCModel(Abstract $islandora_cmodel_object) /* Verifica que relaciones hasOntology->PID tiene este Modelo, devuelve un Array 
// set_current_ontology_toCModel(Abstract $islandora_object, Abstract $islandora_cmodel_object) /*Relaciona la actual ontologia con un CMODEL agregando hasOntology->$islandora_object->pid
// get_restrictions_onProperty()
// get_restrictions_onClass()



//get_properties_fromOntology_forCModel(Abstract $islandora_cmodel_object,)
?>  