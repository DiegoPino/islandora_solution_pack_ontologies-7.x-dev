<?php

/**
 * This example script will search for a term in the government art collection, and 
 * retrieve some more RDF about a particular type of thing we are interested in.
 * This demonstrates the Follow-Your-Nose Paradigm possible with Linked Data.
 *
 * The program runs like this:
 * - Use the SPARQL API to find some things which have an image and filter out
 *    only those which have a certain term in the title
 * - Then for each thing in the returned graph that is a type of Art Work and...
 * - - Use the Lookup API to find out more about the Art Work.
 * - Then look for people depicted in the works and...
 * - - Use the Lookup API to find out more about the people in the picture.
 * - Then look for any information on these people from dbpedia.
 * - As we go we keep count of the number of triples we find.
 *
 * By the time the script exits and spits out some turtle, we have enough information
 *  to be able to build a display of Art works and detailed information about the 
 *  people in the work - (possibly too much!)
 *
 * Run this script from the commandline
 * $ php -f goverment-art-collection-FYN.php
 *
 *
 * We use the ARC and Graphite libraries to simplify getting and working with RDF.
 * https://github.com/semsol/arc2/wiki
 * http://graphite.ecs.soton.ac.uk/
 * 
 *
 */

include_once("arc/ARC2.php");
include_once("Graphite.php");

$graph = new Graphite();
$graph2 = new Graphite();

// local stuff
// I am using a local proxy to add in the apikey for all my kasabi calls
$kasabi_api_base = "http://127.0.0.1/kasabi" ;
$kasabi_api_key = "";

// You might have your apikey as a local user environment parameter.
// $kasabi_api_base = "http://api.kasabi.com" ;
// $kasabi_api_key = "&apikey=".getenv('KASABI_API_KEY');

$searchTerm = "Queen Victoria";

// get some RDF data (using a describe query)
$query = urlencode('
PREFIX dcterms: <http://purl.org/dc/terms/>
PREFIX foaf: <http://xmlns.com/foaf/0.1/>
PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
PREFIX dc: <http://purl.org/dc/elements/1.1/>
PREFIX gac: <http://data.kasabi.com/dataset/government-art-collection/schema/>

describe ?subject ?image 
WHERE {
  ?subject dcterms:title ?title ;
           foaf:depiction ?image .
  filter (regex(?title ,"'.$searchTerm.'", "i"))
  
}
limit 10
  ');

$search_url=$kasabi_api_base."/dataset/government-art-collection/apis/sparql/?query=".$query."&output=xml".$kasabi_api_key;

print $search_url;
// load it into our graph
$loadCount = $graph->load($search_url) ;
$extraCount=0;

if($loadCount > 0 ){
  
  // if we loaded some data
  print "\nloaded ".$loadCount." triples from search\n";

  // look for all the things which are Art Works.
  foreach ($graph->allOfType("http://data.kasabi.com/dataset/government-art-collection/schema/ArtWork") as $obj){
  
    // lookup more information about this thing and load this new data into our second graph.
    $extra += $graph2->load($kasabi_api_base."/dataset/government-art-collection/apis/lookup?about=".urlencode($obj));


    // a bit of counting to see how many triples we have found after each lookup.
    $extraCount += $extra ;
    print "loaded ".$extra." triples\n";
  }

  // loop through all the things in the RDF
  print "Looking for all the sitters\n";
  foreach ($graph2->allSubjects() as $work ){
    // lookup more information about the sitters
    foreach($work->all("http://data.kasabi.com/dataset/government-art-collection/schema/depicts") as $sitter){
      // lookup more information about this thing and load this new data into our second graph.
      $extra += $graph2->load($kasabi_api_base."/dataset/government-art-collection/apis/lookup?about=".urlencode($sitter));

      // a bit of counting to see how many triples we have found after each lookup.
      $extraCount += $extra ;
      print "loaded ".$extra." triples\n";
    }
  }

  // loop thorugh all the sitters and see if they have any info on dbpedia.
  print "Looking for owl:sameAs links\n";
  foreach ($graph2->allOfType("http://xmlns.com/foaf/0.1/Agent") as $agent) {
    foreach($agent->all("http://www.w3.org/2002/07/owl#sameAs") as $sameThing){
      // lookup more information about this thing and load this new data into our second graph.
      $extra += $graph2->load($sameThing);

      // a bit of counting to see how many triples we have found after each lookup.
      $extraCount += $extra ;
      print "loaded ".$extra." triples\n";

    }

  }

  // how many extra triples have we found?
  print "Found ".$extraCount." triples\n";

}else{
    print  "no resources\n";
}

//spit out what we have found.
print $graph2->serialize("Turtle");

?>