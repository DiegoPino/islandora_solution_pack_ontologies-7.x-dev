#!/usr/bin/php -q
<?php

include_once("/var/wwwsites/tools/arc2/ARC2.php");
include_once("/var/wwwsites/tools/Graphite/Graphite.php");
require_once("/var/wwwsites/tools/PHP-SPARQL-Lib/sparqllib.php");

$open_times = array();

@$file = $argv[1];
if(!(file_exists($file)))
{
	file_put_contents('php://stderr', "File not found. Usage: to_triples [filename]\n");
	exit();
}

$data = csv_to_array( $argv[1] );

$graph = new Graphite();
$graph->ns( "gr", "http://purl.org/goodrelations/v1#" );
$graph->ns( "ov", "http://open.vocab.org/terms/" );
$graph->ns( "soton", "http://id.southampton.ac.uk/ns/" );
$graph->ns( "foodfeature", "http://id.southampton.ac.uk/food-feature/" );
$graph->ns( "cpv", "http://purl.org/cpv/2008/" );

$features_config = array(
	array( 
		"id"=>"feature vegan",
		"label"=>"Vegan",
		"uri"=>"foodfeature:Vegan",
	),
	array( 
		"id"=>"feature vegetarian",
		"label"=>"Vegetarian",
		"uri"=>"foodfeature:Vegetarian",
	),
	array( 
		"id"=>"feature contains alcohol",
		"label"=>"Contains Alcohol",
		"uri"=>"foodfeature:ContainsAlcohol",
	),
	array( 
		"id"=>"feature dairy free",
		"label"=>"Dairy Free",
		"uri"=>"foodfeature:NoDairy",
	),
	array( 
		"id"=>"feature gluten free",
		"label"=>"Gluten Free",
		"uri"=>"foodfeature:NoGluten",
	),
	array( 
		"id"=>"feature nut free",
		"label"=>"Nut Free",
		"uri"=>"foodfeature:NoNuts",
	),
	array( 
		"id"=>"feature nut free",
		"label"=>"Nut Free",
		"uri"=>"foodfeature:NoNuts",
	),
	array( 
		"id"=>"feature special",
		"label"=>"Special",
		"uri"=>"foodfeature:Special",
	),
);

$rank = 0;
$sections = array();
foreach( $data as $row )
{
	$rank++; # for arbitrary ordering of stuff

	$loc = trim("http://id.southampton.ac.uk/point-of-service/" . $row["pos"]);
	$open_time = "";
	$close_time = "";
	if(array_key_exists($loc, $open_times))
	{
		$open_time = $open_times[$loc]['open'];
		$close_time = $open_times[$loc]['close'];
	}
	else
	{
		$obj = getOpenTimes($loc);
		if(count($obj) > 0)
		{
			$open_time = $obj['open'];
			$close_time = $obj['close'];
			$open_times[$loc] = $obj;
		}
	}

	if(strlen($open_time) == 0)
	{
		continue;
	}

	$price_student = trim( str_replace("£", "", $row["student price"] ) ); 
	$price_staff = trim( str_replace("£", "", $row["staff price"] ) );

	$features = array( "foodfeature:Special" );
	foreach( $features_config as $item )
	{
		if( strtoupper( substr(@$row[ $item["id"] ],0,1) ) == "Y" ) { $features []= $item["uri"]; }
	}
	$uri_section = $loc."#pricelist-section-".ccaps( $row["section"] );

	if( !array_key_exists( $uri_section, $sections ) )
	{
		$sections[$uri_section] = true;
		
		$graph->resource( $uri_section )
			->add( "rdfs:label", $row["section"], "literal" )
			->add( "rdf:type", "oo:PriceListSection" )
			->add( "soton:specialsMenu", "true", "xsd:boolean" )
			->add( "ov:rank", $rank, "xsd:integer" )
		;
	}

	$title = $row["name"];

	$uri_product = $loc . "#offering-" . date("Ymd") . "-" . ccaps( $row["section"].$row["name"] );
	
	$graph->resource( $uri_product )
		->add( "a", "gr:ProductOrService" )
		->add( "rdfs:label", $row["section"].": ".$title, "literal" )
		->add( "soton:shortMenuLabel", $title, "literal" )
	;

	if( $price_student )
	{
		$graph->resource( $uri_product."-Student" )
			->add( "a", "gr:Offering" )
	 		->add( "rdfs:label", $row["section"].": ".$title." - £".$price_student." (Student Price)" , "literal")
			->add( "oo:priceListSection", $uri_section )
	 		->add( "gr:availableAtOrFrom", $loc )
			->add( "gr:eligibleCustomerTypes", "soton:peopleWhoDoNotPayVAT" )
			->add( "gr:hasBusinessFunction", "gr:Sell" )
			->add( "gr:includes", $uri_product )
			->add( "gr:hasPriceSpecification", $uri_product."-Student-price" )
			->add( "gr:availabilityStarts", date("c", $open_time), "xsd:dateTime" )
			->add( "gr:availabilityEnds", date("c", $close_time), "xsd:dateTime" )
			->add( "ov:rank", $rank, "xsd:integer" )
		;
	
		$graph->resource( $uri_product."-Student-price" )
			->add( "gr:hasCurrency", "GBP", "xsd:string" )
			->add( "gr:hasCurrencyValue", $price_student, "xsd:float" )
		;
	}

	if( $price_staff != "" )
	{
		$graph->resource( $uri_product."-Staff" )
			->add( "a", "gr:Offering" )
	 		->add( "rdfs:label", $row["section"].": ".$title." - £".$price_staff." (Staff Price)" , "literal")
			->add( "oo:priceListSection", $uri_section )
	 		->add( "gr:availableAtOrFrom", $loc )
			->add( "gr:hasBusinessFunction", "gr:Sell" )
			->add( "gr:includes", $uri_product )
			->add( "gr:hasPriceSpecification", $uri_product."-Staff-price" )
			->add( "gr:availabilityStarts", date("c", $open_time), "xsd:dateTime" )
			->add( "gr:availabilityEnds", date("c", $close_time), "xsd:dateTime" )
			->add( "ov:rank", $rank, "xsd:integer" )
		;
	
		$graph->resource( $uri_product."-Staff-price" )
			->add( "gr:hasCurrency", "GBP", "xsd:string" )
			->add( "gr:hasCurrencyValue", $price_staff, "xsd:float" )
		;
	}

	foreach( $features as $feature )	
	{
		$graph->resource( $uri_product )->add( "soton:hasFoodFeature", $feature );
	}

}

