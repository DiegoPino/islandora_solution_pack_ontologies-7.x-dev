#!/usr/bin/php -q
<?php

include_once("/var/wwwsites/tools/arc2/ARC2.php");
include_once("/var/wwwsites/tools/Graphite/Graphite.php");

@$csvfile = $argv[1];
if(!(file_exists($csvfile)))
{
        exit();
}

// This is a general CSV reader for parsing data output from Planon.

$headers = array();
$data = array();
$fp = fopen($csvfile, "r");
while (($line = fgetcsv($fp, 4096, ",")) !== FALSE)
{
	if(count($headers) == 0)
	{
		$headers = $line;
		continue;
	}
	$c = count($line);
	$item = array();
	for($i = 0; $i < $c; $i++)
	{
		$k = "";
		$v = "";
		@$k = $headers[$i];
		if(strlen($k) == 0)
		{
			continue;
		}
		$k = preg_replace("/[^a-z0-9]/", "", strtolower($k));
		@$v = $line[$i];
		if((strlen($v) == 0) | (strcmp($k, preg_replace("/[^a-z0-9]/", "", strtolower($v))) == 0))
		{
			continue;
		}
		$item[$k] = $v;
	}
	if(count($item) > 0)
	{
		$data[] = $item;
	}
}
fclose($fp);

$g = new Graphite();
$g->ns("sr", "http://data.ordnancesurvey.co.uk/ontology/spatialrelations/");
$g->ns("oo", "http://purl.org/openorg/");
foreach($data as $item)
{
	$room_id = preg_replace("/^.*[^0-9A-Za-z]/", "", trim($item['spacenumber']));
	$building_id = ltrim($item['buildingnumber'], "0");
	if((strlen($room_id) < 4) & (preg_match("/^[0-9]/", $room_id) > 0))
	{
		$room_id = trim(substr("0000" . $room_id, -4));
	}
	$building_uri = "http://id.southampton.ac.uk/building/" . $building_id;
	$room_uri = "http://id.southampton.ac.uk/room/" . $building_id . "-" . $room_id;
	$cfg_uri = $room_uri . "#config";
	$feature_uri = $room_uri . "#feature-shower";

	$g->addCompressedTriple($room_uri, "rdf:type", "http://vocab.deri.ie/rooms#Room");
	$g->addCompressedTriple($room_uri, "rdfs:label", $building_id . " / " . $room_id, "literal");
	$g->addCompressedTriple($room_uri, "sr:within", $building_uri);
	$g->addCompressedTriple($room_uri, "oo:hasFeature", $feature_uri);
	$g->addCompressedTriple($room_uri, "http://purl.org/openorg/space-configuration/spaceConfiguration", $cfg_uri);

	$g->addCompressedTriple($cfg_uri, "rdf:type", "http://purl.org/openorg/space-configuration/Bathroom");

	$g->addCompressedTriple($feature_uri, "rdf:type", "http://id.southampton.ac.uk/location-feature/Shower");
	$g->addCompressedTriple($feature_uri, "rdfs:label", "Shower in B" . $building_id . " R" . $room_id, "literal");
	$g->addCompressedTriple($feature_uri, "rdfs:description", "Please note - this shower may not be available at all times, or to all members of the University.", "literal");
}

$g->addCompressedTriple("http://id.southampton.ac.uk/location-feature/Shower", "rdfs:label", "Shower", "literal");

print($g->serialize("NTriples"))
