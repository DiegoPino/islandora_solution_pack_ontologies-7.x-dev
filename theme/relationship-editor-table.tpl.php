<?php

/**
 * @file
 *
 * @TODO: needs documentation + list of all available variables
 */
?>
<div id="relationship-form-main">
  <div id="relationship-form-table">
    <table>
      <tr>
        <th><?php print t('Namespace'); ?></th>
        <th><?php print t('Relationship'); ?></th>
        <th><?php print t('Target Object'); ?></th>
        <th><?php print t('Delete'); ?></th>
      </tr>
      <?php foreach ($relationships as $relationship) : ?>
      <tr>
        <td><?php print $relationship['namespace'] ?></td>
        <td><?php print $relationship['relationship'] ?></td>
        <td><?php print $relationship['object'] ?></td>
        <td><?php print $relationship['delete'] ?></td>
      </tr>
      <?php endforeach; ?>
    </table>
  </div>
</div>
