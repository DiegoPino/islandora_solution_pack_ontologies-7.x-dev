<?php

/**
 * @file
 * API documentation for Administration menu.
 */

/**
 * Define color and style associated to CMODELS when displaying nodes in vlot graph.
 *
 *
 * @return array
 *   Associative array containing a set of Colors,CMODEL to human mapping and other attributes for the canvas renderer of vlot
*/
function hook_islandora_ontologies_vlot_info() {
  return array('namedstyle'=>array(
       'islandora:anyCmodel' => array(
        'cmodel_label' => 'any',
        'color' => 'red',//or #454545
    	),
        'default' => array(
         'cmodel_label' => 'object',
         'color' => 'blue',//or #454545 or rgb(10,10,10)
     	),
	)
  );
}
