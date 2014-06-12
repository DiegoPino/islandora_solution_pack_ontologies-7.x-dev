<html lang="en"><head><meta content="text/html; charset=UTF-8" http-equiv="Content-Type"/><title>XSLT Documentation: RDFS/OWL presentation stylesheet</title><link href="ns-schema_files/kan01.htm" type="text/css" rel="stylesheet"/><style type="text/css">h3 {background:#eef; padding:0.2em} .pattern {color:maroon} .template-desc {margin-left: 1em} .template-desc ul, .template-desc ol {margin-top:0.3em} li .comnt {color:green}</style></head><body><h1>RDFS/OWL presentation stylesheet</h1><p>This
is an automatically generated XSLT stylesheet description, based on its
template calls, parameters, variables and annotating comments.</p><h2>Document description</h2><p>This stylesheet is designed to convert RDF Schema / OWL Ontology file to visible XHTML with structured definition list.</p><dl><dt>author</dt><dd>
     Masahide Kanzaki
     
    </dd><dt>created</dt><dd>2003</dd><dt>release</dt><dd>
  date: 2005-04-21, 
  version: 1.5</dd><dt>rights</dt><dd>(c) 2003-2005 by the author, copyleft under GPL</dd><dt>license</dt><dd><a href="http://creativecommons.org/licenses/GPL/2.0/">http://creativecommons.org/licenses/GPL/2.0/</a></dd></dl><dl><dt>Included XSLT</dt><dd><a href="http://rhizomik.net/redefer/xsl/banner-footer.xsl">./banner-footer.xsl</a></dd><dt>CSS linked</dt><dd><a href="http://rhizomik.net/parts/kan01.css">/parts/kan01.css</a></dd></dl><h2>Template descriptions</h2><h3 id="t--">match: <code class="pattern">/</code></h3><div class="template-desc"><div class="abstract"></div>
   template calls:
   <ul><li>match: { 
    <a href="#t-rdf.RDF">rdf:RDF</a>
  }</li></ul></div><h3 id="t-rdf.RDF">match: <code class="pattern">rdf:RDF</code></h3><div class="template-desc"><div class="abstract"></div>
   template calls:
   <ul><li>named: <a href="#t-htmlhead">htmlhead</a></li><li>named: <a href="#t-banner">banner</a></li><li>named: <a href="#t-toc">toc</a></li><li>match: { 
    <a href="#t-owl.Ontology_rdf.about%27%27.rdf.Description_rdf.about%27%27">owl:Ontology[@rdf:about='']|rdf:Description[@rdf:about='']</a>
  }</li><li>named: <a href="#t-generalexamples">generalexamples</a></li><li>named: <a href="#t-cplist">cplist</a><code> (
   l:
    rdfs:Class[*]|owl:Class[*]
   )</code></li><li>match: { 
    rdfs:Class|owl:Class
  }</li><li>named: <a href="#t-cplist">cplist</a><code> (
   l:
    rdf:Property[*]|owl:ObjectProperty[*]|owl:DatatypeProperty[*]|owl:AnnotationProperty[*]
   )</code></li><li>match: { 
    rdf:Property|owl:ObjectProperty|owl:DatatypeProperty|owl:AnnotationProperty
  }</li><li>match: { 
    rdf:Description[rdf:type]
  }</li><li>named: <a href="#t-findmore">findmore</a><code>
( l: *[local-name()!='Class' and name()!='rdf:Property' and
name()!='owl:ObjectProperty' and name()!='owl:DatatypeProperty' and
name()!='owl:AnnotationProperty' and name()!='owl:Ontology' and
name()!='ex:Example' and name()!='rdf:Description'] )</code></li><li>named: <a href="#t-footer">footer</a><code> (
   status:
    owl:Ontology/dcterms:modified
   )</code></li></ul>
   (explicitely) called by:
   <ul><li>match: { <a href="#t--">/</a> }</li></ul></div><h3 id="t-x">match: <code class="pattern">*</code></h3><div class="template-desc"><div class="abstract"></div>
   template calls:
   <ul><li>match: { 
    <a href="#t-_x">@*</a>
  }</li><li>match: { 
    <a href="#t-x">*</a>
  }</li></ul>
   (explicitely) called by:
   <ul><li>match: { <a href="#t-x">*</a> }</li></ul></div><h3 id="t-_x">match: <code class="pattern">@*</code></h3><div class="template-desc"><div class="abstract"><span class="comnt"> Displays attribute values  </span></div>
   (explicitely) called by:
   <ul><li>match: { <a href="#t-x">*</a> }</li></ul></div><h3 id="t-_xml.lang">match: <code class="pattern">@xml:lang</code></h3><div class="template-desc"><div class="abstract"><span class="comnt"> xml:lang is displayed with element name as (@en) etc.  </span></div></div><h3 id="t-x_rdf.resource._rdf.about">match: <code class="pattern">*[@rdf:resource|@rdf:about]</code></h3><div class="template-desc"><div class="abstract"><span class="comnt"> If the element has URI ref (rdf:about/rdf:resource) </span></div></div><h3 id="t-rdfs.x_rdf.resource._rdf.about">match: <code class="pattern">rdfs:*[@rdf:resource|@rdf:about]</code></h3><div class="template-desc"><div class="abstract"><span class="comnt"> RDFS element with URI ref (rdf:about/rdf:resource) </span></div></div><h3 id="t-rdf.RDF-rdf.Property.rdf.RDF-rdfs.Class.rdf.RDF-owl.xlocal-name ! 'Ontology'.rdf.RDF-rdf.Descriptionrdf.type">match: <code class="pattern">rdf:RDF/rdf:Property|rdf:RDF/rdfs:Class|rdf:RDF/owl:*[local-name() != 'Ontology']|rdf:RDF/rdf:Description[rdf:type]</code></h3><div class="template-desc"><div class="abstract"><span class="comnt"> Class and Property definition  </span></div>
   template calls:
   <ul><li>named: <a href="#t-idabout">idabout</a></li><li>match: { 
    @*[name()!='rdf:about' and name()!='rdf:ID' and name()!='rdfs:label']
  }</li><li>match: { 
    *[name()!='rdfs:label']
  }</li></ul></div><h3 id="t-rdfs.domain_rdf.resource.rdfs.range_rdf.resource.rdfs.subClassOf_rdf.resource.rdfs.subPropertyOf_rdf.resource">match: <code class="pattern">rdfs:domain[@rdf:resource]|rdfs:range[@rdf:resource]|rdfs:subClassOf[@rdf:resource]|rdfs:subPropertyOf[@rdf:resource]</code></h3><div class="template-desc"><div class="abstract"><span class="comnt"> domain, range, subClassOf, subPropertyOf treated specially  </span></div></div><h3 id="t-owl.Ontology-dc.x.dc.Desciption-dc.x.owl.Ontology-dcterms.x.dc.Desciption-dcterms.x.owl.versionInfo">match: <code class="pattern">owl:Ontology/dc:*|dc:Desciption/dc:*|owl:Ontology/dcterms:*|dc:Desciption/dcterms:*|owl:versionInfo</code></h3><div class="template-desc"><div class="abstract"><span class="comnt"> Annotation properties in ontology/schema description  </span></div></div><h3 id="t-owl.Ontology-dc.x_rdf.resource.dc.Desciption-dc.x_rdf.resource.owl.Ontology-dcterms.x_rdf.resource.dc.Desciption-dcterms.x_rdf.resource">match: <code class="pattern">owl:Ontology/dc:*[@rdf:resource]|dc:Desciption/dc:*[@rdf:resource]|owl:Ontology/dcterms:*[@rdf:resource]|dc:Desciption/dcterms:*[@rdf:resource]</code></h3><div class="template-desc"><div class="abstract"><span class="comnt"> source, relation be put hyper link  </span></div>
   template calls:
   <ul><li>match: { 
    <a href="#t-_dc.format">@dc:format</a>
  }</li></ul></div><h3 id="t-owl.Ontology-dc.description.owl.Ontology-rdfs.comment.dc.Desciption-dc.description">match: <code class="pattern">owl:Ontology/dc:description|owl:Ontology/rdfs:comment|dc:Desciption/dc:description</code></h3><div class="template-desc"><div class="abstract"><span class="comnt"> Ontology/Schema description property </span></div></div><h3 id="t-owl.Ontology_rdf.about''.rdf.Description_rdf.about''">match: <code class="pattern">owl:Ontology[@rdf:about='']|rdf:Description[@rdf:about='']</code></h3><div class="template-desc"><div class="abstract"><span class="comnt"> Description of this ontoloty/schema itself  </span></div>
   template calls:
   <ul><li>match: { 
    dc:description|rdfs:comment
  }</li><li>match: { 
    dcterms:created
  }</li><li>match: { 
    dcterms:modified
  }</li><li>match: { 
    dc:date
  }</li><li>match: { 
    owl:versionInfo
  }</li><li>match: { 
    dc:source
  }</li><li>match: { 
    dc:relation
  }</li></ul>
   (explicitely) called by:
   <ul><li>match: { <a href="#t-rdf.RDF">rdf:RDF</a> }</li></ul></div><h3 id="t-_dc.format">match: <code class="pattern">@dc:format</code></h3><div class="template-desc"><div class="abstract"></div>
   (explicitely) called by:
   <ul><li>match: { <a href="#t-owl.Ontology-dc.x_rdf.resource.dc.Desciption-dc.x_rdf.resource.owl.Ontology-dcterms.x_rdf.resource.dc.Desciption-dcterms.x_rdf.resource">owl:Ontology/dc:*[@rdf:resource]|dc:Desciption/dc:*[@rdf:resource]|owl:Ontology/dcterms:*[@rdf:resource]|dc:Desciption/dcterms:*[@rdf:resource]</a> }</li></ul></div><h3 id="t-x-rdfs.label.x-_rdfs.label">match: <code class="pattern">*/rdfs:label|*/@rdfs:label</code></h3><div class="template-desc"><div class="abstract"><span class="comnt"> Labels of class/property will be shown in a [] blacket. </span></div></div><h3 id="t-ex.example">match: <code class="pattern">ex:example</code></h3><div class="template-desc"><div class="abstract"><span class="comnt"> Show examples of the ontology/schema for readers </span></div></div><h3 id="t-toc">toc</h3><div class="template-desc"><div class="abstract"></div>
   (explicitely) called by:
   <ul><li>match: { <a href="#t-rdf.RDF">rdf:RDF</a> }</li></ul></div><h3 id="t-namespace">namespace</h3><div class="template-desc"><div class="abstract"></div></div><h3 id="t-generalexamples">generalexamples</h3><div class="template-desc"><div class="abstract"></div>
   variables:
   <ul><li><dfn>ex</dfn>: <code>./*[local-name()='Example']</code></li><li><dfn>exc</dfn>: <code>count($ex)</code></li></ul>
   template calls:
   <ul><li>match: { 
    <a href="#t-ex.pfx">ex:pfx</a>
  }</li></ul>
   (explicitely) called by:
   <ul><li>match: { <a href="#t-rdf.RDF">rdf:RDF</a> }</li></ul></div><h3 id="t-ex.pfx">match: <code class="pattern">ex:pfx</code></h3><div class="template-desc"><div class="abstract"></div>
   (explicitely) called by:
   <ul><li>named: <a href="#t-generalexamples">generalexamples</a></li></ul></div><h3 id="t-cplist">cplist</h3><div class="template-desc"><div class="abstract"></div>
   parameters:
   <ul><li><dfn>l</dfn>: </li></ul>
   (explicitely) called by:
   <ul><li>match: { <a href="#t-rdf.RDF">rdf:RDF</a> }</li></ul></div><h3 id="t-findmore">findmore</h3><div class="template-desc"><div class="abstract"></div>
   parameters:
   <ul><li><dfn>l</dfn>: </li></ul>
   template calls:
   <ul><li>named: <a href="#t-idabout">idabout</a></li><li>match: { 
    @*|*
  }</li></ul>
   (explicitely) called by:
   <ul><li>match: { <a href="#t-rdf.RDF">rdf:RDF</a> }</li></ul></div><h3 id="t-idabout">idabout</h3><div class="template-desc"><div class="abstract"><span class="comnt"> determines class and id of the node </span></div>
   template calls:
   <ul><li>match: { 
    rdfs:label|@rdfs:label
  }</li><li>named: <a href="#t-owltypes">owltypes</a></li></ul>
   (explicitely) called by:
   <ul><li>match: { <a href="#t-rdf.RDF-rdf.Property.rdf.RDF-rdfs.Class.rdf.RDF-owl.xlocal-name%20%21%20%27Ontology%27.rdf.RDF-rdf.Descriptionrdf.type">rdf:RDF/rdf:Property|rdf:RDF/rdfs:Class|rdf:RDF/owl:*[local-name() != 'Ontology']|rdf:RDF/rdf:Description[rdf:type]</a> }</li><li>named: <a href="#t-findmore">findmore</a></li></ul></div><h3 id="t-owltypes">owltypes</h3><div class="template-desc"><div class="abstract"></div>
   (explicitely) called by:
   <ul><li>named: <a href="#t-idabout">idabout</a></li></ul></div><h3 id="t-htmlhead">htmlhead</h3><div class="template-desc"><div class="abstract"><span class="comnt"> Generates some XHTML head elements </span></div>
   (explicitely) called by:
   <ul><li>match: { <a href="#t-rdf.RDF">rdf:RDF</a> }</li></ul></div><hr/><address>This document is presented for a visual user agent via XSLT
   <a href="http://www.kanzaki.com/info/disclaimer">©2005 by MK</a>.
  </address></body></html>