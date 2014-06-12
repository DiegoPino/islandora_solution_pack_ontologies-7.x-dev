#!/usr/bin/php -q
<?php

include_once("/var/wwwsites/tools/arc2/ARC2.php");
include_once("/var/wwwsites/tools/Graphite/Graphite.php");

$g = new Graphite();
$g->load("./southampton-food-outlets.nt");
$g->load("http://id.southampton.ac.uk/dataset/southampton-postcodes/latest");

$r = new Graphite();
foreach($g->allSubjects() as $place)
{
	if(!($place->has("http://xmlns.com/foaf/0.1/based_near")))
	{
		continue;
	}
	
	$uri = "" . $place;
	$label = "" . $place->label();
	$lat = (float) ("" . $place->get("http://xmlns.com/foaf/0.1/based_near")->get("http://www.w3.org/2003/01/geo/wgs84_pos#lat"));
	$lon = (float) ("" . $place->get("http://xmlns.com/foaf/0.1/based_near")->get("http://www.w3.org/2003/01/geo/wgs84_pos#long"));
	$postcode_uri = "" . $place->get("http://xmlns.com/foaf/0.1/based_near");

	if(($lat == 0) & ($lon == 0))
	{
		continue;
	}

	$r->addTriple($uri, "http://www.w3.org/2000/01/rdf-schema#label", $label, "literal");
	$r->addTriple($uri, "http://www.w3.org/2003/01/geo/wgs84_pos#lat", $lat, "http://www.w3.org/2001/XMLSchema#double");
	$r->addTriple($uri, "http://www.w3.org/2003/01/geo/wgs84_pos#long", $lon, "http://www.w3.org/2001/XMLSchema#double");
	$r->addTriple($uri, "http://xmlns.com/foaf/0.1/based_near", $postcode_uri);
}

$fp = fopen("./southampton-food-outlets.nt", "w");
fwrite($fp, $r->serialize("NTriples"));
fclose($fp);
