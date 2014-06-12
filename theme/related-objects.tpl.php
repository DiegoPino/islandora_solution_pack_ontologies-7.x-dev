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
        <h2><?php print t('Related Objects'); ?></h2>
        <ul>
          <?php foreach ($related as $obj): ?>
            <li><?php print l($obj->label, "islandora/object/{$obj->id}"); ?></li>
          <?php endforeach; ?>
        </ul>
      </div>
    <?php endif; ?>
  </div>
</div>


