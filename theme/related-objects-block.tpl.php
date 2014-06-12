<?php

/**
 * @file
 * Template for related items
 *
 * @TODO: Add documentation about this file and the available variables
 */
?>

<div class="islandora-related-objects islandora">
  <div class="islandora-related-content-wrapper clearfix">

    <?php if($related): ?>
      <div>
<!--        <h2><?php print t('Related Objects'); ?></h2> -->
        <ul>
          <?php foreach ($related as $obj): ?>
            <li><?php print l($obj->label, "islandora/object/{$obj->id}"); ?></li>
          <?php endforeach; ?>
        </ul>
      </div>
    <?php endif; ?>

<HR>
<a class="colorbox-inline" href="?height=500&inline=true#id-of-content">Current ontology</a>

<div style="display: none;">
<div id="id-of-content">
<?php print $list_ontology; ?>
</div>
</div>

  </div>
</div>

