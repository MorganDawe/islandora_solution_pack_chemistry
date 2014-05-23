<?php
/**
 * @file
 * Holds helper functions for islandora_chemistry.
 */

/**
 * Get all lists for table.
 *
 * General function to retrieve the inchikey.
 *
 * @return SelectQuery
 *   The query return.
 */
function islandora_chemistry_get_inchikey_by_id($inchikey) {
  $inchi = db_select('islandora_chemistry_fingerprints')
  ->fields('islandora_chemistry_fingerprints')
  ->condition('id', $inchikey)
  ->execute();
  $data = $inchi->fetchObject();
  return (isset($data->id) ? $data->id : NULL);
}

/**
 * Create a list.
 *
 * @param array $arr_val
 *   Field array as key => value pairs, used in db_insert.
 */
function islandora_chemistry_set_inchikey($arr_val) {
  $data = db_insert('islandora_chemistry_fingerprints')
  ->fields($arr_val)
  ->execute();
  return $data;
}

/**
 * Dispatch hooks in use by the chemistry solution pack.
 *
 * @param string $hook_name
 *   The Hook name.
 * @param AbstractObject $object
 *   The fedora object in use.
 * @param string $args
 *   The CLI arguments.
 * @return unknown
 */
function islandora_chemistry_dispatch_hook($hook_name, $object, $args) {
  $hooks = islandora_build_hook_list($hook_name, $object->models);
  foreach ($hooks as $hook) {
    $temp = module_invoke_all($hook, $object, $args);
    islandora_as_renderable_array($temp);
    if (!empty($temp)) {
      // Mash together our hook results.
      $output = array_merge_recursive($output, $temp);
    }
  }
  if (empty($output)) {
    // No results, use the default data.
    $output = $args;
  }
  // Toss in an alter hook for good measure.
  drupal_alter($hooks, $object, $output);
  return $output;
}