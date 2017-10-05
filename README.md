# Islandora Ontologies Solution Pack

This module allows users to create, upload, associate and manage multiple OWL ontologies inside an Islandora 7.x repository.
By doing so, LoD and complex (even cyclic) relationships between Object types are possible, extending the use of islandora 
to a variety of more complex discipline specific scenarios, where default hierarchies (like Collection --> book --> page) 
are not enough to explain relationships between objects and the use of higher expressive data modeling via existing, 
published standard RDF/OWL ontology predicates are needed.

This module creates a new Content Model that allows you to create islandora objects that store the ontologies into a Datastream
This special objects provide the functionality to parse that ontology, extract the OWL:class elements and match their values to
existing Content Models in the same repository<sup>1</sup>. The user can then associate the ontology to those which will then
enable additional functionality of all Digital objects that adhere to those. The functionality includes:
* RDF Properties editor that uses OWL Lite / OWL 2 DL based onProperty and onClass restrictions to aid in triple 
creation
* Cached graph traversal and business rule based, simple, reasoning algorithm to find the best path (or all the paths) between objects in
an arbitrary graph based on the previous ontologies.
* Multiple ontologies can live side by side allowing multiple open world semantic interpretations.
* Smart removal of properties in case of object removal.
* Javascript integration (visualization) and Solr based search improvements.
* Much more!


This module includes external libraries (sandbox)

<sup>1</sup> [Example Ontology with owl:class matching Islandora CMODELS ](https://github.com/DiegoPino/islandora_solution_pack_redbiodiversidad-7.x-dev/blob/7.x-dev/rdf/dsw.owl)