foreach( $features_config as $item )
{
	$graph->t( $item["uri"], "a", "soton:FoodFeature" );
	$graph->t( $item["uri"], "rdfs:label", $item["label"],"literal","en");
}

$graph->resource( "soton:FoodFeature" )
	->add( "a", "owl:Class" )
	->add( "rdfs:subClassOf", "gr:QualitativeValue" )
	->add( "rdfs:label", "Food Feature","literal","en" )
;

$graph->resource( "soton:hasFoodFeature" )
	->add( "a", "owl:ObjectProperty" )
	->add( "rdfs:subPropertyOf", "gr:qualitativeProductOrServiceProperty" )
	->add( "rdfs:label", "has food feature","literal","en" )
;

print $graph->serialize( "NTriples" );
exit;

function ccaps( $string )
{
	$string = ucwords( $string );
	$string = preg_replace( "/[^a-zA-Z0-9]/", "", $string );
	return $string;
}



/*
            [pos] => 38-arlott
            [section] => Noodle Bar
            [name] => Szechuan Vegetable Noodles
            [feature vegan] => 
            [feature vegetarian] => YES
            [feature contains alcohol] => 
            [feature dairy free] => 
            [feature gluten free] => 
            [feature nut free] => 
            [student price] => 
            [staff price] => 

*/




function csv_to_array( $filename, $delimiter=',')
{
	$handle = fopen($filename, 'r');
	if( $handle === false )
	{
		print "Failed to open file\n";
		exit( 1 );
	}

	$header = false;
	$data = array();
	while (($row = fgetcsv($handle, 1000, $delimiter)) !== FALSE)
	{
		# clean up whitespace
		foreach( $row as $key=>$value )
		{
			$row[$key] = trim( $value );
		}

		if(!$header)
		{
			$header = array();
			foreach( $row as $item ) { $header []= strtolower( $item ); }
			
		}
		else
		{
			$data[] = array_combine($header, $row);
		}
	}
	fclose($handle);

	if( !$header )
	{
		print "Failed to parse CSV\n";
		exit( 1 );
	}

	return $data;
}

function getOpenTimes($uri)
{
	$ret = array();
	$ret['open'] = strtotime("today 12:00:00 Europe/London");
	$ret['close'] = strtotime("today 14:00:00 Europe/London");
	return $ret;

	# can't assume items availability same as opening hours :(
	$datestring = gmdate("Y-m-d");
	$timestring = gmdate("H:i:s");
	$daystring = gmdate("l");

	$sparql = '
SELECT ?opentime ?closetime WHERE {
    <' . $uri . '> <http://purl.org/goodrelations/v1#hasOpeningHoursSpecification> ?timespan .
    ?timespan <http://purl.org/goodrelations/v1#opens> ?opentime .
    ?timespan <http://purl.org/goodrelations/v1#closes> ?closetime .
    ?timespan <http://purl.org/goodrelations/v1#validFrom> ?dayfrom .
    ?timespan <http://purl.org/goodrelations/v1#validThrough> ?dayto .
    ?timespan <http://purl.org/goodrelations/v1#hasOpeningHoursDayOfWeek> <http://purl.org/goodrelations/v1#' . $daystring . '> .
    FILTER (?dayfrom <= "' . $datestring . 'T' . $timestring . 'Z"^^<http://www.w3.org/2001/XMLSchema#dateTime>) .
    FILTER (?dayto >= "' . $datestring . 'T' . $timestring . 'Z"^^<http://www.w3.org/2001/XMLSchema#dateTime>) .
} LIMIT 10';

	$data = sparql_get( "http://sparql.data.southampton.ac.uk/", $sparql );
	if( !$data ) { print sparql_errno() . ": " . sparql_error(). "\n"; exit( 1 ); }

	$ret = array();
	if(count($data) )
	{
		// I figure this is OK as no university points of service are ever likely to move out of GMT.
		$ret['open'] = strtotime("today " . $data[0]["opentime"] . " Europe/London"); 

		// This is necessary because of that hideous beast known as daylight savings time.
		$ret['close'] = strtotime("today " . $data[0]["closetime"] . " Europe/London"); 
	} 
	else 
	{
		$ret['open'] = strtotime("today 00:00:00 Europe/London");
		$ret['close'] = strtotime("today 23:59:59 Europe/London");
	}
	return($ret);
}

