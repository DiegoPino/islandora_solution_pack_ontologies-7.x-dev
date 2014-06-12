<?php

/**
 * @file
 * Template for related items
 *
 * @TODO: Add documentation about this file and the available variables
 */
?>

<div class="islandora-parsed-ontology islandora">
  <div class="islandora-parsed-ontology-content-wrapper clearfix">

    <?php if($triples): ?>
      <div>
        <h2><?php print t('Triples'); ?></h2>
        <ul>
          <?php foreach($triples as $k => $v): ?>
            <li><?php print "Subject: ".$v['s']." Predicate:".$v['p']." Object:".$v['o']; ?></li>
          <?php endforeach; ?>
        </ul>
      </div>
      <div>
        <h2><?php print t('Name Spaces'); ?></h2>
        <ul>
          <?php foreach($ns as $uri => $prefix): ?>
            <li><?php print "URI: ".$uri." prefix:".$prefix; ?></li>
          <?php endforeach; ?>
        </ul>
      </div>
      <div>
        <h2><?php print t('Index'); ?></h2>
        <pre>
          <?php var_dump($index,TRUE); ?>
         </pre>
        
      </div>
    <?php endif; ?>
  </div>
</div>


